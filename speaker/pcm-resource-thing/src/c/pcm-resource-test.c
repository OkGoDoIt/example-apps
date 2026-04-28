#include <pebble.h>

// Audio constants — must match the raw PCM file format
// Expected format: 8-bit signed mono at 8kHz
#define SAMPLE_RATE 8000
#define TIMER_MS 50
#define SAMPLES_PER_CHUNK (SAMPLE_RATE * TIMER_MS / 1000) // 400
#define BYTES_PER_CHUNK (SAMPLES_PER_CHUNK * 1)            // 400
#define VOLUME 10

// UI elements
static Window *s_window;
static TextLayer *s_title_layer;
static TextLayer *s_status_layer;
static TextLayer *s_progress_layer;
static TextLayer *s_hint_layer;

// State
static AppTimer *s_timer;
static bool s_playing;

// Resource playback state
static ResHandle s_res_handle;
static size_t s_res_size;
static size_t s_res_offset;

// Sample buffer
static uint8_t s_buffer[SAMPLES_PER_CHUNK];

// Progress text buffer
static char s_progress_buf[32];

// --- UI ---

static void update_display(void) {
  if (s_playing) {
    text_layer_set_text(s_status_layer, "Playing");
    int pct = (s_res_size > 0) ? (int)(s_res_offset * 100 / s_res_size) : 0;
    snprintf(s_progress_buf, sizeof(s_progress_buf), "%d%%", pct);
    text_layer_set_text(s_progress_layer, s_progress_buf);
  } else {
    text_layer_set_text(s_status_layer, "Stopped");
    text_layer_set_text(s_progress_layer, "");
  }
}

// --- Playback control ---

// Push as much resource data as the stream's ring buffer will accept. Returns
// true if the resource has been fully queued. speaker_stream_write returns
// short when the buffer is full, which is our signal to stop for this tick —
// the kernel-side ring buffer then absorbs any timer drift until we're called
// again.
static bool prv_fill_stream(void) {
  for (;;) {
    size_t remaining = s_res_size - s_res_offset;
    if (remaining == 0) {
      return true;
    }

    size_t to_read = (remaining < BYTES_PER_CHUNK) ? remaining : BYTES_PER_CHUNK;
    resource_load_byte_range(s_res_handle, s_res_offset, s_buffer, to_read);

    uint32_t written = speaker_stream_write(s_buffer, to_read);
    s_res_offset += written;

    if (written < to_read) {
      return false;
    }
  }
}

static void timer_callback(void *data) {
  if (!s_playing) return;

  if (prv_fill_stream()) {
    // All data queued — drain the remaining buffered audio, then stop.
    s_playing = false;
    speaker_stream_close();
    text_layer_set_text(s_status_layer, "Done");
    text_layer_set_text(s_progress_layer, "100%");
    return;
  }

  update_display();
  s_timer = app_timer_register(TIMER_MS, timer_callback, NULL);
}

static void start_playback(void) {
  if (s_playing) return;

  s_res_handle = resource_get_handle(RESOURCE_ID_PCM_DATA);
  s_res_size = resource_size(s_res_handle);
  s_res_offset = 0;

  if (s_res_size == 0) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "PCM resource is empty");
    return;
  }

  if (!speaker_stream_open(SpeakerPcmFormat_8kHz_8bit, VOLUME)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to open speaker stream");
    return;
  }

  s_playing = true;
  prv_fill_stream();  // prime the ring buffer immediately
  update_display();
  s_timer = app_timer_register(TIMER_MS, timer_callback, NULL);
}

static void stop_playback(void) {
  if (!s_playing) return;

  s_playing = false;
  if (s_timer) {
    app_timer_cancel(s_timer);
    s_timer = NULL;
  }
  // Stop immediately rather than drain — the buffer may hold up to ~1s of
  // already-queued audio and we want a snappy response to user-initiated stop.
  speaker_stop();
  update_display();
}

// --- Button handlers ---

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_playing) {
    stop_playback();
  } else {
    start_playback();
  }
}

static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
  stop_playback();
  window_stack_pop(true);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
}

// --- Window ---

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  int w = bounds.size.w;
  int h = bounds.size.h;

  // Title
  s_title_layer = text_layer_create(GRect(0, h / 6 - 5, w, 30));
  text_layer_set_background_color(s_title_layer, GColorClear);
  text_layer_set_text_color(s_title_layer, GColorWhite);
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_title_layer, "PCM Resource");
  layer_add_child(root, text_layer_get_layer(s_title_layer));

  // Status
  s_status_layer = text_layer_create(GRect(0, h / 2 - 24, w, 30));
  text_layer_set_background_color(s_status_layer, GColorClear);
  text_layer_set_text_color(s_status_layer, GColorWhite);
  text_layer_set_text_alignment(s_status_layer, GTextAlignmentCenter);
  text_layer_set_font(s_status_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(root, text_layer_get_layer(s_status_layer));

  // Progress
  s_progress_layer = text_layer_create(GRect(0, h / 2 + 8, w, 28));
  text_layer_set_background_color(s_progress_layer, GColorClear);
  text_layer_set_text_color(s_progress_layer, GColorWhite);
  text_layer_set_text_alignment(s_progress_layer, GTextAlignmentCenter);
  text_layer_set_font(s_progress_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  layer_add_child(root, text_layer_get_layer(s_progress_layer));

  // Hint
  s_hint_layer = text_layer_create(GRect(0, h - 32, w, 20));
  text_layer_set_background_color(s_hint_layer, GColorClear);
  text_layer_set_text_color(s_hint_layer, GColorWhite);
  text_layer_set_text_alignment(s_hint_layer, GTextAlignmentCenter);
  text_layer_set_font(s_hint_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text(s_hint_layer, "SEL: play/stop");
  layer_add_child(root, text_layer_get_layer(s_hint_layer));

  update_display();
}

static void window_unload(Window *window) {
  text_layer_destroy(s_title_layer);
  text_layer_destroy(s_status_layer);
  text_layer_destroy(s_progress_layer);
  text_layer_destroy(s_hint_layer);
}

// --- App lifecycle ---

static void init(void) {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_click_config_provider(s_window, click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}

static void deinit(void) {
  stop_playback();
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
