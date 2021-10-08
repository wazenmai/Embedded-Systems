/*
 File/Sketch Name: AudioFrequencyDetector

 Version No.: v1.0 Created 12 December, 2019
 
 Original Author: Clyde A. Lettsome, PhD, PE, MEM
 
 Description:  This code/sketch makes displays the approximate frequency of the loudest sound detected by a sound detection module. For this project, the analog output from the 
 sound module detector sends the analog audio signal detected to A0 of the Arduino Uno. The analog signal is sampled and quantized (digitized). A Fast Fourier Transform (FFT) is
 then performed on the digitized data. The FFT converts the digital data from the approximate discrete-time domain result. The maximum frequency of the approximate discrete-time
 domain result is then determined and displayed via the Arduino IDE Serial Monitor.

 Note: The arduinoFFT.h library needs to be added to the Arduino IDE before compiling and uploading this script/sketch to an Arduino.

 License: This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License (GPL) version 3, or any later
 version of your choice, as published by the Free Software Foundation.

 Notes: Copyright (c) 2019 by C. A. Lettsome Services, LLC
 For more information visit https://clydelettsome.com/blog/2019/12/18/my-weekend-project-audio-frequency-detector-using-an-arduino/

*/

#include "arduinoFFT.h"
#include <stdarg.h>
#include <Keypad.h>
#include <Servo.h>

#define SAMPLES 128             //SAMPLES-pt FFT. Must be a base 2 number. Max 128 for Arduino Uno.
#define SAMPLING_FREQUENCY 2048 //Ts = Based on Nyquist, must be 2 times the highest expected frequency.

/* Debug utilities */
#define DEBUG_MODE
void DEBUG(const char* argTypes, ...) {
#ifdef DEBUG_MODE
  va_list vl;
  va_start(vl, argTypes);
  for (int i = 0; argTypes[i] != '\0'; i++) {
    switch(argTypes[i]) {
      case 'i': {
        int v = va_arg(vl, int);
        Serial.print(v);
        break;
      }
      case 's': {
        char* s = va_arg(vl, char*);
        Serial.print(s);
        break;
      }
      default:
        break;
    }

    if (i != 0) Serial.print(", ");
  }
  Serial.println("");
  va_end(vl);
// #else
// NOP
#endif
}
 
arduinoFFT FFT = arduinoFFT();
 
unsigned int samplingPeriod;
unsigned long microSeconds;

// Servo
Servo servo_SG90;
Servo servo_MG995;

int8_t angle = 10;

const int buttonPin = 2;
int buttonState = 0;

int8_t record[16];
int8_t flag = 0;
int8_t record_id = 0;
 
double vReal[SAMPLES]; //create vector of size SAMPLES to hold real values
double vImag[SAMPLES]; //create vector of size SAMPLES to hold imaginary values

// SG90
void knock() {
  servo_SG90.write(80);
  for(angle = 82; angle >= 71; angle--) {                                  
    servo_SG90.write(angle);               
    delay(10);              
  }
  for(angle = 71; angle <= 82; angle++) {                                  
    servo_SG90.write(angle);               
    delay(10);                  
  }
}

// MG995
void change(int8_t note) {
  if (note == 1) {
    servo_MG995.write(0);
  } else if (note == 2) {
    servo_MG995.write(22);
  } else if (note == 3) {
    servo_MG995.write(40);
  } else if (note == 4) {
    servo_MG995.write(54);
  } else if (note == 5) {
    servo_MG995.write(64);
  } else if (note == 6) {
    servo_MG995.write(80);
  } else if (note == 7) {
    servo_MG995.write(98);
  } else if (note == 8) {
    servo_MG995.write(111);
  }
}

// play xylophone
void play(int8_t note, int d_time) {
  if (note == 0) {
    delay(200 + d_time);
    return;
  }
  change(note);
  delay(200);
  knock();
  delay(100);
}
 
void setup() {
  Serial.begin(9600); //Baud rate for the Serial Monitor

  pinMode(buttonPin, INPUT);

  servo_SG90.attach(4);
  servo_MG995.attach(3);
  servo_MG995.write(0);

  record_id = 0;
  samplingPeriod = round(1000000 * (1.0 / SAMPLING_FREQUENCY)); //Period in microseconds
  for (int i = 0; i < 16; i++) {
    play(1, 100);
  }
}
 
void loop() {  
  buttonState = digitalRead(buttonPin);

  if (buttonState == HIGH) {
      /*Sample SAMPLES times*/
    for(int i = 0; i < SAMPLES; i++) {
        microSeconds = micros();
        
        // Reads the value from analog pin 0 (A0), quantize it and save it as a real term.
        vReal[i] = analogRead(0); 
        //Makes imaginary term 0 always
        vImag[i] = 0; 

        /*remaining wait time between samples if necessary*/
        while(micros() < (microSeconds + samplingPeriod)) {};
    }

    /*Perform FFT on samples*/
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

    /*Find peak frequency and print peak*/
    double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
  //      Serial.println(peak);     //Print out the most dominant frequency.

    /*Script stops here. Hardware reset required.*/
    if (record_id < 16) {
      if ((peak >= 46) && (peak <= 51)) {
        DEBUG("sii", "record ", record_id, 1);
        record[record_id] = 1;
        record_id += 1;
        delay(100);
      } else if ((peak >= 174) && (peak <= 178)) {
        DEBUG("sii", "record ", record_id, 2);
        record[record_id] = 2;
        record_id += 1;
        delay(100);
      } else if ((peak >= 468) && (peak <= 472)) {
        DEBUG("sii", "record ", record_id, 3);
        record[record_id] = 3;
        record_id += 1;
        delay(100);
      } else if ((peak >= 678) && (peak <= 682)) {
        DEBUG("sii", "record ", record_id, 4);
        record[record_id] = 4;
        record_id += 1;
        delay(100);
      } else if ((peak >= 993) && (peak <= 997)) {
        DEBUG("sii", "record ", record_id, 5);
        record[record_id] = 5;
        record_id += 1;
        delay(100);
      }
    }
    delay(200);
  } else {
    for (int8_t j = 0; j < 5; j++) {
      for (int8_t i = 0; i < record_id; i++) {
        DEBUG("sii", "replay ", i, record[i]);
        play(record[i], 200);
      }
      delay(3000);
    }
    record_id = 0;
  }
  delay(100);   
}
