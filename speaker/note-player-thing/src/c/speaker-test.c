#include <pebble.h>

// MIDI note definitions
#define REST 0
#define C4   60
#define D4   62
#define Eb4  63
#define E4   64
#define F4   65
#define G4   67
#define Ab4  68
#define A4   69
#define Bb4  70
#define B4   71
#define C5   72
#define D5   74
#define Eb5  75
#define E5   76
#define F5   77
#define G5   79

// Helper macro for creating notes
#define N(midi, wave, dur) \
  { .midi_note = (midi), .waveform = (wave), .duration_ms = (dur), .velocity = 0, .reserved = 0 }

// --- Melodies ---

// Ode to Joy - Beethoven (Sine wave)
static const SpeakerNote s_ode_to_joy[] = {
  N(E4, SpeakerWaveformSine, 400),
  N(E4, SpeakerWaveformSine, 400),
  N(F4, SpeakerWaveformSine, 400),
  N(G4, SpeakerWaveformSine, 400),
  N(G4, SpeakerWaveformSine, 400),
  N(F4, SpeakerWaveformSine, 400),
  N(E4, SpeakerWaveformSine, 400),
  N(D4, SpeakerWaveformSine, 400),
  N(C4, SpeakerWaveformSine, 400),
  N(C4, SpeakerWaveformSine, 400),
  N(D4, SpeakerWaveformSine, 400),
  N(E4, SpeakerWaveformSine, 400),
  N(E4, SpeakerWaveformSine, 600),
  N(D4, SpeakerWaveformSine, 200),
  N(D4, SpeakerWaveformSine, 800),
  // Second phrase
  N(E4, SpeakerWaveformSine, 400),
  N(E4, SpeakerWaveformSine, 400),
  N(F4, SpeakerWaveformSine, 400),
  N(G4, SpeakerWaveformSine, 400),
  N(G4, SpeakerWaveformSine, 400),
  N(F4, SpeakerWaveformSine, 400),
  N(E4, SpeakerWaveformSine, 400),
  N(D4, SpeakerWaveformSine, 400),
  N(C4, SpeakerWaveformSine, 400),
  N(C4, SpeakerWaveformSine, 400),
  N(D4, SpeakerWaveformSine, 400),
  N(E4, SpeakerWaveformSine, 400),
  N(D4, SpeakerWaveformSine, 600),
  N(C4, SpeakerWaveformSine, 200),
  N(C4, SpeakerWaveformSine, 800),
};

// Twinkle Twinkle Little Star (Square wave)
static const SpeakerNote s_twinkle[] = {
  N(C4, SpeakerWaveformSquare, 400),
  N(C4, SpeakerWaveformSquare, 400),
  N(G4, SpeakerWaveformSquare, 400),
  N(G4, SpeakerWaveformSquare, 400),
  N(A4, SpeakerWaveformSquare, 400),
  N(A4, SpeakerWaveformSquare, 400),
  N(G4, SpeakerWaveformSquare, 800),
  N(F4, SpeakerWaveformSquare, 400),
  N(F4, SpeakerWaveformSquare, 400),
  N(E4, SpeakerWaveformSquare, 400),
  N(E4, SpeakerWaveformSquare, 400),
  N(D4, SpeakerWaveformSquare, 400),
  N(D4, SpeakerWaveformSquare, 400),
  N(C4, SpeakerWaveformSquare, 800),
  // Second phrase
  N(G4, SpeakerWaveformSquare, 400),
  N(G4, SpeakerWaveformSquare, 400),
  N(F4, SpeakerWaveformSquare, 400),
  N(F4, SpeakerWaveformSquare, 400),
  N(E4, SpeakerWaveformSquare, 400),
  N(E4, SpeakerWaveformSquare, 400),
  N(D4, SpeakerWaveformSquare, 800),
  N(G4, SpeakerWaveformSquare, 400),
  N(G4, SpeakerWaveformSquare, 400),
  N(F4, SpeakerWaveformSquare, 400),
  N(F4, SpeakerWaveformSquare, 400),
  N(E4, SpeakerWaveformSquare, 400),
  N(E4, SpeakerWaveformSquare, 400),
  N(D4, SpeakerWaveformSquare, 800),
  // Repeat first phrase
  N(C4, SpeakerWaveformSquare, 400),
  N(C4, SpeakerWaveformSquare, 400),
  N(G4, SpeakerWaveformSquare, 400),
  N(G4, SpeakerWaveformSquare, 400),
  N(A4, SpeakerWaveformSquare, 400),
  N(A4, SpeakerWaveformSquare, 400),
  N(G4, SpeakerWaveformSquare, 800),
  N(F4, SpeakerWaveformSquare, 400),
  N(F4, SpeakerWaveformSquare, 400),
  N(E4, SpeakerWaveformSquare, 400),
  N(E4, SpeakerWaveformSquare, 400),
  N(D4, SpeakerWaveformSquare, 400),
  N(D4, SpeakerWaveformSquare, 400),
  N(C4, SpeakerWaveformSquare, 800),
};

// Super Mario Bros Theme (Triangle wave)
static const SpeakerNote s_mario[] = {
  N(E5, SpeakerWaveformTriangle, 150),
  N(E5, SpeakerWaveformTriangle, 150),
  N(REST, SpeakerWaveformTriangle, 150),
  N(E5, SpeakerWaveformTriangle, 150),
  N(REST, SpeakerWaveformTriangle, 150),
  N(C5, SpeakerWaveformTriangle, 150),
  N(E5, SpeakerWaveformTriangle, 300),
  N(G5, SpeakerWaveformTriangle, 300),
  N(REST, SpeakerWaveformTriangle, 300),
  N(G4, SpeakerWaveformTriangle, 300),
  N(REST, SpeakerWaveformTriangle, 300),
  // Second part
  N(C5, SpeakerWaveformTriangle, 300),
  N(REST, SpeakerWaveformTriangle, 150),
  N(G4, SpeakerWaveformTriangle, 300),
  N(REST, SpeakerWaveformTriangle, 150),
  N(E4, SpeakerWaveformTriangle, 300),
  N(REST, SpeakerWaveformTriangle, 150),
  N(A4, SpeakerWaveformTriangle, 300),
  N(B4, SpeakerWaveformTriangle, 300),
  N(Bb4, SpeakerWaveformTriangle, 150),
  N(A4, SpeakerWaveformTriangle, 300),
  // Triplet section
  N(G4, SpeakerWaveformTriangle, 200),
  N(E5, SpeakerWaveformTriangle, 200),
  N(G5, SpeakerWaveformTriangle, 200),
  N(A4 + 12, SpeakerWaveformTriangle, 300), // A5
  N(F5, SpeakerWaveformTriangle, 150),
  N(G5, SpeakerWaveformTriangle, 150),
  N(REST, SpeakerWaveformTriangle, 150),
  N(E5, SpeakerWaveformTriangle, 300),
  N(C5, SpeakerWaveformTriangle, 150),
  N(D5, SpeakerWaveformTriangle, 150),
  N(B4, SpeakerWaveformTriangle, 300),
};

// Fur Elise - Beethoven (Sawtooth wave)
static const SpeakerNote s_fur_elise[] = {
  N(E5, SpeakerWaveformSawtooth, 250),
  N(Eb5, SpeakerWaveformSawtooth, 250),
  N(E5, SpeakerWaveformSawtooth, 250),
  N(Eb5, SpeakerWaveformSawtooth, 250),
  N(E5, SpeakerWaveformSawtooth, 250),
  N(B4, SpeakerWaveformSawtooth, 250),
  N(D5, SpeakerWaveformSawtooth, 250),
  N(C5, SpeakerWaveformSawtooth, 250),
  N(A4, SpeakerWaveformSawtooth, 500),
  N(REST, SpeakerWaveformSawtooth, 250),
  N(C4, SpeakerWaveformSawtooth, 250),
  N(E4, SpeakerWaveformSawtooth, 250),
  N(A4, SpeakerWaveformSawtooth, 250),
  N(B4, SpeakerWaveformSawtooth, 500),
  N(REST, SpeakerWaveformSawtooth, 250),
  N(E4, SpeakerWaveformSawtooth, 250),
  N(Ab4, SpeakerWaveformSawtooth, 250),
  N(B4, SpeakerWaveformSawtooth, 250),
  N(C5, SpeakerWaveformSawtooth, 500),
  N(REST, SpeakerWaveformSawtooth, 250),
  // Repeat opening motif
  N(E5, SpeakerWaveformSawtooth, 250),
  N(Eb5, SpeakerWaveformSawtooth, 250),
  N(E5, SpeakerWaveformSawtooth, 250),
  N(Eb5, SpeakerWaveformSawtooth, 250),
  N(E5, SpeakerWaveformSawtooth, 250),
  N(B4, SpeakerWaveformSawtooth, 250),
  N(D5, SpeakerWaveformSawtooth, 250),
  N(C5, SpeakerWaveformSawtooth, 250),
  N(A4, SpeakerWaveformSawtooth, 500),
};

// --- Melody table ---

typedef struct {
  const char *name;
  const SpeakerNote *notes;
  uint32_t num_notes;
  const char *waveform_name;
} MelodyInfo;

static const MelodyInfo s_melodies[] = {
  { "Ode to Joy",       s_ode_to_joy,  ARRAY_LENGTH(s_ode_to_joy),  "Sine" },
  { "Twinkle Twinkle",  s_twinkle,     ARRAY_LENGTH(s_twinkle),     "Square" },
  { "Super Mario",      s_mario,       ARRAY_LENGTH(s_mario),       "Triangle" },
  { "Fur Elise",        s_fur_elise,   ARRAY_LENGTH(s_fur_elise),   "Sawtooth" },
};

#define NUM_MELODIES ((int)ARRAY_LENGTH(s_melodies))
#define VOLUME 40

// --- UI ---

static Window *s_window;
static TextLayer *s_title_layer;
static TextLayer *s_melody_layer;
static TextLayer *s_info_layer;
static TextLayer *s_status_layer;

static int s_current_melody = 0;
static char s_info_buf[48];
static char s_status_buf[32];

static void prv_update_ui(void) {
  const MelodyInfo *melody = &s_melodies[s_current_melody];

  text_layer_set_text(s_melody_layer, melody->name);

  snprintf(s_info_buf, sizeof(s_info_buf), "%d / %d  -  %s",
           s_current_melody + 1, NUM_MELODIES, melody->waveform_name);
  text_layer_set_text(s_info_layer, s_info_buf);

  SpeakerStatus status = speaker_get_status();
  if (status == SpeakerStatusPlaying) {
    snprintf(s_status_buf, sizeof(s_status_buf), "Playing...");
  } else {
    snprintf(s_status_buf, sizeof(s_status_buf), "SELECT: Play");
  }
  text_layer_set_text(s_status_layer, s_status_buf);
}

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  const MelodyInfo *melody = &s_melodies[s_current_melody];
  speaker_play_notes(melody->notes, melody->num_notes, VOLUME);
  prv_update_ui();
}

static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  speaker_stop();
  s_current_melody--;
  if (s_current_melody < 0) {
    s_current_melody = NUM_MELODIES - 1;
  }
  prv_update_ui();
}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  speaker_stop();
  s_current_melody++;
  if (s_current_melody >= NUM_MELODIES) {
    s_current_melody = 0;
  }
  prv_update_ui();
}

static void prv_back_click_handler(ClickRecognizerRef recognizer, void *context) {
  speaker_stop();
  window_stack_pop(true);
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, prv_back_click_handler);
}

static TextLayer *prv_create_text_layer(Layer *parent, GRect frame, const char *font_key,
                                         GTextAlignment alignment) {
  TextLayer *layer = text_layer_create(frame);
  text_layer_set_background_color(layer, GColorClear);
  text_layer_set_text_color(layer, GColorWhite);
  text_layer_set_font(layer, fonts_get_system_font(font_key));
  text_layer_set_text_alignment(layer, alignment);
  layer_add_child(parent, text_layer_get_layer(layer));
  return layer;
}

static void prv_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);

  window_set_background_color(window, GColorBlack);

  int16_t inset = PBL_IF_ROUND_ELSE(20, 4);
  int16_t w = bounds.size.w - inset * 2;

  // Title
  s_title_layer = prv_create_text_layer(root,
    GRect(inset, PBL_IF_ROUND_ELSE(24, 12), w, 30),
    FONT_KEY_GOTHIC_24_BOLD, GTextAlignmentCenter);
  text_layer_set_text(s_title_layer, "Speaker Demo");

  // Melody name (large)
  s_melody_layer = prv_create_text_layer(root,
    GRect(inset, PBL_IF_ROUND_ELSE(62, 52), w, 36),
    FONT_KEY_GOTHIC_28_BOLD, GTextAlignmentCenter);

  // Info line (index + waveform)
  s_info_layer = prv_create_text_layer(root,
    GRect(inset, PBL_IF_ROUND_ELSE(96, 88), w, 24),
    FONT_KEY_GOTHIC_18, GTextAlignmentCenter);

  // Status
  s_status_layer = prv_create_text_layer(root,
    GRect(inset, PBL_IF_ROUND_ELSE(126, 120), w, 30),
    FONT_KEY_GOTHIC_24, GTextAlignmentCenter);

  prv_update_ui();
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_title_layer);
  text_layer_destroy(s_melody_layer);
  text_layer_destroy(s_info_layer);
  text_layer_destroy(s_status_layer);
}

static void prv_init(void) {
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
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
