#include "config.h"
#include "lpm_widgets.h"
#include <Arduino.h>

TTGOClass *ttgo;

void relayTurnOn(void)
{
    ttgo->turnOnRelay();
}

void relayTurnOff(void)
{
    ttgo->turnOffRelay();
}

void setBrightness(uint8_t level)
{
    ttgo->setBrightness(level);
}

void turnOnUSB()
{
    ttgo->turnOnUSB();
}

void turnOffUSB()
{
    ttgo->turnOffUSB();
}

float getVoltage()
{
    return ttgo->getVoltage();
}

void setup()
{
    Serial.begin(115200);

    //Get watch instance
    ttgo = TTGOClass::getWatch();

    // Initialize the hardware
    ttgo->begin();

    // Turn on the backlight
    ttgo->openBL();

    ttgo->lvgl_begin();

    ttgo->lvgl_whirling(3); //Parameters: 0, 1, 2 and 3 represent 0°, 90°, 180° and 270° respectively, and 4 can be set as mirror image.

    lpm_widgets();

}

void loop()
{
    lv_task_handler();
    delay(2);
}
