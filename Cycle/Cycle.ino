/*
 __    _  _____   ___      ___ _  _     __    _ ______ _    
/__|_|/ \(_  |     | |\|    | |_)|_||\|(_ |  |_| |  | / \|\|
\_|| |\_/__) |    _|_| |    | | \| || |__)|__| | | _|_\_/| |

If you enjoy my work and music please consider donating.

https://www.ghostintranslation.com/
https://ghostintranslation.bandcamp.com/
https://www.instagram.com/ghostintranslation/
https://www.youtube.com/channel/UCcyUTGTM-hGLIz4194Inxyw
https://github.com/ghostintranslation
*/

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
