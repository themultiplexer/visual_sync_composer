#ifndef AUDIOANALYZER_H
#define AUDIOANALYZER_H
#include <rtaudio/RtAudio.h>
#include <kissfft/kiss_fft.h>

#define FRAMES 1024
#define FREQUENCIES 64

class AudioAnalyzer
{
public:
    AudioAnalyzer();
    void getdevices();
    void startRecording();


    std::vector<float> getFrequencies();
    std::vector<float> getFullFrequencies();
protected:
    static int static_record(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, unsigned int status, void *userData);
private:
    int record(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status);

    float rawdata[FRAMES/2];
    float freqs[FRAMES/2];
    float red_freqs[FREQUENCIES];
    kiss_fft_cfg cfg;
    RtAudio *adc;
};

#endif // AUDIOANALYZER_H
