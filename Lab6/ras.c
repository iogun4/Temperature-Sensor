#include <stdint.h>
#include <stdbool.h>
#include "ras.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

// -----------------------------------------------------------------------------
// Hardware configuration constants
// -----------------------------------------------------------------------------
#define RAS_ADC_BASE       ADC1_BASE
#define RAS_ADC_SEQ        3
#define RAS_ADC_CHANNEL    ADC_CTL_CH0     // PE3 = AIN0
#define RAS_GPIO_BASE      GPIO_PORTE_BASE
#define RAS_GPIO_PIN       GPIO_PIN_3

// -----------------------------------------------------------------------------
// Internal module state
// -----------------------------------------------------------------------------
typedef struct {
    Event *callback_event;   // event triggered when ADC conversion completes
    uint32_t raw_data;       // most recent ADC raw value (0–4095)
    bool new_input;          // true when new ADC data is ready
} RasState;

RasState ras_state;

// -----------------------------------------------------------------------------
// Function implementations
// -----------------------------------------------------------------------------
void RasInit(void)
{
    ras_state.callback_event = NULL;
    ras_state.new_input = false;

    // Enable ADC1 and Port E
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);

    // Configure PE3 as analog input (AIN0)
    GPIOPinTypeADC(RAS_GPIO_BASE, RAS_GPIO_PIN);

    // Configure ADC1, Sequencer 3, processor trigger, priority 0
    ADCSequenceConfigure(RAS_ADC_BASE, RAS_ADC_SEQ, ADC_TRIGGER_PROCESSOR, 0);

    // Step 0: AIN0, interrupt enable, end of sequence
    ADCSequenceStepConfigure(RAS_ADC_BASE, RAS_ADC_SEQ, 0,
                             RAS_ADC_CHANNEL | ADC_CTL_IE | ADC_CTL_END);

    // Register ISR handler
    ADCIntRegister(RAS_ADC_BASE, RAS_ADC_SEQ, RasISR);
    ADCIntEnable(RAS_ADC_BASE, RAS_ADC_SEQ);

    // Enable the sequencer and its interrupt
    ADCSequenceEnable(RAS_ADC_BASE, RAS_ADC_SEQ);
}

void RasStart(void)
{
    ADCProcessorTrigger(RAS_ADC_BASE, RAS_ADC_SEQ);
}

bool RasDataReady(void)
{
    return ras_state.new_input;
}

int RasGetAngle(void)
{
    int angle = (int)((ras_state.raw_data * 180.0) / 4095.0);
    ras_state.new_input = false;
    return angle;
}

void RasEventRegister(Event *event)
{
    ras_state.callback_event = event;
}

void RasISR(void)
{
    ADCSequenceDataGet(RAS_ADC_BASE, RAS_ADC_SEQ, &ras_state.raw_data);
    ras_state.new_input = true;

    if (ras_state.callback_event)
        EventSchedule(ras_state.callback_event, EventGetCurrentTime());

    ADCIntClear(RAS_ADC_BASE, RAS_ADC_SEQ);
}
