/**************************************************************************/
/*!
    Based heavily on the AdaFruit Industries MMA8451 and NeoPixel examples

    @section  HISTORY

    v1.0  - First release
*/
/**************************************************************************/

#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
#include "Custom8451.h"
#include <Adafruit_NeoPixel.h>
#include "state_defs.h"

//Adafruit_MMA8451 mma = Adafruit_MMA8451();
Custom_MMA8451 mma = Custom_MMA8451();

// Pixel info
const int ledPin = 12;
const int pixelCount = 60;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(pixelCount, ledPin, NEO_GRB + NEO_KHZ800);

// This is a static value to update the whole strip at once
// More complex effects TBI
int color[3] = { 0, 0, 0 };

// Sequence for effects
const int seqLength = 7;
int seq[seqLength] = {1, 3, 1, 3, 1, 3, 2};
const int seq_displayLength = 3;
int seq_display[seq_displayLength] = {5, 6, 7};
int seq_display_index = 0;
int seq_state = STATE_DEFAULT;
unsigned long seq_timer = millis();

// Accelerometer variables
int currentState = 0;
const int accelLength = 10;
int accelState[accelLength] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

float x = 0.0;
float y = 0.0;
float z = 0.0;

// Loop timing
const int delayLoop = 250; //ms
unsigned long now = millis();


void setup(void) {
  Serial.begin(9600);

  for ( int i = 0; i<10; i++) {
    Serial.println(i);
    delay(1000);
  }
  
  Serial.println("Adafruit MMA8451 test!");
  

  while (! mma.begin()) {
    Serial.println("Couldnt start");
    delay(500);
  }
  Serial.println("MMA8451 found!");
  
  mma.setRange(MMA8451_RANGE_2_G);
  
  Serial.print("Range = "); Serial.print(2 << mma.getRange());  
  Serial.println("G");

  Serial.println(mma.initializeTap());

  pixels.begin();

  delay(1000);
 
}

void loop() {
  // Read the 'raw' data in 14-bit counts
  mma.read();
  Serial.print("X:\t"); Serial.print(mma.x); 
  Serial.print("\tY:\t"); Serial.print(mma.y); 
  Serial.print("\tZ:\t"); Serial.print(mma.z); 
  Serial.println();

  /* Get a new sensor event */ 
  sensors_event_t event; 
  mma.getEvent(&event);

  /* Display the results (acceleration is measured in m/s^2) */
  x = event.acceleration.x;
  y = event.acceleration.y;
  z = event.acceleration.z;
  Serial.print("X: \t"); Serial.print(x); Serial.print("\t");
  Serial.print("Y: \t"); Serial.print(y); Serial.print("\t");
  Serial.print("Z: \t"); Serial.print(z); Serial.print("\t");
  Serial.println("m/s^2 ");

  x = abs(x);
  y = abs(y);
  z = abs(z);

  if ( max3(x,y,z) == x ) { currentState = 1; }
  if ( max3(x,y,z) == y ) { currentState = 2; }
  if ( max3(x,y,z) == z ) { currentState = 3; }
 
  /* Get the orientation of the sensor */
  uint8_t o = mma.getOrientation();
  
  switch (o) {
    case MMA8451_PL_PUF: 
      Serial.println("Portrait Up Front");
      break;
    case MMA8451_PL_PUB: 
      Serial.println("Portrait Up Back");
      break;    
    case MMA8451_PL_PDF: 
      Serial.println("Portrait Down Front");
      break;
    case MMA8451_PL_PDB: 
      Serial.println("Portrait Down Back");
      break;
    case MMA8451_PL_LRF: 
      Serial.println("Landscape Right Front");
      break;
    case MMA8451_PL_LRB: 
      Serial.println("Landscape Right Back");
      break;
    case MMA8451_PL_LLF: 
      Serial.println("Landscape Left Front");
      break;
    case MMA8451_PL_LLB: 
      Serial.println("Landscape Left Back");
      break;
    }

  if ( currentState != accelState[0] ) {
    // Update the previous states
    // For each item in the accelState array, starting at the second to last item
    // and moving back through the list to the first item
    for (int i = accelLength - 1; i >= 0; i--) {
      // Set the next item in the array to the value of this item
      accelState[i + 1] = accelState[i];
    }
    // Update the first item in the array to the current state
    accelState[0] = currentState;

    if ( millis() > seq_timer) {
      color[0] = 0;
      color[1] = 0;
      color[2] = 0;

      switch ( seq_state ) {
        case STATE_DEFAULT:
          if ( checkSequence( seq, 5 ) ) {
            seq_state = STATE_SEQ_1_1;
            seq_timer = millis() + 100;
            color[0] = 0;
            color[1] = 150;
            color[2] = 0;
          }
          break;
        case STATE_SEQ_1_1:
          if ( checkSequence( seq, 6 ) ) {
            seq_state = STATE_SEQ_1_2;
            seq_timer = millis() + 100;
            color[0] = 150;
            color[1] = 75;
            color[2] = 0;
          } else {
            seq_state = STATE_DEFAULT;
          }
          break;
        case STATE_SEQ_1_2:
          if ( checkSequence( seq, 7 ) ) {
            seq_state = STATE_SEQ_1_3;
            seq_timer = millis() + 2500;
            clearAccelHistory;
            color[0] = 150;
            color[1] = 0;
            color[2] = 0;
          } else {
            seq_state = STATE_DEFAULT;
          }
          break;
        case STATE_SEQ_1_3:
          seq_state = STATE_DEFAULT;  
          break;
      }
    updateLED();  
    }
  }

  Serial.print("STATE: \t"); Serial.print(currentState); Serial.print("\t");
  
  // Read tap register and report
  uint8_t tap_events = mma.getTapEvents();
  Serial.println(tap_events);
  if (tap_events & 128 == 128) {
    Serial.println("Tap detected!!!");
    //delay(2500);
  }
  
  Serial.println();

  // This is the dumb way to delay the loop
  // Better way TBI
  delay(delayLoop);
  
}

float max3(float a, float b, float c) {
  return max(max(a, b), c);
}

boolean checkSequence(int *array_a, int array_len) {
  // Compare array elements. If different, return false
  for ( int i = 0; i < array_len; i++ ) {
    if ( array_a[i] != accelState[array_len - i - 1] ) {  // As accelState is ordered newest to oldest, reverse iteration order for check
      return false;
    }
  }
  return true;
}

void clearAccelHistory(void) {
  for ( int i = 0; i < accelLength; i++ ) {
    accelState[i] = 0;
  }
}

void updateLED(void) {
  for ( int i = 0; i < pixelCount; i++ ) {
    pixels.setPixelColor(i, pixels.Color(color[0],color[1],color[2]));
  }
  pixels.show();
}

