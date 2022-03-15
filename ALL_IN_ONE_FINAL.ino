/********INCLUDES********/
#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <DHT.h>
#include <BH1750.h>
#include <SoftwareSerial.h>

/********DEFINES********/
#define DHTPIN 4
#define DHTTYPE DHT11
#define LED_PIN 9
#define BUTTON_PIN 7

/********SOFTWARESERIAL********/
SoftwareSerial nodemcu(5, 6);

/********GLOBALS********/
unsigned long currentMillis = 0;
byte lastButtonState = 0;
byte ledState = HIGH;
int power = 1;

/********GLOBALS FOR JSON********/
const size_t capacity = JSON_ARRAY_SIZE(1) + 2*JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + 8*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(6);
DynamicJsonBuffer jsonBuffer(capacity);
JsonObject& root = jsonBuffer.createObject();
JsonArray& sensors = root.createNestedArray("sensors");
JsonObject& sensors_0 = sensors.createNestedObject();
JsonArray& sensors_0_dht11 = sensors_0.createNestedArray("dht11");
JsonObject& sensors_0_dht11_0 = sensors_0_dht11.createNestedObject();
JsonObject& sensors_0_dht11_1 = sensors_0_dht11.createNestedObject();
JsonArray& sensors_0_bmp280 = sensors_0.createNestedArray("bmp280");
JsonObject& sensors_0_bmp280_0 = sensors_0_bmp280.createNestedObject();
JsonObject& sensors_0_bmp280_1 = sensors_0_bmp280.createNestedObject();
JsonObject& sensors_0_pirmini3 = sensors_0.createNestedObject("pirmini3");
JsonObject& sensors_0_bh1750 = sensors_0.createNestedObject("bh1750");
JsonObject& sensors_0_mq2 = sensors_0.createNestedObject("mq2");
JsonObject& sensors_0_power = sensors_0.createNestedObject("power");

/********GLOBALS FOR DHT11********/
DHT dht(DHTPIN, DHTTYPE);

/********GLOBALS FOR BH1750********/
BH1750 lightMeter;
unsigned long delayBH1750 = 5000;
unsigned long previous_BH1750_Millis = 0;

/********GLOBALS FOR BMP280********/
Adafruit_BMP280 bmp;
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();
unsigned long delayBMP280 = 5000;
unsigned long previous_BMP280_Millis = 0;

/********GLOBALS FOR MQ2********/
int RedLed = 12;
int GreenLed = 11;
int Buzzer = 10;
int AirQ = A0;
int BadVal = 400;
unsigned long delayMQ2 = 5000;
unsigned long previous_MQ2_Millis = 0;

/********GLOBALS FOR PIRMINI-3********/
int LED = 2;
int PIR = 3;
unsigned long delayPIRMINI3 = 5000;
unsigned long previous_PIRMINI3_Millis = 0;

void setup() {
  /********ON/OFF BUTTON********/
  digitalWrite(LED_PIN, ledState);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  sensors_0_power["name"] = "onoff";
  sensors_0_power["value"] = power;
  
  /********MQ2********/
  pinMode(RedLed, OUTPUT);
  pinMode(GreenLed, OUTPUT);
  pinMode(Buzzer, OUTPUT);
  pinMode(AirQ, INPUT);
  
  /********PIRMINI-3********/
  pinMode(LED, OUTPUT);
  pinMode(PIR, INPUT);
  
  /********BMP280 CHECK********/
  if (!bmp.begin())
    {
      Serial.println(F("Nem található BMP280 szenzor!"));
      while (1) delay(10);
    }
  
  /********BEGINS********/
  Serial.begin(9600);
  Serial.println("Rendszerindítás...");
  nodemcu.begin(9600);
  Wire.begin();
  dht.begin();
  lightMeter.begin();
}

void loop() {
  /********ON/OFF CHECK********/
  OnOff();

  /********IF ON THEN RUN********/
  if(ledState == HIGH)
    {
      currentMillis = millis();
  
      /********PIRMINI-3********/
      if (currentMillis - previous_PIRMINI3_Millis >= delayPIRMINI3)
        {
          previous_PIRMINI3_Millis += delayPIRMINI3;
          
          if (digitalRead(PIR) == HIGH)
            { 
              digitalWrite(LED, HIGH);
              /********MOTION TO SERIAL********/
              Serial.println("Mozgás észlelve!");
    
              /********FOR JSON SERIALIZE********/
              sensors_0_pirmini3["name"] = "motion";
              sensors_0_pirmini3["value"] = "true";
            } 
          else
            {
              digitalWrite(LED, LOW);       
              Serial.println("Nincs mozgás!");
            }
        }

    /********FUNCTIONS********/
    
      callBMP280AndDHT11();
      callMQ2();
      delay(100);
      callBH1750();
      
      /********SEND DATA TO NODEMCU********/
      root.printTo(nodemcu);
    }
}

void callBMP280AndDHT11(){
  if (currentMillis - previous_BMP280_Millis >= delayBMP280)
    {
      previous_BMP280_Millis += delayBMP280;
      sensors_event_t temp_event, pressure_event; // for BMP280
      bmp_temp->getEvent(&temp_event);            // for BMP280
      bmp_pressure->getEvent(&pressure_event);    //for BMP280

      /********DHT11 TEMPERATURE TO SERIAL********/
      Serial.print(F("DHT11 hőmérséklet: "));
      Serial.print(dht.readTemperature());
      Serial.println(F(" °C"));

      /********BMP280 TEMPERATURE TO SERIAL********/
      Serial.print(F("BMP280 hőmérséklet: "));
      Serial.print(temp_event.temperature);
      Serial.println(" °C");

      /********DHT11 HUMIDITY TO SERIAL********/
      Serial.print(F("DHT11 páratartalom: "));
      Serial.print(dht.readHumidity());
      Serial.println(F("%"));

      /********BMP280 PRESSURE TO SERIAL********/
      Serial.print(F("BMP280 légnyomás: "));
      Serial.print(pressure_event.pressure);
      Serial.println(" hPa");

      /********FOR JSON SERIALIZE********/
      sensors_0_dht11_0["name"] = "temperature";
      sensors_0_dht11_0["value"] = dht.readTemperature();

      sensors_0_dht11_1["name"] = "humidity";
      sensors_0_dht11_1["value"] = dht.readHumidity();

      sensors_0_bmp280_0["name"] = "temperature";
      sensors_0_bmp280_0["value"] = temp_event.temperature;

      sensors_0_bmp280_1["name"] = "pressure";
      sensors_0_bmp280_1["value"] = pressure_event.pressure;    
    }
}

void callBH1750(){
  if (currentMillis - previous_BH1750_Millis >= delayBH1750)
    {
      previous_BH1750_Millis += delayBH1750;
      float lux = lightMeter.readLightLevel();

      /********LIGHT INTENSITY TO SERIAL********/
      Serial.print("Fényerő: ");
      Serial.print(lux);
      Serial.println(" lux");
      Serial.println("------------------------------");
      /********FOR JSON SERIALIZE********/
      sensors_0_bh1750["name"] = "lux";
      sensors_0_bh1750["value"] = lux;
    }
}

void callMQ2(){
  if (currentMillis - previous_MQ2_Millis >= delayMQ2)
    {
      previous_MQ2_Millis += delayMQ2;
      int AirQval = analogRead(AirQ);

      /********AIRQUALITY TO SERIAL********/
      Serial.print("Levegőminőség: ");
      Serial.print(analogRead(AirQval));
      Serial.println(" ppm");

      /********FOR JSON SERIALIZE********/
      sensors_0_mq2["name"] = "airq";
      sensors_0_mq2["value"] = AirQval;
      
      if (AirQval > BadVal)
        {
          digitalWrite(RedLed, HIGH);
          digitalWrite(GreenLed, LOW);
          tone(Buzzer, 1000, 400);
        }
      else
        {
          digitalWrite(RedLed, LOW);
          digitalWrite(GreenLed, HIGH);
          noTone(Buzzer);
        }
    }
}

void OnOff(){
  byte buttonState = digitalRead(BUTTON_PIN);
  if (buttonState != lastButtonState)
    {
      lastButtonState = buttonState;
      power = (ledState == 1) ? 0: 1;
      sensors_0_power["name"] = "onoff";
      sensors_0_power["value"] = power;
      
      Serial.println((ledState == 0) ? "A mérés fut!": "A mérés áll!");
      if (buttonState == 0)
        {
          ledState = (ledState == 1) ? LOW: HIGH;
          digitalWrite(LED_PIN, ledState);
          digitalWrite(LED, LOW); 
          digitalWrite(RedLed, LOW);
          digitalWrite(GreenLed, LOW);
        }
    }
}
