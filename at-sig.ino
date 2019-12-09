/*********************************
 *    A
 *   F B
 *    G
 *   E C
 *    D  DP
 *    
 *    
 *    Modes:
 *     t = SEG_F | SEG_E | SEG_G | SEG_D
 *     C = SEG_E | SEG_G | SEG_D
 *     b = SEG_F | SEG_E | SEG_G | SEG_C | SEG_D
 */

#include <TM1637Display.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <EEPROM.h>

#define USE_SERIAL 1

#define LEFT_INT         2
#define RIGHT_INT        3
#define DISP_CLK         4
#define DISP_DIO         5
#define INPUT_CLK        6
#define INPUT_DT         7
#define INPUT_SW         8
#define CENTER_DECTOR    9

#define EEPROM_KEY       0b11001100
#define EEPROM_KEY_ADDY  0
#define EEPROM_TIME_ADDY 1
#define EEPROM_MODE_ADDY 2

#define MODE_TIME        0
#define MODE_CENTER      1
#define MODE_BOTH        2

enum signalState {
  signalOff = 0,
  signalOn
};

enum centeredState {
   startCentered = 0,
   unCentered,
   centered   
};

const uint8_t modes_disp[] = {
  SEG_F | SEG_E | SEG_G | SEG_D,
  SEG_E | SEG_G | SEG_D,
  SEG_F | SEG_E | SEG_G | SEG_C | SEG_D
};

// Create display object of type TM1637Display:
TM1637Display display = TM1637Display(DISP_CLK, DISP_DIO);

ClickEncoder *encoder;
int16_t last, value;
int16_t last_write;

int prog_mode;
volatile unsigned long long time_tracking;
volatile centeredState center_tracking;
volatile signalState currentLeftState;
volatile signalState currentRightState;

void timerIsr() {
  encoder->service();
}

void leftISR() {
  currentLeftState = signalOn;
  if(prog_mode == MODE_TIME) {

  }else if(prog_mode == MODE_CENTER) {
    int startState = digitalRead(CENTER_DECTOR);
    switch(startState){
      case HIGH:
       center_tracking = startCentered;
       break;
      case LOW:
       center_tracking = unCentered;
      break;       
    }
  }
}

void rightISR() {  
  currentRightState = signalOn;
  if(prog_mode == MODE_TIME) {

  }else if(prog_mode == MODE_CENTER) {
    int startState = digitalRead(CENTER_DECTOR);
    switch(startState){
      case HIGH:
       center_tracking = startCentered;
       break;
      case LOW:
       center_tracking = unCentered;
      break;       
    }
  }
}


void setup() {
#ifdef USE_SERIAL
  Serial.begin(9600);
#endif 

  
  int key = 0b00000000;
  EEPROM.get(EEPROM_KEY_ADDY,key);
  if(key != EEPROM_KEY) {
    EEPROM.write(EEPROM_KEY_ADDY,EEPROM_KEY);
    EEPROM.write(EEPROM_TIME_ADDY,0);
    EEPROM.write(EEPROM_MODE_ADDY,MODE_TIME);
    prog_mode = MODE_TIME;
    last = value = 0;
  }else{
    EEPROM.get(EEPROM_TIME_ADDY,last);
    value=last;
    EEPROM.get(EEPROM_MODE_ADDY,prog_mode);
  }
  display.setSegments(modes_disp[prog_mode],1,0);
  int min,secs;
  min = value / 60;
  secs = value % 60;
  int disp_val = (min * 100)+secs;
  display.showNumberDec(disp_val,false,3,1);
  
  encoder = new ClickEncoder(INPUT_CLK, INPUT_DT,INPUT_SW);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 

  attachInterrupt(digitalPinToInterrupt(LEFT_INT),leftISR,RISING);
  attachInterrupt(digitalPinToInterrupt(RIGHT_INT),rightISR,RISING);

}

void loop() {
 
  value += encoder->getValue();
   
  if (value != last) {
    last = value; 
#ifdef USE_SERIAL
    Serial.print("Encoder Value: ");
    Serial.println(value);
#endif
    int min,secs;
    min = value / 60;
    secs = value % 60;
    int disp_val = (min * 100)+secs;
    display.showNumberDec(disp_val,false,3,1);
  }

  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    
#ifdef USE_SERIAL
    Serial.print("Button: ");
#endif
    switch (b) {
      /*   
      #define VERBOSECASE(label) case label: Serial.println(#label); break;
      VERBOSECASE(ClickEncoder::Pressed);
      VERBOSECASE(ClickEncoder::Held)
      VERBOSECASE(ClickEncoder::Released)
      VERBOSECASE(ClickEncoder::Clicked)
      VERBOSECASE(ClickEncoder::DoubleClicked)
      */
      case ClickEncoder::DoubleClicked:
#ifdef USE_SERIAL
        Serial.println("BTN_doubleclicked");  
#endif
        if(++prog_mode > 3)
         prog_mode = 0;
        EEPROM.write(EEPROM_MODE_ADDY,prog_mode);
        display.setSegments(modes_disp[prog_mode],1,0);
        break;
      case ClickEncoder::Clicked:
#ifdef USE_SERIAL
        Serial.println("BTN_clicked");
#endif 
        if(last_write != value){
         EEPROM.write(EEPROM_TIME_ADDY,value);
         last_write=value;
        }
        break;
    }
  }

  if(prog_mode == MODE_TIME) {


  }else if(prog_mode == MODE_CENTER) {


  }else {


  }
  
}
