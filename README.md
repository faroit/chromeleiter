# The Chromeleiter

![img_2401](https://cloud.githubusercontent.com/assets/72940/19220516/561b7c4e-8e2f-11e6-9c99-3cd7b599a5d6.JPG)

Electronic Musician playing live in a Jazz band:
> "Hey that sounds cool, what are you playing?"

The Guitar player answers
> "Thats A# Minor"

Electronic Musician
> ???


The Chromeleiter helps you to not embarrass yourself anymore. It shows you the pitches/notes of what the others are currently playing, and enables you to play along with them without knowing what the are actually playing. Essentially it is a realtime visualisation of harmonic pitch class profiles or __chroma__ vector mapped to the chromatic keyboard mode of a Novation Launchpad / Ableton Push device. It lights up the pads of the corresponding notes that are currently used by other players as well as an chord estimate. It is a provided as a C++ command line application that listens to the Audio input and connects to the Launchpad via MIDI.

## Usage

```./Chromeleiter 0```

runs the chromeleiter application and listens to audio on the first audio input
device.

## How does it work

 - Audio signal is captured from microphone
 - Chromagram is computed via method described in [1]
 - Resulting pitch class vector is scaled, thresholded and mapped to MIDI sysex output.
 - ChordDetection is used to detect the quality of the chords (Major and Minor for now) and change the colors of the pads accordingly.

## Used tools

* [Realtime Chromagram](https://github.com/adamstark/Chord-Detector-and-Chromagram)
* [Portaudio](http://www.portaudio.com)
* [RTMidi](https://www.music.mcgill.ca/~gary/rtmidi/)

## Compile on OS X

1. Install dependencies with homebrew
  - `portaudio`

2. Run ```make```

## References

[1] Real-Time Chord Recognition For Live Performance", A. M. Stark and M. D. Plumbley. In Proceedings of the 2009 International Computer Music Conference (ICMC 2009), Montreal, Canada, 16-21 August 2009

## License
GPL
