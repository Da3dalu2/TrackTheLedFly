#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>
#include "trackTheLedFly.h"

#define NUM_ELEMS 2 // used for creating a table buttonPin <-> ledPin

/*
 * Used to access elements of the |ledButtonTable|.
 */
#define BUTTONS 1
#define LEDS 0

#define MIN_DIFF 1
#define MAX_DIFF 8

#ifdef DEBUG
#define DEBUG_SETTINGS
#endif

extern uint8_t difficulty;
extern uint16_t newSensorValue;
extern uint32_t lastDebounceTime;
extern const uint8_t yellowLedPin;
extern const uint8_t startButtonPin;
extern const uint8_t numberButtons;
extern const uint8_t buttonsArray[];
extern const uint8_t numberLeds;
extern const uint8_t ledsArray[];
extern const uint8_t potPin;

void createButtonLedTable();

void setupTimer();

void setupLeds();

void setupButtons();

void changeBtnISR(uint8_t btnPin, void(*isr)(void));

void updateTimer(uint32_t period, void(*isr)(void));

void changeTimerPeriod(uint32_t tNextHop);

void setDifficulty();

void turnLedOff();

bool checkBtnPressed();

bool isBtnPressed();

bool isCorrectBtnPressed();

bool isGameStarted();

void checkBouncing();


#endif /* SETTINGS_H */
