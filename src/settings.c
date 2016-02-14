/*
 * settings.c
 * Holds code for the settings window
 */

#include <pebble.h>
#include "settings.h"
#include "gvars.h"
#include "main.h"
#include "stats.h"

static Window *s_settings_window;
static SimpleMenuLayer *s_simple_menu_layer;
static SimpleMenuSection s_menu_sections[1];
static SimpleMenuItem s_menu_items[6];
static char strbuffer[12] = "";
static char stats_subtitle[24] = "";

static void menu_select_callback(int index, void *ctx) {
  switch(index) {
    case 0: // Toggle history
      use_history = !use_history;
      persist_write_bool(FLAG_USE_HISTORY, use_history);
      update_history_frame();
      s_menu_items[0].subtitle = (use_history ? "Enabled" : "Disabled");
      break;
    case 1: // Toggle vibrations
      use_vibrations = !use_vibrations;
      persist_write_bool(FLAG_USE_VIBRATIONS, use_vibrations);
      s_menu_items[1].subtitle = (use_vibrations ? "Enabled" : "Disabled");
      break;
    case 2: // Toggle animations
      use_animations = !use_animations;
      persist_write_bool(FLAG_USE_ANIMATIONS, use_animations);
      s_menu_items[2].subtitle = (use_animations ? "Enabled" : "Disabled");
      break;
    case 3: // Toggle accelerometer
      use_accelerometer = !use_accelerometer;
      persist_write_bool(FLAG_USE_ACCELEROMETER, use_accelerometer);
      s_menu_items[3].subtitle = (use_accelerometer ? "Enabled" : "Disabled");
      break;
    case 4: // Toggle edge flipping
      use_edge = !use_edge;
      persist_write_bool(FLAG_USE_EDGE, use_edge);
      s_menu_items[4].subtitle = (use_edge ? "Enabled" : "Disabled");
      break;
    case 5: // View stats
      show_stats();
      break;
    default:
      break;
  }
  layer_mark_dirty(simple_menu_layer_get_layer(s_simple_menu_layer));
}

static void settings_window_load(Window* window) {
  // use_animations, use_history, use_vibrations, use_edge;
  
  int total_flips = total_heads + total_tails + total_edge;
  snprintf(strbuffer, sizeof(strbuffer), "%d", total_flips);
  strcpy(stats_subtitle, "Total flips: ");
  strcat(stats_subtitle, strbuffer);
  
  s_menu_items[0] = (SimpleMenuItem) {
    .title = "History",
    .subtitle = (use_history ? "Enabled" : "Disabled"),
    .callback = menu_select_callback,
  };
  s_menu_items[1] = (SimpleMenuItem) {
    .title = "Vibrations",
    .subtitle = (use_vibrations ? "Enabled" : "Disabled"),
    .callback = menu_select_callback,
  };
  s_menu_items[2] = (SimpleMenuItem) {
    .title = "Animations",
    .subtitle = (use_animations ? "Enabled" : "Disabled"),
    .callback = menu_select_callback,
  };
  s_menu_items[3] = (SimpleMenuItem) {
    .title = "Use accelerometer",
    .subtitle = (use_accelerometer ? "Enabled" : "Disabled"),
    .callback = menu_select_callback,
  };
  s_menu_items[4] = (SimpleMenuItem) {
    .title = "Can land on edge",
    .subtitle = (use_edge ? "Enabled" : "Disabled"),
    .callback = menu_select_callback,
  };
  s_menu_items[5] = (SimpleMenuItem) {
    .title = "View stats",
    .subtitle = stats_subtitle,
    .callback = menu_select_callback,
  };

  s_menu_sections[0] = (SimpleMenuSection) {
    .num_items = 6,
    .items = s_menu_items,
  };
  
  s_simple_menu_layer = simple_menu_layer_create(GRect(0,0,144,168), window, s_menu_sections, 1, NULL);
  layer_add_child(window_get_root_layer(window), simple_menu_layer_get_layer(s_simple_menu_layer));
}

static void settings_window_unload(Window* window) {
  simple_menu_layer_destroy(s_simple_menu_layer);
  window_destroy(s_settings_window);
}

void show_settings(void) {
  s_settings_window = window_create();
  window_set_window_handlers(s_settings_window, (WindowHandlers) {
    .load = settings_window_load,
    .unload = settings_window_unload,
  });
  window_stack_push(s_settings_window, true);
}

void hide_settings(void) {
  window_stack_remove(s_settings_window, true);
}