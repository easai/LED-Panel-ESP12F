#include <MD_Parola.h>
#include <MD_MAX72xx.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 12  // Chip Select

const int BUZZ = 4;   // buzzer pin
const int SW   = 5;   // switch pin

volatile bool beepEnabled = true;
volatile bool toggleRequest = false;

void IRAM_ATTR handleSwitch() {
  toggleRequest = true;   // set a flag, do NOT do heavy work here
}

MD_Parola ledMatrix = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

void setup() {
  pinMode(BUZZ, OUTPUT);
  pinMode(SW, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(SW), handleSwitch, FALLING);

  ledMatrix.begin();
  ledMatrix.setIntensity(10);  // Brightness: 0–15
  ledMatrix.displayClear();
}

void loop() {
  if (toggleRequest) {
    toggleRequest = false;
    beepEnabled = !beepEnabled;
  }

  if (beepEnabled) {
    ledMatrix.print("Error!!!");
    tone(BUZZ, 2000);
  } else {
    ledMatrix.print("UP");
    noTone(BUZZ);
  }

  delay(10);  // debounce smoothing
}
