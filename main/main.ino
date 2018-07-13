const int ledPin = 3;
const int microphonePin = A0;
double microphoneOffset = 0;

const int distanceTriggerPin = 4;
const int distanceOutPin = A1;
const int distanceEchoPin = 5;

const int lightPin = A1;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    pinMode(ledPin, OUTPUT);
    pinMode(distanceTriggerPin, OUTPUT);
    pinMode(distanceEchoPin, INPUT);
    Serial.begin(9600);

    // gaugeMicrophoneAnalog(3000);

    digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
    lightLoop();
}

void distanceLoop() {
    unsigned long distance = measureDistanceCm();
    Serial.println(distance);
    delay(100);
}

double spreadd = 0;
void micLoop() {
    const long measureWindowMillis = 10;
    const long startTime = millis();
    int minimum = 1023;
    int maximum = 0;
    while(millis() < startTime + measureWindowMillis) {
        int val = measureMicrophoneDigital();
        minimum = min(minimum, val);
        maximum = max(maximum, val);
    }
    int spread = maximum - minimum;
    spreadd = spreadd + 0.3 * ((double) spread - spreadd);

    double movingAverage;
    movingAverage = 1023 * sigmoid(spreadd, 0.7, 15);
    Serial.print(movingAverage);

    analogWrite(ledPin, constrain(movingAverage, 0, 1023));

    Serial.println("");
}

void gaugeMicrophoneAnalog(long measureWindowMillis) {
    microphoneOffset = 0;
    const long startTime = millis();
    double sumOfMeasurements = 0;
    long numMeasurements = 0;
    while(millis() < startTime + measureWindowMillis) {
        ++numMeasurements;
        sumOfMeasurements += measureMicrophoneAnalog();
    }
    microphoneOffset = sumOfMeasurements / numMeasurements;
}
double measureMicrophoneAnalog() {
    return (double) analogRead(microphonePin) - microphoneOffset;
}

double measureMicrophoneAnalogAbs() {
    return abs(measureMicrophoneAnalog());
}

int measureMicrophoneDigital() {
    return analogRead(microphonePin);
}

double sigmoid(double x, double a, double b) {
    return 1 / (1 + exp(-a * (x - b)));
}

unsigned long lastMeasurementCm = 0;
unsigned long measureDistanceCm() {
    digitalWrite(distanceTriggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(distanceTriggerPin, LOW);

    unsigned long deltaTus = pulseIn(distanceEchoPin, HIGH, 1e6);

    unsigned long distanceCm = deltaTus * 343 / 20000;
    if(distanceCm > 1000) {
        return lastMeasurementCm;
    } else {
        lastMeasurementCm = distanceCm;
        return distanceCm;
    }
}

double lightMovingAverage = 0;
void lightLoop() {
    int val = analogRead(lightPin);
    lightMovingAverage = lightMovingAverage + 0.1 * ((double) val - lightMovingAverage);
    Serial.print(lightMovingAverage);
    int ledValue = constrain(map(lightMovingAverage, 600, 900, 1023, 0), 0, 1023);
    Serial.print(", ");
    Serial.print(ledValue);
    Serial.println("");
    analogWrite(ledPin, ledValue);
}
