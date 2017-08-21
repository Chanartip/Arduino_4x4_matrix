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
#define BUTTON 2
#define MANUAL 12
#define ADJ_COLOR 4
#define ADJ_SPEED A1

// SPEED
#define SLOW 2000
#define MED  1000
#define FAST 200

// BRIGHTNESS
#define BRIGHT 4095
#define BLUR   2000
#define DIM    1000
#define DARK   500
#define OFF    0

// VARIABLE
#define NUM_LED 16
#define NUM_COLOR 16

// PIN for TLC5940
const uint8_t Blue_pin[16]  = {2, 5, 8, 11, 14, 17, 20, 23, 26, 29, 32, 35, 38, 41, 44, 47};
const uint8_t Green_pin[16] = {1, 4, 7, 10, 13, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46};
const uint8_t Red_pin[16]   = {0, 3, 6,  9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45};

unsigned char MODE = 0;
unsigned char CURRENT_MODE = 0;
uint8_t       CURRENT_COLOR = 0;


//                       Red, Green, Blue
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
const uint8_t *COLOR[]    = {R_PINK, PINK, PURPLE, VIOLET, L_PURPLE, M_PURPLE, BLUE, L_BLUE, TURQUOISE, CYAN, GREEN, LIME, G_YELLOW, YELLOW, ORANGE, RED};

void setup() {
  Serial.begin(9600);
  Tlc.init();
  pinMode(BUTTON, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON), btn_isr, RISING);
  pinMode(ADJ_COLOR, INPUT);
  pinMode(MANUAL, INPUT);

  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(0));

  delay(1);
  check_all_pin(MED);
}

void loop() {
  // Updating MODE
  CURRENT_MODE = MODE;

  switch (MODE) {
    case 1:
      rainbow(SLOW);
      break;
    case 2:
      L_to_R(SLOW);
      break;
    case 3:
      knight_rider(MED);
      break;
    case 4:
      breathing(SLOW);
      break;
    default:
      star(SLOW);
      break;
  }

}

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
    MODE = (MODE + 1) % 5;
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
    return random(NUM_COLOR);
}

void Delay(uint16_t velocity) {
  if (digitalRead(MANUAL) == HIGH) {
    velocity = analogRead(ADJ_SPEED) / 3;
    if (velocity < 50)
      velocity = 50;
  }

  delay(velocity);
}

bool isValidPin(uint8_t led) {
  if (led < NUM_LED) {
    return true;
  }
  else {
    return false;
  }
}

void GroupOf4(uint8_t led, uint8_t color_index) {
  if (isValidPin(led + 3)) {
    setRGB(COLOR[color_index], led, BRIGHT);
    setRGB(COLOR[color_index], led + 1, BRIGHT);
    setRGB(COLOR[color_index], led + 2, BRIGHT);
    setRGB(COLOR[color_index], led + 3, BRIGHT);
  }
}

/**************************************************************************
   Rainbow Pattern
**************************************************************************/
void rainbow(uint8_t velocity) {

  CURRENT_COLOR =  Index_if_Manual();

  if (CURRENT_COLOR % 2 == 1) {
    rainbow_flag(velocity);
  }
  else {
    // rainbow from 0->15
    for (uint8_t pos = 0; pos < NUM_LED && CURRENT_MODE == MODE; pos++) {
      for (uint8_t pin = 0; pin < NUM_LED; pin++) {
        setRGB(COLOR[pin], (pin + pos) % NUM_LED, BRIGHT);
      }
      Tlc.update();
      Delay(velocity);
    }
  }

}

void rainbow_flag(uint8_t velocity) {
  uint8_t color = CURRENT_COLOR;

  for (uint8_t pos = 0; pos < NUM_COLOR && CURRENT_MODE == MODE; pos++) {
    uint8_t hold_color = color;
    for (uint8_t start_led = 0; start_led < NUM_LED; start_led += 4) {
      GroupOf4(start_led, hold_color);
      hold_color = ++hold_color % NUM_COLOR;
    }
    color = ++color % NUM_COLOR;
    Tlc.update();
    Delay(velocity);
  }
}

void L_to_R(uint8_t velocity) {

  for (int pin = 0; pin < 4 && CURRENT_MODE == MODE; pin++) {

    uint8_t index = Index_if_Manual();
    Tlc.clear();
    setRGB(COLOR[index], ( 0 + pin), DARK   );
    setRGB(COLOR[index], ( 4 + pin), DIM    );
    setRGB(COLOR[index], ( 8 + pin), BLUR   );
    setRGB(COLOR[index], (12 + pin), BRIGHT );
    Tlc.update();
    Delay(velocity);

  }
}

void knight_rider(uint8_t velocity) {

  for (int pin = 0; pin < NUM_LED && CURRENT_MODE == MODE; pin++) {

    uint8_t index = Index_if_Manual();
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
  delay(1000);
}

void breathing(uint8_t velocity) {

  for (int dim = 0; dim < BRIGHT && CURRENT_MODE == MODE; dim += 500) {
    uint8_t index = Index_if_Manual();
    for (int pin = 0; pin < NUM_LED; pin++) {
      setRGB(COLOR[index], pin, dim);
    }
    Tlc.update();
    Delay(velocity);
  }
  for (int dim = BRIGHT; dim >= 0 && CURRENT_MODE == MODE; dim -= 500) {
    uint8_t index = Index_if_Manual();
    for (uint8_t pin = 0; pin < NUM_LED; pin++) {
      setRGB(COLOR[index], pin, dim);
    }
    Tlc.update();
    Delay(velocity);
  }

}

void star(uint8_t velocity) {
  uint8_t num_star = random(NUM_LED);
  uint8_t star[num_star];
  for (uint8_t i = num_star; i < num_star; i++) {
    star[i] = random(NUM_LED);
  }

  for (uint8_t dim = 0; dim < BRIGHT && CURRENT_MODE == MODE; dim += random(MED, SLOW)) {

  }
  for (uint8_t led = 0; led < num_star; led++) {
    setRGB(COLOR[CURRENT_COLOR], star[led], BRIGHT);
    Tlc.update();
    Delay(velocity);
  }


  
}

class Star {
  private:
    uint8_t location_pin;
    uint8_t color;
    
  public:
    Star(uint8_t loc_pin, uint8_t color) {
      this->location_pin = loc_pin;
      this->color = color;
    };
    uint8_t get_pin() {
      return this->location_pin;
    };
    uint8_t get_color(){
      return this->color;
    };
    
};

