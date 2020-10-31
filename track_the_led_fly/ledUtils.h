#ifndef LED_UTILS_H
#define LED_UTILS_H
#include <stdint.h>

extern uint8_t fadeAmount;
extern uint8_t brightness;
extern volatile bool blinking;

uint8_t fadeLed(uint8_t ledPin);

void blinkLed(uint8_t ledPin);

uint8_t moveFlyingLed(uint8_t ledPos);

uint8_t putFlyingLed();

#endif /* LED_UTILS_H */
