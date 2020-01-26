/* reciever_test.ino
 *  
 *  Mark Johnsen 20200126
 *  
 *  Using an arduino Nano
 *  This program is based off the box bots program and is used to check that the input pins
 *  from a 3-channel RC transmitter and receiver are working and obtaining data to drive the
 *  RC Car.  
 * 
 * In this system, 
 * Channel one is steering and on pin 10 
 * Channel two (throttle) is on pin 11
 * Channel three (weapon) is on pin 12
 * 
 * Because we want to use high-speed interrupts to capture the PWM signals from the receiver
 * we use an additional library called EnableInterrupt.h.  It allows interrupts to be used
 * on pins other than the Arduino basics for a nano which are pins 2 and 3.  
 * 
 * Also, there is a check to ensure the transmitter is ON.  Otherwise, the vehicle could
 * potentially drive off. 
 */

#include <EnableInterrupt.h>

#define   ch1_pin  10    // input channel one (steering) is on pin 10
#define   ch2_pin  11    // input channel two (throttle) is on pin 11
#define   ch3_pin  12    // input channel three is on pin 12

#define   ch1_index  0
#define   ch2_index  1
#define   ch3_index  2

int ch1_rcvalue; // Steering
int ch2_rcvalue; // Thottle
int ch3_rcvalue; // Weapon Switch

uint16_t rc_values[3];    //array of PWM values rec'd 
uint32_t rc_start[3];     //time at start of data collection
volatile uint16_t rc_shared[3];     //temp array for PWM values during reception

void setup()
{
  //setup input pins
  pinMode(ch1_pin, INPUT);       // channel one of RC receiver, steering
  pinMode(ch2_pin, INPUT);       // channel two of RC receiver, throttle
  pinMode(ch3_pin, INPUT);       // channel three of RC receiver, switch

  //turn on interrupts
  enableInterrupt(ch1_pin, get_ch1, CHANGE);
  enableInterrupt(ch2_pin, get_ch2, CHANGE);
  enableInterrupt(ch3_pin, get_ch3, CHANGE);

  Serial.begin(9600);
}

void loop()
{
  // get PWM data from transmitter through pin interrupts
  rc_read_values();

  //assign PWM values to 'working' variables
  ch1_rcvalue = rc_values[ch1_index]; // Steering : 1000 Left, 2000 Right
  ch2_rcvalue = rc_values[ch2_index]; // Throttle : 1000 Reverse, 2000 Forward
  ch3_rcvalue = rc_values[ch3_index]; // Switch, toggle switch between 1000 (99x) and 2000 (199x) value

  if (bTransmitterON() == true) {
    Serial.print("ch1_rcvalue:"); Serial.print(ch1_rcvalue);    Serial.print("\t");
    Serial.print("ch2_rcvalue:"); Serial.print(ch2_rcvalue);    Serial.print("\t");
    Serial.print("ch3_rcvalue:"); Serial.println(ch3_rcvalue);
  }
  
  delay(1);
}

bool bTransmitterON(){
  //check to see if transmitter is ON, if not, loop until it is..
  //  On the FlySky FS-GT2B transmitter, if the transmitter is OFF the following output is observed:
  //  Ch1 steering - 0 
  //  Ch2 throttle - 1500
  //  Ch3 'weapton' - 0

  if (ch1_rcvalue < 750){  //check to see if the transmitter is ON.  Only check channel 1 steering.
    Serial.println("Turn ON your Transmitter!!"); //Serial.print("\t");
    return false;
  }
  else return true;
}

//------------------------- ISR READ INTERRUPTS -------------------------- 
//  get pwm data from channel using interupts & return
//  ISR is Interrupt Service Routine (or I've seen Interrupt SubRoutine)

void rc_read_values() {
  noInterrupts();
  memcpy(rc_values, (const void *)rc_shared, sizeof(rc_shared));
  interrupts();
}

void get_input(uint8_t channel, uint8_t input_pin) {
  if (digitalRead(input_pin) == HIGH) {
    rc_start[channel] = micros();
  } else {
    uint16_t rc_compare = (uint16_t)(micros() - rc_start[channel]);
    rc_shared[channel] = rc_compare;
  }

//  Serial.print("count # = ");
//  Serial.print(count);          Serial.print("\t");
//  Serial.print(rc_shared[0]);   Serial.print("\t");
//  Serial.print(rc_shared[1]);   Serial.print("\t");
//  Serial.println(rc_shared[2]);
//  count ++;    
}

void get_ch1() { get_input(ch1_index, ch1_pin); }
void get_ch2() { get_input(ch2_index, ch2_pin); }
void get_ch3() { get_input(ch3_index, ch3_pin); }
