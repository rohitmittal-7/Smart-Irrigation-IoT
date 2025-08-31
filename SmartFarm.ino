#define BLYNK_TEMPLATE_ID "TMPL3StyBgDFn"
#define BLYNK_TEMPLATE_NAME "SmartFarm"
#define BLYNK_AUTH_TOKEN "wLCjepXyoiEJJnpPMHqHVzKdbEkywkq5"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "DHT.h"

// WiFi credentials
char ssid[] = "OnePlus Nord CE4";
char pass[] = "1234554321";

// Pin configuration
#define MOISTURE_PIN A0      // Moisture sensor (Analog)
#define LDR_PIN D2           // Light sensor (Digital)
#define DHT_PIN D4           // DHT11 sensor 
#define RELAY_PIN D1         // Pump relay control

#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

// Variables
int moistureValue = 0;      // Will use V0
int ldrValue = 0;           // Will use V1 (Light)
float temperature = 0;      // Will use V2
float humidity = 0;         // Will use V3
bool manualPumpControl = false; // Pump control flag (V4)

BlynkTimer timer;

// Blynk virtual pin assignments:
// V0 - Soil Moisture
// V1 - Light (LDR)
// V2 - Temperature
// V3 - Humidity
// V4 - Pump Control


// Pump control (V4) - Toggle button in Blynk app
BLYNK_WRITE(V4) {
  manualPumpControl = param.asInt();
  digitalWrite(RELAY_PIN, manualPumpControl ? LOW : HIGH); // LOW = Relay ON
  Serial.print("Manual Pump Control: ");
  Serial.println(manualPumpControl ? "ON" : "OFF");
}

void sendSensorData() {
  // Read soil moisture (V0)
  moistureValue = analogRead(MOISTURE_PIN);
  Blynk.virtualWrite(V0, moistureValue);
  
  // Read light sensor (V1)
  ldrValue = digitalRead(LDR_PIN) == HIGH ? 0 : 1;
  Blynk.virtualWrite(V1, ldrValue);

  // Read DHT sensor (V2 & V3)
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (!isnan(h) && !isnan(t)) {
    humidity = h;
    temperature = t;
    Blynk.virtualWrite(V2, temperature);
    Blynk.virtualWrite(V3, humidity);
  } else {
    Serial.println("Failed to read DHT sensor!");
  }

  // Auto pump control (unless manual mode is active)
  if (!manualPumpControl) {
    if (moistureValue > 400) {  // Adjust threshold as needed
      digitalWrite(RELAY_PIN, HIGH);  // Pump ON
      Blynk.virtualWrite(V4, 1);     // Update Blynk app
    } else {
      digitalWrite(RELAY_PIN, LOW); // Pump OFF
      Blynk.virtualWrite(V4, 0);     // Update Blynk app
    }
  }

  // Debug output
  Serial.print("Moisture: "); Serial.print(moistureValue);
  Serial.print(" | Light: "); Serial.print(ldrValue);
  Serial.print(" | Temp: "); Serial.print(temperature);
  Serial.print(" | Humidity: "); Serial.print(humidity);
  Serial.print(" | Pump: "); Serial.println(manualPumpControl ? "MANUAL" : (digitalRead(RELAY_PIN) == HIGH ? "AUTO-ON" : "AUTO-OFF"));
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Initially OFF
  pinMode(LDR_PIN, INPUT);
  
  dht.begin();
  
  // Connect to Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // Setup timer to send data every 2 seconds
  timer.setInterval(2000L, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();
}
