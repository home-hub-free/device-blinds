#include <Arduino.h>
#include <motor-handler.h>

// Dictates the default direction of the motor
bool default_clockwise = true;
// Direction actually depends on the motor coils but for simplicity
// we use clockwise/anti-clockwise wording

/**
 * Make sure to swap HIGH and LOW depending on your coil
 * configuration if you notice that clockwise is not your
 * default spin OR swap the motor coil connections from your
 * motor driver
*/
int direction = default_clockwise ? HIGH : LOW;

// This values need to be set when the board is plugged in
int motor_position = 0;
int motor_limit = 24000;

int motor_start_speed = 490;
int motor_end_speed = 490;
int motor_speed = motor_start_speed;

// D5
const int enablePin = D5;
// D6
const int stepPin = D6;
// D7
const int dirPin = D7;

const int button_1 = D8;
const int button_2 = D0;

/*
Initializes motor pins, states and a quick full rotation to test
*/
void init_motor() {

  digitalWrite(button_1, LOW);
  digitalWrite(button_2, LOW);

  pinMode(button_1, INPUT);
  pinMode(button_2, INPUT);

  pinMode(enablePin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  // Initial state of motor is clockwise (HIGH state)
  digitalWrite(dirPin, direction);
  // Motor is disabled unless it needs to execute steps
  digitalWrite(enablePin, HIGH);

  // Initial motor movement test
  step_times(200);
  delay(1000);
  set_direction(!default_clockwise);
  delay(1000);
  step_times(200);
  set_direction(default_clockwise);
}

bool step_times(long steps) {
  bool reachedLimit = false;
  for (long x = 0; x < steps; x++) {
    if (check_stop()) {
      reachedLimit = true;
      break;
    } else {
      digitalWrite(enablePin, LOW);
      single_step();
    }
  }

  String dir = direction == 1 ? "Clockwise" : "Counter Clockwise";
  Serial.print("Stepped: ");
  Serial.print(steps);
  Serial.println(" times, ");

  Serial.print("Motor position: ");
  Serial.println(motor_position);

  digitalWrite(enablePin, HIGH);
  return reachedLimit;
}

void single_step() {
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(motor_speed);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(motor_speed);

  // This section updates global 'motor_position' depending on the 'default_clockwise' value
  int multiplier = default_clockwise ? 1 : -1;
  if (direction == HIGH) {
    motor_position = motor_position + (1 * multiplier);
  } else {
    motor_position = motor_position + (-1 * multiplier);
  }

  yield();
}

/**
 * Uses a boolean to spin on its default initial direction or the contrary
 * */
void set_direction(bool spin_default) {
  direction = true;
  if (default_clockwise) {
    direction = spin_default ? HIGH : LOW;
  } else {
    direction = spin_default ? LOW : HIGH;
  }

  digitalWrite(dirPin, direction);
}

/**
 * Checks against motor limit position to prevent for whatever reason accidentally steping
 *  over a user defined limit
 * */
bool check_stop() {
  bool stop = false;
  if (default_clockwise) {
    stop = (direction == HIGH && motor_position >= motor_limit) || (direction == LOW && motor_position <= 0);
  } else {
    stop = (motor_position == HIGH && motor_position <= 0) || (motor_position == LOW && motor_position >= motor_limit);
  }
  return stop;
}

/**
 * Infinite loop over stepping a full rotation just to test the motor
 * */
void test_motor() {
  step_times(3600);
  set_direction(false);
  delay(2000);
  step_times(3600);
  set_direction(true);
  delay(2000);
}

/**
 * Turns a given percentage into a number of steps that the motor needs execute to achieve that percent
 * */
long parse_percent_to_steps(int percentage) {
  float steps_per_percent = motor_limit / 100;
  float new_position = percentage * steps_per_percent;
  long steps = abs(motor_position - new_position);
  set_direction(!(motor_position > new_position));
  return steps;
}

/**
 * Moves motor to given percent based on current position and limit position
 * SYNC (Stops the ESP8266 loop)
 * */
void move_to_position_sync(int percentage) {
  long steps = parse_percent_to_steps(percentage);
  if (steps > 0) {
    step_times(steps);
  }
}

void switch_direction() {
  default_clockwise = !default_clockwise;
}

void home_position() {
  motor_position = 0;
}

void set_limit() {
  motor_limit = motor_position;
}

void motor_loop_handler() {
  int button_1_input = digitalRead(button_1);
  int button_2_input = digitalRead(button_2);

  if (button_1_input == LOW || button_2_input == LOW) {
    if (button_1_input == HIGH) {
      set_direction(true);
      step_times(200);
    }
    if (button_2_input == HIGH) {
      set_direction(false);
      step_times(200);
    }
  }
}