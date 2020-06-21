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
    elapsedMillis clock_count;
    elapsedMillis clock_count_blink;
    elapsedMillis clock_count_display;
    const byte interval_time = 50;
    void displayDirection();
    void displayScale();
    void displayAccent();

  public:
    Display(Motherboard12 *device);
    void update();
    void setCurrentDisplay(DisplayMode displayMode);
    DisplayMode getCurrentDisplayMode();
    void displaySteps(byte currentStep);
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
//      this->displaySteps();
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
//  this->device->setDisplay(this->selectedVoice, 1);
}

inline void Display::displayScale() {
  this->device->resetDisplay();

}

inline void Display::displayAccent() {
  this->device->resetDisplay();

}

inline void Display::displaySteps(byte currentStep){
  this->device->resetDisplay();
  this->device->setDisplay(currentStep, 5);
}

inline DisplayMode Display::getCurrentDisplayMode(){
  return currentDisplay;
}

#endif
