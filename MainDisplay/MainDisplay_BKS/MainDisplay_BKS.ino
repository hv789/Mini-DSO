#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h> // TFT display library

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 5 //4
#define TFT_RST 8
#define TFT_MOSI 6 //11
#define TFT_MISO 12
#define TFT_CLK 7 //13

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

// Analog input pin for signal
const int analogPin = A0;

// Set display dimensions
const int screenWidth = 320;
const int screenHeight = 240;

// Number of samples
const int numSamples = 320; // One sample per pixel width

// Sampling rate (time between samples in microseconds)
const int samplingInterval = 100;  // 100 microseconds = 10kHz sampling rate

// Buffer to store samples
int sampleBuffer[numSamples];

// Previous sample buffer for erasing the old waveform pixel
int prevSampleBuffer[numSamples];

// X-axis position for scrolling the waveform
int currentX = 0;

// FOR INTERNAL SINEWAVE
// Variables for simulating the sine wave
const float frequency = 10.0; // 10Hz sine wave
const float samplingRate = 10000.0; // 10kHz sampling rate
const float timeStep = 1.0 / samplingRate; // Time step between samples
unsigned long startTime = 0;

void setup() {
  Serial.begin(9600);

  // Initialize TFT display
  tft.begin();
  tft.setRotation(3); // Adjust rotation to fit your setup
  tft.fillScreen(ILI9341_BLACK);

  // Display initial message
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.print("Mini DSO");

  // Initialize sample buffer
  for (int i = 0; i < numSamples; i++) {
    sampleBuffer[i] = 0;
    prevSampleBuffer[i] = 0;
  }
}

void loop() {
  // Read a new sample from the analog input
  // int newSample = analogRead(analogPin);

  // FOR INTERNAL SINEWAVE
  float time = (millis() - startTime) / 1000.0;  // Convert millis to seconds
  int newSample = simulateSineWave(time);
  // int newSample = simulateTriangularWave(time);

  // Shift the waveform left, clearing old data as it moves
  scrollWaveform(newSample);

  // Small delay between frames
  delayMicroseconds(samplingInterval);
}


// FOR INTERNAL SINEWAVE
int simulateSineWave(float time) {
  // Sine wave formula: A * sin(2 * PI * f * t) + offset
  float amplitude = 512.0;  // Half of 1023 for analog range
  float offset = 512.0;     // Midpoint of 0-1023
  float sineValue = amplitude * sin(2 * PI * frequency * time) + offset;

  // Convert sine value to an integer in the range of 0-1023
  return (int)sineValue;
}

int simulateTriangularWave(float time) {
    // Parameters
    float amplitude = 512.0;  // Half of 1023 for the analog range
    float offset = 0;     // Midpoint of 0-1023
    float period = 1.0 / frequency;  // Period of the waveform

    // Calculate the time within the current period (wraps around after one period)
    float t = fmod(time, period);

    // Convert to a triangular wave: increases linearly for the first half of the period, decreases for the second half
    if (t < period / 2) {
        // Rising edge: linearly interpolate from 0 to amplitude
        return (int)(2 * amplitude * (t / (period / 2))) + offset;
    } else {
        // Falling edge: linearly interpolate from amplitude back to 0
        return (int)(2 * amplitude * (1 - (t - (period / 2)) / (period / 2))) + offset;
    }
}



// Function to scroll the waveform and draw the new sample
void scrollWaveform(int newSample) {
  // Map the new sample to the display height (0 to screenHeight)
  int newY = map(newSample, 0, 1023, screenHeight - 50, 50);
  
  // Erase the previous sample at the current X position by drawing a black pixel
  int prevY = prevSampleBuffer[currentX];
  tft.drawPixel(currentX, prevY, ILI9341_BLACK);

  // Draw the new sample as a green pixel
  tft.drawPixel(currentX, newY, ILI9341_GREEN);

  // Store the new sample in the previous buffer for clearing next time
  prevSampleBuffer[currentX] = newY;

  // Move to the next X position
  currentX++;

  // If we reach the end of the screen width, wrap around and start from the beginning
  if (currentX >= screenWidth) {
    currentX = 0;
    // Optionally: clear the screen when wrapping around
    // tft.fillScreen(ILI9341_BLACK);
    // Redraw axes after clearing
    tft.drawLine(0, screenHeight / 2, screenWidth, screenHeight / 2, ILI9341_WHITE); // X-axis
    tft.drawLine(0, 0, 0, screenHeight, ILI9341_WHITE);  // Y-axis
  }
}