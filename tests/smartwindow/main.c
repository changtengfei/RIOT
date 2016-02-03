/*
 * Copyright (C) 2016 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       SmartWindow test application
 *
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>

#include "xtimer.h"
#include "periph/pwm.h"
#include "servo.h"

#define DEV         PWM_0
#define CHANNEL     0

#define STEP_LOWER_BOUND (1800U)
#define STEP_UPPER_BOUND (2700U)

static servo_t servo;


bool openWindow(servo_t *servo){
    if (servo_set(servo, STEP_UPPER_BOUND)) {
        return false;
    }
    return true;
}

bool closeWindow(servo_t *servo) {
    if (servo_set(servo,STEP_LOWER_BOUND)) {
        return false;
    }
    return true;
}

int main(void)
{
    puts("Hello World!");

    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);


    if (servo_init(&servo, DEV, CHANNEL, STEP_LOWER_BOUND, STEP_UPPER_BOUND) < 0) {
        puts("Couldn't initialize servo");
    }

    while(1) {
        puts("Open");
        if(!openWindow(&servo)){
             puts("error!\n");
         }

        xtimer_sleep(2);
        
        puts("Close");
        if(!closeWindow(&servo)){
             puts("error!\n");
        }

        xtimer_sleep(2);
    }

    return 0;
}
