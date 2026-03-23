/****************************************************************************
   Example of the OOCSI-ESP library connecting to WiFi and receiving messages
   over OOCSI. Designed to work with the Processing OOCSI sender example
   that is provided in the same directory
 ****************************************************************************/

#include "OOCSI.h"

// libararies for Servo Motor control
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

const char* ssid = "";
// Password of your Wifi network.
const char* password = "";


// name for connecting with OOCSI (unique handle)
const char* OOCSIName = "M12_MYY_final_receiver_morning_NEW_####";
// put the adress of your OOCSI server here, can be URL or IP address string
const char* hostserver = "oocsi.id.tue.nl";

// OOCSI reference for the entire sketch
OOCSI oocsi = OOCSI();

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

//pulse width is with the unit of time. The min. and max. are defined here: https://www.upesy.com/blogs/tutorials/esp32-servo-motor-sg90-on-micropython#:~:text=With%20the%20SG90%20servo%20from,to%20one%20of%202.4ms. 
#define MIN_PULSE_WIDTH       650
#define MAX_PULSE_WIDTH       2350
#define DEFAULT_PULSE_WIDTH   1500
#define FREQUENCY             50 //https://www.kjell.com/globalassets/mediaassets/701916_87897_datasheet_en.pdf?ref=4287817A7A 

int dayoftoday = 0;
int timeslot = 1; 
float Averaged_sound_level = 0.0;
boolean timetomove = false;

// servo number for every day, this needs to be checked
#define servo_mon 13
#define servo_tue 11
#define servo_wed 7
#define servo_thu 5
#define servo_fri 2
const int servoPin [] = {13, 11, 7, 5, 2};

// put your setup code here, to run once:
void setup() {
  Serial.begin(115200);

  // setting up OOCSI. processOOCSI is the name of the fucntion to call when receiving messages, can be a random function name
  // connect wifi and OOCSI to the server
  oocsi.connect(OOCSIName, hostserver, ssid, password, processOOCSI);

  // subscribe to a channel
  Serial.println("subscribing to the NOFRAME data channel");
  oocsi.subscribe("M12_NOFRAME");

  // check if we are in the client list
  Serial.print("is ");
  Serial.print(OOCSIName);
  Serial.print(" a client? --> ");
  Serial.println(oocsi.containsClient(OOCSIName));

  Serial.println("16 channel Servo test!");
  pwm.begin();
  pwm.setPWMFreq(FREQUENCY);  // Analog servos run at ~60 Hz updates
  
  pwm.setPWM (servo_mon, 0, pulseWidth (0));
  pwm.setPWM (servo_tue, 0, pulseWidth (0));
  pwm.setPWM (servo_wed, 0, pulseWidth (0));
  pwm.setPWM (servo_thu, 0, pulseWidth (0));
  pwm.setPWM (servo_fri, 0, pulseWidth (0));


}

void loop() {
  oocsi.check();
}

// function which OOCSI calls when an OOCSI message is received
void processOOCSI() {

  // printing the output of different variables of the message; standard call is get<data type>(key, standard value)
  // the standard value -200 will be returned when the key is not included in the OOCSI message
  Serial.print ("Day of Today: ");
  Serial.println (oocsi.getInt ("Day of Today", -200.0));
  dayoftoday = oocsi.getInt ("Day of Today", -200.0);

  timetomove = oocsi.getBool("timetomove", false);
  Serial.print ("Timeslot: ");
  Serial.println (oocsi.getInt ("Timeslot", -200.0));
  Serial.print ("Averaged Sound Level: ");
  Serial.print (oocsi.getFloat ("Averaged Sound Level", -200.0));
  // Serial.print("\t time: ");
  // Serial.print(oocsi.getInt("time", -200));

  // event meta-data (sender, recipient, and timestamp)
  Serial.print("\t sender: ");
  Serial.print(oocsi.getSender());
  Serial.print("\t recipient: ");
  Serial.print(oocsi.getRecipient());
  // Serial.print("\t Timestamp: ");
  // Serial.print(oocsi.getTimeStamp());
  Serial.println();

  motorcontrol();
  delay(6000); // check every minute

  // use this to print out the raw message that was received
  oocsi.printMessage();

}

void motorcontrol (){

  int pin = servoPin[dayoftoday - 1];

//the 2 lines of "fake" code
  //timetomove = true;
  //if (timetomove && oocsi.getInt ("Timeslot", -200.0) == 2) {

   if (timetomove && oocsi.getInt ("Timeslot", -200.0) == 1) {
     Serial.println ("It is morning.");

    if (oocsi.getFloat ("Averaged Sound Level", -200) <= 30) {
      //<30 dB turn 30 degrees
      Serial.println ("Averaged sound level: <30 dB"); 
      pwm.setPWM (pin, 0, pulseWidth (30));
      Serial.println ("Motor turned: 30 degrees"); 
    }

    if (30 < oocsi.getFloat ("Averaged Sound Level", -200) && oocsi.getFloat ("Averaged Sound Level", -200) <= 35) {
      Serial.println ("Averaged sound level: 30-35 dB"); 
      pwm.setPWM (pin, 0, pulseWidth (60));
      Serial.println ("Motor turned: 60 degrees"); 
    }

    if (35 < oocsi.getFloat ("Averaged Sound Level", -200) && oocsi.getFloat ("Averaged Sound Level", -200) <= 40) {
      Serial.println ("Averaged sound level: 35-40 dB"); 
      pwm.setPWM (pin, 0, pulseWidth (90));
      Serial.println ("Motor turned: 90 degrees"); 
    }
    
      if (40 < oocsi.getFloat ("Averaged Sound Level", -200) && oocsi.getFloat ("Averaged Sound Level", -200) <= 45) {
      Serial.println ("Averaged sound level: 40-45 dB"); 
      pwm.setPWM (pin, 0, pulseWidth (120));
      Serial.println ("Motor turned: 120 degrees"); 
    }

      if (45 < oocsi.getFloat ("Averaged Sound Level", -200) && oocsi.getFloat ("Averaged Sound Level", -200) <= 50) {
      Serial.println ("Averaged sound level: 45-50 dB"); 
      pwm.setPWM (pin, 0, pulseWidth (150));
      Serial.println ("Motor turned: 150 degrees"); 
    }

      if (50 < oocsi.getFloat ("Averaged Sound Level", -200)) {
      Serial.println ("Averaged sound level: 50+ dB"); 
      pwm.setPWM (pin, 0, pulseWidth (180));
      Serial.println ("Motor turned: 180 degrees"); 
    }
  }
}

int pulseWidth(int angle)
{
  int pulse_wide, analog_value;
  pulse_wide   = map(angle, 0, 180, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
  analog_value = int(float(pulse_wide) / 1000000 * FREQUENCY * 4096);
  return analog_value;
}
