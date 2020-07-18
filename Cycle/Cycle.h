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
    
    bool play = true;
    byte currentStep = 0;
    unsigned int timeBetweenSteps = 0;
    elapsedMillis stepClock;
    byte beatCount = 0;
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
    void sendStop();
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
inline static Cycle *Cycle::getInstance()    {
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

  MIDI.begin(MIDI_CHANNEL_OMNI);
}

inline void Cycle::update(){
  this->device->update();
  
  MIDI.read();
  usbMIDI.read();
  
  switch(this->display->getCurrentDisplayMode()){
    case DisplayMode::Steps:
    { // This bracket is to prevent issues with variables declared inside the case
      this->display->setCursorIndex(this->currentStep);
      
      // Reading buttons to switch mode
      bool directionInput = this->device->getInput(9);
      bool scaleInput = this->device->getInput(10);
      bool accentInput = this->device->getInput(11);
      if(this->bounceClock > 500){
        if(directionInput){
          this->display->setCurrentDisplay(DisplayMode::Direction);
         this->bounceClock = 0;
        }
        else if(scaleInput){
          this->display->setCurrentDisplay(DisplayMode::Scale);
          this->bounceClock = 0;
        }
        else if(accentInput){
          this->display->setCurrentDisplay(DisplayMode::Accent);
          this->bounceClock = 0;
        }
      }

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
          if(!this->play && note != this->notes[i] && !this->activeNotes[i]){
            // Play the note
            this->sendNoteOn(note);
            this->activeNotes[i] = true;
          }
          this->notes[i] = note;
        }
      }
      
      break;
    }
    case DisplayMode::Direction:
    {
      this->display->setCursorIndex(this->getDirectionIndex());
      
      bool directionInput = this->device->getInput(9);
      if(directionInput){
        if(this->bounceClock > 500){
          this->iterateDirection();
          this->display->keepCurrentDisplay();
          this->bounceClock = 0;
        }
      }
      break;
    }
    case DisplayMode::Scale:
    {
      this->display->setCursorIndex(this->scale);
      
      bool scaleInput = this->device->getInput(10);
      if(scaleInput){
        if(this->bounceClock > 500){
          this->scale = (this->scale + 1) % 8;
          this->display->keepCurrentDisplay();
          this->bounceClock = 0;
        }
      }
    }
    break;
    case DisplayMode::Accent:
//      this->display->setCurrentDisplay(DisplayMode::Accent);
      break;
  }

  // Tempo
  int tempoInput = device->getInput(8);
  if(tempoInput != 0){
    this->tempo += tempoInput;
    if(this->tempo > 0){
      this->setTempo(this->tempo);
    }else{
      this->setTempo(0);
    }
  }
  
//  bool playInput = this->device->getEncoderSwitch(8);
  if(this->device->getEncoderSwitch(8)){
    // Play / Stop
    if(this->bounceClock > 500){
      this->play = !this->play;
      this->currentStep = 0;
      this->bounceClock = 0;
      this->stepClock = 0;
      this->beatCount = 0;
      if(!this->play){
        this->sendStop();
      }
    }
  }

  if(this->tempo > 0 && this->stepClock >= this->timeBetweenSteps){
    this->stepClock = 0;

    this->beatCount++;
    this->beatCount = this->beatCount % 4;

    // Display the tempo
    if(this->beatCount == 0){
      this->device->setDisplay(8, 4);  
    }

    // Moving steps
    if (this->play) {
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

      if(this->notes[this->currentStep] > 0){
        this->sendNoteOn(this->notes[this->currentStep]);
        this->activeNotes[this->currentStep] = true;
      }else{
        this->sendNoteOff(this->notes[this->currentStep]);
        this->activeNotes[this->currentStep] = false;
      }
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
}

inline void Cycle::sendNoteOn(byte note){
  MIDI.sendNoteOn(note, 127, 1);
  usbMIDI.sendNoteOn(note, 127, 1);
}

inline void Cycle::sendNoteOff(byte note){
  MIDI.sendNoteOff(note, 127, 1);
  usbMIDI.sendNoteOff(note, 127, 1);
}

inline void Cycle::sendStop(){
//  MIDI.sendRealTime(MIDI_NAMESPACE::Stop);
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
#endif
