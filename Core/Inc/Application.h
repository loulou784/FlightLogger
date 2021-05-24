/*
 * Application.h
 *
 *  Created on: May 23, 2021
 *      Author: alexmorin
 */

#ifndef INC_APPLICATION_H_
#define INC_APPLICATION_H_

#include "main.h"
#include "fatfs.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include <stdint.h>
#include <stdbool.h>
#include "sd_hal_mpu6050.h"
#include "string.h"


#define MAX_LINE_PER_FILE 100000


void ApplicationInit();
void ApplicationTask();

#endif /* INC_APPLICATION_H_ */
