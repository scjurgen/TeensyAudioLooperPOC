
#include "AudioReader.h"

#include "SdFat.h"

SdFatSdioEX sdEx;

#define MAXCHANNELS 4

class AudioReader
{
  public:
    AudioReader()
        : currentBuffer(0)
        , currentPosition(0)
        , m_loadNewData(false)
        , m_isPlaying(false)
    {
    }

    File rawfile;

    static const int c_buffersize = 1024;
    uint16_t         currentBuffer;
    uint32_t         currentPosition;
    bool             m_loadNewData;
    uint8_t          buffer[2][c_buffersize];

    uint8_t readOne()
    {
        uint8_t res = buffer[currentBuffer][currentPosition++];
        if (currentPosition >= c_buffersize)
        {
            // Serial.printf("need reload buffer %d\n", currentBuffer);
            currentBuffer   = 1 - currentBuffer;
            m_loadNewData   = true;
            currentPosition = 0;
        }
        return res;
    }

    void getData(uint8_t* target, uint32_t size)
    {
        if (!m_isPlaying)
        {
            memset(target,0,size);
            return;
        }
        while (m_delayStart && size--)
        {
            --m_delayStart;
            *target++ = 0;
        }
        while (size--) { *target++ = readOne(); }
    }

    bool playNew(const char* filename, uint32_t delayStart)
    {
        m_delayStart = delayStart;
        // Serial.printf("playnew %s\n", filename);
        if (!rawfile.open(filename, O_RDWR))
        {
            // Serial.printf("error reading\n");
            return false;
        }
        // file_size = rawfile.size();
        // Serial.printf("Filesize: %d\n", file_size);
        currentBuffer = 0;
        fillBuffer();
        currentBuffer = 1;
        fillBuffer();
        file_offset = 0;
        m_isPlaying = true;
        return true;
    }

    void stop(void)
    {
        rawfile.close();
    }

    void fillBuffer()
    {
        int      remain = c_buffersize;
        unsigned p      = 0;
        while (remain > 0)
        {
            if (!rawfile.available()) { rawfile.rewind(); }
            int blockSize = 4096;
            if (remain < blockSize) blockSize = remain;
            unsigned readCount = rawfile.read(&buffer[1 - currentBuffer][p], blockSize);
            if (readCount == 0) { return; }
            remain -= readCount;
            p += readCount;
        }
    }

    void update(void)
    {
        if (m_loadNewData)
        {
            // printf("\n@@@\n");
            fillBuffer();
            m_loadNewData = false;
        }
    }

  private:
    uint32_t file_size;
    uint32_t file_offset;
    uint32_t m_delayStart;
    bool m_isPlaying;
};

static AudioReader ld[MAXCHANNELS];

void update()
{
    for (int i = 0; i < MAXCHANNELS; ++i) 
    {
      ld[i].update(); 
    }
}

void playNew(uint8_t channel, const char* filename, uint32_t delayStart)
{
    if (!sdEx.begin()) { sdEx.initErrorHalt("SdFatSdioEX begin() failed"); }
    sdEx.chvol();
    ld[channel].playNew(filename, delayStart);
}

void getData(uint8_t channel, uint8_t* target, uint32_t size)
{
    ld[channel].getData(target, size);
}

