#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE(); // MIDI library init

#include "Motherboard12.h"
#include "Sequencer.h"

// 0 = empty, 1 = button, 2 = potentiometer, 3 = encoder
byte controls[12] = {2,2,2,2, 2,2,2,2, 3,1,1,1};
Motherboard12 device(controls);
Sequencer sequencer(&device);
    
void setup() {

  MIDI.begin(MIDI_CHANNEL_OMNI);
  
  Serial.begin(115200);
  
  while (!Serial && millis() < 2500); // wait for serial monitor

  // Starting sequence
  Serial.println("Ready!");
  
  device.init();
}

void loop() {
  device.update();
  sequencer.update();
}
