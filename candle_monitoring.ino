#include <Arduino.h>
#include <WiFiNINA.h>
#include <SPI.h>
#include <NTPClient.h>
#include <ArduinoHttpClient.h>
#include "WaveshareSharpDustSensor.h"
#include "config.h"


bool openedCandle = true;
String lokiClient = LOKI_CLIENT;
int port = 80;
WaveshareSharpDustSensor dustSensor;
WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP);
WiFiClient wifi;
HttpClient client = HttpClient(wifi, lokiClient, port);

char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASSWORD;


void setup(void)
{
  pinMode(DUST_SENSOR_LED, OUTPUT);
  digitalWrite(DUST_SENSOR_LED, LOW);                                     
  pinMode(FLAME_SENSOR, INPUT);
  pinMode(TOUCH_SENSOR, INPUT);
  pinMode(MOTOR_A, OUTPUT);
  pinMode(MOTOR_B, OUTPUT);
	Serial.begin(9600);  
  while (!Serial) {

    ; // wait for serial port to connect. Needed for native USB port only

  }  
 setupWiFi();    

  // Initialize a NTPClient to get time
  ntpClient.begin();                                 
	
}

void loop(void) {

  // Reconnect to WiFi if required
 if (WiFi.status() != WL_CONNECTED) {
   WiFi.disconnect();
   yield();
   setupWiFi();
 }

   // Update time via NTP if required
  while (!ntpClient.update()) {
    yield();
    ntpClient.forceUpdate();
  }

	// Get values
	float dustDensity = getDustValue();
  int flameValue = digitalRead(FLAME_SENSOR);
  int touchValue = digitalRead(TOUCH_SENSOR);

  // Get current timestamp
  unsigned long timestamp = ntpClient.getEpochTime();

	// Print results
	Serial.println(timestamp);
	Serial.println(dustDensity);
  
  if (flameValue) {
    Serial.println("No fire detected");
  } else {
    Serial.println("Fire detected");
  }

  if (touchValue) {
    Serial.println("Touched");
    if (openedCandle) {
       closeCandle();
    } else {
      openCandle();
    }
  } else {
    Serial.println("Not touched");
  }

   submitToLoki(timestamp, flameValue);
	delay(1000);
}

void setupWiFi() {
  Serial.print("Connecting to '");
  Serial.print(WIFI_SSID);
  Serial.print("' ...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(10000);
    Serial.print(".");
  }

  Serial.println("connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}


float getDustValue() {
  digitalWrite(DUST_SENSOR_LED, HIGH);
	delayMicroseconds(280);

	int dustMonitorVoltage = analogRead(DUST_SENSOR_AOUT);
	digitalWrite(DUST_SENSOR_LED, LOW);
	dustMonitorVoltage = dustSensor.Filter(dustMonitorVoltage);

	float dustDensity = dustSensor.Conversion(dustMonitorVoltage);
  return dustDensity;
}

void submitToLoki(unsigned long ts, int flameValue)
{
  String body = String("{\"streams\": [{ \"stream\": { \"candle_id\": \"1\", \"monitoring_type\": \"candle\"}, \"values\": [ [ \"") + ts + "000000000\", \"" + "flame=" + flameValue + "\" ] ] }]}";
  client.beginRequest();
  client.post("/loki/api/v1/push");
  client.sendHeader("Authorization", LOKI_TOKEN);
  client.sendHeader("Content-Type", "application/json");
   client.sendHeader("Content-Length", String(body.length()));
  client.beginBody();
  client.print(body);
  client.endRequest();

  // read the status code and body of the response
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
}

void openCandle() {
  Serial.println("Opening candle");
  digitalWrite(MOTOR_A, LOW);
  digitalWrite(MOTOR_B, HIGH); 
  delay(13500);
  digitalWrite(MOTOR_B, LOW); 
  openedCandle = true;
}

void closeCandle() {
  Serial.println("Closing candle");
  digitalWrite(MOTOR_A, HIGH);
  digitalWrite(MOTOR_B, LOW); 
  delay(12400);
  digitalWrite(MOTOR_A, LOW); 
  openedCandle = false;
}

