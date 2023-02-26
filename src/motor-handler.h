#ifndef _MOTOR_HANDELR_H
#define _MOTOR_HANDLER_H

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

void init_motor();
bool step_times(long);
void single_step();
void set_direction(bool);
bool check_stop();
void test_motor();
void move_to_position_sync(int);
void switch_direction();
void home_position();
void set_limit();
void motor_loop_handler();

#ifdef __cplusplus
}
#endif

#endif