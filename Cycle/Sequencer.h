#ifndef Sequencer_h
#define Sequencer_h

#include "Display.h"
#include "Motherboard12.h"


/*
 * Sequencer
 */
class Sequencer{
  private:
    bool play = true;
    byte currentStep = 0;
    unsigned int timeBetweenSteps = 0;
    elapsedMillis stepClock;
    byte beatCount = 0;
    enum Direction { Forward, Backward, Pendulum, Transposer };
    Direction direction = Forward;
    bool pendulumState = true;
    void iterateDirection();
    byte getDirectionIndex();
//    enum Mode { Play, Direction, Scale, Accent };
//    Mode mode = Play;
    byte tempo = 0;
    Display *display;
    Motherboard12 *device;
    elapsedMillis bounceClock;
    byte notes[8];
    bool activeNotes[8];

  public:
    Sequencer(Motherboard12 *device);
    void update();
    byte getCurrentStep();
    void setTempo(byte tempo);
    void sendNoteOn(byte note);
    void sendNoteOff(byte note);
    void sendStop();
};

/**
 * Constructor
 */
inline Sequencer::Sequencer(Motherboard12* device){
  this->device = device;
  this->display = new Display(this->device);
  this->setTempo(60);

  for(byte i=0; i<8; i++){
    this->notes[i] = 0;
  }
  
  for(byte i=0; i<8; i++){
    this->activeNotes[i] = false;
  }
}

inline void Sequencer::update(){
  
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
//      this->display->setCurrentDisplay(DisplayMode::Direction);
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
//      this->display->setCurrentDisplay(DisplayMode::Scale);
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

inline byte Sequencer::getCurrentStep(){
  return this->currentStep;
}

inline void Sequencer::setTempo(byte tempo){
  this->tempo = tempo;
  this->timeBetweenSteps = (float)1/tempo*60*1000/4;
}

inline void Sequencer::sendNoteOn(byte note){
  MIDI.sendNoteOn(note, 127, 1);
  usbMIDI.sendNoteOn(note, 127, 1);
}

inline void Sequencer::sendNoteOff(byte note){
  MIDI.sendNoteOff(note, 127, 1);
  usbMIDI.sendNoteOff(note, 127, 1);
}

inline void Sequencer::sendStop(){
//  MIDI.sendRealTime(MIDI_NAMESPACE::Stop);
  for(byte i=0; i<8; i++){
    this->sendNoteOff(this->notes[i]);
    this->activeNotes[i] = false;
  }
}

inline void Sequencer::iterateDirection(){
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

inline byte Sequencer::getDirectionIndex(){
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
