#include <Servo.h>

//#define DEBUG 1

enum columns {left, center, right};

// Pin assignements
int pin_switches[6] = {9, 12, 10, 13, 8, 11};
int pin_rotary_switches [4] = {4, 6, 7, A0};
int pin_rotary_encoder [2] = {2, 3};
Servo myservo; // pin 5
int pin_linear_potentiometers [3] = {A4, A3, A5};
int pin_pitch_potentiometer = A2;
int pin_roll_potentiometer = A1;
#define encoder0PinA  2
#define encoder0PinB  3

// needle_positions
int needle_top = 75;
int needle_center = 100;
int needle_bottom = 125;

// linear potentiometer position
float linear_potentiometers[3];

// pitch/rool position
float pitch;
float roll;

// switches
int switches[6];
int rotary_switch;

// trim position
int trim_pos = 0;

/********************************************
   Return a val encoding the status of the 6 switches
 ********************************************/
int get_switches_val (void)
{
  int R = 0;
  int i;
  for (i = 0; i < 6; i++) if (switches[i]) R |= 1 << i;
  return R;
}

/********************************************
   Setting needle position in range -1.0 .. 1.0
 ********************************************/
void set_needle_position (float position)
{
  int pos;

  if (position < 0.0) {
    // trim down
    pos = needle_center + (needle_bottom - needle_center) * position;
  } else {
    // trim up
    pos = needle_center + (needle_center - needle_top) * position;
  }
  myservo.write(pos);
}

/********************************************
   Read linear potentiometers
 ********************************************/
void update_linear_potentiometers (void)
{
  int i;
  float v;
  for (i = 0; i < 3; i++) {
    v = analogRead(pin_linear_potentiometers[i]);
    if (i==1) v *= 1.05;
    if (v > 1000) v = 1000;
    linear_potentiometers[i] = v / 1000.0;
  }
}

/********************************************
   Read pitch/rool potentiometers
 ********************************************/

const int r_middle = 647;
const int r_flat = 10; 
const int r_max = 995;
const int r_min = 260;

const int r_base_top = r_middle + r_flat;
const int r_range_top = r_max - r_base_top;
const int r_base_bottom = r_middle - r_flat;
const int r_range_bottom = r_base_bottom - r_min;


const int p_middle = 525;
const int p_flat = 20; 
const int p_max = 960;
const int p_min = 160;

const int p_base_top = p_middle + p_flat;
const int p_range_top = p_max - p_base_top;
const int p_base_bottom = p_middle - p_flat;
const int p_range_bottom = p_base_bottom - p_min;


 
void update_pitch_roll (void)
{
  int r = analogRead(pin_roll_potentiometer);
  int p = analogRead(pin_pitch_potentiometer);

  if (r>r_max) r = r_max;
  if (r<r_min) r = r_min;
  
  if (r > r_middle) {
    // top
    if (r <= r_base_top) {
      roll = 0.0; 
    } else {
      roll = (((float)(r-r_base_top))/((float)(r_range_top))) * 100.0;
    }
  } else {
    // bottom
    if (r >= r_base_bottom) {
      roll = 0.0; 
    } else {
      roll = -(((float)(r_base_bottom-r))/((float)(r_range_bottom))) * 100.0;
    }
  }

  if (p>p_max) p = p_max;
  if (p<p_min) p = p_min;
  
  if (p > p_middle) {
    // top
    if (p <= p_base_top) {
      pitch = 0.0; 
    } else {
      pitch = (((float)(p-p_base_top))/((float)(p_range_top))) * 100.0;
    }
  } else {
    // bottom
    if (p >= p_base_bottom) {
      pitch = 0.0; 
    } else {
      pitch = -(((float)(p_base_bottom-p))/((float)(p_range_bottom))) * 100.0;
    }
  }

}

/********************************************
   Read pitch/rool potentiometers
 ********************************************/
void update_switches (void)
{
  int i, j;
  for (i = 0; i < 6; i++) {
    switches[i] = ! (digitalRead (pin_switches[i]));
  }
}

/********************************************
   Read rotary switch
 ********************************************/
void update_rotary_switch (void)
{
  int i;
  for (i = 0; i < 4; i++) {
    if (! digitalRead (pin_rotary_switches[i])) {
      rotary_switch = i;
      return;
    }
  }
}

/********************************************
   Read all inputs
 ********************************************/
void update_all (void)
{
  update_linear_potentiometers();
  update_pitch_roll();
  update_switches();
  update_rotary_switch();

  if (switches[3]) {trim_pos = 0;}

  if ( (trim_pos > -10) && (trim_pos < 10)) {
    set_needle_position (0.0);
  } else {
    set_needle_position (((float)trim_pos) / 100.0);
  }
}

/********************************************
   Trim wheel
 ********************************************/
void update (int direct) {
  trim_pos -= direct;
  if (trim_pos > 100) trim_pos = 100;
  if (trim_pos < -100) trim_pos = -100;
}

void doEncoderA()
{
  // look for a low-to-high on channel A
  if (digitalRead(encoder0PinA) == HIGH) {
    // check channel B to see which way encoder is turning
    if (digitalRead(encoder0PinB) == LOW) {
      update(1);
    } else {
      update(-1);
    }
  }
  else // must be a high-to-low edge on channel A
  {
    // check channel B to see which way encoder is turning
    if (digitalRead(encoder0PinB) == HIGH) {
      update(1);
    } else {
      update(-1);
    }
  }
}

void doEncoderB()
{
  // look for a low-to-high on channel B
  if (digitalRead(encoder0PinB) == HIGH) {
    // check channel A to see which way encoder is turning
    if (digitalRead(encoder0PinA) == HIGH) {
      update(1);
    } else {
      update(-1);
    }
  }
  // Look for a high-to-low on channel B
  else {
    // check channel B to see which way encoder is turning
    if (digitalRead(encoder0PinA) == LOW) {
      update(1);
    } else {
      update(-1);
    }
  }
}



/********************************************
    Initialisation
 ********************************************/
void setup() {
  int pin;
  int i;

  // Configure serial communication
#ifdef DEBUG 
  Serial.begin(9600);
#else
  Serial.begin(1000000);
#endif
  
  attachInterrupt(0, doEncoderA, CHANGE);
  attachInterrupt(1, doEncoderB, CHANGE);

  // Configure digital pins to be input except for the servo pin (output)
  for (pin = 0; pin < 8; pin++) {
    pinMode(pin, INPUT);
  }
  for (pin = 8; pin < 14; pin++) {
    pinMode(pin, INPUT_PULLUP);
  }
  for (pin = 0;  pin < 4; pin++) {
    pinMode(pin_rotary_switches[pin], INPUT_PULLUP);
  }
  myservo.attach(5);
  myservo.write(needle_center);
  for (i=needle_center; i>=needle_top; i--) {
    myservo.write(i);
    delay(20);  
  }
  for (i=needle_top; i<=needle_bottom; i++) {
    myservo.write(i);
    delay(20);  
  }
  for (i=needle_bottom; i>=needle_center; i--) {
    myservo.write(i);
    delay(20);  
  }
  myservo.write(needle_center);
    
  Serial.println ("Started");
}



void loop()
{
  String msg = "";

  update_all();

#ifdef DEBUG
  //Serial.println (roll);
  delay (250);
  
#else
  msg += "PL=";
  msg += linear_potentiometers[left];
  msg += ",PC=";
  msg += linear_potentiometers[center];
  msg += ",PR=";
  msg += linear_potentiometers[right];
  msg += ",YP=";
  msg += pitch;
  msg += ",YR=";
  msg += roll;
  msg += ",TP=";
  msg += trim_pos;
  msg += ",RS=";
  msg += rotary_switch;
  msg += ",SW=";
  msg += get_switches_val();
  msg += ",";

  Serial.println (msg);
#endif

  delay(10);
}
