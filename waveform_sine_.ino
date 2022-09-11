#include <Esp.h>
#include <esp8266_peri.h>
#include <eagle_soc.h>
#define WDT_CTL 0x60000900
#define WDT_CTL_ENABLE (BIT(0))

// Configurable from 0 to 15; 16 not available
// Please refer to your breakout board datasheet for pin mapping
// WARNING: some pins are used internally for connecting the ESP to ROM chip;
// DO NOT USE THEM or your ESP will be bricked
#define PINOUT D0

double freq; // Hz
double offset; // percent (0.0 to 1.0)
double width; // percent (0.0 to 1.0)

// unit: microsecond
unsigned long cycle_time;
unsigned long raising_edge;
unsigned long falling_edge;
unsigned long prev_micros;

// compare 2 unsigned value
// true if X > Y while for all possible (X, Y), X - Y < Z
#define TIME_CMP(X, Y, Z) (((X) - (Y)) < (Z))

inline void setHigh() {
  GPOS = (1 << PINOUT);
}

inline void setLow() {
  GPOC = (1 << PINOUT);
}

void setup() {
  // disable hardware watchdog
  CLEAR_PERI_REG_MASK(WDT_CTL, WDT_CTL_ENABLE);
  // disable software watchdog
  ESP.wdtDisable();
  // set IO pin mode
  pinMode(PINOUT, OUTPUT);

  // calculate arguments
  freq = 500;
  width = 0.5;
  offset = 5.0;

  cycle_time = 1000000 / freq;
  raising_edge = (unsigned long)(offset * cycle_time) % cycle_time;
  falling_edge = (unsigned long)((offset + width) * cycle_time) % cycle_time;
    
  prev_micros = micros();

  // do pinout shifting
  while(1) {
    if (width + offset < 1) {
      // raising edge should appear earlier
      while (TIME_CMP(micros(), prev_micros + raising_edge, cycle_time)); setHigh();
      while (TIME_CMP(micros(), prev_micros + falling_edge, cycle_time)); setLow();
    } else {
      // falling edge should appear earlier
      while (TIME_CMP(micros(), prev_micros + falling_edge, cycle_time)); setLow();
      while (TIME_CMP(micros(), prev_micros + raising_edge, cycle_time)); setHigh();
    }
    prev_micros += cycle_time;
  }
}

void loop() {
  // it won't ever get there;
  // however if this function is missing
  // the ESP8266 Arduino refuse to compile
}
