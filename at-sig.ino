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

#define DISP_CLK  2
#define DISP_DIO  3
#define INPUT_CLK 4
#define INPUT_DT  5
#define INPUT_SW  6

#define EEPROM_KEY 0b11001100
#define EEPROM_KEY_ADDY 0
#define EEPROM_TIME_ADDY 1
#define EEPROM_MODE_ADDY 2

#define MODE_TIME   0
#define MODE_CENTER 1
#define MODE_BOTH   2

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
unsigned long long time_tracking;

void timerIsr() {
  encoder->service();
}

void setup() {
  Serial.begin(9600);

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
  
}

void loop() {
 
  value += encoder->getValue();
   
  if (value != last) {
    last = value; 
    Serial.print("Encoder Value: ");
    Serial.println(value);
    int min,secs;
    min = value / 60;
    secs = value % 60;
    int disp_val = (min * 100)+secs;
    display.showNumberDec(disp_val,false,3,1);
  }

  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    Serial.print("Button: ");
    #define VERBOSECASE(label) case label: Serial.println(#label); break;
    switch (b) {
      /*   
      VERBOSECASE(ClickEncoder::Pressed);
      VERBOSECASE(ClickEncoder::Held)
      VERBOSECASE(ClickEncoder::Released)
      VERBOSECASE(ClickEncoder::Clicked)
      VERBOSECASE(ClickEncoder::DoubleClicked)
      */
      case ClickEncoder::DoubleClicked:
        Serial.println("BTN_doubleclicked");  
        if(++prog_mode > 3)
         prog_mode = 0;
        EEPROM.write(EEPROM_MODE_ADDY,prog_mode);
        display.setSegments(modes_disp[prog_mode],1,0);
        break;
      case ClickEncoder::Clicked:
        Serial.println("BTN_clicked"); 
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
