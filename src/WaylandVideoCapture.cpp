#include <cstdint>
#include <iostream>

#include "WaylandVideoCapture.h"

// TODO: Use comments to separate for better readability

WaylandVideoCapture::WaylandVideoCapture() {
    // Set props to NULL for proper init checking
    this->xdpPortal = NULL;
    this->xdpSession = NULL;
    this->xdpSessionFd = -1;

    this->pwLoop = NULL;
    this->pwProps = NULL;
    this->pwContext = NULL;
    this->pwCore = NULL;
    this->pwStream = NULL;

    this->isInitialized = false;
    this->isConnected = false;

    this->pwInit();
    this->xdpInit();

    this->waitForInitComplete();
}

WaylandVideoCapture::~WaylandVideoCapture() {
    if (this->pwStream != NULL) {
        pw_stream_set_active(this->pwStream, false);
        pw_stream_disconnect(this->pwStream);
        pw_stream_destroy(this->pwStream);
    }
    if (this->pwCore != NULL) pw_core_disconnect(this->pwCore);
    if (this->pwContext != NULL) pw_context_destroy(this->pwContext);
    if (this->pwProps != NULL) pw_properties_free(this->pwProps);
    if (this->pwLoop != NULL) {
        pw_thread_loop_stop(this->pwLoop);
        pw_thread_loop_destroy(this->pwLoop);
    }
    if (this->xdpSessionFd > -1) close(this->xdpSessionFd);
    if (this->xdpSession != NULL) xdp_session_close(this->xdpSession);

    pw_deinit();

    // TODO: Do I need to free pointers manually?
    // TODO: g_clear_object
    // delete this->xdpPortal;
    // delete this->xdpSession;

    // delete this->pwLoop;
    delete this->pwProps;
    // delete this->pwContext;
    // delete this->pwCore;
    // delete this->pwStream;

    // delete this->spaPodParams;
}

void WaylandVideoCapture::setXdpSession(XdpSession* session) {
    if (this->xdpSession != NULL) xdp_session_close(this->xdpSession);

    this->xdpSession = session;
}
XdpSession* WaylandVideoCapture::getXdpSession() { return this->xdpSession; }

void WaylandVideoCapture::setXdpSessionFd(int fd) {
    if (this->xdpSessionFd != -1) close(this->xdpSessionFd);

    this->xdpSessionFd = fd;
}
int WaylandVideoCapture::getXdpSessionFd() { return this->xdpSessionFd; }

void WaylandVideoCapture::setPwLoop(pw_thread_loop* loop) {
    if (this->pwLoop != NULL) {
        pw_thread_loop_stop(this->pwLoop);
        pw_thread_loop_destroy(this->pwLoop);
    }

    this->pwLoop = loop;
}
pw_thread_loop* WaylandVideoCapture::getPwLoop() { return this->pwLoop; }

void WaylandVideoCapture::setPwProps(pw_properties* props) {
    if (this->pwProps != NULL) pw_properties_free(this->pwProps);

    this->pwProps = props;
}
pw_properties* WaylandVideoCapture::getPwProps() { return this->pwProps; }

void WaylandVideoCapture::setPwCore(pw_core* core) {
    if (this->pwCore != NULL) pw_core_disconnect(this->pwCore);

    this->pwCore = core;
}
pw_core* WaylandVideoCapture::getPwCore() { return this->pwCore; }

void WaylandVideoCapture::setPwContext(pw_context* context) {
    if (this->pwContext != NULL) pw_context_destroy(this->pwContext);

    this->pwContext = context;
}
pw_context* WaylandVideoCapture::getPwContext() { return this->pwContext; }

void WaylandVideoCapture::setPwStream(pw_stream* stream) {
    if (this->pwStream != NULL) {
        pw_stream_set_active(this->pwStream, false);
        pw_stream_disconnect(this->pwStream);
        pw_stream_destroy(this->pwStream);
    }

    this->pwStream = stream;
}
pw_stream* WaylandVideoCapture::getPwStream() { return this->pwStream; }

void WaylandVideoCapture::setSpaFormat(spa_video_info format) { this->spaFormat = format; }
spa_video_info* WaylandVideoCapture::getSpaFormat() { return &this->spaFormat; }

// TODO: Change to pw_thread_loop
// TODO: Error
void WaylandVideoCapture::pwLoopRun() {
    pw_thread_loop_start(this->getPwLoop());
}

void WaylandVideoCapture::pwLoopInterrupt() {
    pw_thread_loop_stop(this->getPwLoop());
}

// TODO: Most likely needs to be split so that thread can keep being open
void WaylandVideoCapture::pwInit() {
    pw_init(NULL, NULL);

    this->setPwLoop(pw_thread_loop_new("Screen capture loop", NULL));

    this->setPwContext(
        pw_context_new(
            pw_thread_loop_get_loop(this->pwLoop),
            NULL,
            0
        )
    );

    this->setPwProps(
        pw_properties_new(
            PW_KEY_MEDIA_TYPE,"Video",
            PW_KEY_MEDIA_CATEGORY,"Capture",
            PW_KEY_MEDIA_ROLE, "Screen",
            NULL
        )
    );

    // TODO: Formats probably need to be updated, needs to be formatted properly
    uint8_t spaPodBuilderBuf[1024];
    spa_pod_builder spaPodBuilder = SPA_POD_BUILDER_INIT(spaPodBuilderBuf, sizeof(spaPodBuilderBuf));
    this->spaPodParams[0] = static_cast<spa_pod*>(spa_pod_builder_add_object(&spaPodBuilder,
        SPA_TYPE_OBJECT_Format, SPA_PARAM_EnumFormat,
        SPA_FORMAT_mediaType,       SPA_POD_Id(SPA_MEDIA_TYPE_video),
        SPA_FORMAT_mediaSubtype,    SPA_POD_Id(SPA_MEDIA_SUBTYPE_raw),
        SPA_FORMAT_VIDEO_format,    SPA_POD_CHOICE_ENUM_Id(7,
                                        SPA_VIDEO_FORMAT_RGB,
                                        SPA_VIDEO_FORMAT_RGB,
                                        SPA_VIDEO_FORMAT_RGBA,
                                        SPA_VIDEO_FORMAT_RGBx,
                                        SPA_VIDEO_FORMAT_BGRx,
                                        SPA_VIDEO_FORMAT_YUY2,
                                        SPA_VIDEO_FORMAT_I420),
        SPA_FORMAT_VIDEO_size,      SPA_POD_CHOICE_RANGE_Rectangle(
                                        &rec_def,
                                        &rec_min,
                                        &rec_max),
        SPA_FORMAT_VIDEO_framerate, SPA_POD_CHOICE_RANGE_Fraction(
                                        &frac_def,
                                        &frac_min,
                                        &frac_max)));
}

void WaylandVideoCapture::pwStreamBegin() {
    this->isInitialized = true;

    // TODO: Throw error if core is NULL
    this->setPwCore(
        pw_context_connect_fd(
            this->pwContext,
            this->xdpSessionFd,
            NULL,
            0
        )
    );

    this->setPwStream(
        pw_stream_new(
            this->pwCore,
            "screen-capture",
            this->pwProps
        )
    );

    pw_stream_add_listener(
        this->getPwStream(),
        &this->stream_listener,
        &this->stream_events,
        this
    );

    // TODO: use logging function
    std::cout << "Connecting to stream\n";

    // TODO: Check for failure
    pw_stream_connect(
        this->getPwStream(),
        PW_DIRECTION_OUTPUT,
        PW_ID_ANY,
        static_cast<pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS),
        this->spaPodParams,
        1
    );

    // Handlers must be installed between loop lock and start
    pw_thread_loop_lock(this->getPwLoop());

    this->pwLoopRun();

    this->isConnected = true;

    this->initPromise.set_value(); // Stop waiting for init to be complete
}

void WaylandVideoCapture::xdpInit() {
    // TODO: Use initable version of the function for error handling
    if (this->xdpPortal == NULL) this->xdpPortal = xdp_portal_initable_new(NULL);
    if (this->xdpPortal == NULL) { std::cout << "Portal is null\n"; return; }

    // Async, uses callback
    // TODO: Might have to make own function?
    xdp_portal_create_screencast_session(
        this->xdpPortal,
        static_cast<XdpOutputType>(XDP_OUTPUT_MONITOR | XDP_OUTPUT_WINDOW),
        XDP_SCREENCAST_FLAG_NONE, // Can allowing multiple streams be useful?
        XDP_CURSOR_MODE_EMBEDDED, // Give option to toggle
        XDP_PERSIST_MODE_TRANSIENT, // Set to _TRANSIENT mode if issues?
        NULL,
        NULL, // Implement cancellable for error handling
        WaylandVideoCapture::xdpInitComplete,
        this // Pass WaylandVideoCapture to the static function so that it may modify the object
    );
}

void WaylandVideoCapture::xdpInitComplete(GObject* source_object, GAsyncResult* res, gpointer data) {
    std::cout << "XDP Screencast Session Init Start\n";

    // TODO: check if source obj is portal for error handling, include error ptr
    WaylandVideoCapture* obj = static_cast<WaylandVideoCapture*>(data);
    XdpSession* session = xdp_portal_create_screencast_session_finish(XDP_PORTAL(source_object), res, NULL);
    int fd = xdp_session_open_pipewire_remote(session);

    obj->setXdpSession(session);
    obj->setXdpSessionFd(fd);

    std::cout << "XDP Screencast Session Init Complete!\n";
    
    obj->xdpStartSession();
}

void WaylandVideoCapture::xdpStartSession() {
    xdp_session_start(
        this->getXdpSession(),
        NULL,
        NULL, // TODO: Handle errors
        WaylandVideoCapture::xdpStartSessionComplete,
        this
    );
}

void WaylandVideoCapture::xdpStartSessionComplete(GObject* source_object, GAsyncResult* res, gpointer data) {
    WaylandVideoCapture* obj = static_cast<WaylandVideoCapture*>(data);
    
    xdp_session_start_finish(XDP_SESSION(source_object), res, NULL);

    obj->pwStreamBegin();
}

// TODO: Change
void WaylandVideoCapture::pwOnProcess(void* data) {
    WaylandVideoCapture* obj = static_cast<WaylandVideoCapture*>(data);
    pw_buffer* pw_buf;
    spa_buffer* spa_buf;

    if ((pw_buf = pw_stream_dequeue_buffer(obj->getPwStream())) == NULL) {
        pw_log_warn("out of buffers: %m");
        return;
    }

    std::cout << "Queueing stream buffer";

    spa_buf = pw_buf->buffer;
    if (spa_buf->datas[0].data == NULL) return;
    std::cout << "got a frame of size" << spa_buf->datas[0].chunk->size << "\n";

    pw_stream_queue_buffer(obj->getPwStream(), pw_buf);
}

void WaylandVideoCapture::pwOnParamChanged(void* data, uint32_t id, const struct spa_pod* param) {
    WaylandVideoCapture* obj = static_cast<WaylandVideoCapture*>(data);
    spa_video_info* format = obj->getSpaFormat();

    // TODO: Print info
}

void WaylandVideoCapture::waitForInitComplete() {
    this->initPromise.get_future().wait();
}