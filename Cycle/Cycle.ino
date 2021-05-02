/*
 __    _  _____   ___      ___ _  _     __    _ ______ _    
/__|_|/ \(_  |     | |\|    | |_)|_||\|(_ |  |_| |  | / \|\|
\_|| |\_/__) |    _|_| |    | | \| || |__)|__| | | _|_\_/| |

CYCLE
v1.2.0

Support my work:
https://www.paypal.com/paypalme/ghostintranslation

https://www.ghostintranslation.com/
https://ghostintranslation.bandcamp.com/
https://www.instagram.com/ghostintranslation/
https://www.youtube.com/channel/UCcyUTGTM-hGLIz4194Inxyw
https://github.com/ghostintranslation
*/

#include "Cycle.h"

// Instanciation of Cycle
Cycle * cycle = Cycle::getInstance();
    
void setup() {
  Serial.begin(115200);
  
  while (!Serial && millis() < 2500); // wait for serial monitor
  
  cycle->init();
  
  // Starting sequence
  Serial.println("Ready!");
}

void loop() {
  cycle->update();
}
