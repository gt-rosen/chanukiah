/**
  Flame-free chanukiah!
  Perfect for pets, apartments, and any other situation where fire is a bad idea. 

  To use:  
    1. click green ">" button to start the program running. 
    2. click yellow circle button to light the shamash. 
    3.  

  ***How to change candle colors: 

     special candle color gradients: 
     "BCYR": blue, cyan, yellow, red 
     "GYR": green, yellow, red

  *** How to change burn duration: 
      use #define DRIP_ROLL_MAX, DRIP_CUTOFF_REG, DRIP_CUTOFF_LAST to change the burn duration!
      defaults (180,10,2) burn for about 5 minutes. 
      DRIP_ROLL_MAX: increase this to make candles burn for longer overall. 
      DRIP_CUTOFF_REG: increase this to make candles to burn more unevenly. 
      DRIP_CUTOFF_LAST: decrease this (don't go less than 1) to make the last nub of a candle last longer.


  Implementation info: 
    this code uses Charlieplexing to light a large number of LEDs using relatively few pins! 
    read more about Charlieplexing here: https://en.wikipedia.org/wiki/Charlieplexing

  by G Rosen
*/

/* TODO: consider, do i need the burning state? some kind of "at rest"/burned out state?
*/

#define LEDS_PER_CANDLE 10
#define LED_PIN_COUNT 4
#define NUM_CANDLES 9
#define SCAN_DELAY 1
#define DISABLE_LDR 99 

#define DRIP_ROLL_MAX 360
#define DRIP_CUTOFF_REG 10
#define DRIP_CUTOFF_LAST 2 



#define WAITING 1
#define LIGHTING 2
#define BURNING 3

struct candleStruct {
  uint8_t setLedPins[LED_PIN_COUNT];
  uint8_t ledPins[LEDS_PER_CANDLE][2]; //format: {anodePin,cathodePin}
  uint8_t height;
  //flicker timer stuff
  unsigned long prevMillis;
  int flickerDuration;
  bool flickerState;
  //photoresistor pin (if necessary)
  uint8_t ldrPin;
} ;


//Pins n states
const uint8_t lighterPin = 2;
uint8_t chanukiahState = 0;
unsigned long prevMillisDrip = 0; 

candleStruct candle0 = {
  {4, 5, 6, 7},
  { {4, 5},
    {4, 6},
    {4, 7},
    {5, 4},
    {5, 6},
    {5, 7},
    {6, 4},
    {6, 5},
    {6, 7},
    {7, 5}
  },
  0, //height
  0, //prevMillis
  500, //flickerDuration
  1, //flickerState
  A9
};

candleStruct candle1 = {
  {8, 9, 10, 11},
  { {8, 9},
    {8, 10},
    {8, 11},
    {9, 8},
    {9, 10},
    {9, 11},
    {10, 8},
    {10, 9},
    {10, 11},
    {11, 8}
  },
  0,
  0,
  500,
  1,
  A0
};

candleStruct candle2 = {
  {12, 13, 14, 15},
  { {12, 13},
    {12, 14},
    {12, 15},
    {13, 12},
    {13, 14},
    {13, 15},
    {14, 12},
    {14, 13},
    {14, 15},
    {15, 12}
  },
  0,
  0,
  500,
  1,
  A1
};

candleStruct candle3 = {
  {16, 17, 18, 19},
  { {16, 17},
    {16, 18},
    {16, 19},
    {17, 16},
    {17, 18},
    {17, 19},
    {18, 16},
    {18, 17},
    {18, 19},
    {19, 16}
  },
  0,
  0,
  500,
  1,
  A2
};

candleStruct candle4 = {
  {20,21,22,23},
  { {20, 21},
    {20, 22},
    {20, 23},
    {21, 20},
    {21, 22},
    {21, 23},
    {22, 20},
    {22, 21},
    {22, 23},
    {23, 20}
  },
  0,
  0,
  500,
  1,
  A3
};

candleStruct candle5 = {
  {24,25,26,27},
  { {24, 25},
    {24, 26},
    {24, 27},
    {25, 24},
    {25, 26},
    {25, 27},
    {26, 24},
    {26, 25},
    {26, 27},
    {27, 24}
  },
  0,
  0,
  500,
  1,
  A4
};
candleStruct candle6 = {
  {28,29,30,31},
  { {28, 29},
    {28, 30},
    {28, 31},
    {29, 28},
    {29, 30},
    {29, 31},
    {30, 28},
    {30, 29},
    {30, 31},
    {31, 28}
  },
  0,
  0,
  500,
  1,
  A5
};
candleStruct candle7 = {
  {32,33,34,35},
  { {32, 33},
    {32, 34},
    {32, 35},
    {33, 32},
    {33, 34},
    {33, 35},
    {34, 32},
    {34, 33},
    {34, 35},
    {35, 32}
  },
  0,
  0,
  500,
  1,
  A6
};
candleStruct candle8 = {
  {36,37,38,39},
  { {36, 37},
    {36, 38},
    {36, 39},
    {37, 36},
    {37, 38},
    {37, 39},
    {38, 36},
    {38, 37},
    {38, 39},
    {39, 36}
  },
  0,
  0,
  500,
  1,
  A7
}; 

candleStruct allCandles[NUM_CANDLES] = {candle0, candle1, candle2, candle3, candle4, 
                                                 candle5, candle6, candle7, candle8};


void setup() {
  // set pins
  for (int i = 0; i < NUM_CANDLES; i++) {
    pinMode(allCandles[i].ldrPin, INPUT);
  }
  pinMode(lighterPin, INPUT_PULLUP);

  chanukiahState = WAITING;

  Serial.begin(96500); // for debugging 
}

void loop() {
  //Serial.println("chanukiahState: " + String(chanukiahState));

  switch (chanukiahState) {
    case WAITING:
      if (digitalRead(lighterPin) == LOW) {
        //light the shamash!
        chanukiahState = LIGHTING;
        allCandles[0].height = LEDS_PER_CANDLE;
      }
      break;

    case LIGHTING:

      bool stillLighting = 0;
      for (int i = 1; i < NUM_CANDLES ; i++) { //yeah it's supposed to start at 1. 0 is the shamash
        int currentLdrPin = allCandles[i].ldrPin;
        if (allCandles[i].height == 0 && currentLdrPin != DISABLE_LDR) {
          stillLighting = 1;
          // if the candle's not lit yet, check its ldr
            int ldrVal = analogRead(allCandles[i].ldrPin);
            //  Serial.println("candle:"+ String(i)+ " pin: " + String(allCandles[i].ldrPin)+ " ldrVal: " + String(ldrVal));
            if (ldrVal < 200) {
              //when each candle is lit, initialize its height
              allCandles[i].height = LEDS_PER_CANDLE;
              allCandles[i].ldrPin = DISABLE_LDR; //special value so we don't read the ldr again after we've lit a candle once
            }
          
        }
      }
      if (!stillLighting) {
        chanukiahState = BURNING;
      }
      break;
      //no special actions to do for state BURNING 
    default:
      Serial.println("how'd you get into the default case?");
      break;

  }

  /* candle burning */

  for (uint8_t i = 0; i < LED_PIN_COUNT; i++) {
    for (uint8_t j = 0; j < LED_PIN_COUNT; j++) {
      if (i != j) {
        //loop *here* over all candles so we can limit the scan-delay-ing-
        // turn on i,j for each candle, then delay, then off for each candle
        for (int k = 0; k < NUM_CANDLES; k++) {
          //first, flicker the top

          int currentTopC = allCandles[k].ledPins[allCandles[k].height - 1][1];
          int currentTopA = allCandles[k].ledPins[allCandles[k].height - 1][0];
          //    Serial.println("currentTopA: " + String(currentTopA) + " currentTopC: " + String(currentTopC)
          //    + " height: "+ String(allCandles[k].height));
          if ((allCandles[k].setLedPins[i] == currentTopC && allCandles[k].setLedPins[j] == currentTopA)
              && allCandles[k].height > 0) {
            //light the top light!
            if (allCandles[k].flickerState == LOW) {
              pinMode(allCandles[k].setLedPins[i], INPUT);
              pinMode(allCandles[k].setLedPins[j], INPUT);
            } else {
              //you gotta charlie-light it, you can't just turn it to on- itll mess up the later charlie-lights
              pinMode(allCandles[k].setLedPins[i], OUTPUT);
              pinMode(allCandles[k].setLedPins[j], OUTPUT);
              digitalWrite(allCandles[k].setLedPins[i], LOW);
              digitalWrite(allCandles[k].setLedPins[j], HIGH);
              delay(SCAN_DELAY);
              pinMode(allCandles[k].setLedPins[i], INPUT);
              pinMode(allCandles[k].setLedPins[j], INPUT);
            }

            //update flicker state if necessary
            unsigned long currentMillis = millis();
            if (currentMillis - allCandles[k].prevMillis >= allCandles[k].flickerDuration) {
              //time to switch!
              allCandles[k].prevMillis = currentMillis;
              allCandles[k].flickerDuration = (random(4) + 1) * 100;
              allCandles[k].flickerState = !allCandles[k].flickerState;
              if (allCandles[k].height == 1 ) {
                //Sputter if candle is low; ON is shorter, OFF is more varied + possibly longer 
                if (allCandles[k].flickerState) {
                  allCandles[k].flickerDuration = (random(3) + 1) * 10;
                } else {
                  allCandles[k].flickerDuration = (random(10) + 1) * 100;
                }
              }
            }
          } // end "light the top light" loop

          //next, charlieplex-turn all the rest of the leds in the candle on
          int currentC = allCandles[k].setLedPins[i];
          int currentA = allCandles[k].setLedPins[j];
          int ledIdx = findPair(currentA, currentC, allCandles[k].ledPins);
          if (ledIdx < allCandles[k].height - 1 ) {
            pinMode(allCandles[k].setLedPins[i], OUTPUT);
            pinMode(allCandles[k].setLedPins[j], OUTPUT);
            digitalWrite(allCandles[k].setLedPins[i], LOW);
            digitalWrite(allCandles[k].setLedPins[j], HIGH);
          }
        }
        delay(SCAN_DELAY);

        for (int k = 0; k < NUM_CANDLES; k++) {
          //turn the led off
          pinMode(allCandles[k].setLedPins[i], INPUT);
          pinMode(allCandles[k].setLedPins[j], INPUT);
        }
      } //end "i != j" action loop
    } //end "iterate over 4 pins" inner loop
  } //end "iterate over 4 pins" outer loop


  //roll to drip
  unsigned long currentMillisDrip = millis();
  if (currentMillisDrip - prevMillisDrip > 1000 ){
    prevMillisDrip = currentMillisDrip; 
  for (int i = 0; i < NUM_CANDLES; i++) {
    if (allCandles[i].height > 0) {
      int dripRoll = random(DRIP_ROLL_MAX);
      int dripMin = DRIP_CUTOFF_REG; 
      //linger a bit on the last LED
      if (allCandles[i].height == 1) {
        dripMin = DRIP_CUTOFF_LAST; 
      }
      if (dripRoll < dripMin) {
        allCandles[i].height--;
      }
    }
  }
  }

}

int findPair(uint8_t valA, uint8_t valC, uint8_t pairList[LEDS_PER_CANDLE][2]) {
  //findPair(valA,valC,pairList): finds index of the pair (valA,valC) in pairList, if it exists
  //"A" and "C" in valA and valC refer to intended use for finding anode/cathode pairs
  int pairIdx = -1;
  int matchesA[LEDS_PER_CANDLE];
  int matchesC[LEDS_PER_CANDLE];
  for (int i = 0; i < LEDS_PER_CANDLE; i++ ) {
    matchesA[i] = 0;
    matchesC[i] = 0;
  }

  for (int i = 0; i < LEDS_PER_CANDLE; i++) {
    if (pairList[i][0] == valA && pairList[i][1] == valC) {
      pairIdx = i;
    }
  }

  return pairIdx;
}
