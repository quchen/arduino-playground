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

#define KERNEL_SIZE 5

double ker[KERNEL_SIZE] = {1,2,0,-2,-1};
double ringBuffer[KERNEL_SIZE] = {0,0,0,0,0};
int bufferIx = 0;

void micLoop() {

    unsigned long start = millis();
    unsigned long windowSize = 20;
    double micMeasurement = 0;
    double sumMicSquared = 0;
    int numMeasurements = 0;
    while(millis() < start + windowSize) {
        micMeasurement = measureMicrophoneAnalogAbs();
        sumMicSquared += micMeasurement * micMeasurement;
        ++numMeasurements;
        if(numMeasurements > 1000) {
            break;
        }
    }
    double averageMicPower = sumMicSquared / numMeasurements;
    ringBuffer[bufferIx] = averageMicPower;

    double convolutionResult = 0;
    for(int i = 0; i < KERNEL_SIZE; ++i) {
        convolutionResult += ker[i] * ringBuffer[(i + bufferIx) % KERNEL_SIZE];
    }
    bufferIx = (bufferIx + 1) % KERNEL_SIZE;

    int result = 1023 * sigmoid(convolutionResult, 0.25, 25);
    Serial.println(result);
    analogWrite(ledPin, result);
}
