#include <avr/wdt.h>                             // Supplied Watch Dog Timer Macros 
#include <avr/sleep.h>                           // Supplied AVR Sleep Macros
#include <avr/power.h>                           // Power management
#include <math.h>                                //self explanitory
#include <avr/io.h>                              //AVR pin management
#include <Arduino.h>

#define adc_disable() (ADCSRA &= ~(1<<ADEN))      //to make the code a little more clear when disabling the ADC
#define watchdogRegister WDTCR                    //just to be clear for wdregister

const uint8_t powerMosfet = 3;                    //pin to output power mosfets
const uint8_t statusLED = 2;                      //pin to Status LED
// const uint8_t auxPin0 = 0;                     //AUX pin
// const uint8_t auxPin1 = 1;                     //AUX pin
const uint8_t capADC = A2;                        //ADC pin testing cap bank voltage
float calV = 0.9999;                              //calibration decimal value for 1.1Vref chip specific (lower value = higher voltage detected)
float dividerMultiplier = 3;                      //multiplier for resistor voltage divider
int powerOn = 0;                                  //variable determined when power is on
int voltageTarget = 2000;                         //the target voltage to charge cap bank
int voltageMinimum = 1000;                        //the target voltage to charge cap bank

void setup() {
  wdt_reset();                      //reset watchdog just as precaution
  DDRB |= (1 << powerMosfet);  			//replaces pinMode(PB4, OUTPUT);
  PORTB |= (1 << powerMosfet);			//replaces digitalWrite(PB3, HIGH);
  pinMode(statusLED, OUTPUT);
  pinMode(capADC, INPUT);
}

float readVcc(void){
  float result = 0;                                     //reset variables
  long raw = 0;
  ADCSRA |= bit(ADPS1) | bit(ADPS0);                    //prescale to 125kHz
  ADCSRA &= ~bit(ADPS2);
  ADMUX &= ~bit(REFS2);                                 //Reference VCC
  ADMUX &= ~bit(REFS0);
  ADMUX &= ~bit(REFS1);
  ADMUX &= ~bit(MUX1);
  ADMUX &= ~bit(MUX0);
  ADMUX |= bit(MUX3) | bit(MUX2);                       //point to Vbg
  ADCSRA |= bit(ADEN);                                  //enable ADC
  delay(50);                                           // Wait for Vref to settle
  ADCSRA |= bit(ADSC);                                  // Start conversion
  while (bit_is_set(ADCSRA, ADSC));                     // measuring
  raw = ADC;
  result = (1023 * 1.1 * 1000) / raw;                   // Calculate Vcc (in mV)
  result *= calV;                                       //apply chip specific vref calibration value (~1 most of the time)
  adc_disable();                                        //disable ADC
  return result;                                        // Vcc in milivolts
}

float testADCV(uint8_t inputADC, float refV){
  ADCSRA |= bit(ADEN);                                                    //enable ADC
  delay(50);
  float readPin = analogRead(inputADC);
  float miliVolt = (readPin * dividerMultiplier / 1023.00f) * refV;       //compare ADC value to VCC reference
  return miliVolt;
}

void goToSleep(void)
{
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);
  noInterrupts ();                                                        // timed sequence coming up
  wdt_reset();                                                            // pat the dog
  MCUSR = 0;                                                              // clear various "reset" flags
  watchdogRegister = bit (WDCE) | bit (WDE) | bit (WDIF);                 // allow changes, disable reset, clear existing interrupt
                                                                          // set interrupt mode and an interval (WDE must be changed from 1 to 0 here)
  watchdogRegister = 0xD8 | 1 << WDP0 | 1 << WDP3; /* 8.0 seconds */      // set WDIE, and 2 seconds delay
  ADCSRA &= ~bit(ADEN);                                                   //disable ADC
  sleep_enable ();                                                        // ready to sleep
  interrupts ();                                                          // interrupts are required now
  sei();                                                                  // Enable Interrupts 
  sleep_cpu ();                                                           // sleep
  sleep_disable ();                                                       // precaution
}


void loop() {
  digitalWrite(statusLED, HIGH);
  int voltCapBank = 0;
  int targetReached = 0;                             //variable determined when voltage reaches target level 
  do{
    float refVcc = 0;
    refVcc = readVcc();                              //Grab referrence voltage from Vcc
    voltCapBank = int(testADCV(capADC, refVcc));     //cap bank ADC in milivolts
    if (voltCapBank >= voltageTarget){   
      powerOn = 1;
      targetReached = 1;
      PORTB &= ~(1 << powerMosfet);		               //replaces digitalWrite(PB3, LOW);
    }else{
      powerOn = 0;
    }
    if(targetReached == 1 && voltCapBank >= voltageMinimum){
      powerOn = 1;
      PORTB &= ~(1 << powerMosfet);		               //replaces digitalWrite(PB3, LOW);
    }else{
      powerOn = 0;
    }
  }while(powerOn == 1);
  digitalWrite(statusLED, LOW);
  PORTB |= (1 << powerMosfet);			                 //replaces digitalWrite(PB3, HIGH);
  pinMode(powerMosfet, INPUT);
  goToSleep();                                       //sleep for 8 seconds
}

                            // watchdog interrupt
ISR (WDT_vect) 
{
  sleep_disable();          // Disable Sleep on Wakeup
  wdt_disable();            // disable watchdog
}
