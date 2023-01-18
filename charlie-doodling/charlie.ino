#define SCAN_DELAY 1
#define LEDS_PER_CANDLE 10


const uint8_t LED_PIN_COUNT = 4;
const uint8_t NUM_CANDLES = 2;


struct candleStruct {
  uint8_t allLedPins[LED_PIN_COUNT];
  uint8_t ledPins[LEDS_PER_CANDLE][2]; //format: {anodePin,cathodePin}
  uint8_t height;
  //flicker timer stuff
  unsigned long prevMillis;
  int flickerDuration;
  bool flickerState;
  //photoresistor pin (if necessary)
  uint8_t ldrPin;
} ;

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
  10,
  0,
  500,
  A1
};

candleStruct allCandles[NUM_CANDLES] = {candle1,candle2};//{allCandles[k]};

void setup() {
  Serial.begin(96500); // Any baud rate should work
}

void loop() {
  for (uint8_t i = 0; i < LED_PIN_COUNT; i++) {
    for (uint8_t j = 0; j < LED_PIN_COUNT; j++) {
      if (i != j) {
        //loop *here* over all candles so we can limit the scan-delay-ing-
        // turn on i,j for each candle, then delay, then off for each candle
        for (int k = 0; k < NUM_CANDLES; k++) {
          //first, flicker the top

          int currentTopC = allCandles[k].ledPins[allCandles[k].height - 1][1];
          int currentTopA = allCandles[k].ledPins[allCandles[k].height - 1][0];
          //Serial.println("currentTopA: " + String(currentTopA) + " currentTopC: " + String(currentTopC));
          if (allCandles[k].allLedPins[i] == currentTopC && allCandles[k].allLedPins[j] == currentTopA) {
            //light the top light!
            if (allCandles[k].flickerState == LOW) {
              pinMode(allCandles[k].allLedPins[i], INPUT);
              pinMode(allCandles[k].allLedPins[j], INPUT);
            } else {
              //you gotta charlie-light it, you can't just turn it to on- itll mess up the later charlie-lights
              pinMode(allCandles[k].allLedPins[i], OUTPUT);
              pinMode(allCandles[k].allLedPins[j], OUTPUT);
              digitalWrite(allCandles[k].allLedPins[i], LOW);
              digitalWrite(allCandles[k].allLedPins[j], HIGH);
              delay(SCAN_DELAY);
              pinMode(allCandles[k].allLedPins[i], INPUT);
              pinMode(allCandles[k].allLedPins[j], INPUT);
            }

            //update flicker state if necessary
            unsigned long currentMillis = millis();
            if (currentMillis - allCandles[k].prevMillis >= allCandles[k].flickerDuration) {
              //time to switch!
              allCandles[k].prevMillis = currentMillis;
              allCandles[k].flickerDuration = (random(4) + 1) * 100;
              allCandles[k].flickerState = !allCandles[k].flickerState;
            }

          }


          //next, charlieplex-turn all the rest of the leds in the candle on

          int currentC = allCandles[k].allLedPins[i];
          int currentA = allCandles[k].allLedPins[j];
          int ledIdx = findPair(currentA, currentC, allCandles[k].ledPins);
          if (ledIdx < allCandles[k].height - 1 ) {
            pinMode(allCandles[k].allLedPins[i], OUTPUT);
            pinMode(allCandles[k].allLedPins[j], OUTPUT);
            digitalWrite(allCandles[k].allLedPins[i], LOW);
            digitalWrite(allCandles[k].allLedPins[j], HIGH);
          }
        }
        delay(SCAN_DELAY);

        for (int k = 0; k < NUM_CANDLES; k++) {
          //turn the led off
          pinMode(allCandles[k].allLedPins[i], INPUT);
          pinMode(allCandles[k].allLedPins[j], INPUT);
        }
      } //end "i != j" action loop 
    } //end "iterate over 4 pins" inner loop
  } //end "iterate over 4 pins" outer loop
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