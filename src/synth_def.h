#include <Audio.h>
#include <noteToFreq.h>

// TeensyAudioDesign: begin automatically generated code
// the following JSON string contains the whole project, 
// it's included in all generated files.

//Setting this number will automatically create the
//amunt of voices needed
#define NUM_VOICES 8

// Oscillator options
int wave1Type = WAVEFORM_SINE;
int wave2Type = WAVEFORM_SINE;
int wave3Type = WAVEFORM_SINE;
float wave1Vol = 1.0f;
float wave2Vol = 0.0f;
float wave3Vol = 0.0f;
int wave1Oct = 0;
int wave2Oct = 0;
int wave3Oct = 0;
int wave1Coarse = 0;
int wave2Coarse = 0;
int wave3Coarse = 0;
float wave1Fine = 0.0f;
float wave2Fine = 0.0f;
float wave3Fine = 0.0f;
float wave1pwm = 0.5f;
float wave2pwm = 0.5f;
float wave3pwm = 0.5f;

// FM and phase mod from oscs
float fm1to2 = 0.0f;
float shape1to2 = 0.0f;
float fm2to3 = 0.0f;
float shape2to3 = 0.0f;

int lfoWave = WAVEFORM_SINE;
float lfoFreq = 0.0f;
float lfoAAmt = 0.0f;
float lfoFAmt = 0.0f;

float cutoff = 15000.f;
float resonance = 1.f;
float attackF = 0;
float decayF = 200;
float sustainF = 0.9f;
float releaseF = 300;
float envFAmount = 0.f;
int filterType = 0;

float attackA = 0;
float decayA = 200;
float sustainA = 0.9f;
float releaseA = 300;

float detune = 0.0f;
float phaseOffset = 0.0f;

int modEffect = 0;
float modEffectParam1 = 0.0f;
float modEffectParam2 = 0.0f;
float delayMix = 0.0f;
float delayTime = 0.1f;
float delayFeedback = 0.5f;
float reverbSize = 0.0f;
float reverbMix = 0.0f;
char modEffectName[7] = "None";

char filterTypeName[3] = "NF";

class voice
{
public:
    int osc1Coarse = 0;
    int osc2Coarse = 0;
    int osc3Coarse = 0;
    float osc1Fine = 0;
    float osc2Fine = 0;
    float osc3Fine = 0;
    int osc1Oct = 0;
    int osc2Oct = 0;
    int osc3Oct = 0;
    byte filterType = 0; //0 is off, 1/2/3 is LP/BP/HP
    int osc1FreqMod = 0;
    int osc2FreqMod = 0;
    int osc3FreqMod = 0;
    
    AudioSynthWaveform               wave1;
    AudioSynthWaveformDc             wave2Shape;
    AudioSynthWaveformDc             wave3Shape;
    AudioMixer4                      wave2FreqMix;
    AudioMixer4                      wave2PhaseMix;
    AudioSynthWaveformModulated      wave2;
    AudioSynthWaveformDc             dc;
    AudioMixer4                      wave3FreqMix;
    AudioMixer4                      wave3PhaseMix;
    AudioEffectEnvelope              envF;
    AudioSynthWaveformModulated      wave3;
    AudioMixer4                      mixer4_11;
    AudioMixer4                      waveMix;
    AudioEffectEnvelope              envA;
    AudioFilterStateVariable         filter1;
    AudioFilterLadder                filter2;
    AudioMixer4                      filterSelect;
    AudioConnection                  *patchCord[24]; // total patchCordCount:22 including array typed ones.

    voice() { // constructor (this is called when class-object is created)
        int pci = 0; // used only for adding new patchcords


        patchCord[pci++] = new AudioConnection(wave1, 0, wave2FreqMix, 0);
        patchCord[pci++] = new AudioConnection(wave1, 0, wave2PhaseMix, 0);
        patchCord[pci++] = new AudioConnection(wave1, 0, waveMix, 0);
        patchCord[pci++] = new AudioConnection(wave2Shape, 0, wave2PhaseMix, 2);
        patchCord[pci++] = new AudioConnection(wave3Shape, 0, wave3PhaseMix, 2);
        patchCord[pci++] = new AudioConnection(wave2FreqMix, 0, wave2, 0);
        patchCord[pci++] = new AudioConnection(wave2PhaseMix, 0, wave2, 1);
        patchCord[pci++] = new AudioConnection(wave2, 0, wave3FreqMix, 1);
        patchCord[pci++] = new AudioConnection(wave2, 0, wave3PhaseMix, 1);
        patchCord[pci++] = new AudioConnection(wave2, 0, waveMix, 1);
        patchCord[pci++] = new AudioConnection(dc, 0, envF, 0);
        patchCord[pci++] = new AudioConnection(wave3FreqMix, 0, wave3, 0);
        patchCord[pci++] = new AudioConnection(wave3PhaseMix, 0, wave3, 1);
        patchCord[pci++] = new AudioConnection(envF, 0, mixer4_11, 1);
        patchCord[pci++] = new AudioConnection(wave3, 0, waveMix, 2);
        patchCord[pci++] = new AudioConnection(mixer4_11, 0, filter1, 1);
        patchCord[pci++] = new AudioConnection(mixer4_11, 0, filter2, 1);
        patchCord[pci++] = new AudioConnection(waveMix, 0, envA, 0);
        patchCord[pci++] = new AudioConnection(envA, 0, filter1, 0);
        patchCord[pci++] = new AudioConnection(envA, 0, filter2, 0);
        patchCord[pci++] = new AudioConnection(envA, 0, filterSelect, 0);
        patchCord[pci++] = new AudioConnection(filter2, 0, filterSelect, 1);
        //patchCord[pci++] = new AudioConnection(filter1, 0, filterSelect, 1);
        patchCord[pci++] = new AudioConnection(filter1, 1, filterSelect, 2);
        patchCord[pci++] = new AudioConnection(filter1, 2, filterSelect, 3);
        
        wave1.begin(1.0f, 100.0f, WAVEFORM_SINE);
        wave2.begin(0.0f, 100.0f, WAVEFORM_SINE);
        wave3.begin(0.0f, 100.0f, WAVEFORM_SINE);
        wave2.frequencyModulation(1.0f);
        wave3.frequencyModulation(1.0f);
        wave2.phaseModulation(1.0f);
        wave3.phaseModulation(1.0f);
        filter1.octaveControl(7.0f);
        filter2.octaveControl(7.0f);
        mixer4_11.gain(1, 0.0f);
        dc.amplitude(1.0f);
        for (byte j = 0; j < 4; j++) {
          filterSelect.gain(j, j==0 ? 1.0f : 0.f);
        }
        filter1.frequency(15000.0f);
        filter2.frequency(15000.0f);
        filter1.resonance(1.0f);
        filter2.resonance(0.0f);
        for (int j = 0; j < 4; j++) {
          wave2FreqMix.gain(j, 0.0f);
        }
        for (int j = 0; j < 4; j++) {
          wave2PhaseMix.gain(j, 0.0f);
        }
        for (int j = 0; j < 4; j++) {
          wave3FreqMix.gain(j, 0.0f);
        }
        for (int j = 0; j < 4; j++) {
          wave3PhaseMix.gain(j, 0.0f);
        }
        wave2Shape.amplitude(1.0);
        wave3Shape.amplitude(1.0);
    }

    float lerp(float v1, float v2, float amt) {
      return v1 + ((v2 - v1) * amt);
    }

    void noteOn(byte pitch, byte velocity) {
      float amp = 0.25f * (velocity / 128.f);
      int note_idx = pitch + (12*osc1Oct) + osc1Coarse;
      if (osc1Fine > 0.0) {
        wave1.frequency(lerp(noteToFreq[note_idx], noteToFreq[note_idx+1], osc1Fine));
      } else if (osc1Fine < 0.0) {
        wave1.frequency(lerp(noteToFreq[note_idx], noteToFreq[note_idx-1], -osc1Fine));
      } else {
        wave1.frequency(noteToFreq[note_idx]);
      }
      note_idx = pitch + (12*osc2Oct) + osc2Coarse;
      if (osc2Fine > 0.0) {
        wave2.frequency(lerp(noteToFreq[note_idx], noteToFreq[note_idx+1], osc2Fine));
      } else if (osc2Fine < 0.0) {
        wave2.frequency(lerp(noteToFreq[note_idx], noteToFreq[note_idx-1], -osc2Fine));
      } else {
        wave2.frequency(noteToFreq[note_idx]);
      }
      note_idx = pitch + (12*osc3Oct) + osc3Coarse;
      if (osc3Fine > 0.0) {
        wave3.frequency(lerp(noteToFreq[note_idx], noteToFreq[note_idx+1], osc3Fine));
      } else if (osc3Fine < 0.0) {
        wave3.frequency(lerp(noteToFreq[note_idx], noteToFreq[note_idx-1], -osc3Fine));
      } else {
        wave3.frequency(noteToFreq[note_idx]);
      }
      wave1.amplitude(amp);
      wave2.amplitude(amp);
      wave3.amplitude(amp);
      envA.noteOn();
      envF.noteOn();
    }

    void noteOff() {
      envA.noteOff();
      envF.noteOff();
    }
};
// TeensyAudioDesign: end automatically generated code

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

// Audio Processing Nodes
AudioSynthWaveform               lfo1; //xy=130,475
AudioSynthWaveform               lfo2; //xy=151,534
AudioSynthWaveform               lfo3; //xy=173,601
AudioSynthWaveform               lfo4; //xy=154,653
mod_matrix                       mod_matrix1; //xy=355,515
voice *                          voices = new voice[NUM_VOICES];
AudioMixer4                      voicemix1; //xy=930,475
AudioMixer4                      voicemix2; //xy=935,655
AudioMixer4                      voicemix3; //xy=1160,500
AudioEffectChorus                chorus; //xy=1495,315
AudioEffectFlange                flange; //xy=1495,380
AudioEffectBitcrusher            bitcrusher; //xy=1485,440
AudioMixer4                      effectmix1; //xy=1758,471
AudioEffectDelay                 delay1; //xy=1900,355
AudioMixer4                      delaymix1; //xy=2045,330
AudioMixer4                      delaymix2; //xy=2040,400
AudioMixer4                      delaymix3; //xy=2200,435
AudioEffectFreeverb              freeverb; //xy=2350,400
AudioMixer4                      reverbmix; //xy=2505,420
AudioOutputI2S                   i2s;  //xy=1290,495

// Audio Connections (all connections (aka wires or links))

/*
 *  
 * MOD MATRIX
 * 
 * Four LFOs input
 * Output 1: Wave 2 frequency mod
 * Output 2: Wave 2 phase mod
 * Output 3: Wave 3 frequency mod
 * Output 4: Wave 3 phase mod
 * Output 5: Filter Cutoff Frequency
 * 
 */

AudioConnection          patchCord1(lfo1, 0, mod_matrix1.amp, 0);
AudioConnection          patchCord2(lfo2, 0, mod_matrix1.amp1, 1);
AudioConnection          patchCord3(lfo3, 0, mod_matrix1.amp2, 2);
AudioConnection          patchCord4(lfo4, 0, mod_matrix1.amp3, 3);

AudioConnection          patchCord5(mod_matrix1.mixer4_1, 0, voices[0].wave2FreqMix, 1);
AudioConnection          patchCord6(mod_matrix1.mixer4_1, 0, voices[1].wave2FreqMix, 1);
AudioConnection          patchCord7(mod_matrix1.mixer4_1, 0, voices[2].wave2FreqMix, 1);
AudioConnection          patchCord8(mod_matrix1.mixer4_1, 0, voices[3].wave2FreqMix, 1);
AudioConnection          patchCord9(mod_matrix1.mixer4_1, 0, voices[4].wave2FreqMix, 1);
AudioConnection          patchCord10(mod_matrix1.mixer4_1, 0, voices[5].wave2FreqMix, 1);
AudioConnection          patchCord11(mod_matrix1.mixer4_1, 0, voices[6].wave2FreqMix, 1);
AudioConnection          patchCord12(mod_matrix1.mixer4_1, 0, voices[7].wave2FreqMix, 1);
AudioConnection          patchCord13(mod_matrix1.mixer4_2, 0, voices[0].wave2PhaseMix, 1);
AudioConnection          patchCord14(mod_matrix1.mixer4_2, 0, voices[1].wave2PhaseMix, 1);
AudioConnection          patchCord15(mod_matrix1.mixer4_2, 0, voices[2].wave2PhaseMix, 1);
AudioConnection          patchCord16(mod_matrix1.mixer4_2, 0, voices[3].wave2PhaseMix, 1);
AudioConnection          patchCord17(mod_matrix1.mixer4_2, 0, voices[4].wave2PhaseMix, 1);
AudioConnection          patchCord18(mod_matrix1.mixer4_2, 0, voices[5].wave2PhaseMix, 1);
AudioConnection          patchCord19(mod_matrix1.mixer4_2, 0, voices[6].wave2PhaseMix, 1);
AudioConnection          patchCord20(mod_matrix1.mixer4_2, 0, voices[7].wave2PhaseMix, 1);
AudioConnection          patchCord21(mod_matrix1.mixer4_3, 0, voices[0].wave3FreqMix, 1);
AudioConnection          patchCord22(mod_matrix1.mixer4_3, 0, voices[1].wave3FreqMix, 1);
AudioConnection          patchCord23(mod_matrix1.mixer4_3, 0, voices[2].wave3FreqMix, 1);
AudioConnection          patchCord24(mod_matrix1.mixer4_3, 0, voices[3].wave3FreqMix, 1);
AudioConnection          patchCord25(mod_matrix1.mixer4_3, 0, voices[4].wave3FreqMix, 1);
AudioConnection          patchCord26(mod_matrix1.mixer4_3, 0, voices[5].wave3FreqMix, 1);
AudioConnection          patchCord27(mod_matrix1.mixer4_3, 0, voices[6].wave3FreqMix, 1);
AudioConnection          patchCord28(mod_matrix1.mixer4_3, 0, voices[7].wave3FreqMix, 1);
AudioConnection          patchCord29(mod_matrix1.mixer4_4, 0, voices[0].wave3PhaseMix, 1);
AudioConnection          patchCord30(mod_matrix1.mixer4_4, 0, voices[1].wave3PhaseMix, 1);
AudioConnection          patchCord31(mod_matrix1.mixer4_4, 0, voices[2].wave3PhaseMix, 1);
AudioConnection          patchCord32(mod_matrix1.mixer4_4, 0, voices[3].wave3PhaseMix, 1);
AudioConnection          patchCord33(mod_matrix1.mixer4_4, 0, voices[4].wave3PhaseMix, 1);
AudioConnection          patchCord34(mod_matrix1.mixer4_4, 0, voices[5].wave3PhaseMix, 1);
AudioConnection          patchCord35(mod_matrix1.mixer4_4, 0, voices[6].wave3PhaseMix, 1);
AudioConnection          patchCord36(mod_matrix1.mixer4_4, 0, voices[7].wave3PhaseMix, 1);
AudioConnection          patchCord37(mod_matrix1.mixer4_5, 0, voices[0].mixer4_11, 0);
AudioConnection          patchCord38(mod_matrix1.mixer4_5, 0, voices[1].mixer4_11, 0);
AudioConnection          patchCord39(mod_matrix1.mixer4_5, 0, voices[2].mixer4_11, 0);
AudioConnection          patchCord40(mod_matrix1.mixer4_5, 0, voices[3].mixer4_11, 0);
AudioConnection          patchCord41(mod_matrix1.mixer4_5, 0, voices[4].mixer4_11, 0);
AudioConnection          patchCord42(mod_matrix1.mixer4_5, 0, voices[5].mixer4_11, 0);
AudioConnection          patchCord43(mod_matrix1.mixer4_5, 0, voices[6].mixer4_11, 0);
AudioConnection          patchCord44(mod_matrix1.mixer4_5, 0, voices[7].mixer4_11, 0);
AudioConnection          patchCord45(voices[0].filterSelect, 0, voicemix1, 0);
AudioConnection          patchCord46(voices[1].filterSelect, 0, voicemix1, 1);
AudioConnection          patchCord47(voices[2].filterSelect, 0, voicemix1, 2);
AudioConnection          patchCord48(voices[3].filterSelect, 0, voicemix1, 3);
AudioConnection          patchCord49(voices[4].filterSelect, 0, voicemix2, 0);
AudioConnection          patchCord50(voices[5].filterSelect, 0, voicemix2, 1);
AudioConnection          patchCord51(voices[6].filterSelect, 0, voicemix2, 2);
AudioConnection          patchCord52(voices[7].filterSelect, 0, voicemix2, 3);
AudioConnection          patchCord53(voicemix1, 0, voicemix3, 0);
AudioConnection          patchCord54(voicemix2, 0, voicemix3, 1);
AudioConnection          patchCord55(voicemix3, 0, chorus, 0);
AudioConnection          patchCord56(voicemix3, 0, flange, 0);
AudioConnection          patchCord57(voicemix3, 0, bitcrusher, 0);
AudioConnection          patchCord58(voicemix3, 0, effectmix1, 3);
AudioConnection          patchCord59(chorus, 0, effectmix1, 0);
AudioConnection          patchCord60(flange, 0, effectmix1, 1);
AudioConnection          patchCord61(bitcrusher, 0, effectmix1, 2);
//AudioConnection          patchCord62(effectmix1, 0, freeverb, 0);
//AudioConnection          patchCord63(effectmix1, 0, reverbmix, 1);
AudioConnection          patchCord62(effectmix1, 0, delay1, 0);
AudioConnection          patchCord63(effectmix1, 0, delaymix3, 2);
AudioConnection          patchCord64(delay1, 0, delaymix1, 0);
AudioConnection          patchCord65(delay1, 1, delaymix1, 1);
AudioConnection          patchCord66(delay1, 2, delaymix1, 2);
AudioConnection          patchCord67(delay1, 3, delaymix1, 3);
AudioConnection          patchCord68(delay1, 4, delaymix2, 0);
AudioConnection          patchCord69(delay1, 5, delaymix2, 1);
AudioConnection          patchCord70(delay1, 6, delaymix2, 2);
AudioConnection          patchCord71(delay1, 7, delaymix2, 3);
AudioConnection          patchCord72(delaymix1, 0, delaymix3, 0);
AudioConnection          patchCord73(delaymix2, 0, delaymix3, 1);
AudioConnection          patchCord74(delaymix3, 0, freeverb, 0);
AudioConnection          patchCord75(delaymix3, 0, reverbmix, 1);
AudioConnection          patchCord76(freeverb, 0, reverbmix, 0);
AudioConnection          patchCord77(reverbmix, 0, i2s, 0);
AudioConnection          patchCord78(reverbmix, 0, i2s, 1);
//AudioConnection          patchCord77(voicemix3, 0, i2s, 0);
//AudioConnection          patchCord78(voicemix3, 0, i2s, 1);
//AudioConnection          patchCord77(effectmix1, 0, i2s, 0);
//AudioConnection          patchCord78(effectmix1, 0, i2s, 1);

// Control Nodes (all control nodes (no inputs or outputs))
AudioControlSGTL5000     sgtl5000;       //xy=606,153

// TeensyAudioDesign: end automatically generated code