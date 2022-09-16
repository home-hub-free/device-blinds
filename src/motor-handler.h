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
bool check_emergency_stop();
void test_motor();
void move_to_position_sync(int);
void move_to_position_async(int);
long get_global_steps();
void set_global_steps(long);
void motor_handler_loop();
void speed_sound_test();
#ifdef __cplusplus
}
#endif

#endif