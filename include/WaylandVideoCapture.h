// TODO: Organize better
#include <spa-0.2/spa/param/video/format-utils.h>
#include <spa-0.2/spa/param/video/type-info.h>
#include <spa-0.2/spa/param/param-types.h>
#include <spa-0.2/spa/debug/types.h>

#include <pipewire-0.3/pipewire/pipewire.h>
#include <pipewire-0.3/pipewire/main-loop.h>
#include <pipewire-0.3/pipewire/properties.h>
#include <pipewire-0.3/pipewire/keys.h>
#include <pipewire-0.3/pipewire/stream.h>
#include <pipewire-0.3/pipewire/port.h>

#include <libportal/portal.h>

#include <future>

// TODO: Exceptions, error handling
// TODO: Comment lots (make it enough so someone can walk through it to learn video capture)
// TODO: Does this work on X?
// TODO: Comprehensive logging
// TODO: Investigate PW Properties
class WaylandVideoCapture {
    // TODO: Only get/set where necessary
    public:
        WaylandVideoCapture();
        ~WaylandVideoCapture();

        void setXdpSession(XdpSession* session);
        XdpSession* getXdpSession();

        void setXdpSessionFd(int fd);
        int getXdpSessionFd();

        void setPwLoop(pw_thread_loop* loop);
        pw_thread_loop* getPwLoop();

        void setPwProps(pw_properties* props);
        pw_properties* getPwProps();

        void setPwContext(pw_context* context);
        pw_context* getPwContext();

        void setPwCore(pw_core* core);
        pw_core* getPwCore();

        void setPwStream(pw_stream* stream);
        pw_stream* getPwStream();
        
        void setSpaFormat(spa_video_info format);
        spa_video_info* getSpaFormat();

        void xdpStartSession();

        void waitForInitComplete();

    private:
        // Stream event handling info for PW
        const pw_stream_events stream_events = {
            PW_VERSION_STREAM_EVENTS,
            .param_changed = WaylandVideoCapture::pwOnParamChanged,
            .process = WaylandVideoCapture::pwOnProcess,
        };

        const spa_rectangle rec_def = SPA_RECTANGLE(320, 240);
        const spa_rectangle rec_min = SPA_RECTANGLE(1, 1);
        const spa_rectangle rec_max = SPA_RECTANGLE(4096, 4096);

        const spa_fraction frac_def = SPA_FRACTION(25, 1);
        const spa_fraction frac_min = SPA_FRACTION(0, 1);
        const spa_fraction frac_max = SPA_FRACTION(1000, 1);

        const spa_pod* spaPodParams[1];

        XdpPortal* xdpPortal;
        XdpSession* xdpSession;
        int xdpSessionFd;

        pw_thread_loop* pwLoop;
        pw_properties* pwProps;
        pw_context* pwContext;
        pw_core* pwCore;
        pw_stream* pwStream;
        
        spa_video_info spaFormat;
        spa_hook stream_listener;

        // TODO: Should have getter
        bool isInitialized;
        bool isConnected;

        void pwLoopRun();
        void pwLoopInterrupt();
        
        std::promise<void> initPromise;

        void xdpInit();
        static void xdpInitComplete(GObject* source_object, GAsyncResult* res, gpointer data); // Callbacks must be static

        static void xdpStartSessionComplete(GObject* source_object, GAsyncResult* res, gpointer data);

        void pwInit();
        void pwStreamBegin();

        static void pwOnProcess(void* userdata);
        static void pwOnParamChanged(void* userdata, uint32_t id, const struct spa_pod* param);
};