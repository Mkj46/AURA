#include <SPI.h>
#include <MFRC522.h>

// Define pins for RFID readers
#define RST_PIN 9
#define SDA1 6     // Entry Reader for Route 1
#define SDA2 7     // Entry Reader for Route 2
#define SDA3 10    // Entry Reader for Route 3
#define SDA4 2     // Exit Reader for Route 1
#define SDA5 4     // Exit Reader for Route 2
#define SDA6 3     // Exit Reader for Route 3

MFRC522 reader1(SDA1, RST_PIN);
MFRC522 reader2(SDA2, RST_PIN);
MFRC522 reader3(SDA3, RST_PIN);
MFRC522 reader4(SDA4, RST_PIN);
MFRC522 reader5(SDA5, RST_PIN);
MFRC522 reader6(SDA6, RST_PIN);

// Vehicle counts
int count1 = 0, count2 = 0, count3 = 0;

// Timing variables
unsigned long lastUpdateTime = 0;
bool firstUpdateSent = false;

// Emergency vehicle UIDs (example values, adjust as needed)
const String EMERGENCY_UID_1 = "84973202"; // Replace with actual UID
const String EMERGENCY_UID_2 = "E3DF7422"; // Replace with actual UID
bool emergencyDetected = false;
unsigned long emergencyStartTime = 0;

void setup() {
    Serial.begin(9600);
    SPI.begin();
    reader1.PCD_Init();
    reader2.PCD_Init();
    reader3.PCD_Init();
    reader4.PCD_Init();
    reader5.PCD_Init();
    reader6.PCD_Init();
    Serial.println("RFID System Ready");
}

void loop() {
    unsigned long currentTime = millis(); // Declare currentTime here for global scope access

    checkRFID(reader1, 1, true, currentTime);  // Route 1 Entry
    checkRFID(reader2, 2, true, currentTime);  // Route 2 Entry
    checkRFID(reader3, 3, true, currentTime);  // Route 3 Entry
    checkRFID(reader4, 1, false, currentTime); // Route 1 Exit
    checkRFID(reader5, 2, false, currentTime); // Route 2 Exit
    checkRFID(reader6, 3, false, currentTime); // Route 3 Exit

    // Send counts at t=25s, then every 30s
    if (!firstUpdateSent) {
        if (currentTime >= 25000) {
            sendCounts();
            lastUpdateTime = currentTime;
            firstUpdateSent = true;
        }
    } else if (!emergencyDetected && currentTime - lastUpdateTime >= 30000) {
        sendCounts();
        lastUpdateTime = currentTime;
    } else if (emergencyDetected && currentTime - emergencyStartTime >= 25000) {
        // After 25s of emergency, send counts and resume normal operation
        sendCounts();
        lastUpdateTime = currentTime;
        emergencyDetected = false;
        Serial.println("Emergency mode ended, resuming normal operation");
    }

    delay(500);
}

void checkRFID(MFRC522 &reader, int route, bool isEntry, unsigned long currentTime) {
    if (reader.PICC_IsNewCardPresent() && reader.PICC_ReadCardSerial()) {
        String uid = getUID(reader);
        if (uid == EMERGENCY_UID_1 || uid == EMERGENCY_UID_2) {
            emergencyDetected = true;
            emergencyStartTime = currentTime;
            Serial.print("Emergency vehicle detected at Route ");
            Serial.print(route);
            Serial.println(isEntry ? " Entry" : " Exit");
            Serial.println("EMERGENCY");
        } else if (!emergencyDetected || (emergencyDetected && currentTime - emergencyStartTime < 25000)) {
            if (isEntry) {
                switch (route) {
                    case 1: count1++; Serial.println("Entry detected on Route 1"); break;
                    case 2: count2++; Serial.println("Entry detected on Route 2"); break;
                    case 3: count3++; Serial.println("Entry detected on Route 3"); break;
                }
            } else {
                switch (route) {
                    case 1: if (count1 > 0) { count1--; Serial.println("Exit detected on Route 1"); } break;
                    case 2: if (count2 > 0) { count2--; Serial.println("Exit detected on Route 2"); } break;
                    case 3: if (count3 > 0) { count3--; Serial.println("Exit detected on Route 3"); } break;
                }
            }
        }
        reader.PICC_HaltA();
    }
}

String getUID(MFRC522 &reader) {
    String uid = "";
    for (byte i = 0; i < reader.uid.size; i++) {
        uid += String(reader.uid.uidByte[i] < 0x10 ? "0" : "");
        uid += String(reader.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    return uid;
}

void sendCounts() {
    if (emergencyDetected) {
        Serial.println("EMERGENCY");
    } else {
        Serial.print(count1);
        Serial.print(",");
        Serial.print(count2);
        Serial.print(",");
        Serial.println(count3);
    }
    Serial.print("Counts sent at t = ");
    Serial.print(millis() / 1000);
    Serial.println("s");
}