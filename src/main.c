/*
 * main.c
 * Main window. Flips the coin. Shows the history. Jumps over the moon.
*/

#include <pebble.h>
#include "main.h"
#include "gvars.h"
#include "settings.h"

// Window objects and variables
static Window *s_main_window;
static Layer *s_coin_layer;
static Layer *s_impact_layer;
static TextLayer *s_history_text_layer;
static TextLayer *s_info_text_layer;
static GFont veramono_font;
static BitmapLayer *s_bitmap_layer;
static GBitmap *s_bitmap = NULL;
static int impact_stage = -1; // Prevent drawing
static bool displaying_info = false; // For edge
// History layer objects
static PropertyAnimation *s_history_anim;
static GRect start;
static GRect end;
// Flip animation variables
static int frame_number, target_frame, animation_length, frame_delay, frame_multiplier;
static bool currently_flipping = false;
// Custom vibration pattern
static const VibePattern PAT = {
	.durations = (uint32_t []) {60},
	.num_segments = 1
};
// Coin sprite frames
static const int TOTALFRAMES = 24;
static const int coin_frame[] = {
  RESOURCE_ID_SPRITE_0,  RESOURCE_ID_SPRITE_1,  RESOURCE_ID_SPRITE_2,  RESOURCE_ID_SPRITE_3,
  RESOURCE_ID_SPRITE_4,  RESOURCE_ID_SPRITE_5,  RESOURCE_ID_SPRITE_6,  RESOURCE_ID_SPRITE_7,
  RESOURCE_ID_SPRITE_8,  RESOURCE_ID_SPRITE_9,  RESOURCE_ID_SPRITE_10,  RESOURCE_ID_SPRITE_11,
  RESOURCE_ID_SPRITE_12,  RESOURCE_ID_SPRITE_13,  RESOURCE_ID_SPRITE_14,  RESOURCE_ID_SPRITE_15,
  RESOURCE_ID_SPRITE_16,  RESOURCE_ID_SPRITE_17,  RESOURCE_ID_SPRITE_18,  RESOURCE_ID_SPRITE_19,
  RESOURCE_ID_SPRITE_20,  RESOURCE_ID_SPRITE_21,  RESOURCE_ID_SPRITE_22,  RESOURCE_ID_SPRITE_23,
};

static void update_target_frame() {
  target_frame = (result == 0 ? 0 : (result == 1 ? 12 : 6));
}

static void update_result() {
  if (use_edge && rand() % 6000 == 0) // 1 in 6000 chance
    result = 2;
  else
    result = rand() % 2;
  update_target_frame();
  
  // Update stats
  switch (result) {
    case 0: // Heads
      total_heads++;
      break;
    case 1: // Tails
      total_tails++;
      break;
    case 2: // Edge
      total_edge++;
      break;
    default: // This should never EVER happen
      break;
  }
}

static void vibrate(bool initial_vibration) {
  if (use_vibrations) {
    if (result == 2 && !initial_vibration)
      vibes_long_pulse();
    else
      vibes_enqueue_custom_pattern(PAT);
  }
}

// Shows or hides the history frame
void update_history_frame() {
  // Hide history layer and move coin layer
  layer_set_hidden(text_layer_get_layer(s_history_text_layer), !use_history);
  layer_set_bounds(s_coin_layer, GRect(0,(use_history ? 9 : 0),144,168));
}

static void update_history() {
  // Remove last result (history is 18 chars long)
  char temp1[19] = "";
  strncpy(temp1, history, 15);
  // Concatenate result in the front
  char temp2[19] = "   ";
  temp2[1] = (result == 0 ? 'H' : (result == 1 ? 'T' : 'E'));
  strcat(temp2, temp1);
  strcpy(history, temp2);
  text_layer_set_text(s_history_text_layer, history);
  
  if (use_history && use_animations) {
    s_history_anim = property_animation_create_layer_frame(text_layer_get_layer(s_history_text_layer), &start, &end);
    animation_set_curve((Animation*)s_history_anim, AnimationCurveEaseOut); 
    animation_set_duration((Animation*)s_history_anim, 200);
    animation_schedule((Animation*)s_history_anim);
  }
}

// Updates the impact layer with the impact_stage
static void impact_layer_update_proc(Layer *this_layer, GContext *ctx) {
  if (impact_stage < 0 || impact_stage >= 6) return; // Clear
  GRect bounds = layer_get_bounds(this_layer);
  GPoint center = GPoint(bounds.size.w / 2, (bounds.size.h / 2));
  // Draw circles (64 pixel radius, 111 corner radius)
  int radius = impact_stage*4 + 73; // Initial radius of 73
  int thickness = (20/(impact_stage+1))-2; // Initial thickness of 20
  graphics_context_set_stroke_width(ctx, thickness);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_draw_circle(ctx, center, radius);
}
static void impact_loop(void *context) {
  layer_mark_dirty(s_impact_layer);
  impact_stage++;
  if (impact_stage >= 7) return;
  app_timer_register(16, impact_loop, NULL);
}
static void impact() {
  if (use_animations) {
    impact_stage = 0;
    app_timer_register(1, impact_loop, NULL);
  }
}

// Update frame for animations and displaying the result
static void update_frame() {
  if (s_bitmap) { // Destroy bitmap if it exists
    gbitmap_destroy(s_bitmap);
    s_bitmap = NULL;
  }
  s_bitmap = gbitmap_create_with_resource(coin_frame[(frame_number * frame_multiplier) % TOTALFRAMES]);
  bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
  layer_mark_dirty(bitmap_layer_get_layer(s_bitmap_layer));
  frame_number++;
}
static void end_info_display(void *context) {
  displaying_info = false;
  layer_set_hidden(text_layer_get_layer(s_info_text_layer), true);
}
void display_result() {
  update_target_frame();
  frame_multiplier = 1;
  frame_number = target_frame;
  update_frame();
  // Highly unlikely edge info display
  if (result == 2) {
    displaying_info = true;
    layer_set_hidden(text_layer_get_layer(s_info_text_layer), false);
    app_timer_register(15000, end_info_display, NULL);
  }
}
// Slow flipping animation and reveal
static void flip_animation_final(void *context) {
  if (frame_number % TOTALFRAMES == target_frame) {
    display_result();
    update_history();
    vibrate(false);
    impact();
    currently_flipping = false;
    return;
  }
  update_frame();
  frame_delay += 1; // Ease out each frame
  app_timer_register(frame_delay, flip_animation_final, NULL);
}
// Fast flipping animation
static void flip_animation_initial(void *context) {
  // Fast animation done, move on to the final animation
  if (frame_number >= animation_length) {
    frame_multiplier = 1;
    frame_number = frame_number % TOTALFRAMES;
    if (frame_number == target_frame) { // Ensures at least one full spin
      frame_number++;
      frame_number = frame_number % TOTALFRAMES;
    }
    app_timer_register(frame_delay, flip_animation_final, NULL);
    return;
  }
  update_frame();
  if (frame_number % TOTALFRAMES == 0 && frame_multiplier > 1) frame_multiplier--; // Ease out each loop
  app_timer_register(frame_delay, flip_animation_initial, NULL);
}

static void flip(bool animate) {
  if (!currently_flipping && !displaying_info) {
    update_result();
    if (animate && use_animations) { // Standard flip
      currently_flipping = true;
      vibrate(true);
      frame_number = 0;
      animation_length = 60 + (rand() % 12) - 6;
      frame_delay = 18;
      frame_multiplier = 5;
      app_timer_register(1, flip_animation_initial, NULL);
    } else { // Quick flip
      display_result();
      update_history();
      vibrate(false);
      impact();
    }
  }
}

// Toggle visibility
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (!currently_flipping && !displaying_info)
    show_settings();
}
// Normal flipping
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  flip(true);
}
static void tap_handler(AccelAxisType axis, int32_t direction) {
  if (use_accelerometer)
    flip(true);
}
// Quick flipping (regardless of use_animation)
static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  flip(false);
}
static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(s_main_window);
  //GRect bounds = layer_get_frame(window_layer);
  
  // Create base coin layer
  s_coin_layer = layer_create(GRect(0,0,144,168));
  layer_add_child(window_layer, s_coin_layer);
  // Create bitmap layer
  s_bitmap_layer = bitmap_layer_create(GRect(0, 12, 144, 144));
  layer_add_child(s_coin_layer, bitmap_layer_get_layer(s_bitmap_layer));
  // Create impact layer
  s_impact_layer = layer_create(layer_get_bounds(s_coin_layer));
  layer_add_child(s_coin_layer, s_impact_layer);
  layer_set_update_proc(s_impact_layer, impact_layer_update_proc);
  
  // Initialize font
  veramono_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_VERA_MONO_16));
  // Create textlayer for history display
  s_history_text_layer = text_layer_create(GRect(-2, 0, 180, 19));
  text_layer_set_background_color(s_history_text_layer, GColorBlack);
  text_layer_set_text_color(s_history_text_layer, GColorWhite);
  text_layer_set_text_alignment(s_history_text_layer, GTextAlignmentLeft);
  text_layer_set_font(s_history_text_layer, veramono_font);
  layer_add_child(window_layer, text_layer_get_layer(s_history_text_layer));
  
  // Create textlayer for edge info display
  s_info_text_layer = text_layer_create(GRect(0, 108, 144, 60));
  text_layer_set_text_alignment(s_info_text_layer, GTextAlignmentCenter);
  text_layer_set_text(s_info_text_layer, "The coin landed on its edge! That's a 1 in 6000 chance! (locking app for 15 seconds)");
  layer_set_hidden(text_layer_get_layer(s_info_text_layer), true); // Never to be seen...
  layer_add_child(window_layer, text_layer_get_layer(s_info_text_layer));
  
  // Initialize history animation variables
  start = GRect(-17, 0, 180, 19);
  end = GRect(-2, 0, 180, 19);
  
  // When window is ready:
  update_history_frame();
  text_layer_set_text(s_history_text_layer, history);
  if (launch_reason() == 5) // If quicklaunched, do a flip
    flip(true);
  else // Otherwise just display last flip result
    display_result();
}

static void main_window_unload(Window *window) {
  animation_unschedule_all();
  text_layer_destroy(s_history_text_layer);
  text_layer_destroy(s_info_text_layer);
  if (s_bitmap) {
    gbitmap_destroy(s_bitmap);
    s_bitmap = NULL;
  }
  bitmap_layer_destroy(s_bitmap_layer);
  fonts_unload_custom_font(veramono_font);
  layer_destroy(s_coin_layer);
  layer_destroy(s_impact_layer);
  
  // Write persistent data
  persist_write_int(LAST_FLIP_RESULT, result);
  persist_write_string(LAST_HISTORY, history);
  persist_write_int(STAT_HEADS, total_heads);
  persist_write_int(STAT_TAILS, total_tails);
  persist_write_int(STAT_EDGE, total_edge);
}

static void init() {
  // Seed random values
  srand(time(NULL));
  // Get persistent data
  use_animations = persist_exists(FLAG_USE_ANIMATIONS) ? persist_read_bool(FLAG_USE_ANIMATIONS) : true;
  use_history = persist_exists(FLAG_USE_HISTORY) ? persist_read_bool(FLAG_USE_HISTORY) : false;
  use_vibrations = persist_exists(FLAG_USE_VIBRATIONS) ? persist_read_bool(FLAG_USE_VIBRATIONS) : true;
  use_accelerometer = persist_exists(FLAG_USE_ACCELEROMETER) ? persist_read_bool(FLAG_USE_ACCELEROMETER) : true;
  use_edge = persist_exists(FLAG_USE_EDGE) ? persist_read_bool(FLAG_USE_EDGE) : false;
  result = persist_exists(LAST_FLIP_RESULT) ? persist_read_int(LAST_FLIP_RESULT) : 0;
  total_heads = persist_exists(STAT_HEADS) ? persist_read_int(STAT_HEADS) : 0;
  total_tails = persist_exists(STAT_TAILS) ? persist_read_int(STAT_TAILS) : 0;
  total_edge = persist_exists(STAT_EDGE) ? persist_read_int(STAT_EDGE) : 0;
  if (persist_exists(LAST_HISTORY))
    persist_read_string(LAST_HISTORY, history, 19);
  else
    strcpy(history, "                  ");
  // Load window
  s_main_window = window_create();
  accel_tap_service_subscribe(tap_handler);
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}
static void deinit(void) {
  window_destroy(s_main_window);
}
int main(void) {
  init();
  app_event_loop();
  deinit();
}