// alsa_device.h
// The AlsaDevice class is used to handle audio input and output using the ALSA library.
// It provides methods for starting and stopping the audio device,
// as well as reading and writing audio data to and from the device.

#ifndef ALSA_DEVICE_H
#define ALSA_DEVICE_H

#include <iostream>
#include <alsa/asoundlib.h>

class AlsaDevice
{
public:
    AlsaDevice(const char *audio_interface, unsigned int input_channels, unsigned int output_channels, unsigned int rate, snd_pcm_format_t format);
    ~AlsaDevice();

    void start();
    void stop();

    snd_pcm_sframes_t read(void *buffer, snd_pcm_uframes_t size);
    snd_pcm_sframes_t write(void *buffer, snd_pcm_uframes_t size);

private:
    const char *audio_interface;
    snd_pcm_t *capture_handle;
    snd_pcm_t *playback_handle;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_format_t format;
    unsigned int rate;
    unsigned int input_channels;
    unsigned int output_channels;
    snd_pcm_uframes_t frames;
};

AlsaDevice::AlsaDevice(const char *audio_interface, unsigned int input_channels, unsigned int output_channels, unsigned int rate, snd_pcm_format_t format)
    : audio_interface(audio_interface),
      input_channels(input_channels),
      output_channels(output_channels),
      rate(rate),
      format(format),
      capture_handle(nullptr),
      playback_handle(nullptr)
{
}

AlsaDevice::~AlsaDevice()
{
    stop();
}

// Open capture_handle and playback_handle, configure ALSA
void AlsaDevice::start()
{
    int err;

    // Allocate memory for hardware parameters structure.
    snd_pcm_hw_params_alloca(&hw_params);

    // Open the audio capture device using ALSA's snd_pcm_open function.
    // If it fails, print an error message and stop processing.
    if ((err = snd_pcm_open(&capture_handle, audio_interface, SND_PCM_STREAM_CAPTURE, 0)) < 0)
    {
        std::cerr << "Cannot open audio interface for capture: " << snd_strerror(err) << std::endl;
        stop();
    }

    // Open the audio playback device using ALSA's snd_pcm_open function.
    // If it fails, print an error message and stop processing.
    if ((err = snd_pcm_open(&playback_handle, audio_interface, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    {
        std::cerr << "Cannot open audio interface for playback: " << snd_strerror(err) << std::endl;
        stop();
    }

    // Create handles vector to iterate over both handles
    std::initializer_list<snd_pcm_t *> handles = {capture_handle, playback_handle};
    // Set capture and playback parameters
    for (snd_pcm_t *handle : handles)
    {
        // Get the default hardware parameters.
        snd_pcm_hw_params_any(handle, hw_params);
        // Set access type to interleaved.
        snd_pcm_hw_params_set_access(handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
        // Set audio format (e.g. SND_PCM_FORMAT_S16_LE for 16-bit signed little-endian).
        snd_pcm_hw_params_set_format(handle, hw_params, format);
        // Set the sample rate near the requested rate.
        snd_pcm_hw_params_set_rate_near(handle, hw_params, &rate, 0);

        // Set the correct number of channels for each handle
        if (handle == capture_handle)
        {
            snd_pcm_hw_params_set_channels(handle, hw_params, input_channels);
        }
        else
        {
            snd_pcm_hw_params_set_channels(handle, hw_params, output_channels);
        }

        // Apply the hardware parameters to the device.
        snd_pcm_hw_params(handle, hw_params);
        // Prepare the device for operation.
        snd_pcm_prepare(handle);

        // Set buffer and period sizes for the device.
        snd_pcm_hw_params_set_buffer_size_near(handle, hw_params, &frames);
        snd_pcm_hw_params_set_period_size_near(handle, hw_params, &frames, nullptr);
    }
}

void AlsaDevice::stop()
{
    if (capture_handle)
    {
        snd_pcm_close(capture_handle);
        capture_handle = nullptr;
    }
    if (playback_handle)
    {
        snd_pcm_drain(playback_handle);
        snd_pcm_close(playback_handle);
        playback_handle = nullptr;
    }
}

// Read and write functions
snd_pcm_sframes_t AlsaDevice::read(void *buffer, snd_pcm_uframes_t size)
{
    // Read audio data from the capture device to the buffer
    snd_pcm_sframes_t read_frames = snd_pcm_readi(capture_handle, buffer, size);
    // If reading fails, try to recover the capture device.
    if (read_frames < 0)
    {
        read_frames = snd_pcm_recover(capture_handle, read_frames, 0);
    }
    // Return the number of frames read.
    return read_frames;
}

snd_pcm_sframes_t AlsaDevice::write(void *buffer, snd_pcm_uframes_t size)
{
    // Write audio data from the buffer to the playback device
    snd_pcm_sframes_t write_frames = snd_pcm_writei(playback_handle, buffer, size);
    // If writing fails, try to recover the playback device.
    if (write_frames < 0)
    {
        write_frames = snd_pcm_recover(playback_handle, write_frames, 0);
    }
    // Return the number of frames written.
    return write_frames;
}

#endif // ALSA_DEVICE_H
