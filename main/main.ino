// With direct audio input: low pass 75 ohm

const int ledPin = 3;
const int microphonePin = A0;
double microphoneOffset = 0;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    pinMode(ledPin, OUTPUT);
    Serial.begin(115200);

    gaugeMicrophoneAnalog(1000);

    digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
    micLoop();
    // measureLoop();
}

void gaugeMicrophoneAnalog(unsigned long measureWindowMillis) {
    microphoneOffset = 0;
    unsigned long startTime = millis();
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

double sigmoid(double x, double slope, double center) {
    return 1 / (1 + exp(-slope * (x - center)));
}

////////////////////////////////////////////////////////////////////////////////

#define KERNEL_SIZE 3

double ker[KERNEL_SIZE] = {1,0,-1};
double ringBuffer[KERNEL_SIZE] = {0,0,0};
int bufferIx = 0;
const double longTermSmoothingAlpha = 0.025;
double longTermAverage = 1;
const double ledThreshold = 1;

double ledFuel = 0;

void micLoop() {

    // Measure average microphone power (amplitude^2)j over a time interval
    unsigned long start = millis();
    unsigned long windowSize = 25;
    double micMeasurement = 0;
    double sumMicSquared = 0;
    int numMeasurements = 0;
    while(millis() < start + windowSize) {
        micMeasurement = measureMicrophoneAnalog();
        sumMicSquared += micMeasurement * micMeasurement;
        ++numMeasurements;
        if(numMeasurements > 1000) {
            break;
        }
    }
    double averageMicPower = sumMicSquared / numMeasurements;
    longTermAverage = longTermAverage + longTermSmoothingAlpha * (averageMicPower - longTermAverage);
    ringBuffer[bufferIx] = averageMicPower / longTermAverage;

    // Edge detection
    double slope = 0;
    {
        for(int i = 0; i < KERNEL_SIZE; ++i) {
            slope += ker[i] * ringBuffer[(i + bufferIx) % KERNEL_SIZE];
        }
        bufferIx = (bufferIx + 1) % KERNEL_SIZE;
    }

    double slopeN = sigmoid(slope, 10, ledThreshold);

    ledFuel = 0.9 * ledFuel + slopeN;
    int ledValue;
    if(ledFuel > 0.5) {
        ledValue = constrain((ledFuel - 0.5) * 255, 0, 255);
        analogWrite(ledPin, ledValue);
    } else {
        digitalWrite(ledPin, LOW);
    }

    Serial.print(slope);
    // Serial.print(",");
    // Serial.print(longTermAverage);
    Serial.print(",");
    Serial.print(ledFuel);

    Serial.println("");
}

void measureLoop() {
    double val = measureMicrophoneAnalog();
    Serial.println(val);
}
