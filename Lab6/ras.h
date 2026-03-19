/*
 * ras.h
 *
 *  Created on: Oct 26, 2025
 *      Author: iogun
 */

#ifndef RAS_H_
#define RAS_H_

#include <stdint.h>
#include <stdbool.h>
#include "launchpad.h"

/**
 * Initializes the RAS module.
 * Configures ADC0 (Sequencer 3) to read from PE3 (AIN0) with interrupt.
 */
void RasInit(void);

/**
 * Starts an ADC conversion for the RAS.
 * The ISR will automatically handle the interrupt and data storage.
 */
void RasStart(void);

/**
 * Returns the latest RAS angular reading (0–180 degrees).
 * The angle is linearly mapped from ADC value range 0–4095.
 */
int RasGetAngle(void);

/**
 * Returns the raw 12-bit ADC value (0–4095).
 */
uint32_t RasRawData(void);

/**
 * Registers an event to be scheduled when the ADC conversion completes.
 * This event is triggered inside the ISR.
 */
void RasEventRegister(Event *event);

/**
 * ADC interrupt service routine for RAS.
 * Reads the ADC result and triggers the registered callback event.
 */
void RasISR(void);

#endif /* RAS_H_ */
