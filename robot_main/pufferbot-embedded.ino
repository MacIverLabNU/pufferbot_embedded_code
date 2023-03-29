#include "encoder.h"
#include "ota.h"
#include <EEPROM.h>
#include <WiFi.h>


ENCODER (Left, 17, 23);     
ENCODER (Right, 16, 4);

#define RIGHT_F_PIN 18  
#define RIGHT_F     0 
#define RIGHT_B_PIN 19  
#define RIGHT_B     1

#define LEFT_F_PIN  26  
#define LEFT_F      2 
#define LEFT_B_PIN  25  
#define LEFT_B      3 

#define PUFF_F_PIN  32
#define PUFF_F      4 
#define PUFF_B_PIN  33
#define PUFF_B      5 

#define LED0_PIN    12
#define LED0        6

//#define LED1_PIN    12
//#define LED1        7
//
//#define LED2_PIN    14
//#define LED2        8

// setting PWM properties
#define freq  1600
#define led_freq 5000
#define ledChannel 0
#define resolution 8
#define PUFF_LEN 200
#define BRAKE_LEN 500
unsigned char led_brightness;

// timer interupt globals
volatile int interruptCounter; // shared between main loop and ISR - signals main loop that an interuppt has occurred
hw_timer_t * timer = NULL;     // need to configure timer
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED; // synchronization 

int pwm_left = 0;  // max pwm is 255
int pwm_right = 0;
int current_left = 0;
int current_right = 0;
float P = 1 ; // 1 
float D = 2;
// error
int e_l = 0;
int eprev_l = 0;
int e_r = 0;
int eprev_r = 0;
int edot_l;
int edot_r;


// WIFI SETUP
const char* ssid     = "mazenet";
const char* password = "Albifrons2020";

// 192.168.137.155
IPAddress local_IP(192, 168, 137, 155);
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional
WiFiServer wifiServer(4500);
OTA(5000);


// STRUCTS
struct Encoder {
  const uint8_t A;
  const uint8_t B;
};

struct Command {
  int8_t left, right;
  uint8_t data;
} command;

uint8_t *buffer = (uint8_t *)&command;
size_t buffer_size = sizeof(Command);
WiFiClient client;


//struct Data{
//  bool puff, led0, led1, led2, increase, decrease, brake, reset;
//} data;

struct Data{
  bool puff, reset;
} data;



// timer interupt
long last_left = 0;
long last_right = 0;

void IRAM_ATTR onTimer(){ // IRAM_ATTR: compiler place code in IRAM
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  long left = get_encoder_tick_counter( Left );
  long right = get_encoder_tick_counter( Right );
  current_left = left - last_left;
  current_right = right - last_right;
  last_left = left;
  last_right = right;
  portEXIT_CRITICAL_ISR(&timerMux); 
}

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
//  set_on(LED1);
//  set_on(LED2);
}

void all_off(){
  set_off(LED0);
//  set_off(LED1);
//  set_off(LED2);
}

void dim_all(){
  dim_on(LED0);
//  dim_on(LED1);
//  dim_on(LED2);
}

void puff1(){
  ledcWrite(PUFF_B, 0); // make this a function
  ledcWrite(PUFF_F, 255);
//  all_off();
  delay(PUFF_LEN);
  ledcWrite(PUFF_B, 255);
  delay(50);
  ledcWrite(PUFF_B, 0);
  ledcWrite(PUFF_F, 0);
}

void puff2(){
    ledcWrite(PUFF_F, 0);
    ledcWrite(PUFF_B, 255);
//    all_off();
    delay(PUFF_LEN);
    ledcWrite(PUFF_F, 255);
    delay(50);
    ledcWrite(PUFF_B, 0);
    ledcWrite(PUFF_F, 0);
}

long keep_alive = 0;

// SETUP
void setup() {
  led_brightness = 125;  // 100
  
  Serial.begin(115200);
  delay(10);

  // Configure I/O Pins
  ENCODER_INIT(Left);
  ENCODER_INIT(Right);

  // configuring PWM channels
  ledcSetup(LEFT_F, freq, resolution);
  ledcSetup(LEFT_B, freq, resolution);
  ledcSetup(RIGHT_F, freq, resolution);
  ledcSetup(RIGHT_B, freq, resolution);
  ledcSetup(PUFF_F, led_freq, resolution);  // change this frequency back
  ledcSetup(PUFF_B, led_freq, resolution);
  ledcSetup(LED0, led_freq, resolution);
//  ledcSetup(LED1, led_freq, resolution);
//  ledcSetup(LED2, led_freq, resolution);
  
  // configuring PWM pins
  ledcAttachPin(LEFT_F_PIN, LEFT_F);
  ledcAttachPin(LEFT_B_PIN, LEFT_B);
  ledcAttachPin(RIGHT_F_PIN, RIGHT_F);
  ledcAttachPin(RIGHT_B_PIN, RIGHT_B);
  ledcAttachPin(PUFF_F_PIN, PUFF_F);
  ledcAttachPin(PUFF_B_PIN, PUFF_B);
  ledcAttachPin(LED0_PIN, LED0);
//  ledcAttachPin(LED1_PIN, LED1);
//  ledcAttachPin(LED2_PIN, LED2);

  // timer setup
  timer = timerBegin(0, 80, true); //  initialize timer: hardware timer 0, prescaler = 80 (microseconds), flag to count up = true  --> returns pointer to struct of tyoe hw_timer_t
  timerAttachInterrupt(timer, &onTimer, true); // attach timer to handling function (called when interrupt is generated) (input: pointer, address of function, edge trigger (true))
  timerAlarmWrite(timer, 7500, true); // counter value in which the timer interupt will be generated (true - timer reloads upon generating interrupt)
  timerAlarmEnable(timer); // enables timer

  // We start by connecting to a WiFi network
  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  delay(100);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    set_on(LED0);
    delay(100);
    set_off(LED0);
    delay(100);
  }
  for (int i =0;i<3;i++) {
    all_on();
    delay(100);
    all_off();
    delay(100);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  wifiServer.begin();
  OTA_INIT();
  delay(100);
  keep_alive = millis();
}


int prev_count = 0;
uint8_t puff_state = 0;
bool right_spin = false;
bool left_spin = false;
bool initial_reset = true;
bool wifi_reset = false;


void loop() {
  OTA_CHECK_UPDATES();

  if (!client) {
    client = wifiServer.available();
    if (client) {
      Serial.println("Client connected");
      dim_all();
      delay(100);
    }
  }
  
  if (client){
      size_t message_size;
      while( message_size = client.read(buffer, buffer_size)){
        if (message_size == sizeof(Command)){
          data.puff = command.data & 8;
          data.reset = command.data & 16;
        }
        if (data.puff || data.reset) break;
      };

      
      
      // check for software reset - client need to be connected for reset to occur
      if (data.reset){
        Serial.println("RESTART NOW");
        all_off();
        delay(100);
        ESP.restart();
      }
      
      // handles timer interupt after being signalled by ISR
      if (interruptCounter > 0) { 
        // decrement shared variable in a critical section - shared var needs to be synchronized
        portENTER_CRITICAL(&timerMux);
        interruptCounter--; // decrement since interrupt has been acknoledged
        portEXIT_CRITICAL(&timerMux);
        
        // PD CONTROL
        e_l = command.left - current_left;
        e_r = command.right - current_right;
        edot_l = e_l - eprev_l;
        edot_r = e_r - eprev_r;
//        Serial.print(command.left);
//        Serial.print(" , ");
//        Serial.println(command.right);
        
        
        pwm_left += round(P * (float)(e_l) + D * (float) edot_l);
        if (pwm_left > 255) pwm_left = 255;
        if (pwm_left < -255) pwm_left = -255;
        if (command.left == 0 && abs(pwm_left < 50)) pwm_left = 0;

        pwm_right += round(P * (float)(e_r) + D * (float) edot_r); 
        if (pwm_right > 255) pwm_right = 255;
        if (pwm_right < -255) pwm_right = -255;
        if (command.right == 0 && abs(pwm_right < 50)) pwm_right = 0;

        eprev_l = e_l;
        eprev_r = e_r;
      }    
      
      // SET MOTORS
      if (pwm_left > 0) {
        ledcWrite(LEFT_B, 0);
        ledcWrite(LEFT_F, pwm_left);
      } else {
        ledcWrite(LEFT_F, 0);
        ledcWrite(LEFT_B, -pwm_left);
      }
      if (pwm_right > 0) {
        ledcWrite(RIGHT_B, 0);
        ledcWrite(RIGHT_F, pwm_right);
      } else {
        ledcWrite(RIGHT_F, 0);
        ledcWrite(RIGHT_B, -pwm_right);
      }



//      data.puff = command.data & 8;/
      if (data.puff){
          if (puff_state){
            puff1();
          } else {
            puff2();
          }
          puff_state = !puff_state;
          data.puff = false;
          delay(100);
      }  
  } else{
      Serial.println("disconnect");
      all_off();
      delay(100);
  } 
}
