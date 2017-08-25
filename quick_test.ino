/**************************************************************************
**  Project: 4x4 LED Matrix using TLC5940
**  By     : Chanartip Soonthornwan
**  Email  : Chanartip.Soonthornwan@gmail.com
**  Version: 1.0
**  Date   : 8/20/2017
**************************************************************************/
// LIBRARY
#include "Tlc5940.h"

// PIN
#define BUTTON     2
#define ADJ_COLOR  4
#define MANUAL    12
#define ADJ_SPEED A1

// SPEED
#define SLOW    1500
#define MED      700
#define FAST     100

// BRIGHTNESS
#define BRIGHT  4095
#define BLUR    2000
#define DIM     1000
#define DARK     500
#define OFF        0

// VARIABLE
#define NUM_LED   16
#define NUM_COLOR 16
#define NUM_FUNC   7

// PIN for TLC5940
const uint8_t Red_pin  [16] = {2, 5, 8, 11, 14, 17, 20, 23, 26, 29, 32, 35, 38, 41, 44, 47};
const uint8_t Green_pin[16] = {1, 4, 7, 10, 13, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46};
const uint8_t Blue_pin [16] = {0, 3, 6,  9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45};

unsigned char MODE = 0;
unsigned char CURRENT_MODE  = 0;
uint8_t       CURRENT_COLOR = 0;
uint8_t       REPEAT_COUNT  = 0;
uint16_t      CURRENT_SPEED = 0;
uint16_t      SPEED[] = {SLOW, MED, FAST};

//                          Red, Green, Blue
const uint8_t R_PINK[]    = {255,   0,  10};
const uint8_t PINK[]      = {255,   0,  31};
const uint8_t PURPLE[]    = {255,   0,  62};
const uint8_t VIOLET[]    = {255,   0, 255};
const uint8_t L_PURPLE[]  = { 62,   0, 255};
const uint8_t M_PURPLE[]  = { 31,   0, 255};
const uint8_t BLUE[]      = {  0,   0, 255};
const uint8_t L_BLUE[]    = {  0,  31, 255};
const uint8_t TURQUOISE[] = {  0, 255, 255};
const uint8_t CYAN[]      = {  0, 255,  62};
const uint8_t GREEN[]     = {  0, 255,   0};
const uint8_t LIME[]      = { 10, 255,   0};
const uint8_t G_YELLOW[]  = {255, 255,   0};
const uint8_t YELLOW[]    = {255,  62,   0};
const uint8_t ORANGE[]    = {255,  10,   0};
const uint8_t RED[]       = {255,   0,   0};

const uint8_t WHITE[]     = {255, 255, 255};
const uint8_t *COLOR[]    = {   R_PINK,     PINK, PURPLE, VIOLET,
                                L_PURPLE, M_PURPLE,   BLUE, L_BLUE,
                                TURQUOISE,     CYAN,  GREEN,   LIME,
                                G_YELLOW,   YELLOW, ORANGE,    RED
                            };

/**************************************************************************
  Button Interrupt Service Routine (ISR) with Debounce
  Increment Mode by one for each clicked.
  Input: pin2
**************************************************************************/
void btn_isr() {

  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();

  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time < 200)
  {
    MODE = (MODE + 1) % 7;
  }
  else
    last_interrupt_time = interrupt_time;
}

/**************************************************************************
  Running through all pin displaying Red, Green, and Blue
  Input:  velocity    speeding of the pattern
**************************************************************************/
void check_all_pin(uint8_t velocity) {

  for (uint8_t pin = 0; pin < NUM_TLCS * NUM_LED && CURRENT_MODE == MODE; pin++) {
    Tlc.clear();
    Tlc.set(pin, BRIGHT);
    Tlc.update();
    Delay(velocity);
  }
}

/**************************************************************************
    Utility functions
**************************************************************************/
/*
  Adjust RGB color and brightness of pin 'index'
  Note: Tlc.set(PIN, BRIGHTNESS 0-4095);
*/
void setRGB(const uint8_t color[], uint8_t pin, uint16_t brightness) {
  uint16_t R_bright = color[0] / 255.0 * brightness;
  uint16_t G_bright = color[1] / 255.0 * brightness;
  uint16_t B_bright = color[2] / 255.0 * brightness;

  Tlc.set(Red_pin[pin]  , R_bright);
  Tlc.set(Green_pin[pin], G_bright);
  Tlc.set(Blue_pin[pin] , B_bright);
}

uint8_t Index_if_Manual() {
  if (digitalRead(MANUAL) == HIGH) {
    if (digitalRead(ADJ_COLOR) == HIGH) {
      CURRENT_COLOR = (CURRENT_COLOR + 1) % 16;
    }
    return CURRENT_COLOR;
  }
  else
    return CURRENT_COLOR;
}

void Delay(uint16_t velocity) {
  if (digitalRead(MANUAL) == HIGH) {
    velocity = analogRead(ADJ_SPEED) / 3;
    if (velocity < 50)
      velocity = 50;
  }

  delay(velocity);
}

/**************************************************************************
   Rainbow Pattern
**************************************************************************/
void rainbow(uint8_t velocity, uint8_t index) {

  index = Index_if_Manual();

  if (index % 2 == 1) {
    rainbow_flag(velocity);
  }
  else {

    // rainbow from 0->15
    for (uint8_t pos = 0; pos < NUM_LED && CURRENT_MODE == MODE && digitalRead(ADJ_COLOR) == LOW; pos++) {
      for (uint8_t pin = 0; pin < NUM_LED; pin++) {
        setRGB(COLOR[pin], (pin + pos) % NUM_LED, BRIGHT);
      }
      Tlc.update();
      Delay(velocity);
    }
  }

}

void rainbow_flag(uint8_t velocity) {
  uint8_t color = Index_if_Manual();

  for (uint8_t pos = 0; pos < NUM_COLOR && CURRENT_MODE == MODE && digitalRead(ADJ_COLOR) == LOW; pos++) {

    uint8_t hold_color = color;

    for (uint8_t start_led = 0; start_led < NUM_LED; start_led += 4) {
      setRGB(COLOR[hold_color], start_led    , BRIGHT);
      setRGB(COLOR[hold_color], start_led + 1, BRIGHT);
      setRGB(COLOR[hold_color], start_led + 2, BRIGHT);
      setRGB(COLOR[hold_color], start_led + 3, BRIGHT);
      hold_color = ++hold_color % NUM_COLOR;
    }

    color = ++color % NUM_COLOR;
    Tlc.update();
    Delay(velocity);
  }
}

void L_to_R(uint8_t velocity, uint8_t index) {

  for (int pin = 0; pin < 4 && CURRENT_MODE == MODE; pin++) {

    index = Index_if_Manual();
    Tlc.clear();
    setRGB(COLOR[index], ( 0 + pin), DARK   );
    setRGB(COLOR[index], ( 4 + pin), DIM    );
    setRGB(COLOR[index], ( 8 + pin), BLUR   );
    setRGB(COLOR[index], (12 + pin), BRIGHT );
    Tlc.update();
    Delay(velocity);

  }
}

void knight_rider(uint8_t velocity, uint8_t index) {

  for (int pin = 0; pin < NUM_LED && CURRENT_MODE == MODE; pin++) {

    index = Index_if_Manual();
    Tlc.clear();

    if (pin == 0) {
      setRGB(COLOR[index], pin + 1, DARK   );
    }
    else if (pin == NUM_LED - 1) {
      setRGB(COLOR[index], pin - 1, DARK   );
    }
    else {
      setRGB(COLOR[index], pin - 1, DARK   );
      setRGB(COLOR[index], pin + 1, DARK   );
    }

    setRGB(COLOR[index]  , pin    , BRIGHT );
    Tlc.update();
    Delay(velocity);
  }

  Tlc.clear();
  Tlc.update();
  delay(FAST);
}

void breathing(uint8_t velocity, uint8_t index) {

  for (int dim = OFF; dim < BRIGHT && CURRENT_MODE == MODE; dim += 300) {
    index = Index_if_Manual();
    for (int pin = 0; pin < NUM_LED; pin++) {
      setRGB(COLOR[index], pin, dim);
    }
    Tlc.update();
    Delay(velocity);
  }
  for (int dim = BRIGHT; dim >= OFF && CURRENT_MODE == MODE; dim -= 300) {
    index = Index_if_Manual();
    for (uint8_t pin = 0; pin < NUM_LED; pin++) {
      setRGB(COLOR[index], pin, dim);
    }
    Tlc.update();
    Delay(velocity);
  }

}

void star(uint8_t velocity, uint8_t index) {
  uint8_t num_star = random(1, NUM_LED - 5);
  uint8_t pin[num_star];

  for (int i = 0; i < num_star; i++) {
    pin[i] = random(NUM_LED);
  }

  Tlc.clear();
  for (int dim = 0; dim < BRIGHT && CURRENT_MODE == MODE; dim += 300) {
    index = Index_if_Manual();
    for (int j = 0; j < num_star; j++) {
      setRGB(COLOR[index], pin[j], dim);
    }

    Tlc.update();
    Delay(velocity);
  }
  for (int dim = BRIGHT; dim >= OFF && CURRENT_MODE == MODE; dim -= 300) {
    index = Index_if_Manual();
    for (int j = 0; j < num_star; j++) {
      setRGB(COLOR[index], pin[j], dim);
    }
    Tlc.update();
    Delay(velocity);
  }

}

void square(uint8_t velocity, uint8_t index) {

  int dim = 0;
  int fade = 300;
  int step = 0;
  int index2 = 0;

  while (CURRENT_MODE == MODE) {

    if (step < 5) {
      index = Index_if_Manual();
      index2 = (index + 8) % NUM_COLOR;
    } else if (step < 8) {
      index2 = Index_if_Manual();
      index = (index2 + 8) % NUM_COLOR;
    }

    Tlc.clear();

    // Outter square
    for (int x = 0; x < NUM_LED; x += 4) {
      for (int y = 0; y < 4; y++) {
        if ( x == 0 ||
             (x == 4 && y == 0) || (x == 4 && y == 3) ||
             (x == 8 && y == 0) || (x == 8 && y == 3) ||
             x == 12
           )
        {
          setRGB(COLOR[index], x + y, dim);
        }
      }
    }

    // Inner square
    for (int x = 0; x < NUM_LED; x += 4) {
      for (int y = 0; y < 4; y++) {
        if ( (x == 4 && y == 1) || (x == 4 && y == 2) ||
             (x == 8 && y == 1) || (x == 8 && y == 2)
           )
        {
          setRGB(COLOR[(index + 8) % NUM_COLOR], x + y, dim);
        }
      }
    }

    Tlc.update();
    Delay(velocity);

    if (step >= 8) {
      step = 0;
      break;
    }

    if (dim > BRIGHT - 200 ) {
      dim = BRIGHT;
      fade = -fade;
    }
    else if (dim < 0 + 200) {
      dim = 0;
      fade = -fade;
      step++;
    }
    dim += fade;
  }
}

void snake(uint8_t velocity, uint8_t index) {

  int step = 0;
  int dir_x = 4;
  int dir_y = 1;
  int start_pin = random(4);
  uint8_t direction = random(4);
  int pin;
  int prev_pin;
  int prev_pin2;

  // x edge
  if (start_pin == 0) {
    dir_x = 4;
    dir_y = 1;
    pin = 0;
  }
  else if (start_pin == 1) {
    dir_x = 4;
    dir_y = -1;
    pin = 3;
  }
  else if (start_pin == 2) {
    dir_x = -4;
    dir_y = 1;
    pin = 12;
  }
  else {
    dir_x = -4;
    dir_y = -1;
    pin = 15;
  }

  prev_pin = pin;
  prev_pin2 = pin;

  while (CURRENT_MODE == MODE) {

    index = Index_if_Manual();

    prev_pin2 = prev_pin;
    prev_pin  = pin;

    if (step <  3) {
      pin += dir_y;
    } else if (step <  6) {
      pin += dir_x;
    } else if (step <  9) {
      pin -= dir_y;
    } else if (step < 11) {
      pin -= dir_x;
    } else if (step < 13) {
      pin += dir_y;
    } else if (step < 14) {
      pin += dir_x;
    } else if (step < 15) {
      pin -= dir_y;
    } else if (step < 17) {
      pin -= dir_x;
    } else if (step < 19) {
      pin += dir_y;
    } else if (step < 22) {
      pin += dir_x;
    } else if (step < 25) {
      pin -= dir_y;
    } else  // 24->27
      pin -= dir_x;

    Tlc.clear();
    setRGB(COLOR[index], prev_pin2, DARK);
    setRGB(COLOR[index], prev_pin, DIM);
    setRGB(COLOR[index], pin, BRIGHT);

    Tlc.update();
    Delay(velocity);

    if (step > 26) {
      step = 0;
      break;
    }
    else {
      step++;
    }

  }
}

void(*pattern[NUM_FUNC])(uint8_t velocity, uint8_t index) = {rainbow, L_to_R, knight_rider, breathing, star, square, snake};


void setup() {
  Serial.begin(9600);
  Tlc.init();

  pinMode(ADJ_COLOR, INPUT);
  pinMode(MANUAL   , INPUT);
  pinMode(BUTTON   , INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON), btn_isr, RISING);

  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(0));

  delay(1);
  check_all_pin(FAST / 4);
}

void loop() {

  if (digitalRead(MANUAL) == HIGH) {
    switch (MODE) {
      case 1:
        rainbow(MED, CURRENT_COLOR);
        break;
      case 2:
        L_to_R(MED, CURRENT_COLOR);
        break;
      case 3:
        knight_rider(MED, CURRENT_COLOR);
        break;
      case 4:
        breathing(MED, CURRENT_COLOR);
        break;
      case 5:
        star(FAST, CURRENT_COLOR);
        break;
      case 6:
        square(FAST, CURRENT_COLOR);
        break;
      default:
        snake(FAST, CURRENT_COLOR);
        break;
    }
  }
  else {
    MODE = random(NUM_FUNC);
    CURRENT_SPEED = SPEED[random(1, 3)];

    // rainbow, L_to_R, knight_rider, breathing, star
    if (MODE == 0) {          // rainbow
      REPEAT_COUNT = 3;
    } else if (MODE == 1) {   // L_to_R
      CURRENT_SPEED = SPEED[random(3)];
      REPEAT_COUNT = 5;
    } else if (MODE == 2) {   // knight_rider
      CURRENT_SPEED = SPEED[random(3)];
      REPEAT_COUNT = 6;
    } else if (MODE == 3) {   // breathing
      REPEAT_COUNT = 3;
    } else if (MODE == 4) {   // star
      REPEAT_COUNT = 3;
    } else if (MODE == 5) {   // square
      CURRENT_SPEED = FAST;
      REPEAT_COUNT = 1;
    } else if (MODE == 6) {   // snake
      REPEAT_COUNT = 3;
    }

    for (int num_color = REPEAT_COUNT; num_color > 0; num_color--) {
      for (int rep_color = 0; rep_color < 3; rep_color++) {
        pattern[MODE](CURRENT_SPEED, CURRENT_COLOR);
        if (MODE == 5)
          CURRENT_COLOR = random(NUM_COLOR);
      }
      CURRENT_COLOR = random(NUM_COLOR);
      delay(10);
    }
  }

  // Updating Global Variable
  CURRENT_MODE = MODE;
}

