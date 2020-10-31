#include "settings.h"
#define EI_ARDUINO_INTERRUPTED_PIN
#include <EnableInterrupt.h>
#include <MiniTimerOne.h>
#include <Arduino.h>
#include "gameLogic.h"
#include "ledUtils.h"

const uint8_t potPin = A1;
const uint8_t startButtonPin = 3;
const uint8_t yellowLedPin = 11;
const uint8_t ledsArray[] = {2, 4, 6, 8};
const uint8_t numberLeds = sizeof(ledsArray)/sizeof(ledsArray[0]);
const uint8_t buttonsArray[] = {3, 5, 7, 9};
const uint8_t numberButtons = sizeof(buttonsArray)/sizeof(buttonsArray[0]);
const uint16_t initialFadeDelay = 20000;
const uint8_t debounceDelay = 50;
const uint8_t k = 1.3;    // difficulty multiplier

uint16_t newSensorValue;
uint8_t difficulty;
uint8_t currentState;
uint8_t lastStableState = LOW;
uint8_t lastUnstableState = LOW;
uint32_t lastDebounceTime;

bool correctButtonPress = false;

/* 
 *  |ledButtonTable[][]| is used to store the association between a button 
 *  and the corresponding led.
 *   
 *   0   1   2   3
 * |---|---|---|---|
 * | 2 | 4 | 6 | 8 | 0 (LEDS)
 * |---|---|---|---|
 * | 3 | 5 | 7 | 9 | 1 (BUTTONS)
 * |---|---|---|---|
 * 
 * Data is taken from |ledsArray[]| and |buttonsArray[]|. Both the arrays 
 * must be defined so that in the same index there is a corresponding 
 * button-led pair. Failure in doing so will result in an unplayable game.
 * 
 * This table is used by |isCorrectBtnPressed()| and |turnLedOff()|.
 */
uint8_t ledButtonTable[NUM_ELEMS][numberButtons];

void createButtonLedTable() {
  if(numberButtons == numberLeds) {
    for(uint8_t i = 0; i < numberButtons; i++) {
      ledButtonTable[LEDS][i] = ledsArray[i];
      ledButtonTable[BUTTONS][i] = buttonsArray[i];
    }
  }
}

void checkBouncing() {

    /*
     * Check if a button is pressed. This is done reading the value of the
     * global variable |arduinoInterruptedPin| to know the value of the pin
     * from which the last interrupt occurred and then checking if this value
     * corresponds to one of the pins in which buttons are located.
     */
  
    if( checkBtnPressed() == true ) {
        currentState = digitalRead(arduinoInterruptedPin);
        
        /*
         *  Update the current unstable state.
         */
        
        if( currentState != lastUnstableState ) {
          lastDebounceTime = millis();
          lastUnstableState = currentState;
        }

        /*
         * If it is a stable state, determine whether the button is pressed
         * or released. In case the button is pressed check wheter it is the 
         * start button or not. In case it is a button pressed during the game
         * check if it is pressed on time. If it pressed too late the game is 
         * lost and it enters in the ending phase. Anyway whether a button is 
         * pressed the corresponding led should be turned off.
         * 
         * Since button handlers are empty, to see what was the last button
         * pressed it is used the global variable |arduinoInterruptedPin|.
         * 
         * To measure the time passed since the last valid button press it 
         * is used the function |millis()|.
         */

        if( (millis() - lastDebounceTime) > debounceDelay ) { 
            if( isBtnPressed() && !isGameStarted() &&
                arduinoInterruptedPin == startButtonPin ) {
                gameStarts = false;
            } else if( isBtnPressed() && isGameStarted() ) {
                correctButtonPress = isCorrectBtnPressed();
                if( correctButtonPress ) {
#ifdef DEBUG_SETTINGS
                    Serial.print("[B->C]: Correct button pressed: ");
                    Serial.println(arduinoInterruptedPin);
#endif
                    setVictory();
                } else {
#ifdef DEBUG_SETTINGS
                    Serial.print("[B->C]: Wrong button pressed: ");
                    Serial.println(arduinoInterruptedPin);
#else
                    Serial.print("Wrong button pressed: ");
                    Serial.println(arduinoInterruptedPin);
#endif
                    setDefeat();
                }
            }
            lastStableState = currentState;
        }
    }
}

void changeBtnISR(uint8_t btnPin, void(*isr)(void)) {
    disableInterrupt(btnPin);
    enableInterrupt(btnPin, isr, RISING);
}

void setDifficulty() {
    uint16_t sensorValue = analogRead(potPin);
    if( (sensorValue <= 894  && newSensorValue >= sensorValue + 129) ||
        (sensorValue >= 128 && newSensorValue <= sensorValue - 128) ) {
        newSensorValue = sensorValue;
        difficulty = map(newSensorValue, 0, 1023, MIN_DIFF, MAX_DIFF);

#ifdef DEBUG_SETTINGS
        Serial.print("[A]: \tCurrent difficulty: ");
        Serial.println(difficulty);
#else
        Serial.print("Current difficulty: ");
        Serial.println(difficulty);
#endif
    }
}

/*
 * Helper functions to handle bouncing.
 */
void turnLedOff() {
    uint8_t ledPin = ledButtonTable[LEDS][ledPos];
    if(digitalRead(ledPin) == HIGH)
        digitalWrite(ledPin, LOW);
}
 
bool checkBtnPressed() {
    for(int i = 0; i < numberButtons; i++) {
        if( arduinoInterruptedPin == buttonsArray[i] )
            return true;
    }
    return false;
}

bool isBtnPressed() {
    return lastStableState == LOW && currentState == HIGH;
}

bool isCorrectBtnPressed() {
    return arduinoInterruptedPin == ledButtonTable[BUTTONS][ledPos];
}

/*
 * The game is considered started only if |gameInit()| and |gameRun()|
 * have been executed. This is true only if |gameInit| is true and 
 * |gameRuns| is true.
 */
bool isGameStarted() {
    return gameRuns && gameInit;
}

/*
 * Functions used during the setup.
 */
void setupTimer() {
    MiniTimer1.init();
    MiniTimer1.setPeriod(initialFadeDelay);
    MiniTimer1.attachInterrupt(beginHandler);
    MiniTimer1.start();
}

void setupLeds() {
    for(uint8_t i = 0; i < numberLeds; i++)
        pinMode(ledsArray[i], OUTPUT);
}

void setupButtons() {
    for(uint8_t i = 0; i < numberButtons; i++) {
        pinMode(buttonsArray[i], INPUT);
        if( i == 0 ) {
            enableInterrupt(buttonsArray[i], onStartButtonPress, RISING);
        }
        else {
            enableInterrupt(buttonsArray[i], onButtonPress, RISING);
        }
    }
}

/*
 * Functions used to update period and ISR of the timer.
 */

void updateTimer(uint32_t period, void(*isr)(void)) {
    MiniTimer1.stop();

    if( digitalRead(yellowLedPin) == HIGH ) {
        digitalWrite(yellowLedPin, LOW);
        //blinking = true;
    }
    
    MiniTimer1.setPeriod(period);
    MiniTimer1.detachInterrupt();
    MiniTimer1.attachInterrupt(isr);
    MiniTimer1.start();
}

void changeTimerPeriod(uint32_t tNextHop) {
    MiniTimer1.stop();
    MiniTimer1.setPeriod(tNextHop);
    MiniTimer1.start();
}
