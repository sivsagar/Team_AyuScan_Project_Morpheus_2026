// ─────────────────────────────────────────────
//  AyuScan | BioAmp EXG Pill + ESP32
//  Direct BPM Output (No Plotting)
// ─────────────────────────────────────────────

#define ECG_PIN 34           // BioAmp OUT connected to GPIO34 (ADC)
#define SAMPLE_DELAY 4       // 4 ms = 250 samples per second
#define ADC_RESOLUTION 12    // ESP32 ADC = 12-bit (0–4095)
#define THRESHOLD 2400       // Adjust based on your signal amplitude
#define HYSTERESIS 150       // Prevents false peaks (noise)

bool peakDetected = false;
unsigned long lastPeakTime = 0;
float bpm = 0;

// Optional smoothing
const int FILTER_SIZE = 10;
int buffer[FILTER_SIZE];
int indexBuffer = 0;
long sum = 0;

void setup() {
  Serial.begin(115200);
  analogReadResolution(ADC_RESOLUTION);
  Serial.println("\nAyuScan Direct BPM Reader Started...");
  Serial.println("Place electrodes and hold still...");
  delay(2000);
}

void loop() {
  // Read ECG value
  int rawValue = analogRead(ECG_PIN);

  // Moving average filter to reduce noise
  sum -= buffer[indexBuffer];
  buffer[indexBuffer] = rawValue;
  sum += buffer[indexBuffer];
  indexBuffer = (indexBuffer + 1) % FILTER_SIZE;
  int smoothValue = sum / FILTER_SIZE;

  // --- R-Peak detection ---
  if (smoothValue > THRESHOLD && !peakDetected) {
    peakDetected = true;
    unsigned long now = millis();
    unsigned long interval = now - lastPeakTime;

    if (interval > 300 && interval < 2000) { // 30–200 BPM valid range
      bpm = 60000.0 / interval;
      Serial.print("Heart Rate: ");
      Serial.print(bpm);
      Serial.println(" BPM");
    }

    lastPeakTime = now;
  }

  // Reset detection after signal drops
  if (smoothValue < THRESHOLD - HYSTERESIS) {
    peakDetected = false;
  }

  delay(SAMPLE_DELAY);
}
