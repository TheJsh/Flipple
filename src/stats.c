/*
 * stats.c
 * Displays the flipping statistics window, and is sometimes the home of Chara Dreemurr
 */

#include <pebble.h>
#include "gvars.h"
#include "stats.h"
#include "main.h"

static Window *s_stats_window;
static TextLayer *s_title_text_layer;
static TextLayer *s_total_text_layer;
static TextLayer *s_heads_text_layer;
static TextLayer *s_tails_text_layer;
static TextLayer *s_edge_text_layer;
static TextLayer *s_confirm_text_layer;
static Layer *s_chara_layer;
static char total_heads_str[18] = "";
static char total_tails_str[18] = "";
static char total_edge_str[18] = "";
static char total_flips_str[18] = "";
static int erase_confirm = 0;
static bool chara_appears = false;

static const VibePattern PANICPAT = {
	.durations = (uint32_t []) {2000},
	.num_segments = 1
};

static void pacifist_erase() {
  total_heads = 0; total_tails = 0; total_edge = 0;
  persist_write_int(STAT_HEADS, 0);
  persist_write_int(STAT_TAILS, 0);
  persist_write_int(STAT_EDGE, 0);
  strcpy(history, "                  ");
  persist_write_string(LAST_HISTORY, history);
  result = 0;
  display_result();
  if (!chara_appears) {
    window_stack_pop(true);
    window_stack_pop(true);
  }
}

static void strike(void *context) {
  if (erase_confirm >= 70) {
    window_stack_pop_all(false); // bOi
    return;
  }
  erase_confirm++;
  layer_set_bounds(text_layer_get_layer(s_title_text_layer), GRect(-12 + rand()%12, -12 + rand()%12, 220, 220));
  app_timer_register(24, strike, NULL);
}
static void erase(void *context) {
  // You've done it now...
  pacifist_erase();
  erase_confirm++;
  text_layer_set_font(s_title_text_layer, fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS));
  layer_set_frame(text_layer_get_layer(s_title_text_layer), GRect(-12, -12, 220, 220));
  text_layer_set_text(s_title_text_layer, "9999999999999999999999999999999999999999999999999999999999999999999");
  vibes_enqueue_custom_pattern(PANICPAT);
  app_timer_register(24, strike, NULL);
}

static void draw_it(Layer *this_layer, GContext *ctx) {
  if (erase_confirm == 1 && chara_appears) { // Draw it.
    graphics_fill_rect(ctx, GRect(68,119,3,3), 0, GCornerNone); graphics_fill_rect(ctx, GRect(73,119,3,3), 0, GCornerNone);
    graphics_draw_pixel(ctx, GPoint(70,123)); graphics_draw_pixel(ctx, GPoint(73,123));
    graphics_draw_pixel(ctx, GPoint(71,124)); graphics_draw_pixel(ctx, GPoint(72,124));
  } else if (erase_confirm == 2 && chara_appears) { // DRAW IT.
    graphics_fill_rect(ctx, GRect(68,118,3,2), 0, GCornerNone); graphics_fill_rect(ctx, GRect(74,118,4,2), 0, GCornerNone);
    graphics_draw_pixel(ctx, GPoint(67,119)); graphics_draw_pixel(ctx, GPoint(78,119));
    graphics_draw_pixel(ctx, GPoint(69,120)); graphics_draw_pixel(ctx, GPoint(75,120));
    graphics_fill_rect(ctx, GRect(67,120,2,2), 0, GCornerNone); graphics_fill_rect(ctx, GRect(76,120,3,2), 0, GCornerNone);
    graphics_draw_pixel(ctx, GPoint(74,121)); graphics_draw_pixel(ctx, GPoint(68,122));
    graphics_draw_pixel(ctx, GPoint(70,122)); graphics_draw_pixel(ctx, GPoint(73,122));
    graphics_draw_pixel(ctx, GPoint(74,122)); graphics_draw_pixel(ctx, GPoint(77,122));
    graphics_draw_pixel(ctx, GPoint(78,122)); graphics_draw_pixel(ctx, GPoint(68,123));
    graphics_draw_pixel(ctx, GPoint(77,123)); graphics_draw_pixel(ctx, GPoint(77,124));
    graphics_fill_rect(ctx, GRect(70,123,5,4), 0, GCornerNone);
    graphics_draw_pixel(ctx, GPoint(71,127)); graphics_draw_pixel(ctx, GPoint(72,127));
    graphics_draw_pixel(ctx, GPoint(73,127)); graphics_draw_pixel(ctx, GPoint(72,128));
  }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (erase_confirm == 0) {
    erase_confirm++;
    text_layer_set_text(s_confirm_text_layer, "Press select again to confirm ERASE");
  } else if (erase_confirm == 1) {
    if (chara_appears) {
      text_layer_set_text(s_title_text_layer, "");
      text_layer_set_text(s_total_text_layer, "");
      text_layer_set_text(s_heads_text_layer, "");
      text_layer_set_text(s_tails_text_layer, "");
      text_layer_set_text(s_edge_text_layer, "");
      erase_confirm++;
      text_layer_set_text(s_confirm_text_layer, "");
      app_timer_register(250, erase, NULL);
    } else {
      pacifist_erase();
    }
  }
}
static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (erase_confirm == 0 && !chara_appears) chara_appears = true;
}
static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void stats_window_load(Window* window) {
  erase_confirm = 0;
  chara_appears = rand() % 100 == 0;
  // Convert integers to strings and add to string
  int total_flips = total_heads + total_tails + total_edge;
  static char tmp[18] = "";
  snprintf(tmp, sizeof(tmp), "%d", total_flips);
  strcpy(total_flips_str, "Total flips:\n");
  strcat(total_flips_str, tmp);
  snprintf(tmp, sizeof(tmp), "%d", total_heads);
  strcpy(total_heads_str, "Heads:\n");
  strcat(total_heads_str, tmp);
  snprintf(tmp, sizeof(tmp), "%d", total_tails);
  strcpy(total_tails_str, "Tails:\n");
  strcat(total_tails_str, tmp);
  snprintf(tmp, sizeof(tmp), "%d", total_edge);
  strcpy(total_edge_str, "Edge:\n");
  strcat(total_edge_str, tmp);
  
  // Create base Chara layer
  s_chara_layer = layer_create(GRect(0,0,144,168));
  layer_set_update_proc(s_chara_layer, draw_it);
  // Add individual face text layers
  s_total_text_layer = text_layer_create(GRect(0, 21, 144, 40)); // Total
  text_layer_set_text_alignment(s_total_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_total_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text(s_total_text_layer, total_flips_str);
  layer_add_child(s_chara_layer, text_layer_get_layer(s_total_text_layer));
  s_heads_text_layer = text_layer_create(GRect(0, 60, 48, 40)); // Heads
  text_layer_set_text_alignment(s_heads_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_heads_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text(s_heads_text_layer, total_heads_str);
  layer_add_child(s_chara_layer, text_layer_get_layer(s_heads_text_layer));
  s_tails_text_layer = text_layer_create(GRect(48, 60, 48, 40)); // Tails
  text_layer_set_text_alignment(s_tails_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_tails_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text(s_tails_text_layer, total_tails_str);
  layer_add_child(s_chara_layer, text_layer_get_layer(s_tails_text_layer));
  s_edge_text_layer = text_layer_create(GRect(96, 60, 48, 40)); // Edge
  text_layer_set_text_alignment(s_edge_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_edge_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text(s_edge_text_layer, total_edge_str);
  layer_add_child(s_chara_layer, text_layer_get_layer(s_edge_text_layer));
  // Add confirmation text
  s_confirm_text_layer = text_layer_create(GRect(12,136,120,32));
  text_layer_set_text_alignment(s_confirm_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_confirm_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text(s_confirm_text_layer, "Press select to ERASE all current flip data");
  layer_add_child(s_chara_layer, text_layer_get_layer(s_confirm_text_layer));
  // Add title text last for layering reasons
  s_title_text_layer = text_layer_create(GRect(0, -7, 144, 26));
  text_layer_set_background_color(s_title_text_layer, GColorBlack);
  text_layer_set_text_color(s_title_text_layer, GColorWhite);
  text_layer_set_text_alignment(s_title_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_title_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(s_title_text_layer, "Flip Statistics");
  layer_add_child(s_chara_layer, text_layer_get_layer(s_title_text_layer));
  // Add Chara layer to base layer
  layer_add_child(window_get_root_layer(window), s_chara_layer);
}
static void stats_window_unload(Window* window) {
  text_layer_destroy(s_title_text_layer);
  text_layer_destroy(s_total_text_layer);
  text_layer_destroy(s_heads_text_layer);
  text_layer_destroy(s_tails_text_layer);
  text_layer_destroy(s_edge_text_layer);
  text_layer_destroy(s_confirm_text_layer);
  layer_destroy(s_chara_layer);
  window_destroy(s_stats_window);
}
void show_stats(void) {
  s_stats_window = window_create();
  window_set_click_config_provider(s_stats_window, click_config_provider);
  window_set_window_handlers(s_stats_window, (WindowHandlers) {
    .load = stats_window_load,
    .unload = stats_window_unload,
  });
  window_stack_push(s_stats_window, true);
}
void hide_stats(void) {
  window_stack_remove(s_stats_window, true);
}