/*
 * File: car.cpp
 * Authors: Stijn Boons & Johan Reinhed
 * Date: 2019-11-06
 * Source code for the car.
 */

#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include <stream_buffer.h>

// Motor pins
int left_forward = D8;
int left_back = D10;
int right_forward = D5;
int right_back = D6;

// Sensor pins
int left_line_follow = D12;
int right_line_follow = D11;

// LED pins
int left_led = D3;
int right_led = D2;

StreamBufferHandle_t stream_buffer_motor, stream_buffer_LED;

void task_drive(void *);
void task_follow_line(void *);
void task_LEDs(void *);

enum Direction {
  STRAIGHT,
  LEFT,
  RIGHT
};

void setup() {
  Serial.begin(9600);
  xTaskCreate(task_drive, "drive", 64, NULL, 1, NULL);
  xTaskCreate(task_follow_line, "follow line", 64, NULL, 1, NULL);
  xTaskCreate(task_LEDs, "LED control", 16, NULL, 1, NULL);

  stream_buffer_motor = xStreamBufferCreate(100, 5);
  stream_buffer_LED = xStreamBufferCreate(100, 5);

  vTaskStartScheduler();
}

void loop() {
  // Unused
}

// A task for controlling the motors.
void task_drive(void *args) {
  pinMode(left_back, OUTPUT);
  pinMode(right_back, OUTPUT);
  pinMode(left_forward, OUTPUT);
  pinMode(right_forward, OUTPUT);

  char direction;

  for (;;) {
    xStreamBufferReceive(stream_buffer_motor, (char *)&direction, 1, 5);
    if (direction == STRAIGHT) { // Drive straight
      analogWrite(right_forward, 127);
      analogWrite(left_forward, 127);
      analogWrite(right_back, 0);
      analogWrite(left_back, 0);
    } else if (direction == LEFT) { // Drive left
      analogWrite(right_forward, 127);
      analogWrite(left_forward, 0);
      analogWrite(right_back, 0);
      analogWrite(left_back, 64);
    } else if (direction == RIGHT) { // Drive right
      analogWrite(right_forward, 0);
      analogWrite(left_forward, 127);
      analogWrite(right_back, 64);
      analogWrite(left_back, 0);
    }
  }
}

// A task for reading sensor values.
// Sensor value of 0 = dark surface, 1 = light surface.
void task_follow_line(void *args) {
  pinMode(left_line_follow, INPUT);
  pinMode(right_line_follow, INPUT);

  int left_val, right_val;
  char direction;

  for (;;) {
    // if left == 0 and right == 1 send left
    // if left == 1 and right == 0 send right
    // else send straight
    left_val = digitalRead(left_line_follow);
    right_val = digitalRead(right_line_follow);

    if (left_val == 0 && right_val == 1) { // Line goes left
      direction = LEFT;
    } else if (left_val == 1 && right_val == 0) { // Line goes right
      direction = RIGHT;
    } else if (left_val == 0 && right_val == 0) { // Line goes both ways
      // Nothing implemented yet
    } else { // Line goes straight
      direction = STRAIGHT;
    }

    // Send data to pipes for use in other tasks
    xStreamBufferSend(stream_buffer_motor, (char *) &direction, 1, 5);
    xStreamBufferSend(stream_buffer_LED, (char *) &direction, 1, 5);


  }
}

// A task for LED control.
void task_LEDs(void *args) {
  pinMode(left_led, OUTPUT);
  pinMode(right_led, OUTPUT);

  char direction;

  for (;;) {
    xStreamBufferReceive(stream_buffer_LED, (char *) &direction, 1, 5);

    if (direction == RIGHT) { // Right
     digitalWrite(right_led, HIGH);
     digitalWrite(left_led, LOW);
    } else if (direction == LEFT) { // Left
     digitalWrite(right_led, LOW);
     digitalWrite(left_led, HIGH);
    } else { // Straight
     digitalWrite(right_led, LOW);
     digitalWrite(left_led, LOW);
    }
  }
}
