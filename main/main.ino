#include <fix_fft.h>

#define MEASUREMENTS_EXP 7
#define MEASUREMENTS (1 << MEASUREMENTS_EXP)

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
    Serial.begin(115200);

    gaugeMicrophoneAnalog(3000);

    digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
    micLoop();
    // fourierLoop();
}

double smoothMicMeasurement = 0;
double sigmoidCenter = 10;
double movingAverage = 0;
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
    smoothMicMeasurement = smoothMicMeasurement + 0.3 * ((double) spread - smoothMicMeasurement);

    double sigmoidMovingSpread = 0;
    movingAverage = movingAverage + 0.33 * (sigmoidMovingSpread - movingAverage);
    sigmoidMovingSpread = 1023 * sigmoid(movingAverage, 0.95, 1.1 * sigmoidCenter);

    // sigmoidMovingSpread = smoothMicMeasurement;
    Serial.print(movingAverage);

    analogWrite(ledPin, constrain(movingAverage, 0, 1023));

    sigmoidCenter = sigmoidCenter + 0.01 * (smoothMicMeasurement - sigmoidCenter);

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

void measureLoop() {
    char measurements[MEASUREMENTS];
    digitalWrite(LED_BUILTIN, HIGH);
    for(int i = 0; i < MEASUREMENTS; ++i) {
        measurements[i] = measureMicrophoneDigital() >> 2;
    }
    digitalWrite(LED_BUILTIN, LOW);
    // Serial.println(measurements);
    for(int i = 0; i < MEASUREMENTS; ++i) {
       Serial.println((int) measurements[i]);
    }
}

void fourierLoop() {
    char measurementsReal[MEASUREMENTS];
    char measurementsImag[MEASUREMENTS];
    digitalWrite(LED_BUILTIN, HIGH);
    for(int i = 0; i < MEASUREMENTS; ++i) {
        measurementsReal[i] = measureMicrophoneDigital() >> 2;
        measurementsImag[i] = 0;
    }
    digitalWrite(LED_BUILTIN, LOW);

    // fix_fft(char fr[], char fi[], int m, int inverse);
    fix_fft(measurementsReal, measurementsImag, MEASUREMENTS_EXP, 0);

    Serial.println((int) measurementsReal[3]);
}
