#define GREEN1 6
#define RED1 7
#define GREEN2 2
#define RED2 3
#define GREEN3 4
#define RED3 5

int count1 = 0, count2 = 0, count3 = 0;
unsigned long lastDataReceivedTime = 0;
bool dataReceived = false;
int lastGreenRoute = 0;
int cyclesWithoutGreen[3] = {0, 0, 0};
bool emergencyMode = false;
unsigned long emergencyStartTime = 0;
const unsigned long emergencyDuration = 25000; // 25 seconds

void setup() {
    Serial.begin(9600);
    pinMode(GREEN1, OUTPUT);
    pinMode(RED1, OUTPUT);
    pinMode(GREEN2, OUTPUT);
    pinMode(RED2, OUTPUT);
    pinMode(GREEN3, OUTPUT);
    pinMode(RED3, OUTPUT);
    digitalWrite(GREEN1, LOW);
    digitalWrite(RED1, HIGH);
    digitalWrite(GREEN2, LOW);
    digitalWrite(RED2, HIGH);
    digitalWrite(GREEN3, LOW);
    digitalWrite(RED3, HIGH);
    Serial.println("LED System Ready");
}

void loop() {
    if (Serial.available()) {
        String data = Serial.readStringUntil('\n');
        data.trim();
        if (data == "EMERGENCY") {
            emergencyMode = true;
            emergencyStartTime = millis();
            lastDataReceivedTime = emergencyStartTime;
            dataReceived = true;
            Serial.println("Emergency mode activated at t = " + String(millis() / 1000) + "s");
        } else {
            int comma1 = data.indexOf(',');
            int comma2 = data.indexOf(',', comma1 + 1);
            if (comma1 != -1 && comma2 != -1 && data.indexOf(',', comma2 + 1) == -1) {
                count1 = data.substring(0, comma1).toInt();
                count2 = data.substring(comma1 + 1, comma2).toInt();
                count3 = data.substring(comma2 + 1).toInt();
                lastDataReceivedTime = millis();
                dataReceived = true;
                Serial.print("Counts received at t = ");
                Serial.print(millis() / 1000);
                Serial.print("s: ");
                Serial.println(data);
            } else {
                Serial.println("Invalid data format: " + data);
            }
        }
    }

    unsigned long currentTime = millis();
    if (emergencyMode && (currentTime - emergencyStartTime < emergencyDuration)) {
        if (currentTime - lastDataReceivedTime >= 5000) {
            setAllRed();
            Serial.println("All routes red at t = " + String(millis() / 1000) + "s");
            lastDataReceivedTime = currentTime; // Prevent repeated updates
        }
    } else if (emergencyMode && (currentTime - emergencyStartTime >= emergencyDuration)) {
        emergencyMode = false;
        Serial.println("Emergency mode ended at t = " + String(millis() / 1000) + "s");
    } else if (dataReceived && (currentTime - lastDataReceivedTime >= 5000)) {
        int greenRoute = selectGreenRoute();
        updateLEDs(greenRoute);
        updateCyclesWithoutGreen(greenRoute);
        Serial.print("LEDs updated at t = ");
        Serial.print(millis() / 1000);
        Serial.print("s: greenRoute = ");
        Serial.println(greenRoute);
        dataReceived = false;
        lastGreenRoute = greenRoute;
    }
}

void setAllRed() {
    digitalWrite(GREEN1, LOW); digitalWrite(RED1, HIGH);
    digitalWrite(GREEN2, LOW); digitalWrite(RED2, HIGH);
    digitalWrite(GREEN3, LOW); digitalWrite(RED3, HIGH);
}

int selectGreenRoute() {
    int counts[3] = {count1, count2, count3};
    int greenRoute = 0;
    int totalCount = count1 + count2 + count3;

    if (totalCount > 0) {
        for (int i = 0; i < 3; i++) {
            if (cyclesWithoutGreen[i] >= 3) {
                return i + 1;
            }
        }
    }

    int maxCount = max(count1, max(count2, count3));
    if (maxCount > 0) {
        if (counts[lastGreenRoute - 1] == maxCount) {
            int secondMax = -1;
            int secondMaxRoute = 0;
            for (int i = 0; i < 3; i++) {
                if (i + 1 != lastGreenRoute && counts[i] > secondMax) {
                    secondMax = counts[i];
                    secondMaxRoute = i + 1;
                }
            }
            greenRoute = (secondMax > 0) ? secondMaxRoute : lastGreenRoute;
        } else {
            if (count1 == maxCount) greenRoute = 1;
            else if (count2 == maxCount) greenRoute = 2;
            else if (count3 == maxCount) greenRoute = 3;
        }
    }
    return greenRoute;
}

void updateLEDs(int greenRoute) {
    digitalWrite(GREEN1, (greenRoute == 1) ? HIGH : LOW);
    digitalWrite(RED1, (greenRoute == 1) ? LOW : HIGH);
    digitalWrite(GREEN2, (greenRoute == 2) ? HIGH : LOW);
    digitalWrite(RED2, (greenRoute == 2) ? LOW : HIGH);
    digitalWrite(GREEN3, (greenRoute == 3) ? HIGH : LOW);
    digitalWrite(RED3, (greenRoute == 3) ? LOW : HIGH);
}

void updateCyclesWithoutGreen(int greenRoute) {
    for (int i = 0; i < 3; i++) {
        if (i + 1 == greenRoute) {
            cyclesWithoutGreen[i] = 0;
        } else if (count1 + count2 + count3 > 0) {
            cyclesWithoutGreen[i]++;
        }
    }
}