/* Code for V2 of Box Bots robot control board
 * 
 * Program Name:
 * BoxBotCtrl.ino
 * Jan 31, 2020
 * 
 * Arduino Nano clone w/ CH340 USB connected to TB6612 motor driver board
 * hooked up to receiver of 2.4GHz TX/RX 
 * 
 * standard model: FlySky FS-GT2B 3-Channel RC 
 * 
 * CH1 - Steering
 * CH2 - Throttle
 * CH3 - Switch
 * 
 * TO DO LIST:
 *   1. add tuning for fwd/rev counts to drive straight.
 *   2. add code to turn on LED for weapon
 *   
 * SMOOTHING added 1/31/2020.
 *
 */

#include <EnableInterrupt.h>

#define   lpwm    3     // pulse width modulation for left motor is pin 3
#define   lpin1   4     // left control pin one is pin 4
#define   lpin2   5     // left control pin two is pin 5
#define   standby 7     // standby pin is 7 - LOW=motor driver off, HIGH=motor driver on
#define   rpwm    6     // pulse width modulation for right motor is pin 6
#define   rpin1   8     // right control pin one is pin8
#define   rpin2   9     // right control pin two is pin 9


#define   forward 0     
#define   reverse 1
#define   coast   2
#define   brake   3
#define   rightturn 4
#define   leftturn  5

#define   ch1_pin  10    // input channel one is on pin 10
#define   ch2_pin  11    // input channel two is on pin 11
#define   ch3_pin  12    // input channel three is on pin 12

#define   ch1_index  0
#define   ch2_index  1
#define   ch3_index  2

int mtrspeed = 110 ;

#define numRC_Channels 3
#define numSmoothUnits 4       //number of PWM readings to take and smooth.  Since one PWM signal is 20ms (mille secs), taking 5

int ch1_rcvalue; // Steering value to make output calcs
int ch2_rcvalue; // Thottle value to make output calcs
int ch3_rcvalue; // Weapon Switch to make output calcs
int tdeadband = 10;  // How much in the throttle neutral position does it count as neutral centered on 255  (exp: for 15, deadband is from 240 to 270, 15 each side of 255) 
int sdeadband = 5;  // how much in the steering neutral position does it count as neutral centered on 255  (exp: for 15, deadband is from 240 to 270, 15 each side of 255) 
//int spd = 0;
byte neutral = 255;       // Note this netural is for both steering and throttle.  consider (strongly) breaking into neutralSteer and neutralThrottle to allo independent setting
byte strNeutral = 255;    //steering neutral
byte thrNeutral = 255;   //throttle neutral

int count = 0;

/*uint16_t rc_values[3];    //array of PWM values rec'd 
uint32_t rc_start[3];     //time at start of data collection
volatile uint16_t rc_shared[3];     //temp array for PWM values during reception
*/

//ARRAYS FOR PWM VALUES AND SMOOTHING
uint16_t total[numRC_Channels];      //keeps track of the total for faster average calc
uint16_t readings[numRC_Channels][numSmoothUnits];    //2d-array to keep track of readings
int readIndex[numRC_Channels];       //keeps track of readIndex for each channel

uint16_t rc_values[numRC_Channels];    //array of PWM values rec'd 
uint32_t rc_start[numRC_Channels];     //time at start of data collection
volatile uint16_t rc_shared[numRC_Channels];     //temp array for PWM values to make calcs.  Will be the SMOOTHED values.  

//show raw values along with smoothed values. 
uint16_t rc_raw[numRC_Channels];    //array of PWM values rec'd 
volatile uint16_t rc_raw_shared[numRC_Channels];     //temp array for PWM values to make calcs. 

//offset values to 'tune' the wheels to run at same speed.  These values are entered in PWM counts and add/subracted to 
//   the appropriate wheel based on the direction commanded. 
//   CHANGE THESE VALUES AS NEEDED 
int offsetRightFWD = 0;
int offsetRightREV = 0;
int offsetLeftFWD = 0;
int offsetLeftREV = 0;

//offset calculation values (do not change) 
//  dummy variables to set in direction selection function and then used in locomotion 
//  to calc offset.  offset is a percentage...
int offsetLeftPerc = 0;
int offsetRightPerc = 0;

//--------------------------------------------------------------------------------------  
//------------------------------- void setup() -----------------------------------------     
void setup() {
  // put your setup code here, to run once:

  //setup output pins
  pinMode(lpwm, OUTPUT);
  pinMode(lpin1, OUTPUT);
  pinMode(lpin2, OUTPUT);
  pinMode(rpwm, OUTPUT);
  pinMode(rpin1, OUTPUT);
  pinMode(rpin2, OUTPUT);
  pinMode(standby, OUTPUT);

  //setup input pins
  pinMode(ch1_pin, INPUT);       // channel one of RC receiver, steering
  pinMode(ch2_pin, INPUT);       // channel two of RC receiver, throttle
  pinMode(ch3_pin, INPUT);       // channel three of RC receiver, switch

  // turn on the things
  digitalWrite(standby, HIGH);  

  //start writing to output pins
  digitalWrite(lpin1, LOW);
  digitalWrite(lpin2, LOW);
  digitalWrite(rpin1, LOW);
  digitalWrite(rpin2, LOW);
  analogWrite(lpwm, mtrspeed);
  analogWrite(rpwm, mtrspeed);

  //initalize arrays   
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

  //debugging - comment out serial command if not needed...
  //Serial.begin(9600); 
}


//------------------------------ void loop() --------------------------------------
void loop() {
// put your main code here, to run repeatedly:


// get PWM data from transmitter through pin interrupts
  rc_read_values();
   
  ch1_rcvalue = rc_values[ch1_index]; // Steering : 1000 Left, 2000 Right
  ch2_rcvalue = rc_values[ch2_index]; // Throttle : 1000 Reverse, 2000 Forward
  ch3_rcvalue = rc_values[ch3_index]; // Switch, toggle switch between 1000 (99x) and 2000 (199x) value

  if (bTransmitterON() == true) {
    ch1_rcvalue = map(ch1_rcvalue, 1000,2000, 0, 1000); //center over 500
    ch2_rcvalue = map(ch2_rcvalue, 1000,2000, 0, 1000); //center over 500
    ch3_rcvalue = map(ch3_rcvalue, 1000,2000, 0, 1000); //center over 500
  
    ch1_rcvalue = ch1_rcvalue >> 1;  // right bit shift, divide by 2
    ch2_rcvalue = ch2_rcvalue >> 1;  // right bit shift, divide by 2
  
    ch1_rcvalue = constrain(ch1_rcvalue, 0, 512);
    ch2_rcvalue = constrain(ch2_rcvalue, 0, 512);
    ch3_rcvalue = constrain(ch3_rcvalue, 0, 1000);
    
    // So now both ch1 and ch2 are in the range of 0 to 512, with 255 being neutral
    // ch3 is either 0ish or 1000ish
  
/*  //print values while debugging, comment out when program setup and running as expected.  
    Serial.print("ch1_rcvalue:"); Serial.print(ch1_rcvalue);    Serial.print("\t");
    Serial.print("ch2_rcvalue:"); Serial.print(ch2_rcvalue);    Serial.print("\t");
    Serial.print("ch3_rcvalue:"); Serial.println(ch3_rcvalue);  */
  
    locomotion();   //Calculate and determine direction of vehicle
  } 
    
  delay(1);   //Why this here?  How much should it be??  Seems unnecessary.  
}

//----------------------- TRANSMITTER ON?? ------------------------------
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

//------------------------ MOTORDIRECTION -------------------------------
void motordirection(byte direction) {

  switch (direction) {
    
    case forward:
      digitalWrite(lpin1, HIGH);  
	    digitalWrite(lpin2, LOW);
      offsetLeftPerc = offsetLeftFWD
      digitalWrite(rpin1, HIGH);
      digitalWrite(rpin2, LOW);
      offsetRightPerc =  offsetRightFWD
      break;

    case reverse:
      digitalWrite(lpin1, LOW);
      digitalWrite(lpin2, HIGH);
      offsetLeftPerc = offsetLeftREV
      digitalWrite(rpin1, LOW);
      digitalWrite(rpin2, HIGH);
      offsetRightPerc =  offsetRightREV
      break;

    case brake:
      digitalWrite(lpin1, HIGH);
      digitalWrite(lpin2, HIGH);
      digitalWrite(rpin1, HIGH);
      digitalWrite(rpin2, HIGH);
      break;
	  
	case rightturn:
      digitalWrite(lpin1, LOW);
      digitalWrite(lpin2, HIGH);
      offsetLeftPerc =  offsetLeftFWD
      digitalWrite(rpin1, HIGH);
      digitalWrite(rpin2, LOW);
      offsetRightPerc =  offsetRightREV
      break;

    case leftturn:
      digitalWrite(lpin1, HIGH);
      digitalWrite(lpin2, LOW);
      offsetLeftPerc =  offsetLeftFWD
      digitalWrite(rpin1, LOW);
      digitalWrite(rpin2, HIGH);
      offsetRightPerc =  offsetRightREV
      break;
	
	default:  // coast condition
      digitalWrite(lpin1, LOW);
      digitalWrite(lpin2, LOW);
      digitalWrite(rpin1, LOW);
      digitalWrite(rpin2, LOW);    
  }
}

//--------------------------------------------------------------------------------------
void locomotion() {
  bool turnonly = false;  
  
  if (ch2_rcvalue<(thrNeutral-tdeadband)) {   // outside deadband, in REVERSE
    // if throttle in reverse do this
    motordirection(reverse);   
	  turnonly = false;
    //Serial.println("Reverse");  
  }
  else if (ch2_rcvalue>(thrNeutral+tdeadband)) { // outside deadband, in FORWARD
    // throttle in forward do this
    motordirection(forward);
	  turnonly = false;
    //Serial.println("Forward");  
  }
  else if (ch1_rcvalue>(strNeutral+sdeadband)) { // no throttle, RIGHT TURN
    // throttle in forward do this
    motordirection(rightturn);
	  turnonly = true;
    //Serial.println("RIGHTTURN");
  }
  else if (ch1_rcvalue<(strNeutral-sdeadband)) { // no throttle, LEFT TURN
    // throttle in forward do this
    motordirection(leftturn);
	  turnonly = true;
    //Serial.println("LeftTurn");
  }
  else { // in deadband, bring both motors to a stop   BRAKE
    motordirection(brake);
	  turnonly = false;
    //Serial.println("brake");
  }
    
	
  int turn = abs(strNeutral-ch1_rcvalue);
  int turnPWM = 0;
  // turn = turn >> 1;    // making steering less sensitive by dividing turn result by 4.
 
  if (turnonly == true) { //turn only
    //turn = turn/3;  //make turn only less sensitive and not too fast, modify divider
	  turnPWM = pwmOffsetCalc(turn, offsetLeftPerc);
	  analogWrite(lpwm, turnPWM);
    turnPWM = pwmOffsetCalc(turn, offsetRightPerc);
    analogWrite(rpwm, turnPWM);
  }
  else {                  // straight or straight with turn
    int spd = abs(thrNeutral-ch2_rcvalue);    

    if (turn > (sdeadband)) {   // outside the steering deadband
      int drag = (spd-turn);    // you can play with this value (increase/decrease) to get different behaviour
	  drag = constrain(drag,0,255);   
	  
	  if (ch1_rcvalue<neutral) { //steering left        
        turnPWM = pwmOffsetCalc(turn, offsetLeftPerc);
        analogWrite(lpwm, spd);
        //ADD DRAG VARIABLE PWM OFFSET (?)  OR IS IT IN THE CALC?  
        //  turnPWM = pwmOffsetCalc(turn, offsetRightPerc);
        analogWrite(rpwm, drag);
      }
      else {  // steering right
        analogWrite(lpwm, drag);
        analogWrite(rpwm, spd);
      }
    }
	else { // in the steering deadband
      //ADD SPEED pwm value.  maybe just one dummy variable before each (yes).
      analogWrite(lpwm, spd);
      analogWrite(rpwm, spd);
    }
  }
}

int pwmOffsetCalc (int pwmVal, int offsetPerc) {
  int offset = 0;
  offset = pwmVal*offsetPerc/100 + pwmVal;
  return offset;
}

//----------------------  currently this funciton not implemented ---------------
void writePWMvalue (int leftPWMval, int rightPWMval){
  analogWrite(lpwm, leftPWMval);
  analogWrite(rpwm, rightPWMval);
}
