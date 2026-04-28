#include <pebble.h>

/*
 * Speaker Polyphony Test
 *
 * Plays a 3-voice arrangement of Ode to Joy (Beethoven's 9th, opening
 * phrase) for ~6.4 s using speaker_play_tracks(). Three concurrent
 * tracks, each with its own note sequence and waveform:
 *
 *   Soprano (sawtooth): melody, C4..G4
 *   Alto    (sine):     parallel thirds below, A3..E4
 *   Bass    (triangle): chord roots, C3..G3
 *
 * Tempo: quarter = 400 ms (150 BPM).
 *
 * Verification checklist:
 *  1. SELECT plays a 3-voice melody for ~6.4 s; three independent lines
 *     audible; melody recognizable as Ode to Joy.
 *  2. After playback the text layer reads "Done: Done".
 *  3. SELECT during playback: first call's finish callback fires with
 *     Preempted; a fresh performance begins.
 *  4. BACK during playback exits cleanly; deinit stops the speaker and
 *     unregisters the finish callback before destroying the window.
 */

#define MEL(note, dur)  { (note), SpeakerWaveformSawtooth, (dur), 0, 0 }
#define HARM(note, dur) { (note), SpeakerWaveformSine,     (dur), 0, 0 }
#define BASS(note, dur) { (note), SpeakerWaveformTriangle, (dur), 0, 0 }

// MIDI: C3=48 G3=55 A3=57 B3=59 C4=60 D4=62 E4=64 F4=65 G4=67
static const SpeakerNote s_soprano[] = {
  MEL(64, 400), MEL(64, 400), MEL(65, 400), MEL(67, 400),  // E  E  F  G
  MEL(67, 400), MEL(65, 400), MEL(64, 400), MEL(62, 400),  // G  F  E  D
  MEL(60, 400), MEL(60, 400), MEL(62, 400), MEL(64, 400),  // C  C  D  E
  MEL(64, 600), MEL(62, 200), MEL(62, 800),                // E. D  D (half)
};

static const SpeakerNote s_alto[] = {
  HARM(60, 400), HARM(60, 400), HARM(62, 400), HARM(64, 400),  // C  C  D  E
  HARM(64, 400), HARM(62, 400), HARM(60, 400), HARM(59, 400),  // E  D  C  B3
  HARM(57, 400), HARM(57, 400), HARM(59, 400), HARM(60, 400),  // A3 A3 B3 C
  HARM(60, 600), HARM(59, 200), HARM(59, 800),                 // C. B3 B3 (half)
};

// Bass: I | V | I | V->I
static const SpeakerNote s_bass[] = {
  BASS(48, 1600),                   // C3 whole
  BASS(55, 1600),                   // G3 whole
  BASS(48, 1600),                   // C3 whole
  BASS(55, 400), BASS(48, 1200),    // G3 quarter, C3 dotted half
};

static const SpeakerTrack s_tracks[] = {
  { .notes = s_soprano, .num_notes = sizeof(s_soprano) / sizeof(s_soprano[0]), .sample = NULL },
  { .notes = s_alto,    .num_notes = sizeof(s_alto)    / sizeof(s_alto[0]),    .sample = NULL },
  { .notes = s_bass,    .num_notes = sizeof(s_bass)    / sizeof(s_bass[0]),    .sample = NULL },
};

static Window *s_window;
static TextLayer *s_text_layer;
static char s_status_buf[32];

static const char *prv_reason_name(SpeakerFinishReason reason) {
  switch (reason) {
    case SpeakerFinishReasonDone:      return "Done";
    case SpeakerFinishReasonStopped:   return "Stopped";
    case SpeakerFinishReasonPreempted: return "Preempted";
    case SpeakerFinishReasonError:     return "Error";
  }
  return "?";
}

static void prv_on_finish(SpeakerFinishReason reason, void *ctx) {
  snprintf(s_status_buf, sizeof(s_status_buf), "Done: %s", prv_reason_name(reason));
  text_layer_set_text(s_text_layer, s_status_buf);
}

static void prv_play_melody(void) {
  speaker_play_tracks(s_tracks, sizeof(s_tracks) / sizeof(s_tracks[0]), 80);
  text_layer_set_text(s_text_layer, "Playing...");
}

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  prv_play_melody();
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(GRect(0, bounds.size.h / 2 - 14, bounds.size.w, 28));
  text_layer_set_text(s_text_layer, "Press SELECT to play");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
}

static void prv_init(void) {
  speaker_set_finish_callback(prv_on_finish, NULL);

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
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
