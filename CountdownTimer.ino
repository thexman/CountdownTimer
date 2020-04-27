/*
 *  Countdown timer that controls a relay.
 *  
 *  1. Enter time (00:00). First the right most digit is enter, and the text is shifted left.
 *  2. Press A on the keypad to start/stop the countdown.
 *  3. When timer is active the relay is closed.
 *  4. When the time reaches 0 or A is pressed the relay is open.
 */

#include <WiFi.h>
#include <Keypad.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>


//https://playground.arduino.cc/Code/Keypad/
const byte rows = 4; //four rows
const byte cols = 4; //three columns
char keys[rows][cols] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};
byte rowPins[rows] = {13, 12, 14, 27}; //connect to the row pinouts of the keypad
byte colPins[cols] = {26, 25, 33, 32}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );


// relay
const byte relays = 3;
int relayPin[relays] = {15, 2, 4};



//https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
//https://github.com/olikraus/u8g2/wiki/u8g2reference#nextPage

//SSD1306 address 0x3C
U8G2_SSD1306_128X64_NONAME_1_HW_I2C  u8g2(U8G2_R0, 22, 21);

int loopCounter = 0;

String text = "05:00";
String keyText = "0500";
int secondsLeft = 0;


hw_timer_t * timer = NULL;
volatile int interruptCounter;

portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

//https://techtutorialsx.com/2017/10/07/esp32-arduino-timer-interrupts/
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup()
{
  Serial.begin(115200);    
  delay(10);
  Serial.println("[#] Serial initalized.");
  Wire.begin();
  Serial.println("[#] Wire initalized.");

  u8g2.begin();
  Serial.println("[#] U8G2 initialized.");
  showText();
  Serial.println("[#] U8G2 value shown.");

  interruptCounter = 0;
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000, true);
  timerAlarmEnable(timer);
  Serial.println("[#] Timer initialized.");

  for(int i = 0; i < relays; i++) {
    pinMode(relayPin[i], OUTPUT);    
  }
  Serial.println("[#] Realy pins configured.");

  turnSocketOff();
  Serial.println("[#] Socket off");
  
  Serial.println("Initialized");

}



bool isDigit(char key) {
  return (key >= '0' && key <= '9');
}

void showText() {
  u8g2.firstPage();
  do {      
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.drawStr(0,24, text.c_str());
  } while ( u8g2.nextPage() );
  Serial.print("Text: ");
  Serial.println(text.c_str());
}

void appendMinutes() {
  text = "";
  int v = secondsLeft / 60;
  if (v < 10) {
    text += "0";
  }  
  text += v;
}

void appendSeconds() {
  text += ":";
  int v = secondsLeft % 60;
  if (v < 10) {
    text += "0";
  }  
  text += v;
}

void calculateRemainingAndShow() {
  appendMinutes();
  appendSeconds();
  showText();
}

void turnSocketOff() {
  Serial.println(">>>>>>>>>>>!-------------");
  for(int i = 0; i < relays; i++) {
    digitalWrite(relayPin[i], HIGH);    
  }
}

void turnSocketOn() {
  Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>");
  for(int i = 0; i < relays; i++) {
    digitalWrite(relayPin[i], LOW);
  }
}

void keyTextToText() {
  text = keyText.substring(0,2);
  text += ":";
  text += keyText.substring(2);
}

void onTimeOut() {
  keyTextToText();  
  turnSocketOff();  
  portENTER_CRITICAL(&timerMux);
  secondsLeft = 0;
  interruptCounter = 0;
  portEXIT_CRITICAL(&timerMux);
}

void loop() {  
  char key = keypad.getKey();
  
  if (key != NO_KEY) {
    if (secondsLeft > 0) {
      onTimeOut();
    } else {
      if (isDigit(key)) {
        keyText += key;
        if (keyText.length() > 4) {
          keyText = keyText.substring(1);
        }
        keyTextToText();      
      } else if (key == 'A') {
        secondsLeft = keyText.substring(0,2).toInt() * 60;
        secondsLeft += keyText.substring(2).toInt();
        portENTER_CRITICAL(&timerMux);      
        interruptCounter = 0;
        portEXIT_CRITICAL(&timerMux);
        Serial.print("Starting count down: ");
        Serial.print(secondsLeft);
        Serial.println(" seconds.");
        turnSocketOn();
      }      
    }
    showText();
  }
  if (secondsLeft > 0 && interruptCounter > 0) {
    portENTER_CRITICAL(&timerMux);
    secondsLeft -= interruptCounter;
    interruptCounter = 0;
    portEXIT_CRITICAL(&timerMux);        
    calculateRemainingAndShow();

    if (secondsLeft <= 0) {      
      onTimeOut();
      showText();
    }
  }
}
