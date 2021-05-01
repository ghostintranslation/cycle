#ifndef Cycle_h
#define Cycle_h

#include "Display.h"
#include "Motherboard.h"


/*
 * Sequencer
 */
class Cycle{
  private:
    static Cycle *instance;
    Cycle();
    
    // Motherboard
    Motherboard *device;
    
    enum ClockMode { Leading = 0, Following = 1 };
    ClockMode clockMode = Leading;
    
    bool isPlaying = true;
    byte currentStep = 0;
    byte currentTransposerBar = 0;
    byte previousNotePlayed = 0;
    unsigned int timeBetweenSteps = 0;
    unsigned int timeBetweenTicks = 0;
    elapsedMillis stepClock;
    elapsedMillis ticksClock;
    byte ticksReadings;
    elapsedMillis lastTick;
    unsigned int *ticks;

    enum Direction { Forward, Backward, Pendulum, Sawtooth, Transposer, PendulumTransposer, AlternateTransposer, Random};
    Direction direction = Forward;
    bool pendulumState = true;
    byte scales[8][12] = {
      {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11}, // All notes
      {0, 0, 2, 2, 4, 4, 5, 5, 7, 9, 9,11}, // Major
      {0, 0, 2, 2, 4, 4, 7, 7, 7, 9, 9, 9}, // Major pentatonic
      {0, 0, 3, 3, 5, 5, 5, 7, 7, 7,10,10}, // Minor pentatonic
      {0, 2, 2, 3, 3, 5, 5, 7, 7, 8, 8,11}, // Harmonic minor
      {0, 2, 2, 3, 3, 5, 5, 7, 7, 9, 9,11}, // Melodic minor
      {0, 2, 2, 3, 3, 5, 5, 6, 6, 8, 9,11}, // Diminished
      {0, 0, 1, 1, 5, 5, 7, 7, 7,10,10,10}, // Insen
    };
    byte scale = 0;
    byte octave = 5;
    byte tempo = 0;
    byte division = 1;
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
    void updateAllNotes();
    void sendNoteOn(byte note);
    void sendNoteOff(byte note);
    void sendCC(byte value);
    void sendStart();
    void sendStop();
    void onStepIncrement();
    
    // Controls callbacks
    // Notes
    static void onNoteChange(byte inputIndex, float value, int diffToPrevious);
    // Clock
    static void onClockShortPress(byte inputIndex);
    static void onClockLongPress(byte inputIndex);
    static void onClockChange(byte inputIndex, bool value);
    // Direction
    static void onDirectionPress(byte inputIndex);
    // Scale
    static void onScalePress(byte inputIndex);
    // Octave
    static void onOctavePress(byte inputIndex);
    
    // Midi callbacks
    static void onMidiNoteChange(byte channel, byte control, byte value);
    static void onMidiClockChange(byte channel, byte control, byte value);
    static void onMidiDirectionChange(byte channel, byte control, byte value);
    static void onMidiScaleChange(byte channel, byte control, byte value);
    static void onMidiOctaveChange(byte channel, byte control, byte value);
    static void onMidiClock();
    static void onMidiSongPosition(unsigned beats);
    static void onMidiStart();
    static void onMidiStop();
};

// Singleton pre init
Cycle * Cycle::instance = nullptr;

/**
 * Constructor
 */
inline Cycle::Cycle(){
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
  // Motherboard init
  this->device = Motherboard::init(
    "CYCLE",
    {
      Potentiometer, Potentiometer, Potentiometer, Potentiometer,
      Potentiometer, Potentiometer, Potentiometer, Potentiometer,
      RotaryEncoder,        Button,        Button,        Button
    }
  );
  
  // Device callbacks
  this->device->setHandlePotentiometerChange(0, onNoteChange);
  this->device->setHandlePotentiometerChange(1, onNoteChange);
  this->device->setHandlePotentiometerChange(2, onNoteChange);
  this->device->setHandlePotentiometerChange(3, onNoteChange);
  this->device->setHandlePotentiometerChange(4, onNoteChange);
  this->device->setHandlePotentiometerChange(5, onNoteChange);
  this->device->setHandlePotentiometerChange(6, onNoteChange);
  this->device->setHandlePotentiometerChange(7, onNoteChange);
  this->device->setHandlePressUp(8, onClockShortPress);
  this->device->setHandleLongPressUp(8, onClockLongPress);
  this->device->setHandleRotaryChange(8, onClockChange);
  this->device->setHandlePressUp(9, onDirectionPress);
  this->device->setHandlePressUp(10, onScalePress);
  this->device->setHandlePressUp(11, onOctavePress);

  // MIDI callbacks
  device->setHandleMidiControlChange(0, 0, "Note 1", onMidiNoteChange);
  device->setHandleMidiControlChange(0, 1, "Note 2", onMidiNoteChange);
  device->setHandleMidiControlChange(0, 2, "Note 3", onMidiNoteChange);
  device->setHandleMidiControlChange(0, 3, "Note 4", onMidiNoteChange);
  device->setHandleMidiControlChange(0, 4, "Note 5", onMidiNoteChange);
  device->setHandleMidiControlChange(0, 5, "Note 6", onMidiNoteChange);
  device->setHandleMidiControlChange(0, 6, "Note 7", onMidiNoteChange);
  device->setHandleMidiControlChange(0, 7, "Note 8", onMidiNoteChange);
  device->setHandleMidiControlChange(0, 8, "Clock",  onMidiClockChange);
  device->setHandleMidiControlChange(0, 9, "Direction", onMidiDirectionChange);
  device->setHandleMidiControlChange(0, 10, "Scale", onMidiScaleChange);
  device->setHandleMidiControlChange(0, 11, "Octave", onMidiOctaveChange);
}

inline void Cycle::update(){
  this->device->update();
  
  switch(this->display->getCurrentDisplayMode()){
    case DisplayMode::Sequencer:
    {
      switch(this->direction){
        case Transposer:
        case PendulumTransposer:
        {
          // Displaying the current step and the current bar
          byte ledsOn = 0;
          ledsOn |= 1 << this->currentStep;
          ledsOn |= 1 << (4 + this->currentTransposerBar);
          byte data[3] = {ledsOn, 0, 0};
          this->display->setData(data);
        }
        break;
        case AlternateTransposer:
        { 
          // Displaying the current step and the current bar
          byte ledsCurrentStep = 0;
          ledsCurrentStep |= 1 << this->currentStep;
          byte ledsCurrentTransposerBar = 0;
          ledsCurrentTransposerBar |= 1 << (this->currentTransposerBar);
          byte data[3] = {ledsCurrentStep, ledsCurrentTransposerBar, 0};
          this->display->setData(data);
        }
        break;

        default:
        {
          // Displaying the current step
          byte ledsOn = 0;
          ledsOn |= 1 << this->currentStep;
          byte data[3] = {ledsOn, 0, 0};
          this->display->setData(data);
        }
        break;
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
        MIDI.sendClock();
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
        
        case Transposer:
          this->currentStep++;
          
          if(this->currentStep >= 4){
            this->currentStep = 0;
            this->currentTransposerBar = (this->currentTransposerBar + 1) % 4;
          }
        break;
        
        case PendulumTransposer:
        
          if(this->pendulumState){
            this->currentStep++;
            
            if(this->currentStep >= 4){
              this->currentStep = 0;
              this->pendulumState = !this->pendulumState;
            }
          }else{
            this->currentStep--;
            if(this->currentStep == 0){
              this->pendulumState = !this->pendulumState;
              this->currentTransposerBar = (this->currentTransposerBar + 1) % 4;
            }
          }
          this->currentStep = constrain(this->currentStep, 0, 3);
        break;
        
        case AlternateTransposer:
        
          if(this->currentTransposerBar == 0 && this->currentStep == 0){
            this->currentStep = 4;
          }else{
            
            if(this->currentStep == 4){
              if(this->currentTransposerBar == 7){
                this->currentTransposerBar = 0;
              }else{
                this->currentTransposerBar -= 3;
              }
            }
          }
          
          this->currentStep++;

          if(this->currentStep == 8){
            this->currentTransposerBar += 4;
            this->currentStep = 0;
          }
          
        break;
        
        case Sawtooth:
          if(this->currentStep == 7){
            this->currentStep = 0;
          }else{
            if(this->currentStep < 4){
              this->currentStep += 4;
            }else{
              this->currentStep -=3;
            }
          }
        break;
        
        case Random:
          this->currentStep = rand() % 8;
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
  this->timeBetweenSteps = (float)1/tempo*60*1000/(4/(float)this->division);
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

inline void Cycle::sendCC(byte value){
  MIDI.sendControlChange(0, value, this->device->getMidiChannel()); // TODO be able to configure the control number
  usbMIDI.sendControlChange(0, value, this->device->getMidiChannel());
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
      this->direction = Sawtooth;
    break;
    case Sawtooth:
      this->direction = Transposer;
    break;
    case Transposer:
      this->direction = PendulumTransposer;
    break;
    case PendulumTransposer:
      this->direction = AlternateTransposer;
    break;
    case AlternateTransposer:
      this->direction = Random;
    break;
    case Random:
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
    case Sawtooth:
      return 3;
    break;
    case Transposer:
      return 4;
    break;
    case PendulumTransposer:
      return 5;
    break;
    case AlternateTransposer:
      return 6;
    break;
    case Random:
      return 7;
    break;
    
  }

  return 0;
}


/**
 * Midi clock callback
 */
inline void Cycle::onMidiClock(){
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
inline void Cycle::onMidiSongPosition(unsigned songPosition){
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
inline void Cycle::onMidiStart(){
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
inline void Cycle::onMidiStop(){
  switch(getInstance()->clockMode){
    case ClockMode::Following:
      Serial.println("onStop");
    break;
    
    default:
    break;
  }
}


/**
 * On note change 
 */
inline void Cycle::onNoteChange(byte inputIndex, float value, int diffToPrevious){
  if(value == 0){
    getInstance()->notes[inputIndex] = 0;
  }else{
    switch(getInstance()->scale){
      // MIDI CC mode
      case 7:
      {
        byte midiCC = map(value, getInstance()->device->getAnalogMinValue(), getInstance()->device->getAnalogMaxValue(), 0, 127);
        getInstance()->notes[inputIndex] = midiCC;
        
        // If the sequencer is not playing and a step has changed
        if(!getInstance()->isPlaying && midiCC != getInstance()->notes[inputIndex] && !getInstance()->activeNotes[inputIndex]){
          // Send the MIDI CC message
          getInstance()->sendCC(midiCC);
          getInstance()->activeNotes[inputIndex] = true;
        }
      }
      break;

      // Default mode
      default:
      {
        byte mapNote = map(value, getInstance()->device->getAnalogMinValue(), getInstance()->device->getAnalogMaxValue(), 0, 24);
        byte noteOctave = mapNote / 12;
        byte note = 12 * getInstance()->octave + 12 * noteOctave + getInstance()->scales[getInstance()->scale][mapNote % 12];
        
        // If the sequencer is not playing and a step has changed
        if(!getInstance()->isPlaying && note != getInstance()->notes[inputIndex] && !getInstance()->activeNotes[inputIndex]){
          // Play the note
          getInstance()->sendNoteOn(note);
          getInstance()->activeNotes[inputIndex] = true;
          getInstance()->display->setCursor(mapNote%12);
          getInstance()->display->setCurrentDisplay(DisplayMode::NoteChange);
        }
        getInstance()->notes[inputIndex] = note;
      }
      break;
    }
  }
}

/**
 * On MIDI Note Change
 */
void Cycle::onMidiNoteChange(byte channel, byte control, byte value){
  byte noteIndex = constrain(control, 0, 7);
  
  unsigned int mapValue = map(
    value, 
    0,
    127,
    getInstance()->device->getAnalogMinValue(), 
    getInstance()->device->getAnalogMaxValue()
  );
  
  getInstance()->onNoteChange(noteIndex, mapValue, 255);
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
inline void Cycle::onClockChange(byte inputIndex, bool value){
  int inValue = 1;
  if(!value){
    inValue = -1;
  }
  
  switch(getInstance()->display->getCurrentDisplayMode()){
    case DisplayMode::Sequencer:
    case DisplayMode::ClockDivision:
    {
      switch(getInstance()->clockMode){
        case ClockMode::Leading:
          getInstance()->tempo += inValue;
          if(getInstance()->tempo > 0){
            getInstance()->setTempo(getInstance()->tempo);
          }else{
            getInstance()->setTempo(0);
          }
        break;

        case ClockMode::Following:
          getInstance()->division += inValue;
          if(getInstance()->division < 1){
            getInstance()->division = 1;
          }
          if(getInstance()->division > 8){
            getInstance()->division = 8;
          }
          getInstance()->setTempo(getInstance()->tempo);
          getInstance()->display->setCursor(getInstance()->division - 1);
          getInstance()->display->setCurrentDisplay(DisplayMode::ClockDivision);
        break;
      }
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
 * On MIDI Clock Change
 */
void Cycle::onMidiClockChange(byte channel, byte control, byte value){
  byte mapValue = map(value, 0, 127, 0, 255);
  getInstance()->setTempo(mapValue);
}


/**
 * On Direction press 
 */
inline void Cycle::onDirectionPress(byte inputIndex){
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
 * On MIDI Direction Change
 */
void Cycle::onMidiDirectionChange(byte channel, byte control, byte value){
  byte mapValue = map(value, 0, 127, 0, 7);
  getInstance()->display->setCurrentDisplay(DisplayMode::Direction);
  getInstance()->direction = Direction(mapValue);
  getInstance()->display->setCursor(getInstance()->getDirectionIndex());
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
      getInstance()->updateAllNotes();
      getInstance()->display->setCursor(getInstance()->scale);
      getInstance()->display->keepCurrentDisplay();
    }
    break;  
    default:
    break;
  }
}

/**
 * On MIDI Scale Change
 */
void Cycle::onMidiScaleChange(byte channel, byte control, byte value){
  byte mapValue = map(value, 0, 127, 0, 7);
  getInstance()->display->setCurrentDisplay(DisplayMode::Scale);
  getInstance()->scale = mapValue;
  getInstance()->updateAllNotes();
  getInstance()->display->setCursor(getInstance()->scale);
}

/**
 * On Octave press 
 */
inline void Cycle::onOctavePress(byte inputIndex){
  switch(getInstance()->display->getCurrentDisplayMode()){
    case DisplayMode::Sequencer:
      getInstance()->display->setCursor(getInstance()->octave);
      getInstance()->display->setCurrentDisplay(DisplayMode::Octave);
    break; 
    
    case DisplayMode::Octave:
      switch(getInstance()->scale){
          // MIDI CC mode
        case 7:
          // Octave not doing anything in this case
        break;
        
        // Default mode
        default:
          getInstance()->octave = (getInstance()->octave + 1) % 8;
          getInstance()->updateAllNotes();
          getInstance()->display->setCursor(getInstance()->octave);
          getInstance()->display->keepCurrentDisplay();
        break;
      }
    break;
       
    default:
    break;
  }
}

/**
 * On MIDI Octave Change
 */
void Cycle::onMidiOctaveChange(byte channel, byte control, byte value){
  byte mapValue = map(value, 0, 127, 0, 7);
  getInstance()->display->setCurrentDisplay(DisplayMode::Octave);
  getInstance()->octave = mapValue;
  getInstance()->updateAllNotes();
  getInstance()->display->setCursor(getInstance()->octave);
}

inline void Cycle::onStepIncrement(){
  switch(this->display->getCurrentDisplayMode()){
    case DisplayMode::Sequencer:
    case DisplayMode::Direction:
    case DisplayMode::Scale:
    case DisplayMode::Octave:
    {
      byte noteToPlay = this->notes[this->currentStep];
      bool temporaryMute = false;
      switch(this->direction){
        case Transposer:
        case PendulumTransposer:
        {
          noteToPlay = noteToPlay + this->notes[4 + this->currentTransposerBar] - 12*this->octave;
        }
        break;
        
        case AlternateTransposer:
        {
          noteToPlay = noteToPlay + this->notes[this->currentTransposerBar] - 12*this->octave;
        }
        break;

        case Random:
          if(noteToPlay == this->previousNotePlayed){
            temporaryMute = true;
          }
        break;
        
        default:
        break;
      }

      if(!temporaryMute){
        switch(getInstance()->scale){
          // MIDI CC mode
          case 7:
            if(noteToPlay > 0){
              this->sendCC(noteToPlay);
              this->activeNotes[this->currentStep] = true;
            }
          break;
  
          // Default mode
          default:
            if(noteToPlay > 0){
              this->sendNoteOff(this->previousNotePlayed);
              this->sendNoteOn(noteToPlay);
              this->previousNotePlayed = noteToPlay;
              this->activeNotes[this->currentStep] = true;
            }else{
              this->sendNoteOff(noteToPlay);
              this->activeNotes[this->currentStep] = false;
            }
          break;
        }
      }
    }
    break;
    
    default:
    break;
  }

  // Song position
  switch(this->clockMode){
    case ClockMode::Leading:
      MIDI.sendSongPosition(this->currentStep);
    break;
    
    default:
    break;
  }
}

/**
 * Set all notes according to inputs and octave
 */
inline void Cycle::updateAllNotes(){
  for(byte i=0; i<8; i++){
    unsigned int noteInput = this->device->getInput(i);
    if(noteInput == 0){
      this->notes[i] = 0;
    }else{
      byte note = map(this->device->getInput(i), this->device->getAnalogMinValue(), this->device->getAnalogMaxValue(), 0, 24);
      byte noteOctave = note / 12;
      note = 12 * this->octave + 12 * noteOctave + this->scales[this->scale][note % 12];
      
      // If the sequencer is not playing and a step has changed
//      if(!this->isPlaying && note != this->notes[i] && !this->activeNotes[i]){
//        // Play the note
//        this->sendNoteOn(note);
//        this->activeNotes[i] = true;
//      }
      this->notes[i] = note;
    }
  }
}
#endif
