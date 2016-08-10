#include <EEPROM.h>

byte seven_seg_digits[10][7] = { 
  { 1, 1, 1, 1, 1, 1, 0 }, // = 0
  { 0, 1, 1, 0, 0, 0, 0 }, // = 1
  { 1, 1, 0, 1, 1, 0, 1 }, // = 2
  { 1, 1, 1, 1, 0, 0, 1 }, // = 3
  { 0, 1, 1, 0, 0, 1, 1 }, // = 4
  { 1, 0, 1, 1, 0, 1, 1 }, // = 5
  { 1, 0, 1, 1, 1, 1, 1 }, // = 6
  { 1, 1, 1, 0, 0, 0, 0 }, // = 7
  { 1, 1, 1, 1, 1, 1, 1 }, // = 8
  { 1, 1, 1, 0, 0, 1, 1 } // = 9
};

const int LED = 13;     // the pin for the debug LED
const int BUTTON1 = 11; // the input pin for patch change next
const int BUTTON2 = 12; // the input pin for patch change previous

// state varables:

int ledState = LOW;         // the current state of the output pin

int buttonState1;             // the current reading from the input pin
int lastButtonState1 = LOW;   // the previous reading from the input pin
long lastDebounceTime1 = 0;  // the last time the output pin was toggled

int buttonState2;             // the current reading from the input pin
int lastButtonState2 = LOW;   // the previous reading from the input pin
long lastDebounceTime2 = 0;  // the last time the output pin was toggled

// config varables
long debounceDelay = 50;    // the debounce time; increase if the output flickers

// patch is the curret MIDI patch
int patch = 0;

// memory location where we save current patch number in case of power down
int addr = 0;


void setupPins() {

  // setup 7 segment display:
  //for (byte i = 2; i <= 9; i++) {
    //pinMode(i, OUTPUT);
  //}

  // setup the two input buttons:
  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);

  // setup the LED output, this is used just for debugging
  pinMode(LED, OUTPUT);

}

void setupOutputs() {

  // turn on the LED
  digitalWrite(LED, HIGH);

  //sevenSegWrite(patch);

  // turn the dot off the 7 segment display which is attached to pin 9
  //digitalWrite(9, 0);

  // setup MIDI out at the correct baud rate
  Serial.begin(31250);

}

byte readPatchFromMemory() {
  // read the current patch # from memory
  byte patchFromMemory = EEPROM.read(addr);

  // if for some strange reason it's outside of boundry, reset it
  if (patchFromMemory > 128) {
    patchFromMemory = 128;
  }
  if (patchFromMemory < 1) {
    patchFromMemory = 1;
  }
  return patchFromMemory;
}

void setup() {

  patch = readPatchFromMemory();

  setupPins();

  setupOutputs();

}


void sevenSegWrite(byte digit) {
  byte pin = 2;
  for (byte segCount = 0; segCount < 7; ++segCount) {
    digitalWrite(pin, seven_seg_digits[digit][segCount]);
    ++pin;
  }
}

void turnOffSevenSeg() {
  for (byte i = 2; i <= 9; i++) {
    digitalWrite(i, LOW);
  }
}

void changePatch() {

  // indicate a patch change event
  Serial.write(192);

  // indicate the new patch. Patch #1 is MIDI 0
  Serial.write(patch - 1);

  // blink the power light
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(50);              // wait for a second
  digitalWrite(13, HIGH);    // turn the LED off by making the voltage LOW
  delay(50);              // wait for a second
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(50);              // wait for a second
  digitalWrite(13, HIGH);    // turn the LED off by making the voltage LOW


  // update 7 segment display
  //sevenSegWrite(patch);

  // save patch number to memory in case of power down
  EEPROM.write(addr, patch);

  //digitalWrite(LED, ledState);

  //if (ledState ==1 ) { ledState = 0; }  else { ledState = 1; }

  //digitalWrite(LED, ledState);
}

void loop() {

  int reading = digitalRead(BUTTON1);

  if (reading != lastButtonState1) {
    // reset the debouncing timer
    lastDebounceTime1 = millis();
  }
  if ((millis() - lastDebounceTime1) > debounceDelay) {
    if (reading != buttonState1) {
      buttonState1 = reading;

      if (buttonState1 == HIGH) {

        patch++;
        if (patch > 128) {
          patch = 1;
        }
        changePatch();
      }
    }
  }

  lastButtonState1 = reading;

  
  reading = digitalRead(BUTTON2);

  if (reading != lastButtonState2) {
    // reset the debouncing timer
    lastDebounceTime2 = millis();
  }
  if ((millis() - lastDebounceTime2) > debounceDelay) {
    if (reading != buttonState2) {
      buttonState2 = reading;

      if (buttonState2 == HIGH) {

        if (patch == 1) {
          patch = 128;
        } else {
          patch--;
        }
        changePatch();

      }
    }
  }

  lastButtonState2 = reading;


}
