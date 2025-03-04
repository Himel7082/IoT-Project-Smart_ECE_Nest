#define BLYNK_TEMPLATE_ID "TMPL6alS5GpQ4"
#define BLYNK_TEMPLATE_NAME "SmartECENest"
#define BLYNK_AUTH_TOKEN "lStDIK3UP_T4ikAkrXqp0uWz7IP08xd8"

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Define Wi-Fi credentials
char ssid[] = "Love_Ruet";          
char pass[] = "passworddibona";      

// Define Blynk Auth Token
char auth[] = BLYNK_AUTH_TOKEN;  

// Define the DHT pin and type
#define DHTPIN 2      
#define DHTTYPE DHT11  

// Define L298N Motor Driver Pins
#define IN1 4         
#define IN2 5         
#define ENA 6         

// Define PIR Motion Sensor Pin
#define PIR_PIN 7     

// Define LED Pin
#define LED_PIN 8
#define Blynk_LED 9     

#define FIRE_SENSOR_PIN 18
#define LASER_PIN 10
#define Buzzer_PIN 3
#define Pump_PIN 0

#define Door_PIN 19

// Initialize the DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Global variables
bool enableFanControl = true;  // Fan control by DHT11
int manualFanSpeed = 0;  // Manual fan speed from slider
bool motionSensorEnabled = true;  // PIR sensor control (V7)
bool manualLedOn = false;  // LED manual control flag

// Handle Fan Control Toggle (V5)
BLYNK_WRITE(V5) {
    enableFanControl = param.asInt();
    Serial.println(enableFanControl ? "Fan control ENABLED" : "Fan control DISABLED");

    if (!enableFanControl) {
        analogWrite(ENA, manualFanSpeed);
    }
}

// Handle Fan Speed Slider (V4)
BLYNK_WRITE(V4) {
    manualFanSpeed = param.asInt();  
    if (!enableFanControl) {
        analogWrite(ENA, manualFanSpeed);
        Serial.print("Manual Fan Speed: ");
        Serial.println(manualFanSpeed);
    }
}

// Enable/Disable PIR Motion Sensor (V7)
BLYNK_WRITE(V7) {
    motionSensorEnabled = param.asInt();
    Serial.println(motionSensorEnabled ? "Motion Sensor ENABLED" : "Motion Sensor DISABLED");
}

// Handle Manual LED Button (V8)
BLYNK_WRITE(V8) {
    int buttonState = param.asInt();
    if (buttonState == 1) {
        digitalWrite(Blynk_LED, LOW);
        Serial.println("LED turned ON manually");
        manualLedOn = true;
    } else {
        digitalWrite(Blynk_LED, HIGH);
        Serial.println("LED turned OFF manually");
        manualLedOn = false;
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Smart Home System Initializing...");

    Blynk.begin(auth, ssid, pass);
    dht.begin();

    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(ENA, OUTPUT);
    pinMode(PIR_PIN, INPUT);  
    pinMode(LED_PIN, OUTPUT); 
    pinMode(Blynk_LED, OUTPUT);
    pinMode(FIRE_SENSOR_PIN, INPUT);
    pinMode(LASER_PIN, INPUT);
    pinMode(Buzzer_PIN, OUTPUT);
    pinMode(Pump_PIN, OUTPUT);
    pinMode(Door_PIN, INPUT);

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, 0);    

    digitalWrite(LED_PIN, LOW);
    digitalWrite(Pump_PIN, LOW);
    digitalWrite(Buzzer_PIN, LOW);
}

void loop() {
    Blynk.run();

    // Read temperature
    float temperature = dht.readTemperature();
    if (!isnan(temperature)) {
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.println("°C");
        Blynk.virtualWrite(V2, temperature);
    }

    // Read humidity
    float humidity = dht.readHumidity();
    if (!isnan(humidity)) {
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.println("%");
        Blynk.virtualWrite(V1, humidity);
    }

    // Fan control
    if (enableFanControl) {
        int fanSpeed = 0;
        if (temperature >= 25 && temperature <= 40) {
            fanSpeed = map(temperature, 25, 40, 0, 255);
        } else if (temperature > 40) {
            fanSpeed = 255;
        } else {
            fanSpeed = 0;
        }

        analogWrite(ENA, fanSpeed);
        Serial.print("Fan Speed: ");
        Serial.println(fanSpeed);
        Blynk.virtualWrite(V4, fanSpeed);
    }

    // Motion Detection - Only works if V7 is ON
    if (motionSensorEnabled) {
        int motionState = digitalRead(PIR_PIN);
        if (motionState == HIGH) {
            digitalWrite(LED_PIN, LOW);
            Serial.println("Motion detected! LED ON");
        } else {
            digitalWrite(LED_PIN, HIGH);
            Serial.println("No motion detected. LED OFF");
        }
    }

    int fireState = digitalRead(FIRE_SENSOR_PIN);
    if (fireState == LOW) {  // Fire detected
        Serial.println("🔥 FIRE ALERT! 🔥");
        digitalWrite(Buzzer_PIN, HIGH);
        delay (1000);
        digitalWrite(Pump_PIN, HIGH);
        Blynk.logEvent("fire"); // Send alert
        delay (1000);
    } else {
        digitalWrite(Pump_PIN, LOW);
        digitalWrite(Buzzer_PIN, LOW);
    }

    int doorState = digitalRead(Door_PIN);
    if (doorState == LOW){
    int theifState = digitalRead(LASER_PIN);
    if (theifState == HIGH) {  // Fire detected
        Serial.println("Thief Detected!");
        digitalWrite(Buzzer_PIN, HIGH);
        Blynk.logEvent("theif"); // Send alert
    } else {
        digitalWrite(Buzzer_PIN, LOW);
    }
    }

    delay(2000);  // Reduce delay for faster response
}