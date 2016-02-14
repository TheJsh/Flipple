#include <pebble.h>
#include "gvars.h"

int result; // 0-H, 1-T, 2-E
char history[19];
bool use_animations, use_history, use_vibrations, use_accelerometer, use_edge;
int total_heads, total_tails, total_edge;