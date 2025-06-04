#include <micro_ros_platformio.h>
#include "microros.h"
#include "motor_control.h"
#include "config.h"

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_subscription_t subscriber;
rcl_publisher_t left_speed_pub;
rcl_publisher_t right_speed_pub;
geometry_msgs__msg__Twist twist_msg;
std_msgs__msg__Float32 left_speed_msg;
std_msgs__msg__Float32 right_speed_msg;

microROS_connection_state_t connection_state = MICROROS_DISCONNECTED;
unsigned long last_connection_attempt = 0;
unsigned long last_healthy_communication = 0;
const unsigned long RECONNECTION_DELAY = 1000;
const unsigned long COMMUNICATION_TIMEOUT_MS = 1000;
unsigned long last_cmd_vel_time = 0;

void cmd_vel_callback(const void *msg_in)
{
    const auto *msg = (const geometry_msgs__msg__Twist *)msg_in;
    last_cmd_vel_time = millis();
    last_healthy_communication = millis();

    float linear_vel = msg->linear.x;
    float angular_vel = msg->angular.z;

    left_target_speed = (linear_vel + angular_vel * WHEEL_SEPARATION / 2) / WHEEL_RADIUS;
    right_target_speed = (linear_vel - angular_vel * WHEEL_SEPARATION / 2) / WHEEL_RADIUS;
}

void cleanup_microros_resources()
{
    rcl_ret_t ret;
    
    ret = rclc_executor_fini(&executor);
    ret = rcl_subscription_fini(&subscriber, &node);
    ret = rcl_publisher_fini(&left_speed_pub, &node);
    ret = rcl_publisher_fini(&right_speed_pub, &node);
    ret = rcl_node_fini(&node);
    ret = rclc_support_fini(&support);
    
    (void)ret; // Explicitly ignore return values to suppress warnings
}

void setup_microros()
{
    Serial.println("Initializing micro-ROS...");
    set_microros_serial_transports(Serial);

    allocator = rcl_get_default_allocator();
    
    // Clean up any existing resources
    if (connection_state != MICROROS_DISCONNECTED) {
        cleanup_microros_resources();
    }

    // Initialize topics
    if (rclc_support_init(&support, 0, NULL, &allocator) == RCL_RET_OK &&
        rclc_node_init_default(&node, "esp32_motor_controller", "", &support) == RCL_RET_OK &&
        rclc_subscription_init_default(&subscriber, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist), "/cmd_vel") == RCL_RET_OK &&
        rclc_publisher_init_default(&left_speed_pub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32), "/left_wheel_speed") == RCL_RET_OK &&
        rclc_publisher_init_default(&right_speed_pub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32), "/right_wheel_speed") == RCL_RET_OK &&
        rclc_executor_init(&executor, &support.context, 1, &allocator) == RCL_RET_OK &&
        rclc_executor_add_subscription(&executor, &subscriber, &twist_msg, &cmd_vel_callback, ON_NEW_DATA) == RCL_RET_OK) {
        
        connection_state = MICROROS_CONNECTED;
        last_healthy_communication = millis();
        last_cmd_vel_time = millis();
        Serial.println("micro-ROS connected successfully!");
    } else {
        connection_state = MICROROS_DISCONNECTED;
        Serial.println("micro-ROS initialization failed");
    }
}

void microros_loop()
{
    static unsigned long last_reconnection_attempt = 0;
    rcl_ret_t ret;
    
    switch (connection_state) {
        case MICROROS_DISCONNECTED:
            if (millis() - last_reconnection_attempt > RECONNECTION_DELAY) {
                Serial.println("Attempting to connect to micro-ROS agent...");
                setup_microros();
                last_reconnection_attempt = millis();
            }
            break;
            
        case MICROROS_CONNECTED:
            // Check for communication health
            if (millis() - last_healthy_communication > COMMUNICATION_TIMEOUT_MS) {
                connection_state = MICROROS_CONNECTION_LOST;
                Serial.println("micro-ROS communication timeout detected");
                break;
            }
            
            // Normal operation
            ret = rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
            if (ret == RCL_RET_OK) {
                last_healthy_communication = millis();
            } else if (ret != RCL_RET_TIMEOUT) {
                connection_state = MICROROS_CONNECTION_LOST;
                Serial.println("micro-ROS communication error detected");
            }
            break;
            
        case MICROROS_CONNECTION_LOST:
            Serial.println("Cleaning up micro-ROS resources...");
            cleanup_microros_resources();
            
            connection_state = MICROROS_DISCONNECTED;
            last_reconnection_attempt = millis();
            break;
    }
}

void publish_wheel_speeds(float left_speed, float right_speed)
{
    left_speed_msg.data = left_speed;
    right_speed_msg.data = right_speed;
    
    rcl_ret_t ret_left = rcl_publish(&left_speed_pub, &left_speed_msg, NULL);
    rcl_ret_t ret_right = rcl_publish(&right_speed_pub, &right_speed_msg, NULL);
    
    (void)ret_left;  // Explicitly ignore return values
    (void)ret_right;
}

microROS_connection_state_t get_microROS_connection_state()
{
    return connection_state;
}

void reset_cmd_vel_timeout()
{
    last_cmd_vel_time = millis();
}

bool cmd_vel_timeout_exceeded()
{
    return millis() - last_cmd_vel_time > CMD_VEL_TIMEOUT_MS;
}