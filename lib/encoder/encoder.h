#pragma once

//Function declaration
void setup_encoders();
float calculate_speed(int encoder_count, float delta_time);
extern volatile int left_encoder_count;
extern volatile int right_encoder_count;
