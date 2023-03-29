#pragma once
#include <Esp.h>

#define ENCODER_DEBOUNCE 200 // tick interval at max speed ~ 400

#define ENCODER_FILTER_VALUE 0.9 //was at 0.9

#define ENCODER( NAME, APIN, BPIN ) \
long encoder_ ##NAME## _tick_counter = 0;\
long encoder_ ##NAME## _last_tick_counter = 0;\
long encoder_ ##NAME## _ARise_tick_counter = 0; \
long encoder_ ##NAME## _AFall_tick_counter = 0; \
long encoder_ ##NAME## _BRise_tick_counter = 0; \
long encoder_ ##NAME## _BFall_tick_counter = 0; \
long encoder_ ##NAME## _ARise_last_tick_counter = 0; \
long encoder_ ##NAME## _AFall_last_tick_counter = 0; \
long encoder_ ##NAME## _BRise_last_tick_counter = 0; \
long encoder_ ##NAME## _BFall_last_tick_counter = 0; \
bool encoder_ ##NAME## _ready = false;\
unsigned long encoder_ ##NAME## _tick_interval = 0;\
unsigned long encoder_ ##NAME## _last_BRise_tick = 0; \
unsigned long encoder_ ##NAME## _last_ARise_tick = 0; \
unsigned long encoder_ ##NAME## _last_BFall_tick = 0; \
unsigned long encoder_ ##NAME## _last_AFall_tick = 0; \
unsigned long encoder_ ##NAME## _last_speed_read =0; \
int encoder_ ##NAME## _A_pin = APIN; \
int encoder_ ##NAME## _B_pin = BPIN; \
float encoder_ ##NAME## _filtered_tick_interval = 0;\
float encoder_ ##NAME## _speed = 0;\
float encoder_ ##NAME## _filtered_speed = 0;\
void IRAM_ATTR encoder_AChange_ ##NAME## _isr() { \
  auto m = micros(); \
  encoder_ ##NAME## _ready = true; \
  if (digitalRead(encoder_ ##NAME## _A_pin) == HIGH) {\
    if (m < encoder_ ##NAME## _last_ARise_tick + ENCODER_DEBOUNCE) { return; } \
    encoder_ ##NAME## _last_ARise_tick = m; \
    if (digitalRead(encoder_ ##NAME## _B_pin) == LOW){ \
    encoder_ ##NAME## _ARise_tick_counter ++; \
    } else {  \
    encoder_ ##NAME## _ARise_tick_counter --; }\
  } else { \
    if (m < encoder_ ##NAME## _last_AFall_tick + ENCODER_DEBOUNCE) { return; } \
    encoder_ ##NAME## _last_AFall_tick = m; \
    if (digitalRead(encoder_ ##NAME## _B_pin) == HIGH) {\
    encoder_ ##NAME## _AFall_tick_counter ++; \
    } else { \
    encoder_ ##NAME## _AFall_tick_counter --; }\
  }\
} \
void IRAM_ATTR encoder_BChange_ ##NAME## _isr() { \
  auto m = micros(); \
  encoder_ ##NAME## _ready = true; \
  if (digitalRead(encoder_ ##NAME## _B_pin) == HIGH) {\
    if (m < encoder_ ##NAME## _last_BRise_tick + ENCODER_DEBOUNCE) { return; } \
    encoder_ ##NAME## _last_BRise_tick = m; \
    if (digitalRead(encoder_ ##NAME## _A_pin) == HIGH) {\
    encoder_ ##NAME## _BRise_tick_counter ++; \
    } else {\
    encoder_ ##NAME## _BRise_tick_counter --; }\
  } else {\
    if (m < encoder_ ##NAME## _last_BFall_tick + ENCODER_DEBOUNCE) { return; } \
    encoder_ ##NAME## _last_BFall_tick = m; \
    if (digitalRead(encoder_ ##NAME## _A_pin) == LOW) {\
    encoder_ ##NAME## _BFall_tick_counter ++; \
    } else { \
    encoder_ ##NAME## _BFall_tick_counter --; } \
  }\
}

#define ENCODER_INIT( NAME ) \
  pinMode(encoder_ ##NAME## _A_pin, INPUT_PULLUP); \
  pinMode(encoder_ ##NAME## _B_pin, INPUT_PULLUP); \
  attachInterrupt(encoder_ ##NAME## _A_pin, encoder_AChange_ ##NAME## _isr, CHANGE); \
  attachInterrupt(encoder_ ##NAME## _B_pin, encoder_BChange_ ##NAME## _isr, CHANGE); \
  
#define ENCODER_READY ( NAME ) encoder_ ##NAME## _ready

#define ENCODER_FILTER( NAME ) \
{\
  auto encoder_ ##NAME## _m = micros(); \
  auto encoder_ ##NAME## _t = encoder_ ##NAME## _ARise_tick_counter +\
                              encoder_ ##NAME## _AFall_tick_counter +\
                              encoder_ ##NAME## _BRise_tick_counter +\
                              encoder_ ##NAME## _BFall_tick_counter; \
  auto encoder_ ##NAME## _speed_read_interval = encoder_ ##NAME## _m - encoder_ ##NAME## _last_speed_read;\
  auto encoder_ ##NAME## _tick_difference = encoder_ ##NAME## _t - encoder_ ##NAME## _last_tick_counter;\
  encoder_ ##NAME## _last_tick_counter = encoder_ ##NAME## _t;\
  encoder_ ##NAME## _speed = (float) encoder_ ##NAME## _tick_difference / (float)encoder_ ##NAME## _speed_read_interval * 1000000;\
  encoder_ ##NAME## _filtered_speed = (ENCODER_FILTER_VALUE * encoder_ ##NAME## _filtered_speed) + ((1-ENCODER_FILTER_VALUE) * encoder_ ##NAME## _speed);\
  encoder_ ##NAME## _last_speed_read = encoder_ ##NAME## _m;\
}

#define get_encoder_speed( NAME ) (float) encoder_ ##NAME## _speed                    // unfiltered
#define get_encoder_filtered_speed( NAME ) (float) encoder_ ##NAME## _filtered_speed  // filtered
#define get_encoder_tick_counter( NAME )\
encoder_ ##NAME## _ARise_tick_counter +\
encoder_ ##NAME## _AFall_tick_counter +\
encoder_ ##NAME## _BRise_tick_counter +\
encoder_ ##NAME## _BFall_tick_counter
#define get_encoder_ARise_tick_counter( NAME ) encoder_ ##NAME## _ARise_tick_counter
#define get_encoder_BRise_tick_counter( NAME ) encoder_ ##NAME## _BRise_tick_counter
#define get_encoder_AFall_tick_counter( NAME ) encoder_ ##NAME## _AFall_tick_counter
#define get_encoder_BFall_tick_counter( NAME ) encoder_ ##NAME## _BFall_tick_counter
