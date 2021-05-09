#include <Arduino.h>
#include <WiFiNINA.h>
#include <SPI.h>
#include <NTPClient.h>
#include <ArduinoHttpClient.h>
#include "WaveshareSharpDustSensor.h"
#include "config.h"

//Wifi and http clients
WiFiClient wifi;
WiFiUDP ntpUDP;
HttpClient lokiClient = HttpClient(wifi, LOKI_CLIENT, 80);
HttpClient candleClient = HttpClient(wifi, CANDLE_CLIENT, 80);
NTPClient ntpClient(ntpUDP);

char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASSWORD;

//Sensors
WaveshareSharpDustSensor dustSensor;

//Previous state of candle
bool prevCandleIsOpen = true;

//Setup
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
  //Get current timestamp
  unsigned long timestamp = ntpClient.getEpochTime();

  //Get current open/close state of candle
  bool candleIsOpen = getIsOpen();

  //Get values from sensors
  float dustDensity = getDustValue();
  int flameValue = digitalRead(FLAME_SENSOR);
  int touchValue = digitalRead(TOUCH_SENSOR);

  //Handle opening and closing if necssary
  handleOpeningAndClosing(candleIsOpen, prevCandleIsOpen, touchValue);
  
  //Send data to Loki
  submitToLoki(timestamp, flameValue);

  //Set prevCandleIsOpen
  prevCandleIsOpen = candleIsOpen;
  delay(2000);
}

//Helper functions
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

void openCandle() {
  Serial.println("Opening candle");
  digitalWrite(MOTOR_A, LOW);
  digitalWrite(MOTOR_B, HIGH); 
  delay(13500);
  digitalWrite(MOTOR_B, LOW); 
  toggleCandle();
}

void closeCandle() {
  Serial.println("Closing candle");
  digitalWrite(MOTOR_A, HIGH);
  digitalWrite(MOTOR_B, LOW); 
  delay(12400);
  digitalWrite(MOTOR_A, LOW); 
  toggleCandle();
}

bool getIsOpen() {
  candleClient.get("/isopen");
  String response = candleClient.responseBody();
  if(response.indexOf("1") > 0) {
     Serial.println("candle is open");
    return true;
  } 
  if(response.indexOf("0") > 0) {
    Serial.println("candle is closed");
    return false;
  }
}

void toggleCandle() {
  candleClient.get("/toggle");
}

void handleOpeningAndClosing(bool candleIsOpen, bool prevCandleIsOpen, bool touched) {   
  if (prevCandleIsOpen != candleIsOpen) {
    if (candleIsOpen) {
      openCandle();
    } else {
      closeCandle();
    }
  } else {
    if (touched) {
      if (candleIsOpen) {
        closeCandle();
      } else {
        openCandle();
      }
    }
  }
}

void submitToLoki(unsigned long ts, int flameValue)
{
  String body = String("{\"streams\": [{ \"stream\": { \"candle_id\": \"1\", \"monitoring_type\": \"candle\"}, \"values\": [ [ \"") + ts + "000000000\", \"" + "flame=" + flameValue + "\" ] ] }]}";
  lokiClient.beginRequest();
  lokiClient.post("/loki/api/v1/push");
  lokiClient.sendHeader("Authorization", LOKI_TOKEN);
  lokiClient.sendHeader("Content-Type", "application/json");
  lokiClient.sendHeader("Content-Length", String(body.length()));
  lokiClient.beginBody();
  lokiClient.print(body);
  lokiClient.endRequest();

  // Read the status code and body of the response
  int statusCode = lokiClient.responseStatusCode();
  Serial.print("Status code: ");
  Serial.println(statusCode);
}
