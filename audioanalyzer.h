#ifndef AUDIOANALYZER_H
#define AUDIOANALYZER_H
#include <rtaudio/RtAudio.h>
#include <kissfft/kiss_fft.h>

#define FRAMES 2048
#define FREQUENCIES 64

class AudioAnalyzer
{
public:
    AudioAnalyzer();
    void getdevices();
    void startRecording();

    std::vector<float> getLeftFrequencies();
    std::vector<float> getRightFrequencies();
protected:
    static int static_record(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, unsigned int status, void *userData);
private:
    int record(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status);

    float freqs[FRAMES/2];
    float freqs2[FRAMES/2];
    kiss_fft_cfg cfg;
    RtAudio *adc;
    bool stereo;
    void do_kissfft(void *inputBuffer, float *outputBuffer, int channel);
    void applyHannWindow(float *data, int channel);
};

#endif // AUDIOANALYZER_H
