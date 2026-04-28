#include <pebble.h>
#include <stdio.h>

static Window *s_window;
static Layer *s_canvas;

static bool s_enabled_at_init;

static uint32_t s_count_touchdown;
static uint32_t s_count_liftoff;
static uint32_t s_count_position;

static bool s_has_raw;
static TouchEvent s_last_raw;

static bool s_has_touch_pos;
static GPoint s_touch_pos;

static const char *prv_raw_label(TouchEventType t) {
  switch (t) {
    case TouchEvent_Touchdown:      return "TD";
    case TouchEvent_Liftoff:        return "LO";
    case TouchEvent_PositionUpdate: return "PU";
  }
  return "??";
}

static void prv_touch_handler(const TouchEvent *event, void *context) {
  switch (event->type) {
    case TouchEvent_Touchdown:      s_count_touchdown++; break;
    case TouchEvent_Liftoff:        s_count_liftoff++;   break;
    case TouchEvent_PositionUpdate: s_count_position++;  break;
  }
  s_last_raw = *event;
  s_has_raw = true;

  if (event->type == TouchEvent_Liftoff) {
    s_has_touch_pos = false;
  } else {
    s_touch_pos = GPoint(event->x, event->y);
    s_has_touch_pos = true;
  }
  layer_mark_dirty(s_canvas);
}

static void prv_canvas_update(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  graphics_context_set_text_color(ctx, GColorWhite);
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);

  char line1[32];
  char line2[48];
  char line3[48];

  snprintf(line1, sizeof(line1), "touch en=%s",
           s_enabled_at_init ? "Y" : "N");

  snprintf(line2, sizeof(line2), "TD%lu LO%lu PU%lu",
           (unsigned long)s_count_touchdown,
           (unsigned long)s_count_liftoff,
           (unsigned long)s_count_position);

  if (s_has_raw) {
    snprintf(line3, sizeof(line3), "%s %d,%d",
             prv_raw_label(s_last_raw.type),
             s_last_raw.x, s_last_raw.y);
  } else {
    snprintf(line3, sizeof(line3), "raw: --");
  }

  const int16_t row_h = 22;
  const int16_t y0 = (bounds.size.h / 2) - (row_h * 3 / 2);

  graphics_draw_text(ctx, line1, font,
      GRect(0, y0 + row_h * 0, bounds.size.w, row_h),
      GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, line2, font,
      GRect(0, y0 + row_h * 1, bounds.size.w, row_h),
      GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, line3, font,
      GRect(0, y0 + row_h * 2, bounds.size.w, row_h),
      GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

  if (s_has_touch_pos) {
    GPoint p = s_touch_pos;
    graphics_context_set_stroke_color(ctx, GColorRed);
    graphics_context_set_fill_color(ctx, GColorRed);
    graphics_draw_line(ctx, GPoint(p.x - 8, p.y), GPoint(p.x + 8, p.y));
    graphics_draw_line(ctx, GPoint(p.x, p.y - 8), GPoint(p.x, p.y + 8));
    graphics_fill_circle(ctx, p, 2);
  }
}

static void prv_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);

  s_canvas = layer_create(bounds);
  layer_set_update_proc(s_canvas, prv_canvas_update);
  layer_add_child(root, s_canvas);

  s_enabled_at_init = touch_service_is_enabled();
  touch_service_subscribe(prv_touch_handler, NULL);
}

static void prv_window_unload(Window *window) {
  touch_service_unsubscribe();
  layer_destroy(s_canvas);
}

static void prv_init(void) {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, true);
}

static void prv_deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
