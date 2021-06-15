#include <Arduino.h>
#include <PromLokiTransport.h>
#include <PrometheusArduino.h>
#include "WaveshareSharpDustSensor.h"

#include "config.h"
#include "certificates.h"

//Http client
WiFiClient wifi;
HttpClient candleClient = HttpClient(wifi, CANDLE_CLIENT, 80);

// Prometheus client and transport
PromLokiTransport transport;
PromClient client(transport);

// Create a write request for 2 series.
WriteRequest req(2,1024);

// Define a TimeSeries which can hold up to 5 samples
TimeSeries ts1(5, "flame_value", "{monitoring_type=\"candle\",board_type=\"arduino_mkr1010\",room=\"living_room\"}");
TimeSeries ts2(5, "dust_density",  "{monitoring_type=\"candle\",board_type=\"arduino_mkr1010\",room=\"living_room\"}");

//Sensors
WaveshareSharpDustSensor dustSensor;

// Global variables
bool prevCandleIsOpen;
int loopCounter = 0;

//Setup
void setup(void)
{
  pinMode(DUST_SENSOR_LED, OUTPUT);
  digitalWrite(DUST_SENSOR_LED, LOW);                                     
  pinMode(MOTOR_A, OUTPUT);
  pinMode(MOTOR_B, OUTPUT);
  Serial.begin(9600);   
 
  setupClient();
  setupWiFi();    

  prevCandleIsOpen = getIsOpen(false);                               
}

void loop(void) {
  int64_t time;
  time = transport.getTimeMillis();
  
  //Get current open/close state of candle
  bool candleIsOpen = getIsOpen(prevCandleIsOpen);

  //Get values from sensors
  float dustDensity = getDustValue();
  int flameValue = getFlameValue();

  //Handle opening and closing if necssary
  handleOpeningAndClosing(candleIsOpen, prevCandleIsOpen);

   //Set prevCandleIsOpen
  prevCandleIsOpen = candleIsOpen;
  
  //Send data to Prometheus
  if (loopCounter >= 5) {
    //Send
    loopCounter = 0;
    PromClient::SendResult res = client.send(req);
    if (!res == PromClient::SendResult::SUCCESS) {
      Serial.println(client.errmsg);
    }
    
    // Reset batches after a succesful send.
    ts1.resetSamples();
    ts2.resetSamples();
  } else {
    if (!ts1.addSample(time, flameValue)) {
      Serial.println(ts1.errmsg);
    }
    if (!ts2.addSample(time, dustDensity)) {
      Serial.println(ts2.errmsg);
    }
    loopCounter++;
  }


  delay(INTERVAL * 1000);
}

//Helper functions
void setupWiFi() {
  Serial.print("Connecting to wifi ...'");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(5000);
    Serial.print(".");
  }

  Serial.println("connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Function to set up Prometheus client
void setupClient() {
  Serial.println("Setting up client...");
  
  uint8_t serialTimeout;
  while (!Serial && serialTimeout < 50) {
    delay(100);
    serialTimeout++;
  }
  
  // Configure and start the transport layer
  transport.setUseTls(true);
  transport.setCerts(grafanaCert, strlen(grafanaCert));
  transport.setWifiSsid(WIFI_SSID);
  transport.setWifiPass(WIFI_PASSWORD);
  transport.setDebug(Serial);  // Remove this line to disable debug logging of the client.
  if (!transport.begin()) {
      Serial.println(transport.errmsg);
      while (true) {};
  }

  // Configure the client
  client.setUrl(GC_PROM_URL);
  client.setPath(GC_PROM_PATH);
  client.setPort(GC_PORT);
  client.setUser(GC_PROM_USER);
  client.setPass(GC_PROM_PASS);
  client.setDebug(Serial);  // Remove this line to disable debug logging of the client.
  if (!client.begin()) {
      Serial.println(client.errmsg);
      while (true) {};
  }

  // Add our TimeSeries to the WriteRequest
  req.addTimeSeries(ts1);
  req.addTimeSeries(ts2);
  req.setDebug(Serial);  // Remove this line to disable debug logging of the write request serialization and compression.
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

int getFlameValue() {
  int flameValue = analogRead(FLAME_SENSOR);
  if (flameValue < 200) {
    return 1; 
  }
  return 0;
  
}
void closeCandle() {
  Serial.println("Closing candle");
  digitalWrite(MOTOR_A, LOW);
  digitalWrite(MOTOR_B, HIGH); 
  delay(4000);
  digitalWrite(MOTOR_B, LOW); 
}

void openCandle() {
  Serial.println("Opening candle");
  digitalWrite(MOTOR_A, HIGH);
  digitalWrite(MOTOR_B, LOW); 
  delay(4000);
  digitalWrite(MOTOR_A, LOW); 
}

bool getIsOpen(bool prevValue) {
  candleClient.get("/status");
  String response = candleClient.responseBody();
  if(response.indexOf("1") > 0) {
     Serial.println("candle is open");
    return true;
  } else if(response.indexOf("0") > 0) {
    Serial.println("candle is closed");
    return false;
  } else {
    return prevValue;
  }

}

void toggleCandle() {
  String path = String("/toggle?secret=") + CANDLE_SECRET;
  candleClient.get(path);
}

void handleOpeningAndClosing(bool candleIsOpen, bool prevCandleIsOpen) {   
  if (prevCandleIsOpen != candleIsOpen) {
    if (candleIsOpen) {
      openCandle();
    } else {
      closeCandle();
    }
  }
}

void goUp(int sec) {
  digitalWrite(MOTOR_A, HIGH);
  digitalWrite(MOTOR_B, LOW); 
  delay(sec);
  digitalWrite(MOTOR_A, LOW); 
}

void goDown(int sec) {
  digitalWrite(MOTOR_B, HIGH);
  digitalWrite(MOTOR_A, LOW); 
  delay(sec);
  digitalWrite(MOTOR_B, LOW); 
}

