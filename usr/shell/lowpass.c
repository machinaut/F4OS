/*
 * Copyright (C) 2013 F4OS Authors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <arch/chip/registers.h>
#include <arch/chip/dev/periph/discovery_accel.h>
#include <kernel/sched.h>
#include <math.h>
#include "app.h"

/* INSANE defs to make it obvious where the magic numbers come from */
#define DT_TO_JIFFIES   10000
#define DELTA_T         .01f
#define CUTOFF_FREQ     25
#define TAU             1/CUTOFF_FREQ
#define ACC_LP_GAIN     DELTA_T/(TAU+DELTA_T)
#define G(x)            x*.018f

typedef struct accel_data {
    int8_t x;
    int8_t y;
    int8_t z;
} accel_data;

void ghetto_lp(void);

accel_data asdf = {0,0,0};
accel_data *data = &asdf;
rd_t accelrd = 255;
float theta = 0;

void lowpass_test(int argc, char **argv) {
    if (argc != 1) {
        printf("Usage: %s\r\n", argv[0]);
        return;
    }
    printf("This program will continuously print a mathematically correct filtered roll angle.\r\n\
            It is ghetto and CANNOT BE STOPPED WHEN RUN.\r\n\
            q to quit now, any other key will continue.\r\n");

    if(getc() != 'q') {
        printf("LOL TOO LATE NOW FOOL. HOPE YOU GOT YOUR RESET BUTTON HANDY. SCRUB.\r\n");

        accelrd = open_discovery_accel();
        if (accelrd < 0) {
            printf("Error: unable to open accelerometer.\r\n");
            return;
        }

        new_task(&ghetto_lp, 8, DELTA_T*DT_TO_JIFFIES);
    }
}
DEFINE_APP(lowpass_test)

void ghetto_lp(void) {
    if (read(accelrd, (char *)data, 3) == 3) {
        theta = lowpass(theta, atan2(G((float)data->z), G((float)data->y)), ACC_LP_GAIN);
        printf("Filtered roll: %f\r\n", theta*180/3.1415926f);
    }
    else {
        printf("Unable to read accelerometer.\r\n");
    }
}
