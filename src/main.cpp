#include <Arduino.h>
#include "motor_control.h"
#include "encoder.h"
#include "config.h"
#include "microros.h"
#include "pid_controller.h"

// Create PID controllers for each motor
PIDController left_pid(PID_KP, PID_KI, PID_KD, PID_MAX_OUTPUT, PID_MIN_OUTPUT);
PIDController right_pid(PID_KP, PID_KI, PID_KD, PID_MAX_OUTPUT, PID_MIN_OUTPUT);

// Variables for target speeds
float left_target_speed = 0;
float right_target_speed = 0;

void setup() {
  Serial.begin(115200);
  setup_motors();
  setup_encoders();
  setup_microros();
}

void loop() {
  static unsigned long last_time = 0;
  unsigned long current_time = millis();

  microros_loop();

  float delta_time = (current_time - last_time) / 1000.0f;
  if (delta_time >= 0.1 && get_microROS_connection_state() == MICROROS_CONNECTED) {
    // Calculate actual speeds
    float left_speed = calculate_speed(left_encoder_count, delta_time);
    float right_speed = calculate_speed(right_encoder_count, delta_time);

    // Reset encoder counts
    left_encoder_count = 0;
    right_encoder_count = 0;
    last_time = current_time;

    // Compute PID outputs
    int left_pwm = left_pid.compute(left_target_speed, left_speed);
    int right_pwm = right_pid.compute(right_target_speed, right_speed);
    
    // Apply motor commands
    set_motor_speed(PWMA, AIN1, AIN2, left_pwm);
    set_motor_speed(PWMB, BIN1, BIN2, right_pwm);

    publish_wheel_speeds(left_speed, right_speed);
    // Serial.printf("Left: %.2f rad/s, Right: %.2f rad/s\n", left_speed, right_speed);
    Serial.printf("Left: %.2f (target %.2f) rad/s, Right: %.2f (target %.2f) rad/s\n", left_speed, left_target_speed, right_speed, right_target_speed);
  }

  if (cmd_vel_timeout_exceeded() || get_microROS_connection_state() != MICROROS_CONNECTED) {
    left_target_speed = 0;
    right_target_speed = 0;
    left_pid.reset();
    right_pid.reset();
    set_motor_speed(PWMA, AIN1, AIN2, 0);
    set_motor_speed(PWMB, BIN1, BIN2, 0);
    left_encoder_count = 0;
    right_encoder_count = 0;
  }

  delay(10);
}
