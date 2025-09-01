#include "audioanalyzer.h"

AudioAnalyzer::AudioAnalyzer(): stereo(true), useFilterOutput(false), filter(new audiofilter()) {
    cfg = kiss_fft_alloc(FRAMES, 0, NULL, NULL);
    adc = new RtAudio(RtAudio::Api::LINUX_PULSE);
}

void AudioAnalyzer::applyHannWindow(float* data, int channel) {
    for (int i = 0; i < FRAMES; ++i) {
        int ind = stereo ? i * 2 + channel : i;
        float windowValue = 0.5f * (1.0f - std::cos(2.0f * M_PI * ind / (FRAMES - 1)));
        data[ind] *= windowValue;
    }
}

void AudioAnalyzer::startRecording() {
    RtAudio::StreamParameters iParams, oParams;
    iParams.deviceId = adc->getDefaultInputDevice();
    iParams.nChannels = stereo ? 2 : 1;
    //iParams.firstChannel = 0;

    oParams.deviceId = adc->getDefaultOutputDevice();
    oParams.nChannels = stereo ? 2 : 1;
    //oParams.firstChannel = 0;

    RtAudio::StreamOptions streamOptions;
    //streamOptions.numberOfBuffers = 4;
    //streamOptions.flags = RTAUDIO_NONINTERLEAVED;

    unsigned int sampleRate = 48000;
    unsigned int bufferFrames = FRAMES;

#ifdef _WIN32
    bool failure = false;
    try
    {
        adc->openStream(&oParams, &iParams, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, &AudioAnalyzer::record);
        adc->startStream();
    }
    catch (const RtAudioError e)
    {
        std::cout << '\n' << e.getMessage() << '\n' << std::endl;
    }
#else
    if (adc->openStream(&oParams, &iParams, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, &AudioAnalyzer::static_record, this,  &streamOptions)) {
        std::cout << '\n' << adc->getErrorText() << '\n' << std::endl;
        exit(0); // problem with device settings
    }
    // Stream is open ... now start it.
    if (adc->startStream()) {
        std::cout << adc->getErrorText() << std::endl;
    }
#endif
}

int AudioAnalyzer::static_record(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime, unsigned int status, void* userData) {
    return static_cast<AudioAnalyzer*>(userData)->record(outputBuffer, inputBuffer, nBufferFrames, streamTime, status);
}

void AudioAnalyzer::do_kissfft(void* inputBuffer, float* outputBuffer, int channel) {
    kiss_fft_cpx in[FRAMES] = {};
    for (unsigned int i = 0; i < FRAMES; i++) {
        int ind = stereo ? i * 2 + channel : i;
        in[i].r = ((float*)inputBuffer)[ind];
    }

    kiss_fft_cpx out[FRAMES] = {};
    kiss_fft(cfg, in, out);
    for (int i = 0; i < FRAMES/2; i++) {
        outputBuffer[i] = sqrt(out[i].r * out[i].r + out[i].i * out[i].i);
    }
}

int AudioAnalyzer::record(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime, unsigned int status) {
    float* in = static_cast<float*>(inputBuffer);
    float* out = static_cast<float*>(outputBuffer);

    if (status) {
        std::cout << "Stream overflow detected!" << std::endl;
        return 0;
    }

    if (stereo) {
        if (filter && useFilterOutput) {
            filter->sample(inputBuffer, outputBuffer, nBufferFrames);
        } else {
            memset(out, 0, nBufferFrames * sizeof(float));
        }

        applyHannWindow(((float *)inputBuffer), 0);
        do_kissfft(inputBuffer, freqs, 0);
        applyHannWindow(((float *)inputBuffer), 1);
        do_kissfft(inputBuffer, freqs2, 1);

    } else {
        if (filter && useFilterOutput) {
            filter->sample(inputBuffer, outputBuffer, nBufferFrames);
        } else {
            memset(out, 0, nBufferFrames * sizeof(float));
        }
        applyHannWindow(((float *)inputBuffer), 0);
        do_kissfft(inputBuffer, freqs, 0);
    }
    return 0;
}

bool AudioAnalyzer::getUseFilterOutput() const
{
    return useFilterOutput;
}

void AudioAnalyzer::setUseFilterOutput(bool newUseFilterOutput)
{
    useFilterOutput = newUseFilterOutput;
}

void AudioAnalyzer::setFilter(audiofilter *newFilter)
{
    filter = newFilter;
}

audiofilter *AudioAnalyzer::getFilter() const
{
    return filter;
}

std::vector<float> AudioAnalyzer::getLeftFrequencies() {
    std::vector<float> v;
    v.assign(freqs, freqs + FRAMES/2);
    return v;
}

std::vector<float> AudioAnalyzer::getRightFrequencies() {
    std::vector<float> v;
    v.assign(freqs2, freqs2 + FRAMES/2);
    return v;
}

void AudioAnalyzer::getdevices() {
    // Get the list of device IDs
#ifdef _WIN32
    std::vector<unsigned int> ids(adc->getDeviceCount());
    std::iota(ids.begin(), ids.end(), 0);
#else
    std::vector<unsigned int> ids = adc->getDeviceIds();
#endif
    if (ids.size() == 0) {
        std::cout << "No devices found." << std::endl;
        exit(0);
    }

    // Scan through devices for various capabilities
    RtAudio::DeviceInfo info;
    for (unsigned int n = 0; n < ids.size(); n++) {
        info = adc->getDeviceInfo(ids[n]);
        std::cout << "device name = " << info.name << std::endl;
        std::cout << "device id = " << ids[n] << std::endl;
        std::cout << ": maximum input channels = " << info.inputChannels << std::endl;
        std::cout << ": maximum output channels = " << info.outputChannels << std::endl;
    }
}
