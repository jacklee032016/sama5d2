#pragma once
  
#ifndef _MULTITHREADH_H
#define _MULTITHREADH_H

#include <pthread.h>

int test_flag;

pthread_mutex_t i2c_bus_lock;
pthread_mutex_t cec_code_lock;

pthread_t thread_tx_server;
int thread_tx_server_ret;    //communication_routine

void *tx_server_routine();

pthread_t thread_ADV;
int thread_ADV_ret;    //communication_routine
void *ADV_routine();

pthread_t thread_link_10G;
int thread_link_10G_ret;
void *link_10G_routine();

pthread_t thread_rs232;
int thread_rs232_ret;

pthread_t thread_ir;
int thread_ir_ret;

pthread_t thread_watchdog;
int thread_watchdog_ret;
void *watchdog_routine();

pthread_t thread_power_led;
int thread_power_led_ret;
void *power_led();

pthread_t thread_muxbutton;
int thread_muxbutton_ret;
void *muxbutton_routine;


unsigned int crc32b_server(unsigned char *message);


#endif
