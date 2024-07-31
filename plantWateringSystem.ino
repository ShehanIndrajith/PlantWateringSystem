#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>

#include <FirebaseArduino.h>

// Define the pins and sensor types
#define DHTPIN D2          // DHT11 data pin
#define DHTTYPE DHT11      // DHT 11
#define MOISTURE_PIN A0    // Moisture sensor analog pin
#define RELAY_PIN D3       // Relay control pin

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Initialize the LCD with I2C address 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Firebase configuration
#define FIREBASE_HOST "trial2-bdf92-default-rtdb.firebaseio.com/" // Firebase host
#define FIREBASE_AUTH "2LUgpZRyvT6aWsX0CUdkqDeLjNQ2Jl3JwvY2Qgyr" // Firebase auth code
int fireStatus = 0;
// WiFi credentials
#define WIFI_SSID "" // WiFi SSID
#define WIFI_PASSWORD "" // WiFi password

void setup() {
  // Initialize Serial communication
  Serial.begin(115200);

  // Initialize the DHT sensor
  dht.begin();
  
  // Initialize the relay pin as an output
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Start with the relay off
  
  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // Connect to Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
  // Read moisture sensor value
  int moistureValue = analogRead(MOISTURE_PIN);
  Firebase.setInt("WaterLevel", moistureValue);
  
  // Read temperature in Celsius and humidity from the DHT11 sensor
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  Firebase.setFloat("Humidity", humidity);
  Firebase.setFloat("Temp", temperature);

  // Check if any reads failed and exit early (to try again)
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error");
    return;
  }

  // Display moisture sensor value on Serial Monitor and LCD
  Serial.print("Moisture Sensor Value: ");
  Serial.println(moistureValue);
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Moist:");
  lcd.print(moistureValue);
  lcd.setCursor(0, 1);
  lcd.print("Temp:");
  lcd.print(temperature);
  lcd.print("C Hum:");
  lcd.print(humidity);
  lcd.print("%");

  // Control the water pump based on moisture sensor value
  bool isWatering = false;
  if (moistureValue <= 980) {
    digitalWrite(RELAY_PIN, HIGH); // Turn on the pump
    Serial.println("Watering");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Watering");
    delay(5000); // Run the pump for 5 seconds
    digitalWrite(RELAY_PIN, LOW); // Turn off the pump
    isWatering = true;

  }

  // Update Firebase with sensor data
 // Firebase.setFloat("Humidity", humidity);
 Firebase.setFloat("Temp", temperature);
  Firebase.setInt("WaterLevel", moistureValue);
  Firebase.setBool("status", isWatering);

  // Wait for a second before the next loop
  delay(1000);
}
