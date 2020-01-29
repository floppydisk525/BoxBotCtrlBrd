/* smoothing.ino
 *  
 *  Mark Johnsen 20200128
 *  
 *  Using an arduino Nano
 *  This smoothing.ino program is based off the box bots program and is used to check that the input pins
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
 * 
 * smoothunits is the number of PWM readings to record and smooth by averaging them.  Since one PWM signal is 20ms (mille secs), 
 * taking 5 values would smooth the last 100ms.  The more values to smooth (average), the more laggy the control
 * will become, but the less jerky.  I think starting with 3 or 4 will give decent response and provide some 
 * smoothing.  
 */

#include <EnableInterrupt.h>

#define   ch1_pin  10    // input channel one (steering) is on pin 10
#define   ch2_pin  11    // input channel two (throttle) is on pin 11
#define   ch3_pin  12    // input channel three is on pin 12

#define   ch1_index  0
#define   ch2_index  1
#define   ch3_index  2

#define numRC_Channels 3

int ch1_rcvalue; // Steering
int ch2_rcvalue; // Thottle
int ch3_rcvalue; // Weapon Switch

#define numSmoothUnits 4       //number of PWM readings to take and smooth.  Since one PWM signal is 20ms (mille secs), taking 5

//turn into an array!
uint16_t total[numRC_Channels];      //keeps track of the total for faster average calc
uint16_t readings[numRC_Channels][numSmoothUnits];    //2d-array to keep track of readings
int readIndex[numRC_Channels];       //keeps track of readIndex for each channel

uint16_t rc_values[numRC_Channels];    //array of PWM values rec'd 
uint32_t rc_start[numRC_Channels];     //time at start of data collection
volatile uint16_t rc_shared[numRC_Channels];     //temp array for PWM values to make calcs.  Will be the SMOOTHED values.  

//show raw values along with smoothed values
uint16_t rc_raw[numRC_Channels];    //array of PWM values rec'd 
volatile uint16_t rc_raw_shared[numRC_Channels];     //temp array for PWM values to make calcs. 

void setup()
{
  //setup input pins
  pinMode(ch1_pin, INPUT);       // channel one of RC receiver, steering
  pinMode(ch2_pin, INPUT);       // channel two of RC receiver, throttle
  pinMode(ch3_pin, INPUT);       // channel three of RC receiver, switch

  //initalize arrays here.  
  for (int i=0; i<numRC_Channels; i++)
  {
	total[i]=0;
	readIndex[i]=0;
	rc_values[i]=0;
	rc_start[i]=0;
	rc_shared[i]=0;
    for (int j = 0; j< numSmoothUnits; j++)
    {
       readings[i][j] = 0;
    }
  }  

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
    Serial.print("ch1_rcvalue:"); Serial.print(ch1_rcvalue);   
    Serial.print(" RAW:"); Serial.print(rc_raw[0]);
    Serial.print("\t");
    Serial.print("ch2_rcvalue:"); Serial.print(ch2_rcvalue);  
    Serial.print(" RAW:"); Serial.print(rc_raw[1]);  
    Serial.print("\t");
    Serial.print("ch3_rcvalue:"); Serial.print(ch3_rcvalue);
    Serial.print(" RAW:"); Serial.println(rc_raw[2]);
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
  memcpy(rc_raw, (const void *)rc_raw_shared, sizeof(rc_raw_shared));
  interrupts();
}

void get_input(uint8_t channel, uint8_t input_pin) {
  if (digitalRead(input_pin) == HIGH) {
    rc_start[channel] = micros();
  } else {
    total[channel] = total[channel]-readings[channel][readIndex[channel]];    
    uint16_t rc_compare = (uint16_t)(micros() - rc_start[channel]);
    readings[channel][readIndex[channel]] = rc_compare;
    rc_raw_shared[channel] = rc_compare;  //store a copy of the raw value to display
    total[channel] = total[channel]+readings[channel][readIndex[channel]];
    readIndex[channel] = readIndex[channel]+1;
    if (readIndex[channel] >= numSmoothUnits)  {
      readIndex[channel]=0;
    }

    //avg=total[channel]/numSmoothUnits;
    rc_shared[channel] = total[channel]/numSmoothUnits;
    //rc_shared[channel] = rc_compare;
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
