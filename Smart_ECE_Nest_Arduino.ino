#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

// RFID Module Pins
#define SS_PIN 10
#define RST_PIN 9

// Ultrasonic Sensor Pins
#define TRIG_PIN 4
#define ECHO_PIN 7

// Servo Motor Pins
#define SERVO_DOOR 3
#define SERVO_DUSTBIN 6

#define Theif_PIN 2
#define Sensor_PIN 8
#define LED_PIN 5

// RFID Card UID
String UID = "63 7E 65 1A";
byte lock = 0;

// Object Creation
Servo doorLock;  
Servo dustbinLid;
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
    Serial.begin(9600);

    // Setup RFID
    SPI.begin();
    rfid.PCD_Init();
    
    // Setup LCD
    lcd.init();
    lcd.backlight();

    // Setup Servos
    doorLock.attach(SERVO_DOOR);
    dustbinLid.attach(SERVO_DUSTBIN);

    doorLock.write(70);  // Initially lock the door
    dustbinLid.write(0); // Initially, dustbin lid is closed

    // Setup Ultrasonic Sensor
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    pinMode(Theif_PIN, OUTPUT);
    digitalWrite(Theif_PIN, LOW);

    pinMode(Sensor_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
}

void loop() {
    checkRFID();     // Check for RFID card
    checkDistance(); // Check distance for smart dustbin

    int lightState = digitalRead(Sensor_PIN);
    if (lightState == HIGH) {  // Fire detected
        Serial.println("Night");
        digitalWrite(LED_PIN, HIGH);
    } else {
        digitalWrite(LED_PIN, LOW);
    }
}

// RFID Function
void checkRFID() {
    lcd.setCursor(2, 0);
    lcd.print("SmartECENest");
    lcd.setCursor(1, 1);
    lcd.print("Scan your card");

    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
        return;

    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Scanning");
    Serial.print("NUID tag is: ");
    String ID = "";

    for (byte i = 0; i < rfid.uid.size; i++) {
        lcd.print(".");
        ID.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
        ID.concat(String(rfid.uid.uidByte[i], HEX));
        delay(300);
    }
    ID.toUpperCase();

    if (ID.substring(1) == UID && lock == 0) {
        doorLock.write(10);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Door is locked");
        digitalWrite(Theif_PIN, LOW);
        delay(1500);
        lcd.clear();
        lock = 1;
    } else if (ID.substring(1) == UID && lock == 1) {
        doorLock.write(140);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Door is open");
        digitalWrite(Theif_PIN, HIGH);
        delay(1500);
        lcd.clear();
        lock = 0;
    } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Wrong card!");
        delay(1500);
        lcd.clear();
    }
}

// Ultrasonic Sensor & Dustbin Function
void checkDistance() {
    long duration;
    float distance;

    // Trigger ultrasonic sensor
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Read echo response
    duration = pulseIn(ECHO_PIN, HIGH);
    distance = (duration * 0.0343) / 2; // Convert duration to cm

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    // If an object (hand) is detected within 15 cm, open dustbin lid
    if (distance > 0 && distance < 15) {
        Serial.println("Hand detected! Opening lid...");
        dustbinLid.write(5); // Open dustbin
        delay(3000);  // Keep it open for 3 seconds
        Serial.println("Closing lid...");
        dustbinLid.write(100);  // Close dustbin
        delay(1000);
    }

    delay(500);
}
