#ifndef Cycle_h
#define Cycle_h

#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE(); // MIDI library init

#include "Display.h"
#include "Motherboard12.h"


/*
 * Sequencer
 */
class Cycle{
  private:
    static Cycle *instance;
    Cycle();
    
    // Motherboard
    Motherboard12 *device;
    
    enum ClockMode { Leading = 0, Following = 1 };
    ClockMode clockMode = Leading;
    
    bool isPlaying = true;
    byte currentStep = 0;
    unsigned int timeBetweenSteps = 0;
    unsigned int timeBetweenTicks = 0;
    elapsedMillis stepClock;
    elapsedMillis ticksClock;
    byte ticksReadings;
    elapsedMillis lastTick;
    unsigned int *ticks;

    enum Direction { Forward, Backward, Pendulum, Transposer };
    Direction direction = Forward;
    bool pendulumState = true;
    byte scales[4][12] = {
      {0,1,2,3,4,5,6,7,8,9,10,11}, // All notes
      {0,1,2,3,3,5,5,7,7,9,10,10}, // Adonai Malakh (Israel)
      {0,0,3,3,5,5,6,6,7,7,10,10}, // Blues
      {0,0,3,3,4,4,6,6,8,9,10,10} // Aeolian flat 1
    };
    byte scale = 0;
    byte tempo = 0;
    elapsedMillis bounceClock;
    byte notes[8];
    bool activeNotes[8];
    Display *display;
    void iterateDirection();
    byte getDirectionIndex();


  public:
    static Cycle *getInstance();
    void init();
    void update();
    byte getCurrentStep();
    void setTempo(byte tempo);
    void sendNoteOn(byte note);
    void sendNoteOff(byte note);
    void sendStart();
    void sendStop();
    
    // Midi callbacks
    static void onClock();
    static void onSongPosition(unsigned beats);
    static void onStart();
    static void onStop();
    
    // Controls callbacks
    // Clock
    static void onClockShortPress(byte inputIndex);
    static void onClockLongPress(byte inputIndex);
    static void onClockChange(bool value);
    // Direction
    static void onDirectionPress(byte inputIndex);
    // Scale
    static void onScalePress(byte inputIndex);
    // Accent
    static void onAccentPress(byte inputIndex);
    
    void onStepIncrement();
};

// Singleton pre init
Cycle * Cycle::instance = nullptr;

/**
 * Constructor
 */
inline Cycle::Cycle(){
  this->device = Motherboard12::getInstance();
  this->display = new Display();
  this->setTempo(60);

  this->ticksReadings = 0;
  this->ticks = new unsigned int[20];
  for (byte i = 0; i < 20; i++) {
    this->ticks[i] = 0;
  }

  for(byte i=0; i<8; i++){
    this->notes[i] = 0;
  }
  
  for(byte i=0; i<8; i++){
    this->activeNotes[i] = false;
  }
}

/**
 * Singleton instance
 */
inline Cycle *Cycle::getInstance()    {
  if (!instance)
     instance = new Cycle;
  return instance;
}

/**
 * Init
 */
inline void Cycle::init(){
  // 0 = empty, 1 = button, 2 = potentiometer, 3 = encoder
  byte controls[12] = {2,2,2,2, 2,2,2,2, 3,1,1,1};
  this->device->init(controls);
  
  // Midi callbacks
  MIDI.setHandleSongPosition(onSongPosition);
  MIDI.setHandleClock(onClock);
  MIDI.setHandleStart(onStart);
  MIDI.setHandleStop(onStop);
  MIDI.begin(this->device->getMidiChannel());
  
  // Device callbacks
  this->device->setHandlePressUp(8, onClockShortPress);
  this->device->setHandleLongPressUp(8, onClockLongPress);
  this->device->setHandleRotaryChange(8, onClockChange);
  this->device->setHandlePressUp(9, onDirectionPress);
  this->device->setHandlePressUp(10, onScalePress);
}

inline void Cycle::update(){
  this->device->update();
  
  MIDI.read(this->device->getMidiChannel());
//  usbMIDI.read(this->device->getMidiChannel());
  
  switch(this->display->getCurrentDisplayMode()){
    case DisplayMode::Sequencer:
    {
      this->display->setCursor(this->currentStep);

      // Reading the steps inputs
      for(byte i=0; i<8; i++){
        unsigned int noteInput = this->device->getInput(i);
        if(noteInput == 0){
          this->notes[i] = 0;
        }else{
          byte note = map(this->device->getInput(i), this->device->getAnalogMinValue(), this->device->getAnalogMaxValue(), 60, 84);
          if(this->scale > 0){
            byte octave = note / 12;
            note = 12 * octave + this->scales[this->scale][note % 12];
          }
          // If the sequencer is not playing and a step has changed
          if(!this->isPlaying && note != this->notes[i] && !this->activeNotes[i]){
            // Play the note
            this->sendNoteOn(note);
            this->activeNotes[i] = true;
          }
          this->notes[i] = note;
        }
      }
      
      break;
    }
    default:
    break;
  }

  // Clock
  switch(this->clockMode){
    case ClockMode::Leading:
      if(this->tempo > 0 && this->ticksClock >= this->timeBetweenTicks){
//        MIDI.sendClock();
        this->ticksClock = 0;
      }
    break;
    
    case ClockMode::Following:
    break;
  }
      
  if(this->tempo > 0 && this->stepClock >= this->timeBetweenSteps){
    this->stepClock = 0;

    // Moving steps
    if (this->isPlaying) {
      this->sendNoteOff(this->notes[this->currentStep]);
      this->activeNotes[this->currentStep] = false;
  
      switch(this->direction){
        case Forward:
        default:
          this->currentStep++;
          
          if(this->currentStep == 8){
            this->currentStep = 0;
          }
        break;
        case Backward:
          this->currentStep--;
        break;
        
        case Pendulum:
          if(this->pendulumState){
            this->currentStep++;
            if(this->currentStep > 7){
              this->currentStep = 0;
              this->pendulumState = !this->pendulumState;
            }
          }else{
            this->currentStep--;
            if(this->currentStep == 0){
              this->pendulumState = !this->pendulumState;
            }
          }
        break;
      }
      
      this->currentStep = constrain(this->currentStep, 0, 7);
      
      // Step increment
      this->onStepIncrement();
    }
    else{
      // If not playing then stop any note still active
      for(byte i=0; i<8; i++){
        if(this->activeNotes[i]){
          this->sendNoteOff(this->notes[i]);
          this->activeNotes[i] = false;
        }
      }
    }
  }
  
  this->display->update();
}

inline byte Cycle::getCurrentStep(){
  return this->currentStep;
}

inline void Cycle::setTempo(byte tempo){
  this->tempo = tempo;
  this->timeBetweenSteps = (float)1/tempo*60*1000/4;
  this->timeBetweenTicks = (float)1/tempo*60*1000/24;
}

inline void Cycle::sendNoteOn(byte note){
  MIDI.sendNoteOn(note, 127, this->device->getMidiChannel());
  usbMIDI.sendNoteOn(note, 127, this->device->getMidiChannel());
}

inline void Cycle::sendNoteOff(byte note){
  MIDI.sendNoteOff(note, 127, this->device->getMidiChannel());
  usbMIDI.sendNoteOff(note, 127, this->device->getMidiChannel());
}

inline void Cycle::sendStart(){
  MIDI.sendStart();
}
inline void Cycle::sendStop(){
  MIDI.sendStop();
  for(byte i=0; i<8; i++){
    this->sendNoteOff(this->notes[i]);
    this->activeNotes[i] = false;
  }
}

inline void Cycle::iterateDirection(){
  switch(this->direction){
    case Forward:
      this->direction = Backward;
    break;
    case Backward:
      this->direction = Pendulum;
    break;
    case Pendulum:
      this->direction = Transposer;
    break;
    case Transposer:
      this->direction = Forward;
    break;
  }
}

inline byte Cycle::getDirectionIndex(){
  switch(this->direction){
    case Forward:
      return 0;
    break;
    case Backward:
      return 1;
    break;
    case Pendulum:
      return 2;
    break;
    case Transposer:
      return 3;
    break;
  }

  return 0;
}


/**
 * Midi clock callback
 */
inline void Cycle::onClock(){
  switch(getInstance()->clockMode){
    case ClockMode::Following:

      if(getInstance()->ticksReadings == 0){
        getInstance()->lastTick = 0;
      }else{
        getInstance()->ticks[getInstance()->ticksReadings] = getInstance()->lastTick;
        getInstance()->lastTick = 0;
      }
      
      if (getInstance()->ticksReadings == 20) {
        unsigned int averageTime = 0;
        for (byte i = 0; i < 20; i++) {
          averageTime += getInstance()->ticks[i];
        }
        averageTime = averageTime / 19; // index 0 is always 0
        getInstance()->setTempo((byte)((float)1/averageTime*60*1000/24));

        // Reinit the ticks
        getInstance()->ticksReadings = 0;
        for (byte i = 0; i < 20; i++) {
          getInstance()->ticks[i] = 0;
        }
      }else{
        getInstance()->ticksReadings++;
      }
    break;
    
    default:
    break;
  }
}

/**
 * Midi Song position callback
 */
inline void Cycle::onSongPosition(unsigned songPosition){
  switch(getInstance()->clockMode){
    case ClockMode::Following:
      if(songPosition == 0){
//        getInstance()->currentStep = songPosition % 8;
        getInstance()->currentStep = 0;
        
        // Step increment
        getInstance()->onStepIncrement();
      }
    break;
    
    default:
    break;
  }
}

/**
 * Midi Start callback
 */
inline void Cycle::onStart(){
  switch(getInstance()->clockMode){
    case ClockMode::Following:
      Serial.println("onStart");
    break;
    
    default:
    break;
  }
}

/**
 * Midi Stop callback
 */
inline void Cycle::onStop(){
  switch(getInstance()->clockMode){
    case ClockMode::Following:
      Serial.println("onStop");
    break;
    
    default:
    break;
  }
}

/**
 * On Clock short press 
 */
inline void Cycle::onClockShortPress(byte inputIndex){
  switch(getInstance()->display->getCurrentDisplayMode()){
    case DisplayMode::Sequencer:
      // Play / Stop
      getInstance()->isPlaying = !getInstance()->isPlaying;
      getInstance()->currentStep = 0;
      getInstance()->bounceClock = 0;
      getInstance()->stepClock = 0;
      if(!getInstance()->isPlaying){
        getInstance()->sendStop();
      }
    break;
    
    case DisplayMode::Clock:
      // Exiting the Clock mode
      getInstance()->display->setCurrentDisplay(DisplayMode::Sequencer);
    break;
    
    default:
    break;
  }
}

/**
 * On Clock long press 
 */
inline void Cycle::onClockLongPress(byte inputIndex){
  switch(getInstance()->display->getCurrentDisplayMode()){
    case DisplayMode::Sequencer:
      // Entering in Clock mode
      getInstance()->display->setCursor(getInstance()->clockMode);
      getInstance()->display->setCurrentDisplay(DisplayMode::Clock);
    break;

    default:
    break;
  }
}

/**
 * On Clock change 
 */
inline void Cycle::onClockChange(bool value){
  int inValue = 1;
  if(!value){
    inValue = -1;
  }
  
  switch(getInstance()->display->getCurrentDisplayMode()){
    case DisplayMode::Sequencer:
      getInstance()->tempo += inValue;
      if(getInstance()->tempo > 0){
        getInstance()->setTempo(getInstance()->tempo);
      }else{
        getInstance()->setTempo(0);
      }
    break;
    
    case DisplayMode::Clock:
      getInstance()->clockMode = ClockMode( (getInstance()->clockMode + inValue) % 2 );
      getInstance()->display->setCursor(getInstance()->clockMode);
    break;

    default:
    break;
  }
}


/**
 * On Direction press 
 */
inline void Cycle::onDirectionPress(byte inputIndex){
  Serial.println("onDirectionPress");
  switch(getInstance()->display->getCurrentDisplayMode()){
    case DisplayMode::Sequencer:
      getInstance()->display->setCursor(getInstance()->getDirectionIndex());
      getInstance()->display->setCurrentDisplay(DisplayMode::Direction);
    break;  
    
    case DisplayMode::Direction:
    {
      getInstance()->iterateDirection();
      getInstance()->display->setCursor(getInstance()->getDirectionIndex());
      getInstance()->display->keepCurrentDisplay();
      break;
    }  
    default:
    break;
  }
}

/**
 * On Scale press 
 */
inline void Cycle::onScalePress(byte inputIndex){
  switch(getInstance()->display->getCurrentDisplayMode()){
    case DisplayMode::Sequencer:
      getInstance()->display->setCursor(getInstance()->scale);
      getInstance()->display->setCurrentDisplay(DisplayMode::Scale);
    break;  
        
    case DisplayMode::Scale:
    {
      getInstance()->scale = (getInstance()->scale + 1) % 8;
      getInstance()->display->setCursor(getInstance()->scale);
      getInstance()->display->keepCurrentDisplay();
    }
    break;  
    default:
    break;
  }
}

/**
 * On Accent press 
 */
inline void Cycle::onAccentPress(byte inputIndex){
  switch(getInstance()->display->getCurrentDisplayMode()){
    case DisplayMode::Sequencer:
      getInstance()->display->setCurrentDisplay(DisplayMode::Accent);
    break;    
    default:
    break;
  }
}

inline void Cycle::onStepIncrement(){
  switch(this->display->getCurrentDisplayMode()){
    case DisplayMode::Sequencer:
    case DisplayMode::Direction:
    case DisplayMode::Scale:
      if(this->notes[this->currentStep] > 0){
        this->sendNoteOn(this->notes[this->currentStep]);
        this->activeNotes[this->currentStep] = true;
      }else{
        this->sendNoteOff(this->notes[this->currentStep]);
        this->activeNotes[this->currentStep] = false;
      }
    break;
    
    default:
    break;
  }

  // Song position
//  switch(this->clockMode){
//    case ClockMode::Leading:
//      MIDI.sendSongPosition(this->currentStep);
//    break;
//    
//    default:
//    break;
//  }
}
#endif
