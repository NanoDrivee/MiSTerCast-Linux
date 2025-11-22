#include <cstdint>
#include <iostream>

#include "WaylandVideoCapture.h"

bool InitializeWaylandCapture() {
    // TODO: Make sure it's cpp style
    struct pw_properties* props; // TODO: put in header
    pw_data_t data = { 0, };
    uint8_t buffer[1024];

    const struct spa_pod* params[1];
    struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

    spa_rectangle rec_def = SPA_RECTANGLE(320, 240);
    spa_rectangle rec_min = SPA_RECTANGLE(1, 1);
    spa_rectangle rec_max = SPA_RECTANGLE(4096, 4096);

    spa_fraction frac_def = SPA_FRACTION(25, 1);
    spa_fraction frac_min = SPA_FRACTION(0, 1);
    spa_fraction frac_max = SPA_FRACTION(1000, 1);

    // TODO: Move to header file, update
    params[0] = static_cast<spa_pod*>(spa_pod_builder_add_object(&b,
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
    
    pw_init(NULL, NULL);

    data.loop = pw_main_loop_new(NULL);

    props = pw_properties_new(
        PW_KEY_MEDIA_TYPE,"Video",
        PW_KEY_MEDIA_CATEGORY,"Capture",
        PW_KEY_MEDIA_ROLE, "Camera",
        NULL
    );

    data.stream = pw_stream_new_simple(
        pw_main_loop_get_loop(data.loop),
        "video-capture",
        props,
        &stream_events,
        &data
    );

    // TODO: use logging function
    std::cout << "Connecting to stream\n";

    pw_stream_connect(
        data.stream,
        PW_DIRECTION_INPUT,
        PW_ID_ANY,
        static_cast<pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS),
        params,
        1
    );

    pw_main_loop_run(data.loop);

    std::cout << "Closing stream\n";
    pw_stream_destroy(data.stream);
    pw_main_loop_destroy(data.loop);

    return true;
}