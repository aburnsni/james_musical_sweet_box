#include <Arduino.h>
#include <MIDI.h>
#include <Adafruit_NeoPixel.h>

const bool DEBUG = 0;

// variables for Neopixels
const uint8_t ledPin = 15;
const uint8_t numPixels = 12;
const uint8_t buttons = 12;

// variables for sliders
const uint8_t ccPin = A2;
const uint8_t octPin = A3;
int ccVal = 0;
int lastccVal = 0;
int octVal = 0;
int lastoctVal = 0;
int octave = 0;
int lastoctave = 0;


const uint8_t midiChannel = 1;
uint8_t debounce = 10;

// variables for buttons
const uint8_t buttonPin[buttons] =  {6, 5, 4, 3, 2, 14, 12, 11, 10, 9, 8, 7,};
bool buttonState[buttons];
bool playing[buttons*3];
long lasttrig[buttons];

MIDI_CREATE_DEFAULT_INSTANCE();
Adafruit_NeoPixel pixels(numPixels, ledPin, NEO_GRB + NEO_KHZ800);
uint32_t black = pixels.Color(0, 0, 0);
uint8_t saturation = 250;
uint8_t lightness = 20;

void turnonLED(uint8_t ledNum) {
  uint32_t rgbcolor = pixels.ColorHSV(ledNum*(65535/numPixels), saturation, lightness);
  pixels.setPixelColor(ledNum, rgbcolor);
  pixels.show();
}
void turnoffLED(uint8_t ledNum) {
  pixels.setPixelColor(ledNum, black);
  pixels.show();
}

void setup() {
  MIDI.begin();
  if (DEBUG) {
    Serial.begin(115200);
  }
  pixels.begin();
  pixels.clear();

  // Test Neopixels
  // for (uint8_t i= 0; i<numPixels; i++) {
  //   int32_t rgbcolor = pixels.ColorHSV(i*(65535/numPixels), saturation, lightness);
  //   pixels.setPixelColor(i, rgbcolor);
  //   pixels.show();
  // }
  
  for (uint8_t i=0; i<buttons; i++) {
    buttonState[i] = 0;
    playing[i] = 0;
    playing[i+12]= 0;
    playing[i+36] = 0;
    lasttrig[i] = millis();
    pinMode(buttonPin[i], INPUT_PULLUP);
  }
}

void loop() {

  // Read Buttons
  for (uint8_t i=0; i<buttons; i++) {
    buttonState[i] = digitalRead(buttonPin[i]);
    if ((buttonState[i] == LOW) && (playing[i+12*octave] == false) && (millis() - lasttrig[i] > debounce)) {
      pixels.setPixelColor(i, pixels.Color((i+1)*10, 20, 0));
      pixels.show();
      turnonLED(i);
      if (DEBUG) {
        Serial.println(buttonPin[i]);
      } else {
        MIDI.sendNoteOn(36+(octave*12)+i, 100, midiChannel);
      }
      playing[i+12*octave] = true;
      lasttrig[i] = millis();
    } else if ((buttonState[i] == HIGH) && (playing[i+12*octave] == true))  {
      turnoffLED(i);
      MIDI.sendNoteOff(36+(octave*12)+i, 100, midiChannel);
      playing[i+12*octave] = false;
    }
  }

  // Read Sliders
  ccVal = analogRead(ccPin);
 
//
//  Add 100nF cap between GND and signal of slider
//  Maybe 47uF elect accross PSU
//

  if (abs(ccVal-lastccVal) > 6) {
    if (DEBUG) {
      Serial.print(ccVal);
      Serial.print("\t");
      Serial.println(ccVal/8);
    }
    else {
      MIDI.sendControlChange(16,127-(ccVal/8),1);
    }
    lastccVal = ccVal;
  }

  octVal = analogRead(octPin);
  if (abs(lastoctVal-octVal) >6) {
    if (octVal< 40) {
      octave = 0;
    } else if (octVal < 880) {
      octave = 1;
    } else {
      octave = 2;
    }
    if (lastoctave != octave) {
      for (int i=0; i<buttons*3; i++) {
        if (playing[i] == true)  {
          turnoffLED(i);
          MIDI.sendNoteOff(36+i, 100, midiChannel);
          playing[i] = false;
        }
      }
    }
    if (DEBUG) {
      Serial.println(octave);
    }
    lastoctave = octave;
    lastoctVal = octVal;
  }
  // if (octVal< 15) {
  //   octave = 1;
  // } else if (octVal < 460) {
  //   octave = 2;
  // } else if (octVal < 990) {
  //   octave = 3;
  // } else {
  //   octave = 4;
  // }

}