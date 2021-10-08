#include <Arduino_FreeRTOS.h>
#include <LiquidCrystal_I2C.h>
#include <stdarg.h>
#include <Servo.h>
#include <Keypad.h>
#include <Key.h>

#define SAMPLES 64               // Max 128 for Arudino Uno.
#define SAMPLING_FREQUENCY 2048  // Must be 2 times the highest expected frequency
#define KEY_ROWS 4
#define KEY_COLS 4

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

// Servo
Servo servo_SG90;
Servo servo_MG995;

int8_t angle = 10;

// LCD
LiquidCrystal_I2C lcd(0x3F, 16, 2);

int8_t pos_x;

// Keypad
char keymap[KEY_ROWS][KEY_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte colPins[KEY_COLS] = {9, 8, 7, 6};
byte rowPins[KEY_ROWS] = {13, 12, 11, 10};

Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, KEY_ROWS, KEY_COLS);

// song
const int8_t star[48] = {1, 1, 5, 5, 6, 6, 5, 0, 4, 4, 3, 3, 2, 2, 1, 0, 5, 5, 4, 4, 3, 3, 2, 0, 5, 5, 4, 4, 3, 3, 2, 0, 1, 1, 5, 5, 6, 6, 5, 0, 4, 4, 3, 3, 2, 2, 1, 0};
const int8_t bee[64] = {5, 3, 3, 0, 4, 2, 2, 0, 1, 2, 3, 4, 5, 5, 5, 0, 5, 3, 3, 0, 4, 2, 2, 0, 1, 3, 5, 5, 3, 0, 0, 0, 2, 2, 2, 2, 2, 3, 4, 0, 3, 3, 3, 3, 3, 4, 5, 0, 5, 3, 3, 0, 4, 2, 2, 0, 1, 3, 5, 5, 1, 0, 0, 0};
const int8_t story[80] = {1, 0, 2, 3, 0, 1, 0, 3, 0, 1, 0, 3, 0, 0, 2, 0, 3, 4, 4, 3, 2, 4, 0, 3, 0, 4, 5, 0, 3, 0, 5, 0, 3, 0, 5, 0, 4, 0, 5, 6, 6, 5, 4, 6, 0, 5, 0, 1, 2, 3, 4, 5, 6, 0, 6, 0, 2, 3, 4, 5, 6, 7, 0, 7, 0, 3, 4, 5, 6, 7, 8, 0, 8, 7, 6, 4, 7, 5, 8, 0};
int8_t record[16];
int8_t record_id;

char key;
uint8_t i;
int8_t mode = 1;

/* Function */

// SG90
void knock() {
  servo_SG90.write(82);
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
  lcd.setCursor(pos_x, 0);
  if (pos_x == 15) {
    pos_x = 0;
    lcd.clear();
  } else {
    pos_x += 1;
  }
  lcd.print(note);
  delay(d_time);
}

void controlCenter() {
  key = myKeypad.getKey();

  // test
  switch(key) {
    case '0': 
      mode = 0;
      lcd.clear();
      lcd.print("TEST");
      delay(1000);
      lcd.clear();
      break;
    case 'A': 
      mode = 1;
      pos_x = 0;
      lcd.clear();
      lcd.print("SONG");
      delay(1000);
      lcd.clear();
      break;
    case 'B': 
      mode = 2;
      pos_x = 0;
      lcd.clear();
      lcd.print("PLAY");
      delay(1000);
      lcd.clear();
      break;
    case 'C': 
      mode = 3; 
      record_id = 0; 
      pos_x = 0;
      lcd.clear();
      lcd.print("RECORD");
      delay(1000);
      lcd.clear();
      break;
    case 'D': 
      mode = 4;
      pos_x = 0;
      lcd.clear();
      lcd.print("REPLAY");
      delay(1000);
      lcd.clear();
      break;
    default: break;
  }

  switch(mode) {
    case 0: testTask(); break;
    case 1: songTask(); break;
    case 2: playTask(); break;
    case 3: recordTask(); break;
    case 4: replayTask(); break;
    default: break;
  }
}

void testTask() {
  play(1, 100);
  controlCenter();
}

void songTask() {
  if (key == '1') {
    pos_x = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("STAR");
    delay(1000);
    lcd.clear();
    for (i = 0; i < 48; i++) {
      play(star[i], 100);
    } 
  } else if (key == '2') {
    pos_x = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("BEE");
    delay(1000);
    lcd.clear();
    for (i = 0; i < 64; i++) {
      play(bee[i], 100);
    }
  } else if (key == '3') {
    pos_x = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LITTLE WORLD");
    delay(1000);
    lcd.clear();
    for (i = 0; i < 80; i++) {
      play(story[i], 100);
    }
  }
  controlCenter();
}

void playTask() {
  switch(key) {
    case '1': play(1, 200); break;
    case '2': play(2, 200); break;
    case '3': play(3, 200); break;
    case '4': play(4, 200); break;
    case '5': play(5, 200); break;
    case '6': play(6, 200); break;
    case '7': play(7, 200); break;
    case '8': play(8, 200); break;
    default: break;
  }
  controlCenter();
}

void recording(int8_t note) {
  record[record_id] = note;
  record_id += 1;
}

void recordTask() {
  DEBUG("i", record_id);
  if (record_id < 16) {
    switch(key) {
      case '1': 
        play(1, 200);
        recording(1);
        break;
      case '2': 
        play(2, 200);
        recording(2);
        break;
      case '3': 
        play(3, 200);
        recording(3); 
        break;
      case '4':
        play(4, 200);
        recording(4);
        break;
      case '5':
        play(5, 200);
        recording(5);
        break;
      case '6':
        play(6, 200); 
        recording(6);
        break;
      case '7': 
        play(7, 200);
        recording(7);
        break;
      case '8':
        play(8, 200);
        recording(8);
        break;
      default: break;
    }
  }
  controlCenter();
}

void replayTask() {
  if (record_id == 0) {
    lcd.clear();
    lcd.print("NO RECORD");
    delay(1000);
  } else if (key == '#') {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("start music");
    delay(1000);
    lcd.clear();
    for (i = 0; i < record_id; i++) {
      DEBUG("ii", i, record[i]);
      play(record[i], 200);
//      controlCenter();
    }
    delay(1000);
  }
  controlCenter();
}

void setup() {
  Serial.begin(9600);

  servo_SG90.attach(4);
  servo_MG995.attach(3);
  servo_MG995.write(0);

  lcd.init();
  lcd.backlight();
  pos_x = 0;
  record_id = 0;
  testTask();
}

void loop() { }
