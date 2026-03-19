/*
 * temp_sensor.c: Starter code for ECE 266 Lab 6, spring 2024
 *
 * Temperature sensor module
 *
 * Created by Zhao Zhang, fall 2025
 * Last update: Oct. 18, 2024
 */

#include "temp_sensor.h"

/*
 * Global declarations
 */

// Temperature sensor reading states
typedef struct {
    Event *callback_event;      // the callback event for data processing
    uint32_t raw_data;          // the raw reading from the ADC connected to the sensor
    bool new_input;             // if any new data input is ready
} TempSensorState;

TempSensorState temp_sensor;

/*
 * Initialize ADC to use Tiva C's internal temperature sensor
 *
 * Resources: ADC0, sequence #0, special channel TS (temperature sensor)
 * Configurations: processor trigger, interrupt enabled, use step 0 only
 */
void TsInit()
{
    // Initialize sensor state in memory
    temp_sensor.callback_event = NULL;
    temp_sensor.new_input = false;

    // Enable the ADC0 peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    // Configure ADC0's sequencer #0
    ADCSequenceConfigure(ADC0_BASE, 0 /* sequencer */, ADC_TRIGGER_PROCESSOR, 0 /* priority */);

    // Configure step 0 of sequencer 0 to use the temperature sensor, with
    // interrupt enable, as the end of the sequence
    ADCSequenceStepConfigure(ADC0_BASE, 0 /* sequencer */, 0 /* step */,
                             ADC_CTL_TS | ADC_CTL_IE | ADC_CTL_END);

    // Configure the interrupt system for temperature sensor
    ADCIntRegister(ADC0_BASE, 0 /* sequencer */, TempSensorISR);
    ADCIntEnable(ADC0_BASE, 0 /* sequencer */);

    // Enable ADC0, sequencer 0
    ADCSequenceEnable(ADC0_BASE, 0 /* sequencer */);
}

/*
 * Trigger temperature reading
 */
void TsTriggerReading()
{
    // Trigger the sampling
    ADCProcessorTrigger(ADC0_BASE, 0 /* sequencer */);
}

/*
 * Check if any new data is ready (for I/O polling)
 */
bool TsDataReady()
{
    return temp_sensor.new_input;
}

/*
 * Read the current input from the temperature sensor
 */
float TsDataRead()
{
    // Get the current reading and convert it to temperature in Celsius
    // See Tiva C datasheet (ver. DS-TM4C123GH6PM-15842.2741), page 813
    float temp_C = 147.5 - (75 * 3.3 * (float) temp_sensor.raw_data / 4096);

    // Reset new data ready flag and return the reading in Fahrenheit
    temp_sensor.new_input = false;
    return temp_C*1.8 + 32;
}

/*
 * Set callback event for temperature sensor ISR
 */
void TsEventRegister(Event *event)
{
    temp_sensor.callback_event = event;
}

/*
 * The ISR function: Read the data, mark data ready flag, and
 * schedule the callback event
 */
void TempSensorISR()
{
    // Get the data
    ADCSequenceDataGet(ADC0_BASE, 0 /* sequencer */,
                       &temp_sensor.raw_data /* pointer to data buffer */);

    // Set the flag for new data ready
    temp_sensor.new_input = true;

    // Schedule a data processing event
    EventSchedule(temp_sensor.callback_event, EventGetCurrentTime());

    // IMPORTANT: Clear the interrupt flag
    ADCIntClear(ADC0_BASE, 0 /* sequencer */);
}
