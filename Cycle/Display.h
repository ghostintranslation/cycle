#ifndef Display_h
#define Display_h

#include "Motherboard12.h"

enum DisplayMode { Clock, Sequencer, Direction, Scale, Accent };

/*
   Display
*/
class Display {
  private:
    Motherboard12 *device;
    DisplayMode currentDisplay;
    byte cursorIndex = 0;
    elapsedMillis clock_count;
    elapsedMillis clock_count_blink;
    elapsedMillis clock_count_display;
    const byte interval_time = 50;
    void displayClock();
    void displaySequencer();
    void displayDirection();
    void displayScale();
    void displayAccent();

  public:
    Display();
    void update();
    void setCurrentDisplay(DisplayMode displayMode);
    DisplayMode getCurrentDisplayMode();
    void setCursor(byte index);
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
    case Accent:
      this->displayAccent();
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
  //  if(this->currentDisplay != displayMode){
  this->currentDisplay = displayMode;
  this->clock_count_display = 0;
  //  }
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

  for(byte i=0; i<4; i++){
    this->device->setLED(i, 1);
  }
  this->device->setLED(this->cursorIndex, 2);
}

inline void Display::displayScale() {
  this->device->resetAllLED();

  this->device->resetAllLED();

  for(byte i=0; i<8; i++){
    this->device->setLED(i, 1);
  }
  this->device->setLED(this->cursorIndex, 2);
}

inline void Display::displayAccent() {
  this->device->resetAllLED();

}

inline void Display::displaySequencer(){
  for(byte i=0; i<8; i++){
    this->device->setLED(i, 0);
  }
  this->device->setLED(cursorIndex, 1);
}

inline DisplayMode Display::getCurrentDisplayMode(){
  return currentDisplay;
}

inline void Display::setCursor(byte cursorIndex){
  this->cursorIndex = cursorIndex;
}

inline void Display::keepCurrentDisplay(){
  this->clock_count_display = 0;
}
#endif
