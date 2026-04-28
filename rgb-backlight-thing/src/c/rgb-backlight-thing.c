#include <pebble.h>

static Window *s_window;
static TextLayer *s_text_layer;

static void prv_set_color(GColor color, const char *name) {
  light_set_color(color);
  text_layer_set_text(s_text_layer, name);
}

static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  prv_set_color(GColorRed, "RED");
}

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  prv_set_color(GColorGreen, "GREEN");
}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  prv_set_color(GColorBlue, "BLUE");
}

static void prv_select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
//  light_reset_color();
  text_layer_set_text(s_text_layer, "RESET");
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, prv_select_long_click_handler, NULL);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(GRect(0, bounds.size.h / 2 - 20, bounds.size.w, 40));
  text_layer_set_text(s_text_layer, "PRESS A BUTTON");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
}

static void prv_init(void) {
  s_window = window_create();
  window_set_click_config_provider(s_window, prv_click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, true);

//  light_enable(true);
}

static void prv_deinit(void) {
  light_enable(false);
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
