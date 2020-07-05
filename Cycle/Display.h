#ifndef Display_h
#define Display_h

#include "Motherboard12.h"

enum DisplayMode { Steps, Direction, Scale, Accent };

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
    void displaySteps();
    void displayDirection();
    void displayScale();
    void displayAccent();

  public:
    Display(Motherboard12 *device);
    void update();
    void setCurrentDisplay(DisplayMode displayMode);
    DisplayMode getCurrentDisplayMode();
    void setCursorIndex(byte index);
    void keepCurrentDisplay();
};


/**
   Constructor
*/
inline Display::Display(Motherboard12 *device) {
  this->device = device;
  this->currentDisplay = Steps;
}

inline void Display::update() {
  switch (this->currentDisplay) {
    case Direction:
      this->displayDirection();
      
      // Go back to Pattern display after 1000ms
      if (this->clock_count_display >= 1000) {
        this->currentDisplay = Steps;
      }
    break;
    case Scale:
      this->displayScale();
        // Go back to Pattern display after 1000ms
      if (this->clock_count_display >= 1000) {
        this->currentDisplay = Steps;
      }
    break;
    case Accent:
      this->displayAccent();
        // Go back to Pattern display after 1000ms
      if (this->clock_count_display >= 1000) {
        this->currentDisplay = Steps;
      }
    break;
    case Steps:
    default:
      this->displaySteps();
    break;
  }


}


inline void Display::setCurrentDisplay(DisplayMode displayMode) {
  //  if(this->currentDisplay != displayMode){
  this->currentDisplay = displayMode;
  this->clock_count_display = 0;
  //  }
}

inline void Display::displayDirection() {
  this->device->resetDisplay();

  for(byte i=0; i<4; i++){
    this->device->setDisplay(i, 1);
  }
  this->device->setDisplay(this->cursorIndex, 2);
}

inline void Display::displayScale() {
  this->device->resetDisplay();

}

inline void Display::displayAccent() {
  this->device->resetDisplay();

}

inline void Display::displaySteps(){
  for(byte i=0; i<8; i++){
    this->device->setDisplay(i, 0);
  }
  this->device->setDisplay(cursorIndex, 5);
}

inline DisplayMode Display::getCurrentDisplayMode(){
  return currentDisplay;
}

inline void Display::setCursorIndex(byte cursorIndex){
  this->cursorIndex = cursorIndex;
}

inline void Display::keepCurrentDisplay(){
  this->clock_count_display = 0;
}
#endif
