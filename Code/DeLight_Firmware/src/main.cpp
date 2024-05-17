#include <Arduino.h>
#include <Encoder.h>
#define PIN_PWM1 9
#define PIN_PWM2 6
#define PIN_PWM3 5
#define PIN_PWM4 11
#define PIN_PWM5 10
#define PIN_ENCBTN 2
#define MAX_BRIGHTNESS_UNI 63
#define MAX_BRIGHTNESS_OMNI 44
#define WARNING_FLASH_ON_TIME 100
#define WARNING_FLASH_OFF_TIME 500
#define MORSE_DOT 300
#define MORSE_DASH 900
#define MORSE_SYMBOL_SPACE 300
#define MORSE_LETTER_SPACE 900
#define MORSE_WORD_SPACE 4000

Encoder myEnc(4, 3);
int8_t oldBrightness  = -1;
int8_t brightness;
int8_t maxBrightness = MAX_BRIGHTNESS_UNI;
uint32_t lastInterrupt, lastPWMchange;
uint16_t pwmValue;
void adjustBrightness();
void changeMode();
void btn_ISR();
volatile bool buttonPressed;
enum Mode{UNI, OMNI, WARNING, SOS} mode;
uint32_t warningFlashTimer;
uint8_t morseIndex;
uint16_t morseSOS[] = { MORSE_DOT, MORSE_SYMBOL_SPACE, 
                        MORSE_DOT, MORSE_SYMBOL_SPACE, 
                        MORSE_DOT, MORSE_LETTER_SPACE,
                        MORSE_DASH, MORSE_SYMBOL_SPACE,
                        MORSE_DASH, MORSE_SYMBOL_SPACE,
                        MORSE_DASH, MORSE_LETTER_SPACE,
                        MORSE_DOT, MORSE_SYMBOL_SPACE, 
                        MORSE_DOT, MORSE_SYMBOL_SPACE, 
                        MORSE_DOT, MORSE_WORD_SPACE
                      };
uint16_t morseWaitTime;
uint32_t morseTimer;



void setup() 
{
  myEnc.write(4);
  Serial.begin(115200);
  pinMode(PIN_PWM1, OUTPUT);
  analogWrite(PIN_PWM1, 0);
  pinMode(PIN_PWM2, OUTPUT);
  analogWrite(PIN_PWM2, 0);
  pinMode(PIN_PWM3, OUTPUT);
  analogWrite(PIN_PWM3, 0);
  pinMode(PIN_PWM4, OUTPUT);
  analogWrite(PIN_PWM4, 0);
  pinMode(PIN_PWM5, OUTPUT);
  analogWrite(PIN_PWM5, 0);
  pinMode(PIN_ENCBTN, INPUT_PULLUP);
  mode=UNI;
  lastInterrupt=millis();
  lastPWMchange=millis();
  attachInterrupt(digitalPinToInterrupt(PIN_ENCBTN),btn_ISR,RISING);
  warningFlashTimer=millis();
}

void loop() 
{
  adjustBrightness();
  if(buttonPressed)
  {
    changeMode();
    buttonPressed=false;
  }
  if(millis()-lastPWMchange>5)
  {
    lastPWMchange=millis();
    pwmValue=(brightness*brightness/16);
    switch(mode)
    {
      case UNI:
        analogWrite(PIN_PWM1, pwmValue);
        break;
      case OMNI:
        analogWrite(PIN_PWM1, pwmValue);
        analogWrite(PIN_PWM2, pwmValue);
        analogWrite(PIN_PWM3, pwmValue);
        analogWrite(PIN_PWM4, pwmValue);
        analogWrite(PIN_PWM5, pwmValue);
        break;
      case WARNING:
        if(millis()-warningFlashTimer>(5*WARNING_FLASH_ON_TIME+WARNING_FLASH_OFF_TIME))
        {
          warningFlashTimer=millis();
        }
        else if(millis()-warningFlashTimer>(5*WARNING_FLASH_ON_TIME))
        {
          analogWrite(PIN_PWM5, 0);
        }
        else if(millis()-warningFlashTimer>(4*WARNING_FLASH_ON_TIME))
        {
          analogWrite(PIN_PWM4, 0);
          analogWrite(PIN_PWM5, pwmValue);
        }
        else if(millis()-warningFlashTimer>(3*WARNING_FLASH_ON_TIME))
        {
          analogWrite(PIN_PWM3, 0);
          analogWrite(PIN_PWM4, pwmValue);
        }
        else if(millis()-warningFlashTimer>(2*WARNING_FLASH_ON_TIME))
        {
          analogWrite(PIN_PWM2, 0);
          analogWrite(PIN_PWM3, pwmValue);
        }
        else if(millis()-warningFlashTimer>(WARNING_FLASH_ON_TIME))
        {
          analogWrite(PIN_PWM1, 0);
          analogWrite(PIN_PWM2, pwmValue);
        }
        else
        {
          analogWrite(PIN_PWM5, 0);
          analogWrite(PIN_PWM1, pwmValue);
        }
        break;
      case SOS:
        if(millis()-morseTimer>morseWaitTime)
        {
          morseWaitTime=morseSOS[morseIndex];
          Serial.print("MorseIndex: ");
          Serial.println(morseIndex);
          Serial.print("MorseWaitTime: ");
          Serial.println(morseWaitTime);
          if(morseIndex%2==0)
          {
            Serial.println("Light on\n");
            analogWrite(PIN_PWM1, pwmValue);
            analogWrite(PIN_PWM2, pwmValue);
            analogWrite(PIN_PWM3, pwmValue);
            analogWrite(PIN_PWM4, pwmValue);
            analogWrite(PIN_PWM5, pwmValue);
          }
          else
          {
            Serial.println("Light off\n");
            analogWrite(PIN_PWM1, 0);
            analogWrite(PIN_PWM2, 0);
            analogWrite(PIN_PWM3, 0);
            analogWrite(PIN_PWM4, 0);
            analogWrite(PIN_PWM5, 0);
          }
          morseIndex++;
          if(morseIndex>=(sizeof(morseSOS)/sizeof(morseSOS[0])))
          {
            morseIndex=0;
          }
          
          morseTimer=millis();
          
        }
        
        break;
      default:
        break;
    }
  }
  
}

void adjustBrightness()
{
  brightness = myEnc.read();
  if(brightness<0) 
  {
    myEnc.write(0);
    brightness=0;
  }
  else if(brightness>maxBrightness)
  {
    myEnc.write(maxBrightness);
    brightness=maxBrightness;
  }
  if (brightness != oldBrightness) 
  {
    oldBrightness = brightness;
    Serial.print("Brightness: ");
    Serial.println(brightness);
    
  }
}

void btn_ISR()
{
  if(millis()-lastInterrupt>100)
  {
    buttonPressed=true;
    lastInterrupt=millis();
  }
  
}

void changeMode()
{
  analogWrite(PIN_PWM1,0);
  analogWrite(PIN_PWM2,0);
  analogWrite(PIN_PWM3,0);
  analogWrite(PIN_PWM4,0);
  analogWrite(PIN_PWM5,0);
  switch(mode)
  {
  case UNI:
    mode=OMNI;
    maxBrightness=MAX_BRIGHTNESS_OMNI;
    brightness=brightness>maxBrightness?maxBrightness:brightness;
    break;
  case OMNI:
    mode=WARNING;
    maxBrightness=MAX_BRIGHTNESS_UNI;
    warningFlashTimer=millis();
    break;
  case WARNING:
    mode=SOS;
    maxBrightness=MAX_BRIGHTNESS_UNI;
    break;
  case SOS:
    mode=UNI;
    maxBrightness=MAX_BRIGHTNESS_UNI;
    morseTimer=millis();
    morseIndex=0;
    break;
  default:
    break;
  }
  Serial.print("Mode: ");
  Serial.println(mode);
  Serial.print("Max brightness: ");
  Serial.println(maxBrightness);
  Serial.print("Brightness: ");
  Serial.println(brightness);
}

