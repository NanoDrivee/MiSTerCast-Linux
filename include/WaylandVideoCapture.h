#include "spa-0.2/spa/param/video/format-utils.h"
#include "spa-0.2/spa/param/video/type-info.h"
#include "spa-0.2/spa/param/param-types.h"
#include "spa-0.2/spa/debug/types.h"

#include "pipewire-0.3/pipewire/pipewire.h"
#include "pipewire-0.3/pipewire/main-loop.h"
#include "pipewire-0.3/pipewire/properties.h"
#include "pipewire-0.3/pipewire/keys.h"
#include "pipewire-0.3/pipewire/stream.h"
#include "pipewire-0.3/pipewire/port.h"

// Struct for compatibility with C library
typedef struct pw_data {
    struct pw_main_loop* loop;
    struct pw_stream *stream;
} pw_data_t;


bool InitializeWaylandCapture();

// Handle stream
static void on_param_changed(void *userdata, uint32_t id, const struct spa_pod *param) {};
static void on_process(void *userdata) {};

// Stream event parameters
static const struct pw_stream_events stream_events = {
    PW_VERSION_STREAM_EVENTS,
    .param_changed = on_param_changed,
    .process = on_process,
};