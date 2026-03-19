/*
 * temp_sensor.h: Starter code for ECE 266 Lab 6, spring 2024
 *
 * Temperature sensor module
 *
 * Created by Zhao Zhang, fall 2025
 * Last update: Feb. 18, 2024
 */

#ifndef TEMP_SENSOR_H_
#define TEMP_SENSOR_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>
#include <driverlib/sysctl.h>
#include <driverlib/interrupt.h>
#include <driverlib/adc.h>
#include "launchpad.h"

// Initialize ADC to use Tiva C's internal temperature sensor
void TsInit();

// Trigger temperature reading
void TsTriggerReading();

// Check if any new reading is ready (for I/O polling)
bool TsDataReady();

// Return temperature reading in Fahrenheit
float TsDataRead();

// Set callback event for temperature sensor ISR
void TsEventRegister(Event *event);

// The ISR function
void TempSensorISR();

#endif /* TEMP_SENSOR_H_ */
