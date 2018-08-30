

#include <Bounce.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SerialFlash.h>
#include "AudioReader.h"

File frec;

class AudioLoopPlay : public AudioStream
{
  public:
    AudioLoopPlay(int channel)
        : AudioStream(0, NULL)
        , m_channel(channel)
    {
        begin();
    }

    void begin(void)
    {
    }

    void update(void)
    {
        audio_block_t* block = allocate();
        if (block == NULL) return;
        getData(m_channel, (uint8_t*) block->data, AUDIO_BLOCK_SAMPLES * 2);
        transmit(block);
        release(block);
    }
    unsigned int m_channel;
};

AudioLoopPlay        playRaw1(0); 
AudioLoopPlay        playRaw2(1); 
AudioLoopPlay        playRaw3(2);
AudioLoopPlay        playRaw4(3); 
AudioInputI2S        i2s2;       
AudioAnalyzePeak     peak1;       
AudioMixer4          mixer1;      
AudioMixer4          mixer2;      
AudioRecordQueue     queue1;      
AudioOutputI2S       i2s1;       
AudioConnection      patchCordM1(playRaw1, 0, mixer1, 1);
AudioConnection      patchCordM2(playRaw2, 0, mixer1, 2);
AudioConnection      patchCordM3(playRaw3, 0, mixer1, 3);
AudioConnection      patchCordM4(playRaw4, 0, mixer1, 0);
AudioConnection      patchCord4m2(mixer1, 0, mixer2, 1);
AudioConnection      patchCord2(i2s2, 1, queue1, 0);
AudioConnection      patchCord3(i2s2, 1, peak1, 0);
AudioConnection      patchCord4(i2s2, 1, mixer2, 0);
AudioConnection      patchCord6(mixer2, 0, i2s1, 0);
AudioConnection      patchCord7(mixer2, 0, i2s1, 1);
AudioControlSGTL5000 sgtl5000_1;

Bounce buttonRecord = Bounce(24, 8);
Bounce buttonStop   = Bounce(25, 8);
Bounce buttonPlay   = Bounce(26, 8);

const int   myInput = AUDIO_INPUT_LINEIN;
extern void startRecording();
extern void stopRecording();
extern void continueRecording();
extern void startPlaying();
extern void continuePlaying();
extern void stopPlaying();

int mode = 0;

void setup()
{
    Serial.begin(115200);
    pinMode(24, INPUT_PULLUP);
    pinMode(25, INPUT_PULLUP);
    pinMode(26, INPUT_PULLUP);

    AudioMemory(60);

    // Enable the audio shield, select input, and enable output
    sgtl5000_1.enable();
    sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
    sgtl5000_1.volume(0.75);
    sgtl5000_1.lineInLevel(2);
    sgtl5000_1.lineOutLevel(31);
    for (int i = 0; i < 4; ++i)
    {
        mixer1.gain(i, 0.5);
    }
    playNew(1, "LOOP108.RAW", 0);
    playNew(0, "P_0003.RAW", 0);
    playNew(2, "P_0002.RAW", 0);
    playNew(3, "P_0004.RAW", 0);
}

uint32_t tNext = 0;

void analyseAudioUsage() {
    static unsigned long nextAnalysis = 0;
    if (millis() >= nextAnalysis) {
        nextAnalysis += 10000;
        Serial.printf("AudioProcessorUsage: %d ", round(AudioProcessorUsage()));
        Serial.printf("% (max=%d%%)", round(AudioProcessorUsageMax()));
        Serial.printf("AudioMemoryUsage: %d", AudioMemoryUsage());
        Serial.print(" blocks, (max=");
        Serial.print(round(AudioMemoryUsageMax()));
        Serial.println(" blocks)");
        AudioProcessorUsageMaxReset();
    }
}
void loop()
{
    analyseAudioUsage();
    update();
    buttonRecord.update();
    buttonStop.update();
    buttonPlay.update();

    if (buttonRecord.fallingEdge())
    {
        Serial.println("Record Button Press");
        if (mode == 0) startRecording();
    }
    if (buttonStop.fallingEdge())
    {
        Serial.println("Stop Button Press");
        if (mode == 1) stopRecording();
    }
    if (buttonPlay.fallingEdge())
    {
    }

    if (mode == 1) { 
      continueRecording();
    }
}

void startRecording() {
    Serial.println("startRecording");
    if (SD.exists("RECORD.RAW")) {
        // The SD library writes new data to the end of the
        // file, so to start a new recording, the old file
        // must be deleted before new data is written.
        SD.remove("RECORD.RAW");
    }
    frec = SD.open("RECORD.RAW", FILE_WRITE);
    if (frec) {
        queue1.begin();
        continueRecording();
    }
    mode = 1;
}


void stopRecording() {
    Serial.println("stopRecording");
    queue1.end();
    if (mode == 1) {
        while (queue1.available() > 0) {
            frec.write((byte *)queue1.readBuffer(), 256);
            queue1.freeBuffer();
        }
        frec.close();
    }
    mode = 0;
}


void continueRecording()
{
    if (queue1.available() >= 2) {
        byte buffer[512];
        memcpy(buffer, queue1.readBuffer(), 256);
        queue1.freeBuffer();
        memcpy(buffer + 256, queue1.readBuffer(), 256);
        queue1.freeBuffer();
        elapsedMicros usec = 0;
        frec.write(buffer, 512);
        uint32_t elapsed = usec;
//        Serial.print("SD write, us=");
//        Serial.println(elapsed);
    }
}


