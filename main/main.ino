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

#define KERNEL_SIZE 3

double ker[KERNEL_SIZE] = {1,0,-1};
double ringBuffer[KERNEL_SIZE] = {0,0,0};
int bufferIx = 0;
const double longTermSmoothingAlpha = 0.025;
double longTermAverage = 1;
const double ledThreshold = 1;

void micLoop() {

    // Measure average microphone power (amplitude^2)j over a time interval
    {
        unsigned long start = millis();
        unsigned long windowSize = 50;
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
        longTermAverage = longTermAverage + longTermSmoothingAlpha * (averageMicPower - longTermAverage);
        ringBuffer[bufferIx] = averageMicPower / longTermAverage;
    }

    // Edge detection
    double convolutionResult = 0;
    {
        for(int i = 0; i < KERNEL_SIZE; ++i) {
            convolutionResult += ker[i] * ringBuffer[(i + bufferIx) % KERNEL_SIZE];
        }
        bufferIx = (bufferIx + 1) % KERNEL_SIZE;
    }




    double result = sigmoid(convolutionResult, 10, ledThreshold);
    analogWrite(ledPin, 1023 * result);


    Serial.print(convolutionResult);
    Serial.print(",");
    Serial.print(ledThreshold);
    Serial.print(",");
    Serial.print(5 * result);
    Serial.print(",");
    Serial.print(longTermAverage);

    Serial.println("");
}
