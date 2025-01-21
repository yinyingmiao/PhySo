#include <WiFi.h>
#include <OOCSI.h>
#include <TimeLib.h>

#define DB_METER A2
#define VREF 3.3

// const char* ssid = "Aussie";
// const char* password = "Vanessa@2007";
 
const char* ssid = "iotroam";
const char* password = "yinyingesp32";

const char* OOCSIName = "M12_MYY_final_###"; //the sender 
const char* hostserver = "oocsi.id.tue.nl";

OOCSI oocsi = OOCSI ();

unsigned long LastTrigger = 0;
unsigned long Delay = 60000; //  update the data every a minute

unsigned long LastRestart = 0; 
unsigned long RestartDelay = 10800000;

unsigned int readingID = 0;
unsigned long sample_size = 0;
int timeslot = 1;
float decibel = 0;
bool timetomove = false;

int c_dayoftoday = -1;


void setup() {

  Serial.begin (115200);

  pinMode (DB_METER, INPUT);

  pinMode (LED_BUILTIN, OUTPUT);
  oocsi.setActivityLEDPin (LED_BUILTIN);

  oocsi.connect (OOCSIName, hostserver, ssid, password, processTimeInfo);

  Serial.println ("Subscribing to timechannel");
  oocsi.subscribe ("timechannel");

}

void loop() {
  oocsi.check();

  unsigned long currentRestartTime = millis ();
    
  if (currentRestartTime > (LastRestart + RestartDelay)) {
      esp_restart();
      LastRestart = currentRestartTime;
    }
}

void processTimeInfo() {

  // Serial.print("processTimeInfo");
  int c_hour = oocsi.getInt("h", -1);
  int c_min = oocsi.getInt("m", -1);
  //int c_sec = oocsi.getInt("s", -1);
  //int c_day = oocsi.getInt("d", -1);
  //int c_mon = oocsi.getInt("M", -1);
  //int c_year = oocsi.getInt("y", -1);
  c_dayoftoday = oocsi.getInt("dw", -1) - 1; //starting with zero
  unsigned long millisSec = oocsi.getLong("timestamp", -1);

  //Serial.println("=====");  
  //Serial.println (c_dayoftoday);

  // if (c_day != yesterday) {
  //   // setTime(c_hour, c_min, c_sec, c_day, c_mon, c_year);
  //   setTime(millisSec);
  //   dayoftoday = weekday() + 1;
  //   yesterday = c_day;

   
  //   Serial.println (dayoftoday);
  // }

  // if (c_hour == 8) {
  //   // if (WiFi.status() != WL_CONNECTED) {
  //   //   WiFi.begin(ssid, password);
  //   // }
  //   esp_restart();
  // }

 
  if (c_hour >= 8 && c_hour < 13) {
    timeslot = 1;
  } else {
    timeslot = 2;
  }

  if (c_hour >= 8 && c_hour <= 17 ) {
    float datavoltage = analogRead (DB_METER) / 4095.0 * VREF;
    
    if (c_hour == 13 && c_min == 0) {
      sample_size = 0;
      decibel = 0.0;
      timetomove = true;
    }

    decibel += datavoltage * 50.0;
    sample_size += 1;
    unsigned long currentTime = millis ();
    
    if (currentTime > (LastTrigger + Delay)) {
      sendMsg(timeslot);
      LastTrigger = currentTime;
    }

  } else if (c_hour == 18 && c_min == 0) {
    timetomove = true;
    sendMsg(timeslot);
  } else {
    timeslot = 1;
    sample_size = 0;
    decibel = 0.0;
  }

  if (timetomove) {
    timetomove = false;
  }

  delay (1000);
}

void sendMsg(int am_pm) {
  float meanValue = decibel / sample_size;
  oocsi.newMessage ("M12_NOFRAME");
  oocsi.addString ("device_id", "d27808099fd134196");
  oocsi.addInt ("Day of Today", c_dayoftoday);
  oocsi.addInt ("Timeslot", am_pm);
  oocsi.addFloat ("Averaged Sound Level", meanValue);
  oocsi.addBool("timetomove", timetomove);
  oocsi.sendMessage();

  oocsi.printSendMessage();
}
