#include <Arduino.h>
#include "config.h"
#include "encoder.h"

// Define local variable
volatile int left_encoder_count = 0;
volatile int right_encoder_count = 0;

// Interrupt handlers for encoders
void IRAM_ATTR left_encoder_isr() {
  if (digitalRead(ENCODER0_PINB)) {
    left_encoder_count++;
  } else {
    left_encoder_count--;
  }
}

void IRAM_ATTR right_encoder_isr() {
  if (digitalRead(ENCODER1_PINB)) {
    right_encoder_count++;
  } else {
    right_encoder_count--;
  }
}

// Setup encoders
void setup_encoders() {
  pinMode(ENCODER0_PINA, INPUT_PULLUP);
  pinMode(ENCODER0_PINB, INPUT_PULLUP);
  pinMode(ENCODER1_PINA, INPUT_PULLUP);
  pinMode(ENCODER1_PINB, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENCODER0_PINA), left_encoder_isr, RISING);
  attachInterrupt(digitalPinToInterrupt(ENCODER1_PINA), right_encoder_isr, RISING);
}

// Calculate speed in rad/s from encoder counts
float calculate_speed(int encoder_count, float delta_time) {
  float revolutions = encoder_count / (float)(ENCODER_CPR);
  float rad_per_sec = (revolutions * 2 * PI) / delta_time;
  return rad_per_sec;
}
