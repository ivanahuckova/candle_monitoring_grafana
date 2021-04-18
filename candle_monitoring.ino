#include <Arduino.h>
#include <WiFiNINA.h>
#include <SPI.h>
#include <NTPClient.h>
#include <ArduinoHttpClient.h>
#include "WaveshareSharpDustSensor.h"
#include "config.h"

WaveshareSharpDustSensor dustSensor;
WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP);
String lokiClient = String("https://") + LOKI_USER + ":" + LOKI_API_KEY + "@logs-prod-us-central1.grafana.net/loki/api/v1/push";
WiFiClient wifi;
HttpClient client = HttpClient(wifi, lokiClient, 443);

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



void setup(void)
{
	pinMode(DUST_SENSOR_LED, OUTPUT);
	digitalWrite(DUST_SENSOR_LED, LOW);                                     
  pinMode(FLAME_SENSOR, INPUT);
  pinMode(TOUCH_SENSOR, INPUT);
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
  } else {
    Serial.println("Not touched");
  }

  // submitToLoki(timestamp, flameValue);

	delay(1000);
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
  String contentType = "application/json";
  String body = String("{\"streams\": [{ \"stream\": { \"candle_id\": \"1\", \"monitoring_type\": \"candle\"}, \"values\": [ [ \"") + ts + "000000000\", \"" + "flame=" + flameValue + "\" ] ] }]}";
  client.post("/", contentType, body);
  Serial.println("ts: ");
  Serial.println(ts);
  // read the status code and body of the response
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
}
