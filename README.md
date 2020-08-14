# CYCLE

![GitHub version](https://img.shields.io/github/v/release/ghostintranslation/cycle.svg?include_prereleases)

CYCLE is a modular monophonic sequencer, based on eurorack physical format it has however no patch cables in the front but has rather midi jacks in the back.

It is based on [Motherboard12](https://github.com/ghostintranslation/motherboard12).

NOTE: This is a work in progress.

<img src="cycle.jpg" width="200px"/> <img src="cycle-side.jpg" width="200px"/>

## Features

* 8 steps
* 8 modes
* 8 scales
* 8 octaves
* MIDI over USB
* MIDI over mini jack

## Getting Started

### Prerequisites

What things you need to make it work:

```
1 Teensy 4.0
1 MOTHERBOARD12A pcb
1 MOTHERBOARD9B pcb
1 CYCLE front panel pcb
8 vertical potentiometers 10K
1 vertical rotary encoder
9 LED
4 14 pins male header
6 14 pins female header
8 CD4051 multiplexers
8 DIP16 IC sockets (optionak)
2 3.5mm jack connectors
1 resistor ~ 80ohm
1 4 positions dipswitch (optional)
```

In order to run any sketch on the Teensy you have to install the Teensyduino add-on to Arduino.
Follow the instructions from the official page:
https://www.pjrc.com/teensy/teensyduino.html

Then open the sketch located in the Cycle folder of this repo.

In the Tools -> USB Type menu, choose "Serial + midi".

Then you are ready to compile and upload the sketch.

# How to use

Here is a description of the 6 inputs and what they do:

```
1. Default screen
    - Each pair of LED-Potentiometer represent a step.
    - The LED indicates when the step is active.
    - The potentiometer sets the note of the step within 2 octaves.
        - At 0 the note is muted
2. Clock
    1. Usage
        - A long press on the Clock rotary will enter the Clock mode screen.
        - Turn the rotary while in this mode to select between Follower and Leader modes.
        - Press to validate and exit
        1. Leader
            - Sends MIDI clock and song position messages
            - Turn the rotary to increments or decrements the tempo
            - Press to toggle Play/Stop
        2. Follower
            - Synchronize its steps to the received clocks and song position messages
            - Turn the rotary to choose between 8 values to divide the received clock   (TODO)
            - Press to toggle Play/Stop
    2. Display
        - Leader is a slow blink, Follower is a fast blink
3. Direction
    1. Usage
        - Press once the Direction button to enter the Direction screen.
        - Press again the button while in this screen to change the Direction.
    2. Modes
        1. Forward
        2. Backward
        3. Pendulum
            - Going forward and backward alternatively
        4. Sawtooth
            - 1,5,2,6,3,7,4,8
        5. Transposer
            - The bar becomes 4 steps. The 4 steps at the top are being transposed by the 4 steps at the bottom. 
        6. Pendulum Transposer
        7. Alternate Transposer
            - Alternates between 4 top steps and 4 bottom steps being transposed by 1 alternating step on the oposite side.
        8. Random
            - Random steps, avoid repeating notes by leaving blanks.
    3. Display
        - The selected direction is indicated by a blinking LED.
        - Screen stays 1s after last press.
4. Scale
    1. Usage
        - Press once the Scale button to enter the Scale screen.
        - Press again the button while in this screen to change the Scale.
        - Choose between no scale (first choice) and 7 scales.
    2. Display
        - The selected scale is indicated by a blinking LED.
        - Screen stays 1s after last press.
5. Octave
    1. Usage
        - Press once the Octave button to enter the Octave screen.
        - Press again the button while in this screen to change the Octave.
    2. Display
        - The selected octave is indicated by a blinking LED.
        - Screen stays 1s after last press.
```

## About me
You can find me on Bandcamp, Instagram, Youtube and mu own website:

https://ghostintranslation.bandcamp.com/

https://www.instagram.com/ghostintranslation/

https://www.youtube.com/channel/UCcyUTGTM-hGLIz4194Inxyw

https://www.ghostintranslation.com/


## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details