#ifndef Display_h
#define Display_h

#include "Motherboard12.h"

enum DisplayMode { Clock, Sequencer, Direction, Scale, Octave };

/*
   Display
*/
class Display {
  private:
    Motherboard12 *device;
    DisplayMode currentDisplay;
    byte cursorIndex = 0;
    byte data[3];
    elapsedMillis clock_count;
    elapsedMillis clock_count_blink;
    elapsedMillis clock_count_display;
    const byte interval_time = 50;
    void displayClock();
    void displaySequencer();
    void displayDirection();
    void displayScale();
    void displayOctave();

  public:
    Display();
    void update();
    void setCurrentDisplay(DisplayMode displayMode);
    DisplayMode getCurrentDisplayMode();
    void setCursor(byte index);
    void setData(byte data[3]);
    void keepCurrentDisplay();
};


/**
   Constructor
*/
inline Display::Display() {
  this->device = Motherboard12::getInstance();
  this->currentDisplay = Sequencer;
}

inline void Display::update() {
  switch (this->currentDisplay) {
    // Clock display
    case Clock:
      this->displayClock();
    break;
    
    case Direction:
      this->displayDirection();
      
      // Go back to Pattern display after 1000ms
      if (this->clock_count_display >= 1000) {
        this->currentDisplay = Sequencer;
      }
    break;
    case Scale:
      this->displayScale();
        // Go back to Pattern display after 1000ms
      if (this->clock_count_display >= 1000) {
        this->currentDisplay = Sequencer;
      }
    break;
    case Octave:
      this->displayOctave();
        // Go back to Pattern display after 1000ms
      if (this->clock_count_display >= 1000) {
        this->currentDisplay = Sequencer;
      }
    break;
    case Sequencer:
    default:
      this->displaySequencer();
    break;
  }


}


inline void Display::setCurrentDisplay(DisplayMode displayMode) {
  this->currentDisplay = displayMode;
  this->clock_count_display = 0;
}

/**
 * Clock display
 */
inline void Display::displayClock() {
  this->device->resetAllLED();
  this->device->setLED(8, this->cursorIndex + 2);
}

inline void Display::displayDirection() {
  this->device->resetAllLED();
  this->device->setLED(this->cursorIndex, 3);
}

inline void Display::displayScale() {
  this->device->resetAllLED();
  this->device->setLED(this->cursorIndex, 3);
}

inline void Display::displayOctave() {
  this->device->resetAllLED();
  this->device->setLED(this->cursorIndex, 3);
}

inline void Display::displaySequencer(){
  this->device->resetAllLED();

  // Used by Transposer
  for(byte i=0; i<8; i++){

    // Set the LED off
    this->device->setLED(i, 0);
      
    // Set the LED solid
    byte solidLed = bitRead(this->data[0], i);
    if(solidLed){
      this->device->setLED(i, 1);
    }
    
    // Set the LED to blink
    byte blinkingLed = bitRead(this->data[1], i);
    if(blinkingLed){
      this->device->setLED(i, 3);
    }
  }
}

inline DisplayMode Display::getCurrentDisplayMode(){
  return currentDisplay;
}

inline void Display::setCursor(byte cursorIndex){
  this->cursorIndex = cursorIndex;
}

/**
 * Set the display's datas
 */
inline void Display::setData(byte data[3]){
  for(byte i=0; i<3; i++){
    this->data[i] = data[i];
  }
}

inline void Display::keepCurrentDisplay(){
  this->clock_count_display = 0;
}
#endif
