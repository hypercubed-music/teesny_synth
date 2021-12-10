#include <Arduino.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>


 
// TeensyAudioDesign: begin automatically generated code
// the following JSON string contains the whole project, 
// it's included in all generated files.


class mod_matrix
{
public:
    AudioAmplifier                   amp;
    AudioAmplifier                   amp1;
    AudioAmplifier                   amp2;
    AudioAmplifier                   amp3;
    AudioMixer4                      mixer4_1;
    AudioMixer4                      mixer4_2;
    AudioMixer4                      mixer4_3;
    AudioMixer4                      mixer4_4;
    AudioMixer4                      mixer4_5;
    AudioConnection                  *patchCord[20]; // total patchCordCount:20 including array typed ones.

    mod_matrix() { // constructor (this is called when class-object is created)
        int pci = 0; // used only for adding new patchcords


        patchCord[pci++] = new AudioConnection(amp, 0, mixer4_1, 0);
        patchCord[pci++] = new AudioConnection(amp, 0, mixer4_2, 0);
        patchCord[pci++] = new AudioConnection(amp, 0, mixer4_3, 0);
        patchCord[pci++] = new AudioConnection(amp, 0, mixer4_4, 0);
        patchCord[pci++] = new AudioConnection(amp, 0, mixer4_5, 0);
        patchCord[pci++] = new AudioConnection(amp1, 0, mixer4_1, 1);
        patchCord[pci++] = new AudioConnection(amp1, 0, mixer4_2, 1);
        patchCord[pci++] = new AudioConnection(amp1, 0, mixer4_3, 1);
        patchCord[pci++] = new AudioConnection(amp1, 0, mixer4_4, 1);
        patchCord[pci++] = new AudioConnection(amp1, 0, mixer4_5, 1);
        patchCord[pci++] = new AudioConnection(amp2, 0, mixer4_1, 2);
        patchCord[pci++] = new AudioConnection(amp2, 0, mixer4_2, 2);
        patchCord[pci++] = new AudioConnection(amp2, 0, mixer4_3, 2);
        patchCord[pci++] = new AudioConnection(amp2, 0, mixer4_4, 2);
        patchCord[pci++] = new AudioConnection(amp2, 0, mixer4_5, 2);
        patchCord[pci++] = new AudioConnection(amp3, 0, mixer4_1, 3);
        patchCord[pci++] = new AudioConnection(amp3, 0, mixer4_2, 3);
        patchCord[pci++] = new AudioConnection(amp3, 0, mixer4_3, 3);
        patchCord[pci++] = new AudioConnection(amp3, 0, mixer4_4, 3);
        patchCord[pci++] = new AudioConnection(amp3, 0, mixer4_5, 3);
        amp.gain(1.0f);
        amp1.gain(1.0f);
        amp2.gain(1.0f);
        amp3.gain(1.0f);
        mixer4_1.gain(1, 0);
        mixer4_1.gain(2, 0);
        mixer4_1.gain(3, 0);
        mixer4_1.gain(4, 0);
        mixer4_2.gain(1, 0);
        mixer4_2.gain(2, 0);
        mixer4_2.gain(3, 0);
        mixer4_2.gain(4, 0);
        mixer4_3.gain(1, 0);
        mixer4_3.gain(2, 0);
        mixer4_3.gain(3, 0);
        mixer4_3.gain(4, 0);
        mixer4_4.gain(1, 0);
        mixer4_4.gain(2, 0);
        mixer4_4.gain(3, 0);
        mixer4_4.gain(4, 0);
        mixer4_5.gain(1, 0);
        mixer4_5.gain(2, 0);
        mixer4_5.gain(3, 0);
        mixer4_5.gain(4, 0);
        
    }

    void setValue(int inChannel, int outChannel, float value) {
        switch(outChannel) {
            case 0:
                mixer4_1.gain(inChannel, value);
                break;
            case 1:
                mixer4_2.gain(inChannel, value);
                break;
            case 2:
                mixer4_3.gain(inChannel, value);
                break;
            case 3:
                mixer4_4.gain(inChannel, value);
                break;
            case 4:
                mixer4_5.gain(inChannel, value);
                break;
        }
    }
};
// TeensyAudioDesign: end automatically generated code
