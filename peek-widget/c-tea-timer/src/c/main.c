#include <pebble.h>

// Tea steeping timer built around the Quick View widget API.
//
// Start a steep and leave the app: the timer publishes itself as a Quick View widget at the
// bottom of the watchface, a wakeup relaunches the app when the tea is ready, and the ready
// state stays peeked until the user acts on it. Pressing Down on the watchface (depending on
// the user's Settings > Quick View > Open With Down choice) launches this app with
// APP_LAUNCH_PEEK_WIDGET and the widget's launch code, so the app knows exactly which state
// the user tapped through.

#define STEEP_MINUTES_MIN (1)
#define STEEP_MINUTES_MAX (5)
#define STEEP_MINUTES_DEFAULT (3)

// Persisted so a wakeup or widget launch can find the steep state again
enum {
  PersistKeySteepEnd = 1,
  PersistKeyWakeupId = 2,
};

// Widget launch codes: launch_get_args() tells us which widget the user tapped through
enum {
  LaunchCodeSteeping = 1,
  LaunchCodeReady = 2,
};

typedef enum {
  TeaStateIdle,
  TeaStateSteeping,
  TeaStateReady,
} TeaState;

static Window *s_window;
static TextLayer *s_title_layer;
static TextLayer *s_status_layer;
static TextLayer *s_hint_layer;
static char s_status_text[32];
static TeaState s_state = TeaStateIdle;
static int s_steep_minutes = STEEP_MINUTES_DEFAULT;

static void prv_update_ui(void);

// Steep lifecycle
////////////////////////////////////

static time_t prv_steep_end(void) {
  return persist_exists(PersistKeySteepEnd) ? persist_read_int(PersistKeySteepEnd) : 0;
}

static void prv_publish_steeping_widget(time_t steep_end) {
  char subtitle[32];
  struct tm *ready_tm = localtime(&steep_end);
  strftime(subtitle, sizeof(subtitle),
           clock_is_24h_style() ? "Ready at %H:%M" : "Ready at %l:%M %p", ready_tm);
  const PeekWidgetInfo info = {
    .title = "Tea is steeping",
    .subtitle = subtitle,
    .launch_code = LaunchCodeSteeping,
    // If the wakeup somehow never fires, don't linger forever
    .timeout_s = (steep_end - time(NULL)) + SECONDS_PER_MINUTE,
  };
  peek_widget_publish(&info);
}

static void prv_start_steep(void) {
  const time_t steep_end = time(NULL) + (s_steep_minutes * SECONDS_PER_MINUTE);
  const WakeupId wakeup_id = wakeup_schedule(steep_end, LaunchCodeReady,
                                             true /* notify_if_missed */);
  if (wakeup_id < 0) {
    text_layer_set_text(s_status_layer, "Wakeup failed");
    return;
  }
  persist_write_int(PersistKeySteepEnd, steep_end);
  persist_write_int(PersistKeyWakeupId, wakeup_id);
  prv_publish_steeping_widget(steep_end);
  s_state = TeaStateSteeping;
  prv_update_ui();
}

static void prv_tea_is_ready(void) {
  persist_delete(PersistKeySteepEnd);
  persist_delete(PersistKeyWakeupId);
  vibes_double_pulse();
  const PeekWidgetInfo info = {
    .title = "Tea is ready!",
    .subtitle = "Fish out that tea bag",
    .launch_code = LaunchCodeReady,
    .timeout_s = 0, // Stays up until the user acts on it
  };
  peek_widget_publish(&info);
  s_state = TeaStateReady;
  prv_update_ui();
}

static void prv_reset(void) {
  if (persist_exists(PersistKeyWakeupId)) {
    wakeup_cancel(persist_read_int(PersistKeyWakeupId));
  }
  persist_delete(PersistKeySteepEnd);
  persist_delete(PersistKeyWakeupId);
  peek_widget_withdraw();
  s_state = TeaStateIdle;
  prv_update_ui();
}

// UI
////////////////////////////////////

static void prv_update_ui(void) {
  switch (s_state) {
    case TeaStateIdle:
      snprintf(s_status_text, sizeof(s_status_text), "Steep for %d min", s_steep_minutes);
      text_layer_set_text(s_status_layer, s_status_text);
      text_layer_set_text(s_hint_layer, "UP/DOWN adjust\nSELECT start");
      break;
    case TeaStateSteeping: {
      const int remaining = prv_steep_end() - time(NULL);
      snprintf(s_status_text, sizeof(s_status_text), "%d:%02d left",
               remaining / SECONDS_PER_MINUTE, remaining % SECONDS_PER_MINUTE);
      text_layer_set_text(s_status_layer, s_status_text);
      text_layer_set_text(s_hint_layer, "Leave the app!\nSELECT cancel");
      break;
    }
    case TeaStateReady:
      text_layer_set_text(s_status_layer, "Enjoy your tea!");
      text_layer_set_text(s_hint_layer, "SELECT reset");
      break;
  }
}

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if (s_state != TeaStateSteeping) {
    return;
  }
  if (time(NULL) >= prv_steep_end()) {
    // Finished while the app was open; the wakeup is no longer needed
    if (persist_exists(PersistKeyWakeupId)) {
      wakeup_cancel(persist_read_int(PersistKeyWakeupId));
    }
    prv_tea_is_ready();
  } else {
    prv_update_ui();
  }
}

// Click handlers
////////////////////////////////////

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch (s_state) {
    case TeaStateIdle:
      prv_start_steep();
      break;
    case TeaStateSteeping:
    case TeaStateReady:
      prv_reset();
      break;
  }
}

static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_state == TeaStateIdle && s_steep_minutes < STEEP_MINUTES_MAX) {
    s_steep_minutes++;
    prv_update_ui();
  }
}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_state == TeaStateIdle && s_steep_minutes > STEEP_MINUTES_MIN) {
    s_steep_minutes--;
    prv_update_ui();
  }
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
}

// Window lifecycle
////////////////////////////////////

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  const GRect bounds = layer_get_bounds(window_layer);

  s_title_layer = text_layer_create(GRect(0, 18, bounds.size.w, 32));
  text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  text_layer_set_text(s_title_layer, "Tea Timer");
  layer_add_child(window_layer, text_layer_get_layer(s_title_layer));

  s_status_layer = text_layer_create(GRect(0, 64, bounds.size.w, 32));
  text_layer_set_font(s_status_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_status_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_status_layer));

  s_hint_layer = text_layer_create(GRect(0, bounds.size.h - 54, bounds.size.w, 44));
  text_layer_set_font(s_hint_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_hint_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_hint_layer));

  prv_update_ui();
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_title_layer);
  text_layer_destroy(s_status_layer);
  text_layer_destroy(s_hint_layer);
}

// App lifecycle
////////////////////////////////////

static void prv_handle_launch(void) {
  // Resume a steep that is still running regardless of how we were launched
  if (prv_steep_end() > time(NULL)) {
    s_state = TeaStateSteeping;
  }

  switch (launch_reason()) {
    case APP_LAUNCH_WAKEUP:
      // The steep finished while we were closed
      prv_tea_is_ready();
      break;
    case APP_LAUNCH_PEEK_WIDGET:
      // The user pressed the widget's button shortcut on the watchface; the launch code
      // tells us which widget that was
      if (launch_get_args() == LaunchCodeReady) {
        // Acting on "Tea is ready!" clears it
        peek_widget_withdraw();
        s_state = TeaStateReady;
      }
      break;
    default:
      if (s_state != TeaStateSteeping && prv_steep_end() != 0) {
        // A steep ended while we were closed and the wakeup was missed
        prv_tea_is_ready();
      }
      break;
  }
}

static void prv_init(void) {
  prv_handle_launch();

  s_window = window_create();
  window_set_click_config_provider(s_window, prv_click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(SECOND_UNIT, prv_tick_handler);
}

static void prv_deinit(void) {
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
