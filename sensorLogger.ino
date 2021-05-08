// libraries handling digital signals from the temperature sensor
#include <OneWire.h>
#include <DallasTemperature.h>

// Library for data serialization, using the JSON format.
#include <ArduinoJson.h>
StaticJsonDocument<100> root;

// Data wire is plugged into digital pin 7 on the Arduino
#define ONE_WIRE_BUS 7

//use pin A0 for current sensor
#define CURRENT_PIN A0

// use pin A1 for contact switch
#define SWITCH_PIN 9

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);  

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

// variables for temperature sensors
int deviceCount;

// DC variables for current sensor 
const int averageValue = 500;
long int sensorValue = 0;
float voltageOffset;
float adjustedValue;
float amps;

//  AC variable for current sensor
int mVperAmp = 100; // use 100 for 20A Module and 66 for 30A Module

double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;

void setup()
{
  sensors.begin();
  Serial.begin(9600);
  deviceCount = sensors.getDeviceCount();

  // simple on off switch input for open / close
  pinMode(SWITCH_PIN, INPUT);

  // loop for current sensor, this sets assumed 0 current value;
  for (int i = 0; i < averageValue; i++)
  {
    sensorValue += analogRead(CURRENT_PIN);
    delay(2);
  }

  voltageOffset = sensorValue / averageValue;
  sensorValue = 0;
}

void loop()
{ 
  // loop for temperature sensors
  
  // Send command to all the sensors for temperature conversion
  sensors.requestTemperatures();
  
  // Display temperature from each sensor
  for (int i = 0;  i < deviceCount;  i++)
  {
    String sensorNum = "tempC_" + String(i + 1);
    root[sensorNum] = sensors.getTempCByIndex(i);
  } 

  // loop for current sensor
  for (int i = 0; i < averageValue; i++)
  {
    sensorValue += analogRead(CURRENT_PIN);
    delay(2);
  }

  // averages the sum of the sensor values to account for fluctuations
  sensorValue = sensorValue / averageValue;

  // adjusted value is sensor value minus the 0A analog value 508, which is roughly 2.32V from the sensor
  adjustedValue = sensorValue - voltageOffset;

  // the conversion from the ADC output to amps is about 25.6
  amps = adjustedValue / 25.6;
  root["dc_amps"] = amps;

  // reset sensor value;
  sensorValue = 0;

  // ac amps operations
  Voltage = getVPP();
  VRMS = (Voltage/2.0) *0.707;  //root 2 is 0.707
  AmpsRMS = (VRMS * 1000)/mVperAmp;

  root["ac_amps"] = AmpsRMS;

  // switch breaks contact when door is opened
  if (digitalRead(SWITCH_PIN) == HIGH) {
    root["switch"] = "closed";
  } else {
    root["switch"] = "open";
  }
  
  serializeJson(root, Serial);
  Serial.println();
}

float getVPP()
{
  float result;
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here
  
   uint32_t start_time = millis();
   while((millis()-start_time) < 1000) //sample for 1 Sec
   {
       readValue = analogRead(CURRENT_PIN);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           /*record the minimum sensor value*/
           minValue = readValue;
       }
   }
   
   // Subtract min from max
   result = ((maxValue - minValue) * 5.0)/1024.0;
      
   return result;
 }