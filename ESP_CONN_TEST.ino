//ThatsEngineering
//Sending Data from Arduino to NodeMCU Via Serial Communication
//NodeMCU code

//Include Lib for Arduino to Nodemcu
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define WLAN_SSID       "Hopylak_2"
#define WLAN_PASS       "hopyka2019"
/*
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "Kela"
#define AIO_KEY         "aio_yYok94KWY9L9eJJurWi9KzL0yH61"
*/
#define AIO_SERVER      "192.168.0.27"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "mqtt-user"
#define AIO_KEY         "1111"

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish pub_onoff = Adafruit_MQTT_Publish(&mqtt, "lakas/sensors/onoff");
Adafruit_MQTT_Publish pub_dht11_temp = Adafruit_MQTT_Publish(&mqtt, "lakas/dht11/temp");
Adafruit_MQTT_Publish pub_dht11_hum = Adafruit_MQTT_Publish(&mqtt, "lakas/dht11/hum");
Adafruit_MQTT_Publish pub_bmp280_temp = Adafruit_MQTT_Publish(&mqtt, "lakas/bmp280/temp");
Adafruit_MQTT_Publish pub_bmp280_press = Adafruit_MQTT_Publish(&mqtt, "lakas/bmp280/press");

Adafruit_MQTT_Subscribe sub_onoff = Adafruit_MQTT_Subscribe(&mqtt, "lakas/sensors/sub/onoff");


uint32_t x=0;

//D6 = Rx & D5 = Tx
SoftwareSerial nodemcu(5, 4);

char sensor, value;


void setup() {
  // Initialize Serial port
  Serial.begin(9600);
  nodemcu.begin(9600);
  while (!Serial) continue;

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  MQTT_connect();
}

void loop() {

const size_t capacity = JSON_ARRAY_SIZE(1) + 2*JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + 8*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(6) + 270;
DynamicJsonBuffer jsonBuffer(capacity);

JsonObject& data = jsonBuffer.parseObject(nodemcu);
JsonObject& sensors = data["sensors"][0];

 if (data == JsonObject::invalid()) {
    //Serial.println("Invalid Json Object");
    jsonBuffer.clear();
    return;
  }

    //data.prettyPrintTo(Serial);


const char* sensors_0_dht11_0_name = sensors["dht11"][0]["name"]; // "temperature"
float sensors_0_dht11_0_value = sensors["dht11"][0]["value"]; // "20.12"

const char* sensors_0_dht11_1_name = sensors["dht11"][1]["name"]; // "humidity"
float sensors_0_dht11_1_value = sensors["dht11"][1]["value"]; // "54.30"

const char* sensors_0_bmp280_0_name = sensors["bmp280"][0]["name"]; // "temperature"
float sensors_0_bmp280_0_value = sensors["bmp280"][0]["value"]; // "20.12"

const char* sensors_0_bmp280_1_name = sensors["bmp280"][1]["name"]; // "pressure"
float sensors_0_bmp280_1_value = sensors["bmp280"][1]["value"]; // "1012.34"

const char* sensors_0_pirmini3_name = sensors["pirmini3"]["name"]; // "motion"
const char* sensors_0_pirmini3_value = sensors["pirmini3"]["value"]; // "true"

const char* sensors_0_bh1750_name = sensors["bh1750"]["name"]; // "lux"
const char* sensors_0_bh1750_value = sensors["bh1750"]["value"]; // "125.55"

const char* sensors_0_mq2_name = sensors["mq2"]["name"]; // "airq"
const char* sensors_0_mq2_value = sensors["mq2"]["value"]; // "345"

const char* sensors_0_power_name = sensors["power"]["name"]; // "onoff"
int sensors_0_power_value = sensors["power"]["value"]; // "1"
if(sensors_0_power_value == 1){
  //futnia kell
  
  pub_dht11_temp.publish(sensors_0_dht11_0_value);
    pub_dht11_hum.publish(sensors_0_dht11_1_value);
pub_bmp280_temp.publish(sensors_0_bmp280_0_value);
pub_bmp280_press.publish(sensors_0_bmp280_1_value);
pub_onoff.publish(sensors_0_power_value);
  
  Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription(1000))) {
      if (subscription == &sub_onoff) {
        Serial.print(F("Got: "));
        Serial.println((int)sub_onoff.lastread);
      }
    }
}
else {
  //nincs futás
}

//Serial.println(sensors_0_bh1750_value);
    

  
  




  
 






  
}

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
