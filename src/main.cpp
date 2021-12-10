#include <Arduino.h>
#include <Audio.h>
#include <Bounce2.h>
#include <Entropy.h>
#include <synth_def.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>
#include <USBHost_t36.h>
#include <presets.h>

#define TFT_CS        36
#define TFT_RST       -1 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC        37
#define TFT_MOSI      26
#define TFT_MISO      39
#define TFT_SCLK      27

#define CONTROL_NUM_1 14
#define CONTROL_NUM_2 16
#define CONTROL_NUM_3 19
#define CONTROL_NUM_4 20
#define CONTROL_NUM_5 21
#define CONTROL_NUM_6 24
#define CONTROL_NUM_7 26
#define CONTROL_NUM_8 28

#define CHORUS_DELAY_LENGTH (16*AUDIO_BLOCK_SAMPLES)
#define FLANGE_DELAY_LENGTH (6*AUDIO_BLOCK_SAMPLES)
// Allocate the delay lines for left and right channels
short chorusDelayLine[CHORUS_DELAY_LENGTH];
short flangeDelayLine[FLANGE_DELAY_LENGTH];

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
USBHost myusb;
USBHub hub1(myusb);
MIDIDevice midi1(myusb);
Button ButtonA = Button();
Button ButtonB = Button();
Encoder enc(31, 32);

bool sdAvailable = true;

byte knobPins[] = {38, 39, 25, 24, 40, 41, 16, 17};
byte controlNumbers[] = {CONTROL_NUM_1, CONTROL_NUM_2, CONTROL_NUM_3,CONTROL_NUM_4
, CONTROL_NUM_5, CONTROL_NUM_6, CONTROL_NUM_7, CONTROL_NUM_8};
int knobValues[8];
int prevKnobValues[8];

int amountOfNotes = 0;
int notes[NUM_VOICES];
int noteOrder[NUM_VOICES] = {-1, -1, -1, -1, -1, -1, -1, -1};
int paramPage = 0;
char waveformName[3][4] = {"Sin", "Sin", "Sin"};
char lfoWaveformName[4] = "Sin";

float volume;

long timer1;
long timer2;

// 'sprite_0', 17x17px
const unsigned char wavepic_sprite_0 [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x22, 0x00, 0x00, 0x41, 0x00, 0x00, 0x41, 
	0x00, 0x00, 0x41, 0x00, 0x00, 0x80, 0x80, 0x00, 0x80, 0x80, 0x80, 0x00, 0x80, 0x80, 0x00, 0x41, 
	0x00, 0x00, 0x41, 0x00, 0x00, 0x41, 0x00, 0x00, 0x22, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00
};
// 'sprite_1', 17x17px
const unsigned char wavepic_sprite_1 [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x14, 0x00, 0x00, 0x22, 0x00, 0x00, 0x22, 
	0x00, 0x00, 0x41, 0x00, 0x00, 0x41, 0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0x41, 0x00, 0x00, 0x41, 
	0x00, 0x00, 0x22, 0x00, 0x00, 0x22, 0x00, 0x00, 0x14, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00
};
// 'sprite_2', 17x17px
const unsigned char wavepic_sprite_2 [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x01, 0x80, 0x00, 0x02, 0x80, 0x00, 0x0c, 
	0x80, 0x00, 0x10, 0x80, 0x00, 0x60, 0x80, 0x00, 0x80, 0x80, 0x80, 0x00, 0x83, 0x00, 0x00, 0x84, 
	0x00, 0x00, 0x98, 0x00, 0x00, 0xa0, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00
};
// 'sprite_3', 17x17px
const unsigned char wavepic_sprite_3 [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x80, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80, 0x00, 0x00, 
	0x80, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80, 
	0x00, 0x00, 0x80, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80, 0x00, 0x00, 0xff, 0x80, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00
};

// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 320)
const int wavepic_allArray_LEN = 4;
const unsigned char* wavepic_allArray[4] = {
	wavepic_sprite_0,
	wavepic_sprite_1,
	wavepic_sprite_2,
	wavepic_sprite_3
};

enum mode {
  MODE_SYNTH,
  MODE_PRESET_BROWSE,
  MODE_PRESET_SAVE,
  MODE_SEQUENCER
};
mode currentMode = MODE_SYNTH;

void OnNoteOn(byte channel, byte pitch, byte velocity)
{
  bool activeEnv[NUM_VOICES];
  for (int i = 0; i < NUM_VOICES; i++) {
    activeEnv[i] = voices[i].envA.isNoteOn();
  }
  int noteIdx = 0;
  if (amountOfNotes < NUM_VOICES) {
    for (int i = 0; i < NUM_VOICES; i++) {
      if (notes[i] == 0 && !activeEnv[i]) {
        notes[i] = pitch;
        noteIdx = i;
        break;
      }
    }
    noteOrder[noteIdx] = amountOfNotes;
  } else {
    for (int i = 0; i < NUM_VOICES; i++) {
      if (noteOrder[i] == 0 || notes[i] == 0) {
        noteOrder[i] = NUM_VOICES;
        notes[i] = pitch;
        noteIdx = i;
      }
      noteOrder[i]--;
    }
  }
  AudioNoInterrupts();
  voices[noteIdx].noteOn(pitch, velocity);
  AudioInterrupts();
}

void OnNoteOff(byte channel, byte pitch, byte velocity)
{
  tft.drawPixel((int16_t)pitch, 59, ST77XX_BLACK);
  for (int i = 0; i < NUM_VOICES; i++) {
    if (notes[i] == pitch) {
      AudioNoInterrupts();
      voices[i].noteOff();
      AudioInterrupts();
    }
  }
}

void setDelayTime (float time) {
  for (int i = 0; i < 8; i++) {
    delay1.delay(i, (i+1)*time);
  }
}

void setDelayFeedback (float feedback) {
  float currentFeedback = 1.0f;
  for (int i = 0; i < 8; i++) {
    currentFeedback *= feedback;
    if (i < 4) {
      delaymix1.gain(i, currentFeedback);
    } else {
      delaymix2.gain((i-4), currentFeedback);
    }
  }
}

void drawBar(int16_t x, int16_t y, int16_t w, int16_t h, float value, uint16_t color) {
  tft.drawRect(x, y, w, h, color);
  tft.fillRect(x + 1 + (int16_t)((float)(w-2) * value), y + 1, w - 2 - (int16_t)((float)(w-2) * value), h - 2, ST77XX_BLACK);
  tft.fillRect(x + 1, y + 1, (int16_t)((float)(w-2) * value), h - 2, color);
}

void drawCenteredBar(int16_t x, int16_t y, int16_t w, int16_t h, float value, uint16_t color) {
  tft.drawRect(x, y, w, h, color);
  tft.fillRect(x + 1, y + 1, w - 2, h - 2, ST77XX_BLACK);
  tft.fillRect(x + 1 + ((float)(w-2) / 2.0f), y + 1, (int16_t)((float)(w-2) * value * 0.5f), h - 2, color);
}

void drawEnvelope(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t color, int16_t width_ms, float a, float d, float s, float r) {
  int16_t aPointX = (int16_t)((a / ((float)width_ms)) * (float)width);
  int16_t dPointX = aPointX + (int16_t)((d / ((float)width_ms)) * (float)width);
  int16_t dPointY = (int16_t)((float)height) * s;
  int16_t sPointX = dPointX + 1;
  int16_t rPointX = sPointX + (int16_t)((r / ((float)width_ms)) * (float)width);
  tft.fillRect(x+1, y+1, width-2, height-2, ST77XX_BLACK);
  tft.drawRect(x, y, width, height, color);
  //attack
  tft.drawLine(x+1, y+height-1, x + aPointX + 1, y + 1, ST77XX_WHITE);
  //decay
  tft.drawLine(x + 1 + aPointX, y + 1, x + dPointX + 1, y + height - dPointY - 1, ST77XX_WHITE);
  //sustain
  tft.drawLine(x + 1 + dPointX, y + height - dPointY - 1, x + sPointX + 1,  y + height - dPointY - 1, ST77XX_WHITE);
  //release
  tft.drawLine(x + 1 + sPointX,  y + height - dPointY - 1, x + rPointX + 1, y + height - 1, ST77XX_WHITE);
}

void newUpdateScreen(int paramChange) {
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  if (paramPage == 0) {
    switch (paramChange) {
      case 0:
        tft.setCursor(0,8);
        tft.setTextSize(2);
        tft.println("OSC 1");
        tft.setTextSize(1);
        tft.fillRect(66, 6, 17, 17, ST77XX_BLACK);
        tft.drawRect(65, 5, 19, 19, ST77XX_GREEN);
        switch (wave1Type) {
          case WAVEFORM_SINE:
            tft.drawBitmap(66, 6,wavepic_sprite_0, 17, 17, ST77XX_WHITE);
            break;
          case WAVEFORM_TRIANGLE_VARIABLE:
            tft.drawBitmap(66, 6,wavepic_sprite_1, 17, 17, ST77XX_WHITE);
            break;
          case WAVEFORM_SAWTOOTH:
            tft.drawBitmap(66, 6,wavepic_sprite_2, 17, 17, ST77XX_WHITE);
            break;
          case WAVEFORM_PULSE:
            tft.drawBitmap(66, 6,wavepic_sprite_3, 17, 17, ST77XX_WHITE);
            break;
        }
        break;
      case 1:
        tft.setCursor(0, 30);
        tft.println("VOL");
        drawBar(25, 30, 32, 8, wave1Vol, ST77XX_GREEN);
        break;
      case 2:
        tft.setCursor(0, 40);
        tft.print("OCT ");
        tft.print(wave1Oct);
        tft.println("  ");
        break;
      case 3:
        tft.setCursor(0, 50);
        tft.print("COARSE ");
        tft.print(wave1Coarse);
        tft.println("  ");
        break;
      case 4:
        tft.setCursor(0, 60);
        tft.println("FINE");
        drawCenteredBar(25, 60, 32, 8, wave1Fine, ST77XX_GREEN);
        break;
      case 5:
        tft.setCursor(0, 70);
        tft.println("SHAPE");
        drawBar(35, 70, 32, 8, wave1pwm, ST77XX_GREEN);
        break;
    }
  } else if (paramPage == 1) {
    switch (paramChange) {
      case 0:
        tft.setCursor(0,8);
        tft.setTextSize(2);
        tft.println("OSC 2");
        tft.setTextSize(1);
        tft.fillRect(66, 6, 17, 17, ST77XX_BLACK);
        tft.drawRect(65, 5, 19, 19, ST77XX_CYAN);
        switch (wave2Type) {
          case WAVEFORM_SINE:
            tft.drawBitmap(66, 6,wavepic_sprite_0, 17, 17, ST77XX_WHITE);
            break;
          case WAVEFORM_TRIANGLE_VARIABLE:
            tft.drawBitmap(66, 6,wavepic_sprite_1, 17, 17, ST77XX_WHITE);
            break;
          case WAVEFORM_SAWTOOTH:
            tft.drawBitmap(66, 6,wavepic_sprite_2, 17, 17, ST77XX_WHITE);
            break;
          case WAVEFORM_PULSE:
            tft.drawBitmap(66, 6,wavepic_sprite_3, 17, 17, ST77XX_WHITE);
            break;
        }
        break;
      case 1:
        tft.setCursor(0, 30);
        tft.println("VOL");
        drawBar(25, 30, 32, 8, wave2Vol, ST77XX_CYAN);
        break;
      case 2:
        tft.setCursor(0, 40);
        tft.print("OCT ");
        tft.print(wave2Oct);
        tft.println("  ");
        break;
      case 3:
        tft.setCursor(0, 50);
        tft.print("COARSE ");
        tft.print(wave2Coarse);
        tft.println("  ");
        break;
      case 4:
        tft.setCursor(0, 60);
        tft.println("FINE");
        drawCenteredBar(25, 60, 32, 8, wave2Fine, ST77XX_CYAN);
        break;
      case 5:
        tft.setCursor(0, 70);
        tft.println("SHAPE");
        drawBar(35, 70, 32, 8, wave2pwm, ST77XX_CYAN);
        break;
    }
  } else if (paramPage == 2) {
    switch (paramChange) {
      case 0:
        tft.setCursor(0,8);
        tft.setTextSize(2);
        tft.println("OSC 3");
        tft.setTextSize(1);
        tft.fillRect(66, 6, 17, 17, ST77XX_BLACK);
        tft.drawRect(65, 5, 19, 19, ST77XX_YELLOW);
        switch (wave3Type) {
          case WAVEFORM_SINE:
            tft.drawBitmap(66, 6,wavepic_sprite_0, 17, 17, ST77XX_WHITE);
            break;
          case WAVEFORM_TRIANGLE_VARIABLE:
            tft.drawBitmap(66, 6,wavepic_sprite_1, 17, 17, ST77XX_WHITE);
            break;
          case WAVEFORM_SAWTOOTH:
            tft.drawBitmap(66, 6,wavepic_sprite_2, 17, 17, ST77XX_WHITE);
            break;
          case WAVEFORM_PULSE:
            tft.drawBitmap(66, 6,wavepic_sprite_3, 17, 17, ST77XX_WHITE);
            break;
        }
        break;
      case 1:
        tft.setCursor(0, 30);
        tft.println("VOL");
        drawBar(25, 30, 32, 8, wave3Vol, ST77XX_YELLOW);
        break;
      case 2:
        tft.setCursor(0, 40);
        tft.print("OCT ");
        tft.print(wave3Oct);
        tft.println("  ");
        break;
      case 3:
        tft.setCursor(0, 50);
        tft.print("COARSE ");
        tft.print(wave3Coarse);
        tft.println("  ");
        break;
      case 4:
        tft.setCursor(0, 60);
        tft.println("FINE");
        drawCenteredBar(25, 60, 32, 8, wave3Fine, ST77XX_YELLOW);
        break;
      case 5:
        tft.setCursor(0, 70);
        tft.println("SHAPE");
        drawBar(35, 70, 32, 8, wave3pwm, ST77XX_YELLOW);
        break;
    }
  } else if (paramPage == 3) {
    if (paramChange == 5) {
      tft.setCursor(0, 8);
      tft.setTextSize(2);
      tft.println("AMP ENV");
      tft.setTextSize(1);
    }
    drawEnvelope(0, 30, 64, 32, ST77XX_BLUE, 15000, attackA, decayA, sustainA, releaseA);
  } else if (paramPage == 4) {
    tft.setCursor(0,8);
    tft.setTextSize(2);
    tft.println("FILTER");
    tft.setTextSize(1);
    switch (paramChange) {
      case 0:
        tft.setCursor(0, 30);
        tft.print("TYPE ");
        tft.println(filterTypeName);
        break;
      case 1:
        tft.setCursor(0, 40);
        tft.print("CUT ");
        drawBar(35, 40, 32, 8, (cutoff - 20.0f) / 14980.0f, ST77XX_CYAN);
        break;
      case 2:
        tft.setCursor(0, 50);
        tft.print("Q ");
        drawBar(35, 50, 32, 8, resonance, ST77XX_CYAN);
        break;
      case 3:
        tft.setCursor(0, 60);
        tft.println("ENV F");
        drawBar(35, 60, 32, 8, envFAmount, ST77XX_CYAN);
        break;
      case 4:
      case 5:
      case 6:
      case 7:
        drawEnvelope(70, 30, 64, 32, ST77XX_CYAN, 15000, attackF, decayF, sustainF, releaseF);
        break;
    }
  } else if (paramPage == 5) {
    tft.setCursor(0,8);
    tft.setTextSize(2);
    tft.println("MOD");
    tft.setTextSize(1);
    switch(paramChange) {
      case 0: //LFO1 wave
        tft.setCursor(0, 30);
        tft.print("WAVE ");
        tft.fillRect(35, 30, 17, 17, ST77XX_BLACK);
        tft.drawRect(35, 30, 19, 19, ST77XX_GREEN);
        switch (lfoWave) {
          case WAVEFORM_SINE:
            tft.drawBitmap(35, 30,wavepic_sprite_0, 17, 17, ST77XX_WHITE);
            break;
          case WAVEFORM_TRIANGLE_VARIABLE:
            tft.drawBitmap(35, 30,wavepic_sprite_1, 17, 17, ST77XX_WHITE);
            break;
          case WAVEFORM_SAWTOOTH:
            tft.drawBitmap(35, 30,wavepic_sprite_2, 17, 17, ST77XX_WHITE);
            break;
          case WAVEFORM_PULSE:
            tft.drawBitmap(35, 30,wavepic_sprite_3, 17, 17, ST77XX_WHITE);
            break;
        }
        break;
        break;
      case 1: //LFO1 freq
        tft.setCursor(0, 50);
        tft.print("FREQ ");
        drawBar(35, 50, 32, 8, lfoFreq / 32.0f, ST77XX_GREEN);
        break;
      case 2: //LFO1 amp 
        tft.setCursor(0, 60);
        tft.print("AMOD ");
        drawBar(35, 60, 32, 8, lfoAAmt, ST77XX_GREEN);
        break;
      case 3: //LFO1 cutoff
        tft.setCursor(0, 70);
        tft.print("FMOD ");
        drawBar(35, 70, 32, 8, lfoFAmt, ST77XX_GREEN);
        break;
    }
  } else if (paramPage == 6) {
    tft.setCursor(0,8);
    tft.setTextSize(2);
    tft.println("FX");
    tft.setTextSize(1);
    switch (paramChange) {
      case 0:
        tft.setCursor(0, 30);
        tft.print("TYPE ");
        tft.println(modEffectName);
        tft.fillRect(0,40, 67, 20, ST77XX_BLACK);
        if (modEffect == 0) {
          tft.println("  ");
        } else if (modEffect == 1) {
          tft.setCursor(0, 40);
          tft.print("LEN ");
          drawBar(30, 40, 32, 8, modEffectParam1, ST77XX_YELLOW);
        } else if (modEffect == 2) {
          tft.setCursor(0, 40);
          tft.print("DPTH ");
          drawBar(30, 40, 32, 8, modEffectParam1, ST77XX_YELLOW);
          tft.setCursor(0, 50);
          tft.print("RATE ");
          drawBar(30, 50, 32, 8, modEffectParam2, ST77XX_YELLOW);
        } else if (modEffect == 3) {
          tft.println(" ");
          tft.setCursor(0, 40);
          tft.print("BITS ");
          drawBar(30, 40, 32, 8, modEffectParam1, ST77XX_YELLOW);
          tft.setCursor(0, 50);
          tft.print("SR ");
          drawBar(30, 50, 32, 8, modEffectParam2, ST77XX_YELLOW);
        }
        break;
      case 1:
        if (modEffect == 1) {
          tft.setCursor(0, 40);
          tft.print("LEN ");
          drawBar(30, 40, 32, 8, modEffectParam1, ST77XX_YELLOW);
        } else if (modEffect == 2) {
          tft.setCursor(0, 40);
          tft.print("DPTH ");
          drawBar(30, 40, 32, 8, modEffectParam1, ST77XX_YELLOW);
        } else if (modEffect == 3) {
          tft.setCursor(0, 40);
          tft.print("BITS ");
          drawBar(30, 40, 32, 8, modEffectParam1, ST77XX_YELLOW);
        }
        break;
      case 2:
        if (modEffect == 2) {
          tft.setCursor(0, 50);
          tft.print("RATE ");
          drawBar(30, 50, 32, 8, modEffectParam2, ST77XX_YELLOW);
        } if (modEffect == 3) {
          tft.setCursor(0, 50);
          tft.print("SR ");
          drawBar(30, 50, 32, 8, modEffectParam2, ST77XX_YELLOW);
        }
        break;
      case 3:
        tft.setCursor(0, 60);
        tft.println("DLY FEED");
        drawBar(50, 60, 32, 8, delayFeedback * 1.111111111, ST77XX_YELLOW);
        break;
      case 4:
        tft.setCursor(0, 70);
        tft.println("DLY TIME");
        drawBar(50, 70, 32, 8, delayTime / 320.0f, ST77XX_YELLOW);
        break;
      case 5:
        tft.setCursor(80, 30);
        tft.println("DLY MIX");
        drawBar(120, 30, 32, 8, delayMix, ST77XX_YELLOW);
        break;
      case 6:
        tft.setCursor(80, 40);
        tft.println("REV TIME");
        drawBar(120, 40, 32, 8, reverbSize, ST77XX_YELLOW);
        break;
      case 7:
        tft.setCursor(70, 50);
        tft.println("REV MIX");
        drawBar(120, 50, 32, 8, reverbMix / 0.7f, ST77XX_YELLOW);
        break;

    }
  }
}

void updateScreen() {
  tft.setCursor(0,0);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  if (paramPage == 0) { //OSC 1
        tft.print(F("Wave: "));
        tft.println(waveformName[0]);
        tft.print(F("Volume: "));
        tft.println(wave1Vol, 2);
        tft.print(F("Oct: "));
        tft.print(wave1Oct);
        tft.println(F("  "));
        tft.print(F("Coarse: "));
        tft.print(wave1Coarse);
        tft.println(F("  "));
        tft.print(F("Fine: "));
        tft.println(wave1Fine, 1);
        tft.print(F("PWM: "));
        tft.println(wave1pwm, 2);
  } else if (paramPage == 1) { //OSC 2
        tft.println(F("OSC 2"));
        tft.print(F("Wave: "));
        tft.println(waveformName[1]);
        tft.print(F("Volume: "));
        tft.println(wave2Vol, 2);
        tft.print(F("Oct: "));
        tft.print(wave2Oct);
        tft.println(F("  "));
        tft.print(F("Coarse: "));
        tft.print(wave2Coarse);
        tft.println(F("  "));
        tft.print(F("Fine: "));
        tft.println(wave2Fine, 1);
        tft.print(F("PWM: "));
        tft.println(wave2pwm);
  } else if (paramPage == 2) { //OSC 3
        tft.println(F("OSC 3"));
        tft.print(F("Wave: "));
        tft.println(waveformName[2]);
        tft.print(F("Volume: "));
        tft.println(wave3Vol, 2);
        tft.print(F("Oct: "));
        tft.print(wave3Oct);
        tft.println(F("  "));
        tft.print(F("Coarse: "));
        tft.print(wave3Coarse);
        tft.println(F("  "));
        tft.print(F("Fine: "));
        tft.println(wave3Fine, 1);
        tft.print(F("PWM: "));
        tft.println(wave3pwm, 2);
  } else if (paramPage == 3) { //Amp
        tft.println(F("Amp Envelope"));
        tft.print(F("A: "));
        tft.print(attackA, 0);
        tft.print(F(",D: "));
        tft.println(decayA, 0);
        tft.print(F("S: "));
        tft.print(sustainA, 2);
        tft.print(F(",R: "));
        tft.println(releaseA, 0);
        tft.println("FM/Shape mod");
        tft.print("OSC 2 FM from 1:");
        tft.println(fm1to2, 2);
        tft.print("OSC 2 shape from 1:");
        tft.println(shape1to2, 2);
        tft.print("OSC 3 FM from 2:");
        tft.println(fm2to3, 2);
        tft.print("OSC 3 shape from 2:");
        tft.println(shape2to3, 2);
  } else if (paramPage == 4) { //Filter
        tft.println(F("Filter"));
        tft.print(F("Filter Type:"));
        tft.println(filterTypeName);
        tft.print(F("Cutoff:"));
        tft.println(cutoff, 0);
        tft.print(F("Resonance:"));
        tft.println(resonance, 1);
        tft.print(F("EnvF Amount:"));
        tft.println(envFAmount, 2);
        tft.println();
        tft.print(F("A: "));
        tft.print(attackF, 0);
        tft.print(F(",D: "));
        tft.println(decayF, 0);
        tft.print(F("S: "));
        tft.print(sustainF, 2);
        tft.print(F(",R: "));
        tft.println(releaseF, 0);
  } else if (paramPage == 5) { //LFO
        tft.print(F("LFO wave: "));
        tft.println(lfoWaveformName);
        tft.print(F("LFO Freq: "));
        tft.println(lfoFreq, 1);
        tft.print(F("Volume amt: "));
        tft.println(lfoAAmt, 2);
        tft.print(F("Filter amt: "));
        tft.println(lfoFAmt, 2);
  } else if (paramPage == 6) { // Effects
        tft.print(F("Mod Type: "));
        tft.println(modEffectName);
        tft.print(F("Mod Parameter 1: "));
        tft.println(modEffectParam1);
        tft.print(F("Mod Parameter 2: "));
        tft.println(modEffectParam2);
        tft.print(F("Delay Feedback: "));
        tft.println(delayFeedback);
        tft.print(F("Delay Mix: "));
        tft.println(delayMix);
        tft.print(F("Delay Time: "));
        tft.println(delayTime);
        tft.print(F("Reverb Size: "));
        tft.println(reverbSize);
        tft.print(F("Reverb Mix: "));
        tft.println(reverbMix);
  }
}

void handleControls(byte channel, byte control, byte value) {
  if (timer1 - millis() <= 256) {
    return;
  } else {
    timer1 = millis();
  }

  switch(control) {
    case CONTROL_NUM_1:
      newUpdateScreen(0);
      break;
    case CONTROL_NUM_2:
      newUpdateScreen(1);
      break;
    case CONTROL_NUM_3:
      newUpdateScreen(2);
      break;
    case CONTROL_NUM_4:
      newUpdateScreen(3);
      break;
    case CONTROL_NUM_5:
      newUpdateScreen(4);
      break;
    case CONTROL_NUM_6:
      newUpdateScreen(5);
      break;
    case CONTROL_NUM_7:
      newUpdateScreen(6);
      break;
    case CONTROL_NUM_8:
      newUpdateScreen(7);
      break;
  }

  AudioNoInterrupts();
  if (paramPage == 0) { // OSC 1
    switch (control) {
      case CONTROL_NUM_1: //waveform
        if (value < 32) {
          strcpy(waveformName[0], "Sin");
          wave1Type = WAVEFORM_SINE;
        } else if (value < 64) {
          strcpy(waveformName[0], "Tri");
          wave1Type = WAVEFORM_TRIANGLE_VARIABLE;
        } else if (value < 96) {
          strcpy(waveformName[0], "Saw");
          wave1Type = WAVEFORM_SAWTOOTH;
        } else {
          strcpy(waveformName[0], "PWM");
          wave1Type = WAVEFORM_PULSE;
        }
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].wave1.begin(wave1Type);
        }
        break;
      case CONTROL_NUM_2: //volume
        wave1Vol = (value/127.f);
        for(int i = 0; i < NUM_VOICES; i++) {
          voices[i].waveMix.gain(0, wave1Vol);
        }
        break;
      case CONTROL_NUM_3: //octave
        wave1Oct = (int)((value/127.f)*4) - 2;
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].osc1Oct = wave1Oct;
        }
        break;
      case CONTROL_NUM_4: //coarse
        wave1Coarse = (int)((value/127.f)*24) - 12;
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].osc1Coarse = wave1Coarse;
        }
        break;
      case CONTROL_NUM_5: //fine
        if (value <= 56) {
          wave1Fine = -1.f + (value/56.f);
        } else if (value > 56 && value < 72) {
          wave1Fine = 0.f;
        } else {
          wave1Fine = ((9 * (value-72.f))/504.f);
        }
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].osc1Fine = wave1Fine;
        }
        break;
      case CONTROL_NUM_6:
        wave1pwm = (value/127.f);
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].wave1.pulseWidth(wave1pwm);
        }
        break;
    }
  } else if (paramPage == 1) { // OSC 2
    switch (control) {
      case CONTROL_NUM_1:
        if (value < 32) {
          strcpy(waveformName[1], "Sin");
          wave2Type = WAVEFORM_SINE;
        } else if (value < 64) {
          strcpy(waveformName[1], "Tri");
          wave2Type = WAVEFORM_TRIANGLE_VARIABLE;
        } else if (value < 96) {
          strcpy(waveformName[1], "Saw");
          wave2Type = WAVEFORM_SAWTOOTH;
        } else {
          strcpy(waveformName[1], "PWM");
          wave2Type = WAVEFORM_PULSE;
        }
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].wave2.begin(wave2Type);
        }
        break;
      case CONTROL_NUM_2:
        wave2Vol = (value/127.f);
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].waveMix.gain(1, wave2Vol);
        }
        break;
      case CONTROL_NUM_3:
        wave2Oct = (int)((value/127.f)*4) - 2;
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].osc2Oct = wave2Oct;
        }
        break;
      case CONTROL_NUM_4:
        wave2Coarse = (int)((value/127.f)*24) - 12;
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].osc2Coarse = wave2Coarse;
        }
        break;
      case CONTROL_NUM_5:
        if (value <= 56) {
          wave2Fine = -1.f + (value/56.f);
        } else if (value > 56 && value < 72) {
          wave2Fine = 0.f;
        } else {
          wave2Fine = ((9 * (value-72.f))/504.f);
        }
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].osc2Fine = wave2Fine;
        }
        break;
      case CONTROL_NUM_6:
        wave2pwm = (value/127.f);
        for( int i = 0; i < NUM_VOICES; i++) {
          //voices[i].wave2Shape.amplitude(wave2pwm);
          voices[i].wave2PhaseMix.gain(2, wave2pwm);
        }
        break;
    }
  } else if (paramPage == 2) { // OSC 3
    switch (control) {
      case CONTROL_NUM_1:
        if (value < 32) {
          strcpy(waveformName[2], "Sin");
          wave3Type = WAVEFORM_SINE;
        } else if (value < 64) {
          strcpy(waveformName[2], "Tri");
          wave3Type = WAVEFORM_TRIANGLE_VARIABLE;
        } else if (value < 96) {
          strcpy(waveformName[2], "Saw");
          wave3Type = WAVEFORM_SAWTOOTH;
        } else {
          strcpy(waveformName[2], "PWM");
          wave3Type = WAVEFORM_PULSE;
        }
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].wave3.begin(wave3Type);
        }
        break;
      case CONTROL_NUM_2:
        wave3Vol = (value/127.f);
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].waveMix.gain(2, wave3Vol);
        }
        break;
      case CONTROL_NUM_3:
        wave3Oct = (int)((value/127.f)*4) - 2;
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].osc3Oct = wave3Oct;
        }
        break;
      case CONTROL_NUM_4:
        wave3Coarse = (int)((value/127.f)*24) - 12;
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].osc3Coarse = wave3Coarse;
        }
        break;
      case CONTROL_NUM_5:
        if (value <= 56) {
          wave3Fine = -1.f + (value/56.f);
        } else if (value > 56 && value < 72) {
          wave3Fine = 0.f;
        } else {
          wave3Fine = ((9 * (value-72.f))/504.f);
        }
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].osc3Fine = wave3Fine;
        }
        break;
      case CONTROL_NUM_6:
        wave3pwm = (value/127.f);
        for( int i = 0; i < NUM_VOICES; i++) {
          //voices[i].wave3Shape.amplitude(wave3pwm);
          voices[i].wave3PhaseMix.gain(2, wave3pwm);
        }
        break;
    }
  } else if (paramPage == 3) { // Amplitude env and FM
    switch (control) {
      case CONTROL_NUM_1: {//attackA 
        attackA = ((value/127.f) * 5000.f);
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].envA.attack(attackA);
        }
        break; }
      case CONTROL_NUM_2: {//decayA
        decayA = ((value/127.f) * 5000.f);
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].envA.decay(decayA);
        }
        break; }
      case CONTROL_NUM_3: {//sustainA
        sustainA = (value/127.f);
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].envA.sustain(sustainA);
        }
        break; }
      case CONTROL_NUM_4: {//releaseA
        releaseA = ((value/127.f) * 5000.f);
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].envA.release(releaseA);
        }
        break; }

      case CONTROL_NUM_5: { // wave 1 to wave 2 fm
        fm1to2 = (value/127.f)*8.f;
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].wave2FreqMix.gain(0,fm1to2);
        }
        break;
      }
      case CONTROL_NUM_6: { //wave 1 to wave 2 shape
        shape1to2 = (value/127.f)*8.f;
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].wave2PhaseMix.gain(0,shape1to2);
        }
        break;
      }
      case CONTROL_NUM_7: { //wave 2 to wave 3 FM
        fm2to3 = (value/127.f)*8.f;
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].wave3FreqMix.gain(0,fm2to3);
        }
        break;
      }
      case CONTROL_NUM_8: { //wave 2 to wave 3 shape
        shape2to3 = (value/127.f)*8.f;
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].wave3PhaseMix.gain(0,shape2to3);
        }
        break;
      }
    }
  } else if (paramPage == 4) { // Filter / Filter env
    switch (control) {
      case CONTROL_NUM_1: //filter type
        if (value < 32){
          strcpy(filterTypeName, "NF");
          filterType = 0;
        } else if (value < 64) {
          strcpy(filterTypeName, "LP");
          filterType = 1;
        } else if (value < 96) {
          strcpy(filterTypeName, "BP");
          filterType = 2;
        } else {
          strcpy(filterTypeName, "HP");
          filterType = 3;
        }
        for (int i = 0; i < 4; i++) {
          for (int j = 0; j < NUM_VOICES; j++) {
            voices[j].filterSelect.gain(i, i==filterType ? 1.f : 0.f);
          }
        }
        break;
      case CONTROL_NUM_2: //cutoff
        cutoff = (pow(value, 2) / 1.076702270f) + 20;
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].filter1.frequency(cutoff);
          voices[i].filter2.frequency(cutoff);
          voices[i].filter1.octaveControl(log2f(15000.0/(float)cutoff));
          voices[i].filter2.octaveControl(log2f(15000.0/(float)cutoff));
        }
        break;
      case CONTROL_NUM_3: //resonance
        resonance = (value / 127.0f);
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].filter1.resonance((resonance * 4.0f) + 1.0f);
          voices[i].filter2.resonance(resonance);
        }
        break;
      case CONTROL_NUM_4: //envf amount
        envFAmount = (value / 127.f);
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].mixer4_11.gain(1,envFAmount);
        }
        break;
      case CONTROL_NUM_5: {//attackF
        attackF = ((value/127.f) * 5000.f);
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].envF.attack(attackF);
        }
        break; }
      case CONTROL_NUM_6: {//decayF
        decayF = ((value/127.f) * 5000.f);
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].envF.decay(decayF);
        }
        break; }
      case CONTROL_NUM_7: {//sustainF
        sustainF = (value/127.f);
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].envF.sustain(sustainF);
        }
        break; }
      case CONTROL_NUM_8: {//releaseF
        releaseF = ((value/127.f) * 5000.f);
        for (int i = 0; i < NUM_VOICES; i++) {
          voices[i].envF.release(releaseF);
        }
        break; }
    }
  } else if (paramPage == 5) { // LFO
    switch (control) {
      case CONTROL_NUM_1: // LFO wave
        if (value < 32) {
          strcpy(lfoWaveformName, "Sin");
          lfoWave = WAVEFORM_SINE;
          lfo1.begin(WAVEFORM_SINE);
        } else if (value < 64) {
          strcpy(lfoWaveformName, "Tri");
          lfoWave = WAVEFORM_TRIANGLE_VARIABLE;
          lfo1.begin(WAVEFORM_TRIANGLE_VARIABLE);
        } else if (value < 96) {
          strcpy(lfoWaveformName, "Saw");
          lfoWave = WAVEFORM_SAWTOOTH;
          lfo1.begin(WAVEFORM_SAWTOOTH);
        } else {
          strcpy(lfoWaveformName, "Sqr");
          lfoWave = WAVEFORM_SQUARE;
          lfo1.begin(WAVEFORM_SQUARE);
        }
        break;
      case CONTROL_NUM_2: // LFO frequency
        lfoFreq = ((float)value/4.f);
        lfo1.frequency(lfoFreq);
        break;
      case CONTROL_NUM_3: // LFO amplitude amount
        lfoAAmt = (value/127.f);
        mod_matrix1.setValue(0, 0, lfoAAmt);
        break;
      case CONTROL_NUM_4: // LFO cutoff amount
        lfoFAmt = (value/127.f);
        mod_matrix1.setValue(0, 4, lfoFAmt);
        break;
    }
  } else if (paramPage == 6) {
    switch (control) {
      case CONTROL_NUM_1: // Mod effect select
        if (value < 32) {
          strcpy(modEffectName, "None");
          modEffect = 0;
          effectmix1.gain(0, 0.0f);
          effectmix1.gain(1, 0.0f);
          effectmix1.gain(2, 0.0f);
          effectmix1.gain(3, 1.0f);
        } else if (value < 64) {
          strcpy(modEffectName, "Chorus");
          modEffect = 1;
          effectmix1.gain(0, 1.0f);
          effectmix1.gain(1, 0.0f);
          effectmix1.gain(2, 0.0f);
          effectmix1.gain(3, 0.0f);
        } else if (value < 96) {
          strcpy(modEffectName, "Flange");
          modEffect = 2;
          effectmix1.gain(0, 0.0f);
          effectmix1.gain(1, 1.0f);
          effectmix1.gain(2, 0.0f);
          effectmix1.gain(3, 0.0f);
        } else {
          strcpy(modEffectName, "Crush");
          modEffect = 3;
          effectmix1.gain(0, 0.0f);
          effectmix1.gain(1, 0.0f);
          effectmix1.gain(2, 0.3f);
          effectmix1.gain(3, 0.0f);
        }
        break;
      case CONTROL_NUM_2: // Mod effect param 1 (time)
        modEffectParam1 = (value/ 127.0f);
        bitcrusher.bits((uint8_t)(value>>3));
        flange.voices(FLANGE_DELAY_LENGTH / 4, modEffectParam1 * FLANGE_DELAY_LENGTH / 4, modEffectParam2);
        chorus.begin(chorusDelayLine, (int)(CHORUS_DELAY_LENGTH * modEffectParam1), 4);
        break;
      case CONTROL_NUM_3: // Mod effect param 2 (depth)
        modEffectParam2 = (value/ 127.0f);
        bitcrusher.sampleRate(modEffectParam2 * AUDIO_SAMPLE_RATE);
        flange.voices(FLANGE_DELAY_LENGTH / 4, modEffectParam1 * FLANGE_DELAY_LENGTH / 4, modEffectParam2);
        break;
      case CONTROL_NUM_4: // Delay feedback
        delayFeedback = (value/127.0f) * 0.90f;
        setDelayFeedback(delayFeedback);
        break;
      case CONTROL_NUM_5: // Delay time
        delayTime = (value * 2.5f);
        setDelayTime(delayTime);
        break;
      case CONTROL_NUM_6: // Delay mix
        delayMix = (value / 127.0f);
        delaymix3.gain(0, delayMix);
        delaymix3.gain(1, delayMix);
        break;
      case CONTROL_NUM_7: // Reverb time
        reverbSize = (value / 127.0f);
        freeverb.roomsize(reverbSize);
        break;
      case CONTROL_NUM_8: // Reverb mix
        reverbMix = (value / 127.0f) * 0.7f;
        reverbmix.gain(0, reverbMix);
        break;
    }
  }
  AudioInterrupts();
  //updateScreen();
}

bool readKnobs() {
  bool update = false;
  for (byte i = 0; i < 8; i++) {
    knobValues[i] = analogRead(knobPins[i]);
    if (knobValues[i] > prevKnobValues[i] + 8 || knobValues[i] < prevKnobValues[i] - 8) {
      update = true;
      handleControls(1, controlNumbers[i], (knobValues[i]>>3));
      prevKnobValues[i] = knobValues[i];
    }
  }
  return update;
}

void setup()
{
  analogReadAveraging(8);
  ButtonA.attach(33, INPUT_PULLUP);
  ButtonA.interval(5);
  ButtonA.setPressedState(LOW);
  ButtonB.attach(34, INPUT_PULLUP);
  ButtonB.interval(5);
  ButtonB.setPressedState(LOW);
  tft.initR(INITR_MINI160x80);
  tft.setRotation(1);
  tft.fillScreen(0);
  tft.setCursor(0,0);

  SPI.setMOSI(7);  // Audio shield has MOSI on pin 7
  SPI.setSCK(14);
  tft.print(F("Initializing SD card..."));

  sdAvailable = initFlash(tft);
  /*if (!SD.begin(10)) {
    tft.println(F("initialization failed!\nPresets won't be available"));
    sdAvailable = false;
    delay(1000);
  } else {
    tft.println(F("Successfully initialized"));
  }

  if (sdAvailable && !SD.exists("/presets")) {
    SD.mkdir(F("/presets"));
    tft.println(F("Created presets directory"));
  }*/
  
  AudioMemory(1024);
  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();
  //usbMIDI.setHandleNoteOff(OnNoteOff);
  //usbMIDI.setHandleNoteOn(OnNoteOn);
	//usbMIDI.setHandleControlChange(handleControls);
	myusb.begin();
	midi1.setHandleNoteOff(OnNoteOff);
	midi1.setHandleNoteOn(OnNoteOn);
	midi1.setHandleControlChange(handleControls);
  sgtl5000.enable();
  volume = analogRead(A1)/1023.0f;
  sgtl5000.volume(volume);
  
  AudioNoInterrupts();
  voicemix1.gain(0, 0.6f);
  voicemix1.gain(1, 0.6f);
  voicemix1.gain(2, 0.6f);
  voicemix1.gain(3, 0.6f);
  voicemix2.gain(0, 0.6f);
  voicemix2.gain(1, 0.6f);
  voicemix2.gain(2, 0.6f);
  voicemix2.gain(3, 0.6f);
  voicemix3.gain(0, 1.0f);
  voicemix3.gain(1, 1.0f);
  effectmix1.gain(0, 0.0f);
  effectmix1.gain(1, 0.0f);
  effectmix1.gain(2, 0.0f);
  effectmix1.gain(3, 1.0f);
  lfo1.begin(0.0f, 3, WAVEFORM_SINE);
  lfo2.begin(0.0f, 3, WAVEFORM_SINE);
  lfo3.begin(0.0f, 3, WAVEFORM_SINE);
  lfo4.begin(0.0f, 3, WAVEFORM_SINE);
  chorus.begin(chorusDelayLine, CHORUS_DELAY_LENGTH, 2);
  flange.begin(flangeDelayLine, FLANGE_DELAY_LENGTH, FLANGE_DELAY_LENGTH/4, FLANGE_DELAY_LENGTH/4, 0.5);
  //delaymix3.gain(0, 0.0f);
  //delaymix3.gain(1, 0.0f);
  //delaymix3.gain(2, 1.0f);
  freeverb.damping(0.5);
  reverbmix.gain(0, 0.0f);
  reverbmix.gain(1, 1.0f);

  AudioInterrupts();
  timer1 = millis();
  timer2 = millis();
  tft.fillScreen(0);
  //updateScreen();
  for(int i = 0 ; i < 8; i++) {
    newUpdateScreen(i);
  }
  enc.write(25);
}

long encRead = 0;
int lastParamPage = 0;

void loop()
{
  switch(currentMode) {
    case MODE_SYNTH:
      if (!midi1) {
        tft.fillScreen(0);
        tft.setCursor(0,0);
        tft.println("No MIDI device connected");
        while (!midi1) {
          delay(100);
          myusb.Task();
        }
        tft.fillScreen(0);
      } else {
        long encRead = enc.read();
        if (encRead <= -4) {
          enc.write(0);
        } else if (encRead >= 25) {
          enc.write(25);
        }
        paramPage = 6 - encRead / 4;
        if (lastParamPage != paramPage) {
          tft.fillScreen(0);
          //updateScreen();
          for(int i = 0 ; i < 8; i++) {
            newUpdateScreen(i);
          }
          lastParamPage = paramPage;
        }

        for (int i = 0; i < NUM_VOICES; i++) {
          notes[i] = voices[i].envA.isActive() ? notes[i] : 0;
        }
        for (int i = 0; i < NUM_VOICES; i++) {
          noteOrder[i] = notes[i] != 0 ? noteOrder[i] : -1;
        }

        amountOfNotes = 0;
        for (int i = 0; i < NUM_VOICES; i++) {
          if (notes[i] != 0) {
            amountOfNotes++;
          }
        }
        if (timer2 - millis() > 256) {
          if (analogRead(A1)/1023.0f != volume) {
            sgtl5000.volume(analogRead(A1)/1023.0f);
          }
          timer2 = millis();
        }
        midi1.read();
      }
      if (ButtonA.pressed()) {
        currentMode = MODE_PRESET_BROWSE;
      }
      if (ButtonB.pressed()) {
        currentMode = MODE_PRESET_SAVE;
      }
      break;
    case MODE_PRESET_BROWSE:
      if (ButtonA.pressed()) {
        currentMode = MODE_SEQUENCER;
      }
      if (ButtonB.pressed()) {
        currentMode = MODE_SYNTH;
      }
      break;
    case MODE_PRESET_SAVE:
      if (ButtonB.pressed()) {
        currentMode = MODE_SYNTH;
      }
      break;
    case MODE_SEQUENCER:
      break;
  }
}