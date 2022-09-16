#include <Arduino.h>
#include <motor-handler.h>

// Dictates the default direction of the motor
bool default_clockwise = true;
// Direction actually depends on the motor coils but for simplicity
// we use clockwise/anti-clockwise wording
int direction = default_clockwise ? HIGH : LOW;

// This values need to be set when the board is plugged in
int motor_position = 0;
int motor_limit = 24000;

int motor_start_speed = 789;
int motor_end_speed = 489;
int motor_speed = motor_start_speed;
// Value updated by the homekit accessory to execute motor stepping
// asynchronusly
long global_steps = 0;

// Executable steps per loop cycle, this is to keep the loop alive and avoid the ESP8266 watchdog to timeout and restart the board
int steps_by_loop = 1500;

// D5
const int enablePin = 14;
// D6
const int stepPin = 12;
// D7
const int dirPin = 13;

/*
Initializes motor pins, states and a quick full rotation to test
*/
void init_motor() {
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
  digitalWrite(enablePin, LOW);
  bool reachedLimit = false;
  for (long x = 0; x < steps; x++) {
    if (check_stop()) {
      reachedLimit = true;
      break;
    } else {
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

int stepAccCount = 0;
bool accelerate = true;
bool decelerate = false;
void adjustAcceleration() {
  if (accelerate && stepAccCount == 2) {
    if (motor_speed > motor_end_speed) {
      motor_speed = motor_speed - 1;
    } else if (motor_speed == motor_end_speed && global_steps <= 600) {
      accelerate = false;
      decelerate = true;
    }
    stepAccCount = 0;
  }

  if (decelerate && stepAccCount == 2) {
    if (motor_speed < motor_start_speed) {
      motor_speed = motor_speed + 1;
    }
    stepAccCount = 0;
  }

  stepAccCount = stepAccCount + 1;
  if (global_steps == 0) {
    accelerate = true;
    decelerate = false;
    motor_speed = motor_start_speed;
    stepAccCount = 0;
  }
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

  // If async motor steping was called we update the global steps value
  if (global_steps > 0) {
    global_steps--;
  }

  adjustAcceleration();
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
  motor_speed = 550;
  while (motor_speed > 300) {
    Serial.print("Speed: ");
    Serial.println(motor_speed);
    // printf("Speed: " + String(motor_speed));
    motor_speed = motor_speed - 5;
    step_times(3600);
    set_direction(false);
    // delay(2000);
    step_times(3600);
    set_direction(true);
    // delay(2000);
  }
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

/**
 * ASYNC (Keeps the ESP8266 loop alive)
 * */
void move_to_position_async(int percentage) {
  long steps = parse_percent_to_steps(percentage);
  if (steps > 0) {
    global_steps = steps;
  }
}

// long get_global_steps()
// {
//   return global_steps;
// }

// void set_global_steps(long val)
// {
//   global_steps = val;
// }

// We step the motor inside the loop to avoid cutting off the rest of the
// necessary processes in the loop
void motor_handler_loop() {
  if (global_steps > 0) {
    if (global_steps > steps_by_loop) {
      step_times(steps_by_loop);
    } else {
      step_times(global_steps);
    }
  }
}