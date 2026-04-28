#include <pebble.h>

static Window *s_window;
static TextLayer *s_text_layer;
static uint8_t *s_sample_buf;
static uint32_t s_sample_size;

static const SpeakerNote s_scale_notes[8] = {
  { .midi_note = 60, .waveform = 0, .duration_ms = 400, .velocity = 100, .reserved = 0 },
  { .midi_note = 62, .waveform = 0, .duration_ms = 400, .velocity = 100, .reserved = 0 },
  { .midi_note = 64, .waveform = 0, .duration_ms = 400, .velocity = 100, .reserved = 0 },
  { .midi_note = 65, .waveform = 0, .duration_ms = 400, .velocity = 100, .reserved = 0 },
  { .midi_note = 67, .waveform = 0, .duration_ms = 400, .velocity = 100, .reserved = 0 },
  { .midi_note = 69, .waveform = 0, .duration_ms = 400, .velocity = 100, .reserved = 0 },
  { .midi_note = 71, .waveform = 0, .duration_ms = 400, .velocity = 100, .reserved = 0 },
  { .midi_note = 72, .waveform = 0, .duration_ms = 400, .velocity = 100, .reserved = 0 },
};

static void prv_finish_callback(SpeakerFinishReason reason, void *ctx) {
  const char *text;
  switch (reason) {
    case SpeakerFinishReasonDone:      text = "Done: Done"; break;
    case SpeakerFinishReasonStopped:   text = "Done: Stopped"; break;
    case SpeakerFinishReasonPreempted: text = "Done: Preempted"; break;
    case SpeakerFinishReasonError:     text = "Done: Error"; break;
    default:                           text = "Done: ?"; break;
  }
  text_layer_set_text(s_text_layer, text);
}

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  SpeakerSample sample = {
    .data = s_sample_buf,
    .num_bytes = s_sample_size,
    .format = SpeakerPcmFormat_8kHz_16bit,
    .base_midi_note = 60,
    .loop = true,
  };
  SpeakerTrack track = {
    .notes = s_scale_notes,
    .num_notes = 8,
    .sample = &sample,
  };
  if (speaker_play_tracks(&track, 1, 80)) {
    text_layer_set_text(s_text_layer, "Playing...");
  } else {
    text_layer_set_text(s_text_layer, "Done: Error");
  }
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(GRect(0, 60, bounds.size.w, 60));
  text_layer_set_text(s_text_layer, "Press SELECT to play scale");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
}

static void prv_init(void) {
  ResHandle h = resource_get_handle(RESOURCE_ID_C4_SAMPLE_PCM);
  s_sample_size = resource_size(h);
  s_sample_buf = malloc(s_sample_size);
  resource_load(h, s_sample_buf, s_sample_size);

  speaker_set_finish_callback(prv_finish_callback, NULL);

  s_window = window_create();
  window_set_click_config_provider(s_window, prv_click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, true);
}

static void prv_deinit(void) {
  speaker_stop();
  speaker_set_finish_callback(NULL, NULL);
  window_destroy(s_window);
  free(s_sample_buf);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
