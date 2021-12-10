#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_ST7735.h>

File presets[64];
char presetNames[64][64];

boolean initFlash(Adafruit_ST7735 tft) {
    if (!SD.begin(10)) {
        tft.println(F("initialization failed!\nPresets won't be available"));
        return false;
        delay(1000);
    } else {
        tft.println(F("Successfully initialized"));
    }

    if (!SD.exists("/presets")) {
        SD.mkdir(F("/presets"));
        tft.println(F("Created presets directory"));
    }
    return true;
}

int findPresets() {
    if (!SD.exists("/presets")) {
        Serial.println("Created presets directory");
        SD.mkdir("/presets");
    }
    File presetsDir = SD.open("/presets");
    if (!presetsDir.isDirectory()) {
        Serial.println("Presets is not a directory");
        return 0;
    }
    File child = presetsDir.openNextFile();
    int i = 0;
    while (child && i < 64) {
        char* currentFilename = child.name();
        //child.name(currentFilename, sizeof(currentFilename));
        strcpy(presetNames[i], currentFilename);
        Serial.print("Found file ");
        Serial.println(currentFilename);
        presets[i] = child;
        i++;
    }
    return i;
}

boolean savePreset(String name, int idx) {
    if (!SD.exists("/presets")) {
        Serial.println("Created presets directory");
        SD.mkdir("/presets");
    }
    File presetsDir = SD.open("/presets");
    if (!presetsDir.isDirectory()) {
        Serial.println("Presets is not a directory!");
        return false;
    }
    String newPath = "/presets/" + (String)idx + "_" + name;
    Serial.print("Opening file ");
    Serial.println(newPath);
    File newPreset = SD.open(newPath, FILE_WRITE);
    int intParams[] = {wave1Type, wave2Type, wave3Type, 
        wave1Oct, wave2Oct, wave3Oct,
        wave1Coarse, wave2Coarse, wave3Coarse, lfoWave, filterType};
    float floatParams[] = {wave1Vol, wave2Vol, wave3Vol,
        wave1pwm, wave2pwm, wave3pwm,
        wave1Fine, wave2Fine, wave3Fine,
        wave1pwm, wave2pwm, wave3pwm,
        lfoFreq, lfoAAmt, lfoFAmt,
        cutoff, resonance, envFAmount,
        attackF, decayF, sustainF, releaseF,
        attackA, decayA, sustainA, releaseA};
    Serial.println("Writing integer parameters");
    for (unsigned int i = 0; i < sizeof(intParams)/sizeof(int); i++) {
        newPreset.println(intParams[i]);
    }
    Serial.println("Writing float parameters");
    for (unsigned int i = 0; i < sizeof(floatParams)/sizeof(float); i++) {
        newPreset.println(floatParams[i]);
    }
    newPreset.close();
    return true;
}

boolean loadPreset(int idx) {
    if (!SD.exists("/presets")) {
        Serial.println("Created presets directory");
        SD.mkdir("/presets");
    }
    File presetsDir = SD.open("/presets");
    if (!presetsDir.isDirectory()) {
        Serial.println("Presets is not a directory");
        return false;
    }
    presetsDir.rewindDirectory();
    File child = presetsDir.openNextFile();
    File loadedPreset;
    while (child) {
        int cur_idx = child.parseInt();
        if (cur_idx == idx) {
            Serial.print("Found preset file ");
            Serial.println(child.name());
            loadedPreset = child;
            break;
        }
        child = presetsDir.openNextFile();
    }
    if (!loadedPreset) {
        Serial.println("Couldn't find preset file");
        return false;
    }
    
    int * intParams[] = {&wave1Type, &wave2Type, &wave3Type, 
        &wave1Oct, &wave2Oct, &wave3Oct,
        &wave1Coarse, &wave2Coarse, &wave3Coarse, &lfoWave, &filterType};
    float * floatParams[] = {&wave1Vol, &wave2Vol, &wave3Vol,
        &wave1pwm, &wave2pwm, &wave3pwm,
        &wave1Fine, &wave2Fine, &wave3Fine,
        &wave1pwm, &wave2pwm, &wave3pwm,
        &lfoFreq, &lfoAAmt, &lfoFAmt,
        &cutoff, &resonance, &envFAmount,
        &attackF, &decayF, &sustainF, &releaseF,
        &attackA, &decayA, &sustainA, &releaseA};
    
    char currentLine[64];
    
    Serial.println("Loading integer parameters");
    for (int i = 0; i < sizeof(intParams)/sizeof(int*); i++) {
        loadedPreset.readBytesUntil('\n', currentLine, 64);
        *(intParams[i]) = atoi(currentLine);
    }
    Serial.println("Loading float parameters");
    for (int i = 0; i < sizeof(floatParams)/sizeof(float*); i++) {
        loadedPreset.readBytesUntil('\n', currentLine, 64);
        *(floatParams[i]) = atof(currentLine);
    }
    loadedPreset.close();
    //updateParams();
    return true;
}

void listPresets(Adafruit_ST7735 tft, int startIdx, int selected) {
    tft.fillScreen(ST7735_BLACK);
    tft.fillRect(0,0,160,10, ST7735_BLUE);
    tft.drawLine(0,10,0,160, ST7735_WHITE);
    tft.setCursor(0,1);
    tft.println("Load Preset");
    if (!SD.exists("/presets")) {
        Serial.println("Created presets directory");
        SD.mkdir("/presets");
    }
    File presetsDir = SD.open("/presets");
    if (!presetsDir.isDirectory()) {
        tft.println("Presets is not a directory");
        return;
    }
    presetsDir.rewindDirectory();
    File child = presetsDir.openNextFile();
    File loadedPreset;
    int cur_idx = 0;
    while (child) {
        if (cur_idx < startIdx) {
            break;
        } else {
            if (cur_idx == selected) {
                tft.drawRect(0, 9 + ((cur_idx - startIdx) * 8), 160, 10, ST77XX_BLUE);
            }
            tft.setCursor(0, 10 + ((cur_idx - startIdx) * 8));
            tft.println(child.name());
        }
        cur_idx++;
        child = presetsDir.openNextFile();
    }
}