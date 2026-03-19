#include <stdint.h>
#include <stdbool.h>
#include "launchpad.h"
#include "seg7.h"
#include "temp_sensor.h"
#include "ras.h"

// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------
Event toggle_event, temp_event, ras_event;
Seg7Display seg7;
bool show_temp = false;

// -----------------------------------------------------------------------------
// Callback when temperature conversion completes
// -----------------------------------------------------------------------------


void TempCallback(Event *event)
{
    float t = TsDataRead();
    int temp10 = (int)(t * 10);   // temperature * 10 for one decimal place

    seg7.digit[0] = 0xF;  // blank
    seg7.digit[1] = temp10 % 10;
    seg7.digit[2] = (temp10 / 10) % 10;
    seg7.digit[3] = (temp10 / 100) % 10;
    seg7.colon_on = true;

    Seg7Update(&seg7);
}

// -----------------------------------------------------------------------------
// Callback when RAS conversion completes
// -----------------------------------------------------------------------------
void RasCallback(Event *event)
{
    int angle = RasGetAngle();

    seg7.digit[0] = angle % 10;
    seg7.digit[1] = (angle / 10) % 10;
    seg7.digit[2] = (angle / 100) % 10;
    seg7.digit[3] = 0xF;  // blank
    seg7.colon_on = false;

    Seg7Update(&seg7);
}

// -----------------------------------------------------------------------------
// Toggle event: switch between temperature and RAS every 2 seconds
// -----------------------------------------------------------------------------
void ToggleDisplay(Event *event)
{
    if (show_temp)
        TsTriggerReading();   // trigger temperature ADC (ISR will handle callback)
    else
        RasStart();  // trigger RAS ADC (ISR will handle callback)

    show_temp = !show_temp;
    EventSchedule(event, event->time + 2000);  // schedule every 2s
}

// -----------------------------------------------------------------------------
// Main function
// -----------------------------------------------------------------------------
int main(void)
{
    LaunchPadInit();
    TsInit();
    RasInit();
    Seg7Init();

    // Register ADC completion callbacks
    TsEventRegister(&temp_event);
    RasEventRegister(&ras_event);

    EventInit(&temp_event, TempCallback);
    EventInit(&ras_event, RasCallback);
    EventInit(&toggle_event, ToggleDisplay);

    // Initial display setup
    int i;
    for (i = 0; i < 4; i++) seg7.digit[i] = 0;
    seg7.colon_on = false;
    Seg7Update(&seg7);

    // Start periodic toggle
    EventSchedule(&toggle_event, 1000);

    while (true)
    {
        __asm(" wfi");  // Wait for interrupt
        EventExecute(); // Run scheduled events
    }
}
