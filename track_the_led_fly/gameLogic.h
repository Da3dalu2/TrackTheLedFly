#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdint.h>
#include "trackTheLedFly.h"

#define TIME_END 2
#define STD_TMIN 3000000L
#define ONE_SEC 1000000L

#ifdef DEBUG
#define DEBUG_LOGIC
#endif

extern volatile uint8_t ledPos;
extern uint32_t tMin;
extern uint32_t tNextHop;
extern bool correctButtonPress;
extern bool gameStarts;
extern const uint8_t k;
extern bool gameRuns;
extern bool gameInit;
extern bool gameEnds;

void runGame();

void initGame();

void endGame();

void checkRun();

void setVictory();

void setDefeat();

void checkReInit();

void setEndGame();

void beginHandler();

void inGameHandler();

void endHandler();

void onStartButtonPress();

void onButtonPress();

#endif /* GAME_LOGIC_H */
