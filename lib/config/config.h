#pragma once

// Motor Driver Pins (TB6612FNG)

// Right motor
#define PWMA 5
#define AIN1 8
#define AIN2 7
// Left motor
#define PWMB 6
#define BIN1 9
#define BIN2 10

// Motor PWM Range
#define MAX_PWM 100
#define MIN_PWM -100

// Encoder Pins

// Left encoder
#define ENCODER0_PINA 2
#define ENCODER0_PINB 3
// Right encoder
#define ENCODER1_PINA 1
#define ENCODER1_PINB 0

// Encoder Count Per Revolution
#define ENCODER_CPR 1320  // 30 (gear ratio) x 11 (magnetic loops/encoder lines) x 4 (quadrature edges)

// PID Control Parameters
#define PID_KP 10.0f
#define PID_KI 0.1f
#define PID_KD 0.05f
#define PID_MAX_OUTPUT MAX_PWM
#define PID_MIN_OUTPUT MIN_PWM

extern float left_target_speed;
extern float right_target_speed;

// Robot Dimensions
#define WHEEL_RADIUS 0.034  // in meters
#define WHEEL_SEPARATION 0.194  // in meters

// Control Timing
#define CMD_VEL_TIMEOUT_MS 500  // ms before stopping motors if no cmd_vel received

