#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include <stream_buffer.h>

int left_forward = D8;
int left_back = D10;
int right_forward = D5;
int right_back = D6;
int left_line_follow = D12;
int right_line_follow = D11;
int left_led = D3;
int right_led = D2;

StreamBufferHandle_t streambuffer, streambuffer2;

void taskDrive(void *);
void taskFollowLine(void *);
void taskBlink(void *);
void blink(int);

enum Direction {
  STRAIGHT,
  LEFT,
  RIGHT
};

void setup() {
  Serial.begin(9600);
  xTaskCreate(taskDrive, "drive", 64, NULL, 1, NULL);
  xTaskCreate(taskFollowLine, "follow line", 64, NULL, 1, NULL);
  xTaskCreate(taskBlink, "blink", 16, NULL, 1, NULL);

  streambuffer = xStreamBufferCreate(100, 5);
  streambuffer2 = xStreamBufferCreate(100, 5);

  vTaskStartScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:
}

void taskDrive(void *pvParameters) {
  pinMode(left_back, OUTPUT);
  pinMode(right_back, OUTPUT);
  pinMode(left_forward, OUTPUT);
  pinMode(right_forward, OUTPUT);

  char direction;

  for (;;) {
    xStreamBufferReceive(streambuffer, (char *)&direction, 1, 5);
    if (direction == STRAIGHT) {
      analogWrite(right_forward, 127);
      analogWrite(left_forward, 127);
      analogWrite(right_back, 0);
      analogWrite(left_back, 0);
    } else if (direction == LEFT) {
      analogWrite(right_forward, 127);
      analogWrite(left_forward, 0);
      analogWrite(right_back, 0);
      analogWrite(left_back, 64);
    } else if (direction == RIGHT) {
      analogWrite(right_forward, 0);
      analogWrite(left_forward, 127);
      analogWrite(right_back, 64);
      analogWrite(left_back, 0);
    }
  }
}

void taskFollowLine(void *pvParameters) {
  pinMode(left_line_follow, INPUT);
  pinMode(right_line_follow, INPUT);

  int left_val, right_val;
  char direction;

  for (;;) {
    // if left == 0 and right == 1 send left
    // if left == 1 and right == 0 send right
    // else send straight
    // if left == 0 and right == 0 go haywire and attack everyone in the room
    left_val = digitalRead(left_line_follow);
    right_val = digitalRead(right_line_follow);
    //Serial.print("I am retarded "); Serial.print(left_val); Serial.print(" "); Serial.println(right_val);

    if (left_val == 0 && right_val == 1) { // Line goes left
      direction = LEFT;
    } else if (left_val == 1 && right_val == 0) { // Line goes right
      direction = RIGHT;
    } else if (left_val == 0 && right_val == 0) { // Line goes both ways. Does not compute
      //goHaywire();
    } else { // Line goes straight
      direction = STRAIGHT;
    }
    xStreamBufferSend(streambuffer, (char *) &direction, 1, 5);
    xStreamBufferSend(streambuffer2, (char *) &direction, 1, 5);


  }
}

void taskBlink(void *unused) {
  pinMode(left_led, OUTPUT);
  pinMode(right_led, OUTPUT);

  char direction;

  for (;;) {
    xStreamBufferReceive(streambuffer2, (char *) &direction, 1, 5);

    if (direction == RIGHT) {
     //blink(right_led);
     digitalWrite(right_led, HIGH);
     digitalWrite(left_led, LOW);
    } else if (direction == LEFT) {
     //blink(left_led);
     //digitalWrite(right_led, HIGH);
     digitalWrite(right_led, LOW);
     digitalWrite(left_led, HIGH);
    } else {
      digitalWrite(right_led, LOW);
      digitalWrite(left_led, LOW);
    }
    //digitalWrite(left_led, LOW);
    //digitalWrite(right_led, LOW);
  }
}

void blink(int pin) {
  digitalWrite(pin, HIGH);
  vTaskDelay(25);
  digitalWrite(pin, LOW);
  vTaskDelay(25);
}