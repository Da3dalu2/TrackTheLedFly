#include "gameLogic.h"
#include <Arduino.h>
#include "ledUtils.h"
#include "settings.h"

char welcomeMsg[] = "Welcome to the Track to Led Fly Game, Press T1 to Start";
char beginMsg[] = "Go!";
char successMsg[] = "Tracking the fly: pos: ";
char endGameMsg[] = "Game Over - Score: ";

uint32_t tMin;  // microsec
volatile bool defeat = false;

volatile uint8_t ledPos;
uint32_t tNextHop;

uint8_t currentTRun;
uint8_t previousTRun;
volatile uint8_t tRun;

uint8_t currentTEnd;
uint8_t previousTEnd;
volatile uint8_t tEnd;

uint8_t currentScore;
uint8_t previousScore;
volatile uint8_t score;

/*
 * Interrupt handlers.
 */
void beginHandler(void) {
    analogWrite(yellowLedPin, fadeLed(yellowLedPin));
}

void inGameHandler(void) {
    if( tRun == 0 )
        blinkLed(ledsArray[ledPos]);
    
    tRun++;
}

void endHandler(void) {  
    blinkLed(yellowLedPin);
    tEnd++;
}

void onStartButtonPress(void) {}

void onButtonPress(void) {}

/**
 * Functions that define the game state.
 */

/*
 * Initialization of the game 
 */
void initGame() {
    brightness = 0;
    fadeAmount = 5;
    newSensorValue = 0;
    lastDebounceTime = 0;
    
    currentTRun = 0;
    previousTRun = 0;
    tRun = 0;
    
    currentTEnd = 0;
    previousTEnd = 0;
    tEnd = 0;
        
    currentScore = 0;
    previousScore = 0;
    
    ledPos = 0;
    score = 0;
    
    randomSeed(analogRead(A0));
    
    tNextHop = 0;
    tMin = STD_TMIN;
    difficulty = 1;
    
    delay(1000);

#ifdef DEBUG_LOGIC
    Serial.flush();
    Serial.println("[A]: \tGame init");
#else
    Serial.flush();
    Serial.println(welcomeMsg);
#endif

    gameInit = true;    // close the previous
}

/*
 * Running.
 */
void runGame() {
    noInterrupts();
    ledPos = putFlyingLed();
    tMin = tMin - (difficulty * 250000);
    tNextHop = random(tMin, k * tMin);
    updateTimer(tNextHop, inGameHandler);
    changeBtnISR(startButtonPin, onButtonPress);
    interrupts();

#ifdef DEBUG_LOGIC
    Serial.flush();
    Serial.println("[B]: \tGame start");
    Serial.print("[B]: \tChosen difficulty: ");
    Serial.println(difficulty);
    Serial.print("[B]: \tNext leap's location: ");
    Serial.println(ledPos);
    Serial.print("[B]: \tTime until next leap: ");
    Serial.print(tNextHop);
    Serial.println(" μsec");
#else
    Serial.flush();
    Serial.println(beginMsg);
    Serial.print("Chosen difficulty: ");
    Serial.println(difficulty);
#endif

    gameRuns = true;  // close the previous
}

void checkRun() {
    if( currentTRun != tRun ) {
        noInterrupts();
        currentTRun = tRun;
        interrupts();

        if( currentTRun != previousTRun && currentTRun != 0 ) {
            previousTRun = currentTRun;
            if( tRun > 2 && !correctButtonPress) {
#ifdef DEBUG_LOGIC
                Serial.println("[B->C]: Button pressed too late");
#endif
                setDefeat();
            }
        }
    }
}

void setVictory() {

    turnLedOff();
    
#ifndef DEBUG_LOGIC
    Serial.print(successMsg);
    Serial.println(ledPos);
#endif

    noInterrupts();
    tMin = tMin * 7/8;
    tNextHop = random(tMin, k * tMin);
    ledPos = moveFlyingLed(ledPos);

    changeTimerPeriod(tNextHop);
    
    interrupts();

    score++;
    tRun = 0;
    correctButtonPress = false;

#ifdef DEBUG_LOGIC
    Serial.print("[B->C]: Next leap's location: ");
    Serial.println(ledPos);
    Serial.print("[B->C]: Time until next leap: ");
    Serial.print(tNextHop);
    Serial.println(" μsec");
    Serial.print("[B->C]: current score: ");
    Serial.println(score);
#endif

}

void setDefeat() {
    turnLedOff();
    
    gameRuns = false;
    gameStarts = true;  // close the previous
    gameEnds = false;   // open the next
    
#ifdef DEBUG_LOGIC
    Serial.println("[B->C]: Game ending...");
    Serial.print("[B->C]: Final score: ");
    Serial.println(score);
#else
    Serial.print(endGameMsg);
    Serial.println(score);
#endif

    score = 0;
    tRun = 0;
    correctButtonPress = false;
}

/*
 * Ending phase.
 */

void endGame() {
    noInterrupts();
    updateTimer(ONE_SEC, endHandler);
    interrupts();

#ifdef DEBUG_LOGIC
    Serial.flush();
    Serial.println("[C]: \tGame end");
#endif

    gameEnds = true;  // close the previous
    changeBtnISR(startButtonPin, onStartButtonPress);
}

void checkReInit() {
    if( currentTEnd != tEnd ) {
        noInterrupts();
        currentTEnd = tEnd;
        interrupts();
        if( currentTEnd != previousTEnd && currentTEnd != 0 )
            setEndGame(); // FIXME: not always triggered during endgame
    }
}

void setEndGame() {
    previousTEnd = currentTEnd;

#ifdef DEBUG_LOGIC
    Serial.print("[C->A]: Time elapsed since end: ");
    Serial.print(currentTEnd);
    Serial.print("/");
    Serial.println(TIME_END);
#endif

    if( tEnd >= TIME_END ) {
        setupTimer();
        gameInit = false; // open the next
#ifdef DEBUG_LOGIC
        Serial.println("[C->A]: Game restarting...");
#endif
    }
}
