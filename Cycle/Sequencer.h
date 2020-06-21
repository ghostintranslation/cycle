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
    byte currentStep;
    unsigned int timeBetweenSteps;
    elapsedMillis beatClock;
    enum Direction { Forward, Backward, Pendulum, Transposer };
    Direction direction = Forward;
    enum Mode { Play, Direction, Scale, Accent };
    Mode mode = Play;
    byte tempo = 0;
    Display *display;
    Motherboard12 *device;
    elapsedMillis bounceClock;
    byte *notes;

  public:
    Sequencer(Motherboard12 *device);
    void update();
    byte getCurrentStep();
    void setTempo(byte tempo);
    void sendMidiNote(byte note);
};

/**
 * Constructor
 */
inline Sequencer::Sequencer(Motherboard12* device){
  this->device = device;
  this->display = new Display(this->device);
  this->setTempo(60);
}

inline void Sequencer::update(){
  
  switch(this->mode){
    case Play:
    { // This bracket is to prevent issues with variables declared inside the case

      // Reading buttons to switch mode
      bool directionInput = this->device->getInput(9);
      bool scaleInput = this->device->getInput(10);
      bool accentInput = this->device->getInput(11);
      if(directionInput){
        this->mode = Direction;
      }
      else if(scaleInput){
        this->mode = Scale;
      }
      else if(accentInput){
        this->mode = Accent;
      }

      // Reading the steps inputs
      for(byte i=0; i<8; i++){
        this->notes[i] = map(this->device->getInput(i), 0, 1023, 60, 84);
      }
      
      break;
    }
    case Direction:
    {
//      this->display->setCurrentDisplay(DisplayMode::Direction);
      break;
    }
    case Scale:
//      this->display->setCurrentDisplay(DisplayMode::Scale);
      break;
    case Accent:
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
//    Serial.println(playInput);
  if(this->device->getEncoderSwitch(8)){
    // Play / Stop
    if(this->bounceClock > 500){
      this->play = !this->play;
      this->currentStep = -1;
      this->bounceClock = 0;
    }
  }
  
  // Indicating the current step
  if(this->display->getCurrentDisplayMode() == Steps){
    this->display->displaySteps(this->currentStep);
  }
  
  
  // Moving steps
  if (this->play && this->tempo > 0 && this->beatClock >= this->timeBetweenSteps) {
    this->currentStep++;

    // Display the tempo
    if(this->currentStep%4 == 0){
      this->device->setDisplay(8, 4);
    }

    if(this->currentStep == 8){
      this->currentStep = 0;
    }
    
    if(this->notes[this->currentStep] > 0){
      this->sendMidiNote(this->notes[this->currentStep]);
    }

    this->beatClock = 0;
  }


  
  this->display->update();
}

inline byte Sequencer::getCurrentStep(){
  return this->currentStep;
}

inline void Sequencer::setTempo(byte tempo){
  this->tempo = tempo;
  this->timeBetweenSteps = (float)1/tempo*60*1000 /4;
}

inline void Sequencer::sendMidiNote(byte note){
  MIDI.sendNoteOn(note, 127, 1);
  usbMIDI.sendNoteOn(note, 127, 1);
}
#endif
