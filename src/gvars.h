#pragma once

// Persistent data variables
#define LAST_FLIP_RESULT 0 // int
#define FLAG_USE_ANIMATIONS 1 // bool
#define FLAG_USE_HISTORY 2 // bool
#define FLAG_USE_VIBRATIONS 3 // bool
#define FLAG_USE_ACCELEROMETER 4 // bool
#define FLAG_USE_EDGE 5 // bool
#define LAST_HISTORY 6 // char[]
#define STAT_HEADS 7 // int
#define STAT_TAILS 8 // int
#define STAT_EDGE 9 // int

// Allow settings window to access these variables
extern int result; // 0-H, 1-T, 2-E
extern char history[19];
extern bool use_animations, use_history, use_vibrations, use_accelerometer, use_edge;
extern int total_heads, total_tails, total_edge;