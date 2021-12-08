#include <16F877A.H>
#include <math.h>
#include <stdlib.h>

#fuses HS
#fuses NOPUT, NOBROWNOUT, NOWDT, NODEBUG
#use delay (clock=16MHz)

#define boolean int8
#define true 1
#define false 0

#define NO_LEDS_FLAG 0     // 0b00000000;
#define LED1_FLAG 1        // 0b00000001 = 2^0;
#define LED2_FLAG 2        // 0b00000010 = 2^1;
#define LED3_FLAG 4        // 0b00000100 = 2^2;
#define LED4_FLAG 8        // 0b00001000 = 2^3;
#define LED5_FLAG 16       // 0b00010000 = 2^4;
#define LED6_FLAG 32       // 0b00100000 = 2^5;
#define LED7_FLAG 64       // 0b01000000 = 2^6;
#define LED8_FLAG 128      // 0b10000000 = 2^7;
#define ALL_LEDS_FLAGS 255 // 0b11111111 = 2^8 - 1;

#define LEDS_COUNT 8

#define MAX_GAME_STEPS 20
typedef struct answer {
   int8 answers[20];
   int size;
} Answer;

typedef struct state {
   boolean isGameStarted;
   int currentGameStep;
   Answer currentAnswer;
} State;

// ** Procedimentos de estado ** //
void startGame(State*);
void goToNextStep(State*);
void clearGame(State*);
void updateGameCurrentAnswer(State* gameState, Answer newAnswer);

// ** Funções ** //
int8 getInput();
Answer generateNextAnswer(Answer, int);
Answer getUserAnswer(int);
boolean checkAnswers(Answer, Answer);
boolean contains(int8 list[], int8 value, int size);

// ** Procedimentos de entrada e saída** // 
void waitClearInputs();
void showAnswer(Answer);
void showLedsStates(int8 ledsStates[], int size, int delay);
void showLedsStatesWithInterval(int8 ledsStates[], int size, int delay);
void showLedsState(int8 state);

// ** Procedimentos de animações ** //
void showWrongAnswerAnim();
void showCorrentAnswerAnim();

// Usando vetor ao invés de define para evitar funções com muitos ifs 
int ledsFlags[] = {
   LED1_FLAG,
   LED2_FLAG,
   LED3_FLAG,
   LED4_FLAG,
   LED5_FLAG,
   LED6_FLAG,
   LED7_FLAG,
   LED8_FLAG
};

int ledsPins[] = {
   PIN_D0,
   PIN_D1,
   PIN_D2,
   PIN_D3,
   PIN_D4,
   PIN_D5,
   PIN_D6,
   PIN_D7
};

int inputsPins[] = {
   PIN_C0,
   PIN_C1,
   PIN_C2,
   PIN_C3,
   PIN_C4,
   PIN_C5,
   PIN_C6,
   PIN_C7
};

int8 fillingLeftToRight[] = {
   0b00000000,
   0b00000001,
   0b00000011,
   0b00000111,
   0b00001111,
   0b00011111,
   0b00111111,
   0b01111111,
   0b11111111
};

int8 cleaningLeftToRight[] = {
   0b11111111,
   0b11111110,
   0b11111100,
   0b11111000,
   0b11110000,
   0b11100000,
   0b11000000,
   0b10000000,
   0b00000000
};

int8 fillingRightToLeft[] = {
   0b00000000,
   0b10000000,
   0b11000000,
   0b11100000,
   0b11110000,
   0b11111000,
   0b11111100,
   0b11111110,
   0b11111111
};

int8 cleaningRightToLeft[] = {
   0b11111111,
   0b01111111,
   0b00111111,
   0b00011111,
   0b00001111,
   0b00000111,
   0b00000011,
   0b00000001,
   0b00000000
};

void main() {
   State gameState;
   clearGame(&gameState);

   while(true) {
      if (!gameState.isGameStarted) {
         getInput();
         startGame(&gameState);
      }

      Answer newAnswer = generateNextAnswer(gameState.currentAnswer, gameState.currentGameStep);
      showAnswer(newAnswer);
      Answer userAnswer = getUserAnswer(gameState.currentGameStep);
      boolean isCorrentUserAnswer = checkAnswers(userAnswer, newAnswer);
      updateGameCurrentAnswer(&gameState, newAnswer);
      
      if (isCorrentUserAnswer) {
         showCorrentAnswerAnim();
         goToNextStep(&gameState);
      } else {
         showWrongAnswerAnim();
         clearGame(&gameState);
      }
   }
}

// ** Procedimentos de estado ** //

void startGame(State* gameState) {
   gameState->isGameStarted = true;
   showLedsStates(fillingLeftToRight, 9, 25);
   showLedsStates(cleaningLeftToRight, 9, 25);
   showLedsStates(fillingRightToLeft, 9, 25);
   showLedsStates(cleaningRightToLeft, 9, 25);
}

void goToNextStep(State* gameState) {
   if (gameState->currentGameStep == MAX_GAME_STEPS) {
      return;
   }
   gameState->currentGameStep++;
}

void clearGame(State* gameState) {
   gameState->currentGameStep = 1;
   gameState->isGameStarted = false;
   gameState->currentAnswer.size = 0;
}

void updateGameCurrentAnswer(State* gameState, Answer newAnswer) {
   gameState->currentAnswer = newAnswer;
}

// ** Funções ** //

int8 getInput() {
   while (true) {
      int i;
      for (i = 0; i < LEDS_COUNT; i++) {
         int inputPin = inputsPins[i];
         if (input(inputPin)) {
            return ledsFlags[i];
         }
      }
   }
}

Answer generateNextAnswer(Answer oldAnswer, int currentGameStep) {
   Answer answer;
   int i;

   for (i = 0; i < oldAnswer.size; i++) {
      answer.answers[i] = oldAnswer.answers[i];
   }
   
   answer.answers[currentGameStep - 1] = pow(2, rand() % LEDS_COUNT);
   answer.size = currentGameStep;
   return answer;
}

Answer getUserAnswer(int currentGameStep) {
   Answer answer;
   int i;
   answer.size = currentGameStep;
  
   for (i = 0; i < currentGameStep; i++) {
      int8 input = getInput();
      answer.answers[i] = input;
      showLedsState(input);
      waitClearInputs();
      showLedsState(NO_LEDS_FLAG);
   }

   return answer;
}

boolean checkAnswers(Answer a, Answer b) {
   if (a.size != b.size) {
      return false;
   }
   
   int i;
   for (i = 0; i < a.size; i++) {
      if (a.answers[i] != b.answers[i]) {
         return false;
      }
   }
   
   return true;
}

// ** Procedimentos de entrada e saída** // 

void waitClearInputs() {
   boolean hasPressedButton = true;
   while (true) {
      int i;
      hasPressedButton = false;
      for (i = 0; i < LEDS_COUNT; i++) {
         int inputPin = inputsPins[i];
         if (input(inputPin)) {
            hasPressedButton = true;
         }
      }
      
      if (!hasPressedButton) {
         return;
      }
   }
}

void showAnswer(Answer answer) {
   showLedsStatesWithInterval(answer.answers, answer.size, 1000);
}

void showLedsStatesWithInterval(int8 ledsStates[], int size, int delay) {
   int i;
   
   delay_ms(delay);
   for (i = 0; i < size; i++) {
      int8 ledsState = ledsStates[i];
      
      showLedsState(ledsState);
      delay_ms(delay);
      showLedsState(NO_LEDS_FLAG);
      delay_ms(delay);
   }
}

void showLedsStates(int8 ledsStates[], int size, int delay) {
   int i;
   
   delay_ms(delay);
   for (i = 0; i < size; i++) {
      int8 ledsState = ledsStates[i];
      
      showLedsState(ledsState);
      delay_ms(delay);
   }
}

void showLedsState(int8 state) {
   int i;
   for (i = 0; i < LEDS_COUNT; i++) {
      int8 ledFlag = pow(2, i);
   
      if (state & ledFlag) {
         output_high(ledsPins[i]);
      } else {
         output_low(ledsPins[i]);
      }
   }
}

// ** Procedimentos de animações ** //

void showCorrentAnswerAnim() {
   showLedsStates(fillingLeftToRight, 9, 25);
   showLedsStates(cleaningLeftToRight, 9, 25);
}

void showWrongAnswerAnim() {
   delay_ms(100);
   showLedsState(ALL_LEDS_FLAGS);
   delay_ms(300);
   showLedsState(NO_LEDS_FLAG);
   delay_ms(300);
   showLedsState(ALL_LEDS_FLAGS);
   delay_ms(300);
   showLedsState(NO_LEDS_FLAG);
   delay_ms(300);
   showLedsState(ALL_LEDS_FLAGS);
   delay_ms(300);
   showLedsState(NO_LEDS_FLAG);
   delay_ms(300);
}

