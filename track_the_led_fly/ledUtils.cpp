#include "ledUtils.h"
#include "settings.h"
#include <Arduino.h>

uint8_t fadeAmount;
uint8_t brightness;

uint8_t fadeLed(uint8_t ledPin) {
    brightness = brightness + fadeAmount;
    if ( brightness == 0 || brightness == 255 )
        fadeAmount = -fadeAmount ;

    return brightness;
}

void blinkLed(uint8_t ledPin) {
  if(digitalRead(ledPin) != HIGH )
      digitalWrite(ledPin, HIGH);
}

uint8_t moveFlyingLed(uint8_t ledPos) {

    uint8_t newLedPos = ledPos;

    if( random(0, 2) == 1 ) {
        if( newLedPos + 1 >= numberLeds )
            newLedPos = 0;
        else
            newLedPos += 1;

    } else {
        if( newLedPos - 1 < 0 )
            newLedPos = numberLeds - 1;
        else
            newLedPos -= 1;
    }

    return newLedPos;
}

uint8_t putFlyingLed() {
   return random(0, numberLeds);
}
