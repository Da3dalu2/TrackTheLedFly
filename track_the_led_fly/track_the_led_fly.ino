#include "gameLogic.h"
#include "settings.h"

bool gameInit = false;  // set by |endGame()|
bool gameStarts = true; // set by |checkBouncing()|
bool gameRuns = false;  // set by |runGame()|
bool gameEnds = true;   // set by |checkRun()|

/*****************************************************************************/

void setup() {
    pinMode(potPin, INPUT);
    pinMode(yellowLedPin, OUTPUT);
    setupLeds();
    setupButtons();
    setupTimer();
    createButtonLedTable();
    Serial.begin(9600);
}

void loop() {

    checkBouncing();
    
    if(!gameInit)
        initGame();
    
    if(!gameStarts) {
        if(!gameRuns) {
            runGame();
        } else {
            checkRun();
        }
    } else {
        setDifficulty();
    }
    
    if(!gameEnds)
        endGame();

    checkReInit();
}
