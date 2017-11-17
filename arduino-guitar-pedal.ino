#include <SPI.h>
#include "TimerOne.h"

//for setting bits
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

//max value we can output (0-1023)
#define OUTPUT_MAX 1023.0

//for getting sound input
#define INPUT_CHANNEL 1

//You can use 3 of the 4 knobs (top left is gain for input)
#define TOP_RIGHT 2
#define BOT_LEFT  5
#define BOT_RIGHT 4

void setup() {
  Serial.begin(115200);
  
  // set adc prescaler  to 64 for 19kHz sampling frequency on input
  cbi(ADCSRA, ADPS2);
  sbi(ADCSRA, ADPS1);
  sbi(ADCSRA, ADPS0);

  // Timer2 controls PWM pins 3 & 11
  // Timer2 PWM Mode set to fast PWM 
  cbi (TCCR2A, COM2A0);
  sbi (TCCR2A, COM2A1);
  sbi (TCCR2A, WGM20);
  sbi (TCCR2A, WGM21);
  cbi (TCCR2B, WGM22);

  // Timer2 Clock Prescaler to : 1 
  sbi (TCCR2B, CS20);
  cbi (TCCR2B, CS21);
  cbi (TCCR2B, CS22);

  //setup output pins
  pinMode(3, OUTPUT);
  pinMode(11, OUTPUT);
}

short readKnob(int k){
  return 1023 - (short)analogRead(k);
}

//output using 2 channels for 10-bit output instead of 8-bit (still terrible)
void output(int value) {
  analogWrite(3, value >> 2);
  analogWrite(11, (value & B11) << 6);
}

short last_in = 0;
void loop() {
  short in = analogRead(INPUT_CHANNEL);

  //apply a low pass based on top right knob
  short knob = readKnob(TOP_RIGHT);
  float val = (float)knob / OUTPUT_MAX;
  int out1 = in * (1.0 - val) + last_in * val;
  last_in = in;

  knob = readKnob(BOT_LEFT);
  float inc = (float)knob / 10.0;
  int out2 = triangle_wave(0, inc);

  knob = readKnob(BOT_RIGHT);
  inc = (float)knob / 10.0;
  int out3 = triangle_wave(1, inc);

  //output the average
  output((out2+out3)/2);
}

float triangle_vals [] = {0.0, 0.0, 0.0};
int triangle_wave(short buf, float inc){
  float triangle_val = triangle_vals[buf] + inc;
  if(triangle_val > OUTPUT_MAX){
    triangle_val -= OUTPUT_MAX;
  }
  triangle_vals[buf] = triangle_val;
  return (int) triangle_val;
}
