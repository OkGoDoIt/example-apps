#include <pebble.h>

static Window *s_window;
static TextLayer *s_time_layer;
static TextLayer *s_now_playing_layer;
static TextLayer *s_title_layer;
static TextLayer *s_artist_layer;
static TextLayer *s_album_layer;
static Layer *s_progress_layer;
static AppTimer *s_progress_timer;

static char s_time_buffer[8];
static char s_title[MUSIC_SERVICE_BUFFER_LENGTH];
static char s_artist[MUSIC_SERVICE_BUFFER_LENGTH];
static char s_album[MUSIC_SERVICE_BUFFER_LENGTH];
static MusicServicePlaybackInfo s_playback_info;
static bool s_show_media;
static bool s_in_focus = true;

static bool prv_playback_is_active(void) {
  return s_playback_info.playback_state == MusicServicePlaybackStatePlaying ||
      s_playback_info.playback_state == MusicServicePlaybackStateForwarding ||
      s_playback_info.playback_state == MusicServicePlaybackStateRewinding;
}

static void prv_progress_layer_update(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_rect(ctx, bounds, 2, GCornersAll);

  if (!(s_playback_info.capabilities & MusicServiceCapabilityProgress) ||
      s_playback_info.duration_ms == 0) {
    return;
  }

  uint32_t width = ((uint64_t)bounds.size.w * s_playback_info.position_ms) /
      s_playback_info.duration_ms;
  if (width > (uint32_t)bounds.size.w) {
    width = bounds.size.w;
  }
  graphics_context_set_fill_color(ctx, GColorPictonBlue);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 2, GCornersAll);
}

static void prv_progress_timer_callback(void *context);

static void prv_update_music(void) {
  music_service_get_playback_info(&s_playback_info);

  const bool state_available =
      s_playback_info.capabilities & MusicServiceCapabilityPlaybackState;
  s_show_media = music_service_has_now_playing() &&
      (!state_available || prv_playback_is_active());

  layer_set_hidden(text_layer_get_layer(s_now_playing_layer), !s_show_media);
  layer_set_hidden(text_layer_get_layer(s_title_layer), !s_show_media);
  layer_set_hidden(text_layer_get_layer(s_artist_layer), !s_show_media);
  layer_set_hidden(text_layer_get_layer(s_album_layer), !s_show_media);
  layer_set_hidden(s_progress_layer, !s_show_media);

  if (s_show_media) {
    music_service_get_now_playing(s_title, s_artist, s_album);
    text_layer_set_text(s_title_layer, s_title);
    text_layer_set_text(s_artist_layer, s_artist);
    text_layer_set_text(s_album_layer, s_album);
    layer_mark_dirty(s_progress_layer);
  }

  const bool should_update_progress = s_show_media && s_in_focus &&
      (s_playback_info.capabilities & MusicServiceCapabilityProgress) &&
      prv_playback_is_active();
  if (should_update_progress && !s_progress_timer) {
    s_progress_timer = app_timer_register(5000, prv_progress_timer_callback, NULL);
  } else if (!should_update_progress && s_progress_timer) {
    app_timer_cancel(s_progress_timer);
    s_progress_timer = NULL;
  }
}

static void prv_progress_timer_callback(void *context) {
  s_progress_timer = NULL;
  prv_update_music();
}

static void prv_music_event_handler(MusicServiceEventType event_type) {
  if (s_in_focus) {
    prv_update_music();
  }
}

static void prv_focus_handler(bool in_focus) {
  s_in_focus = in_focus;
  if (in_focus) {
    prv_update_music();
  } else if (s_progress_timer) {
    app_timer_cancel(s_progress_timer);
    s_progress_timer = NULL;
  }
}

static void prv_update_time(struct tm *tick_time) {
  strftime(s_time_buffer, sizeof(s_time_buffer),
           clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  text_layer_set_text(s_time_layer, s_time_buffer);
}

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  prv_update_time(tick_time);
}

static TextLayer *prv_create_text_layer(GRect frame, GFont font,
                                        GTextAlignment alignment) {
  TextLayer *layer = text_layer_create(frame);
  text_layer_set_background_color(layer, GColorClear);
  text_layer_set_text_color(layer, GColorWhite);
  text_layer_set_font(layer, font);
  text_layer_set_text_alignment(layer, alignment);
  text_layer_set_overflow_mode(layer, GTextOverflowModeTrailingEllipsis);
  layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(layer));
  return layer;
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  const int16_t inset = PBL_IF_ROUND_ELSE(20, 8);
  const int16_t content_width = bounds.size.w - (2 * inset);

  s_time_layer = prv_create_text_layer(
      GRect(inset, 4, content_width, 52),
      fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD), GTextAlignmentCenter);
  s_now_playing_layer = prv_create_text_layer(
      GRect(inset, 62, content_width, 16),
      fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter);
  text_layer_set_text_color(s_now_playing_layer, GColorPictonBlue);
  text_layer_set_text(s_now_playing_layer, "NOW PLAYING");
  s_title_layer = prv_create_text_layer(
      GRect(inset, 78, content_width, 30),
      fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GTextAlignmentCenter);
  s_artist_layer = prv_create_text_layer(
      GRect(inset, 108, content_width, 22),
      fonts_get_system_font(FONT_KEY_GOTHIC_18), GTextAlignmentCenter);
  s_album_layer = prv_create_text_layer(
      GRect(inset, 130, content_width, 22),
      fonts_get_system_font(FONT_KEY_GOTHIC_18), GTextAlignmentCenter);

  s_progress_layer = layer_create(GRect(inset, bounds.size.h - 10, content_width, 4));
  layer_set_update_proc(s_progress_layer, prv_progress_layer_update);
  layer_add_child(window_layer, s_progress_layer);

  time_t now = time(NULL);
  prv_update_time(localtime(&now));
  prv_update_music();
}

static void prv_window_unload(Window *window) {
  layer_destroy(s_progress_layer);
  text_layer_destroy(s_album_layer);
  text_layer_destroy(s_artist_layer);
  text_layer_destroy(s_title_layer);
  text_layer_destroy(s_now_playing_layer);
  text_layer_destroy(s_time_layer);
}

static void prv_init(void) {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  music_service_subscribe(prv_music_event_handler);
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);
  app_focus_service_subscribe(prv_focus_handler);
}

static void prv_deinit(void) {
  app_focus_service_unsubscribe();
  music_service_unsubscribe();
  tick_timer_service_unsubscribe();
  if (s_progress_timer) {
    app_timer_cancel(s_progress_timer);
  }
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
