#include <EEPROM.h>

// define constants
#define PUFF_F_PIN  32
#define PUFF_F      4 
#define PUFF_B_PIN  33
#define PUFF_B      5 

#define LED0_PIN    12
#define LED0        6


// setting PWM properties
#define freq  1600
#define led_freq 5000
#define ledChannel 0
#define resolution 8
#define PUFF_LEN 200
#define BRAKE_LEN 500
unsigned char led_brightness;


// FUNCTIONS
void set_on(int led){
   ledcWrite(led, 255);
}

void set_off(int led){
   ledcWrite(led, 0);
}

void dim_on (int led) {
   ledcWrite(led, led_brightness);
}

void all_on(){
  set_on(LED0);
}

void all_off(){
  set_off(LED0);
}

void dim_all(){
  dim_on(LED0);
}

void puff1(){
  ledcWrite(PUFF_B, 0); // make this a function
  ledcWrite(PUFF_F, 255);
  delay(PUFF_LEN);
  ledcWrite(PUFF_B, 255);
  delay(50);
  ledcWrite(PUFF_B, 0);
  ledcWrite(PUFF_F, 0);
}

void puff2(){
    ledcWrite(PUFF_F, 0);
    ledcWrite(PUFF_B, 255);
    delay(PUFF_LEN);
    ledcWrite(PUFF_F, 255);
    delay(50);
    ledcWrite(PUFF_B, 0);
    ledcWrite(PUFF_F, 0);
}


// SETUP
void setup() {
  led_brightness = 125;  // 100
  
  Serial.begin(115200);
  delay(10);


  // configuring PWM channels
  ledcSetup(PUFF_F, led_freq, resolution);  // change this frequency back
  ledcSetup(PUFF_B, led_freq, resolution);
  ledcSetup(LED0, led_freq, resolution);

  
  // configuring PWM pins
  ledcAttachPin(PUFF_F_PIN, PUFF_F);
  ledcAttachPin(PUFF_B_PIN, PUFF_B);
  ledcAttachPin(LED0_PIN, LED0);

}

uint8_t puff_state = 0; // motor "state machine"

void loop() {
  Serial.println("Starting Test.....");
  
  // Test LEDS
  set_on(LED0);
  delay(100);    // delay in ms
  dim_all();
  delay(100);

  // Test Puff Motor 
  if (puff_state) puff1();
  else puff2();
  puff_state = !puff_state;
  delay(100);

  // Turn off leds and indicate end of test
  delay(3000);
  all_off();
  Serial.print("Test Done.....");
  delay(3000);
}
