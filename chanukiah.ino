/**
  Birdie-safe menorah!
  by Ghost Rosen
*/
#define PINS_PER_CANDLE 10
#define NUM_CANDLES 1 //8 someday lol

#define WAITING 1
#define LIGHTING 2
#define BURNING 3

//Pins n shit
//digital version: const uint8_t ldrPins[] = {2,3,4,5,6,7,8,9};
const uint8_t ldrPins[] = {A0};
const uint8_t ledPins[NUM_CANDLES + 1][PINS_PER_CANDLE] =
{ {4,5,6,7,8,9,10,11,12,13}, //shamash
  {14,15,16,17,18,19,20,21,22,23}
};
const uint8_t lighterPin = 2;

int chanukiahState = 0;
/*
  const uint8_t ledPins[9][PINS_PER_CANDLE] =
  {{10,11},  //shamash
  {12,13},
  {14,15},
  {16,17},
  {18,19},
  {20,21},
  {22,23},
  {24,25},
  {26,27},
  };
*/

int candleHeights[NUM_CANDLES + 1];


void setup() {
  // set pins
  for (int i = 0; i < (NUM_CANDLES + 1); i++) { //it's +1 for the shamash
    for (int j = 0; j < PINS_PER_CANDLE; j++) {
      pinMode(ledPins[i][j], OUTPUT);
    }
  }

  for (int i = 0; i < NUM_CANDLES; i++) {
    pinMode(ldrPins[i], INPUT);
  }

  pinMode(lighterPin, INPUT_PULLUP);


  //  initialize candle heights
  for (int i = 0; i < NUM_CANDLES + 1; i++) {
    candleHeights[i] = 0; //PINS_PER_CANDLE;
  }

  chanukiahState = WAITING;
  Serial.begin(96500); // Any baud rate should work
}

void loop() {
  Serial.println("chanukiahState: " + String(chanukiahState));

  switch (chanukiahState) {
    case WAITING:
      if (digitalRead(lighterPin) == LOW) {
        //light the shamash!
        chanukiahState = LIGHTING;
        candleHeights[0] = PINS_PER_CANDLE;
      }
      break;
    case LIGHTING:
      bool stillLighting = 0;
      for (int i = 1; i < NUM_CANDLES + 1; i++) { //yeah it's supposed to start at 1. 0 is the shamash
        if (candleHeights[i] == 0) {
          stillLighting = 1;
          // if the candle's not lit yet, check its ldr
          int ldrVal = analogRead(ldrPins[i - 1]);
          Serial.println("ldrVal: " + String(ldrVal));
          if (ldrVal < 200) {
            //when each candle is lit, initialize its height in candleHeights
            candleHeights[i] = PINS_PER_CANDLE;
          }
        }
      }
      if (!stillLighting) {
        chanukiahState = BURNING;
      }
      break;
    case BURNING:

      break;
    default:
      Serial.println("how'd you get into the default case?");
      break;

  }
  if (digitalRead(lighterPin) == LOW) {
    chanukiahState = LIGHTING;
  }

  /* candle burning */

  for (int i = 0; i < NUM_CANDLES + 1; i++) {
    if (candleHeights[i] > 0) {
      burn(i);
    }

    //roll to drip
    //TODO: put this on a bigger interval. millis()? 

    int dripRoll = random(1800);
    if (dripRoll < 10) {
      int top = candleHeights[i] - 1;
      int pin = ledPins[i][top];
      digitalWrite(pin, LOW);
      candleHeights[i]--;
    }
  }

  delay(100);
}

void burn(int candle) {
  /* void burn(severity): burn with a flicker at the top like the flame of a candle!
    severity: number in the range [0,3] signifying how strong the flicker is
  */
  int top = candleHeights[candle] - 1;
  int pin = ledPins[candle][top];
  int severity = 1; //placeholder

  bool isOn = random(severity + 1);
  //Serial.println("top: " + String(top) + " isOn: " + String(isOn)  + " pin: " + String(pin));

  if (isOn) {
    digitalWrite(pin, HIGH);
  } else {
    digitalWrite(pin, LOW);
  }

  top--;
  while (top >= 0) {
    pin = ledPins[candle][top];
    digitalWrite(pin, HIGH);
    top--;
  }
  /* original borrowed code
    analogWrite(ledPin3, random(120)+135);
    delay(random(100);
  */


}

void sputter(int candle) {
  /* void sputter(candle): sputter the candle like it's about to go out
  */
  int pin = ledPins[candle][0];
  int sputterVal = random(18) - random(36);
  Serial.println("sputterVal: " + String(sputterVal));

  if (sputterVal > 0) {
    digitalWrite(pin, HIGH);
  } else {
    digitalWrite(pin, LOW);
  }

}

