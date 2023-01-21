/*
  flame-free Shabbat candles!
  Perfect for pets, apartments, and any other situation where fire is a bad idea.
  Need some more candles, for a holiday perhaps? check out my chanukiah at https://wokwi.com/projects/353600148292695041 !


  To use:
    1. click green ">" button to start the program running.
    2. click yellow circle button to light the shabbat candles.
    3. enjoy watching the candles burn!
    4. once all the candles have burned out, press the gray square STOP button
       or refresh your browser window to reset the simulation.

  ***How to change candle colors:
      in the "diagram.json" tab, find the "wokwi-led-bar-graph" elements. these are the candles!
      for the candle you want, put in the hex code for your desired color, ex. "#FFFF00"
      ***special candle color gradient codes:
         "BCYR": blue, cyan, yellow, red
         "GYR": green, yellow, red

  *** How to change burn duration:
      use #define DRIP_ROLL_MAX, DRIP_CUTOFF_REG, DRIP_CUTOFF_LAST to change the burn duration!
      defaults (1080,7,2) should burn for about 20-30 minutes.
      DRIP_ROLL_MAX: increase this to make candles burn for longer overall.
      DRIP_CUTOFF_REG: increase this to make candles to burn more unevenly.
      DRIP_CUTOFF_LAST: decrease this (don't go less than 1) to make the last nub of a candle last longer.

  Implementation info:
    this code uses Charlieplexing to light a large number of LEDs using relatively few pins!
    read more about Charlieplexing here: https://en.wikipedia.org/wiki/Charlieplexing

  by G Rosen
*/

#define LEDS_PER_CANDLE 10
#define LED_PIN_COUNT 4
#define NUM_CANDLES 2
#define SCAN_DELAY 1
#define DISABLE_LDR 99

#define DRIP_ROLL_MAX 1080
#define DRIP_CUTOFF_REG 7
#define DRIP_CUTOFF_LAST 2

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
unsigned long prevMillisDrip = 0;

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
  0,
  0,
  500,
  1,
  A1
};

candleStruct allCandles[NUM_CANDLES] = {candle1, candle2};

void setup() {
  // set pins
  for (int i = 0; i < NUM_CANDLES; i++) {
    pinMode(allCandles[i].ldrPin, INPUT);
  }
  pinMode(lighterPin, INPUT_PULLUP);

  //Serial.begin(96500); // Any baud rate should work
}

void loop() {

    if (digitalRead(lighterPin) == LOW) {
      //light the candles!
      allCandles[0].height = LEDS_PER_CANDLE;
      allCandles[1].height = LEDS_PER_CANDLE;
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
  if (currentMillisDrip - prevMillisDrip > 2000 ) {
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