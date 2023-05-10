// audio_processor.h
// Main audio processor class. This class is responsible for reading audio data from the ALSA device, processing it, and writing it back to the ALSA device.
// It also provides functions to set the volume and mute of each channel,
// as well as to set the routing of the input channels to the output channels and to set the filters of each channel.

#ifndef AUDIO_PROCESSOR_H
#define AUDIO_PROCESSOR_H

#include <iostream>
#include <vector>
#include <array>
#include <thread>
#include <cmath>
#include <algorithm> // for std::clamp
#include "alsa_device.h"
#include "AudioEffects/biquad_filter.h"
#include "AudioEffects/gain.h"
#include "AudioEffects/mute.h"
#include "AudioEffects/mixer.h"
#include "AudioEffects/meter.h"
#include "AudioEffects/equalizer.h"
#include "Utilities/event_manager.h"
#include "Utilities/type_aliases.h"

class AudioProcessor
{
public:
    // Constructor
    AudioProcessor(const char *audio_interface, unsigned int input_channels, unsigned int output_channels, unsigned int rate);
    // Destructor
    ~AudioProcessor();
    // Start and stop audio processing functions
    void start();
    void stop();

private:
    // Audio parameters
    const char *audio_interface;
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
    unsigned int rate;
    unsigned int input_channels;
    unsigned int output_channels;
    const int buffer_size = 4096;
    // Siganl buffers
    std::vector<char> input_buffer;
    std::vector<short> output_buffer;
    std::vector<short> current_input_frame;
    std::vector<short> current_output_frame;
    // Level metering
    std::unique_ptr<Meter> input_meter;
    std::unique_ptr<Meter> output_meter;
    // Audio Effects
    std::vector<std::unique_ptr<Mute>> input_mutes;
    std::vector<std::unique_ptr<Gain>> input_volumes;
    std::vector<std::unique_ptr<Equalizer>> input_equalizers;
    std::unique_ptr<Mixer> mixer;
    std::vector<std::unique_ptr<Mute>> output_mutes;
    std::vector<std::unique_ptr<Gain>> output_volumes;
    std::vector<std::unique_ptr<Equalizer>> output_equalizers;
    // Audio processing function
    void process();
    bool processing_active = false;
};

// Constructor and destructor
AudioProcessor::AudioProcessor(const char *audio_interface, unsigned int input_channels, unsigned int output_channels,
                               unsigned int rate)
    : audio_interface(audio_interface),
      input_channels(input_channels),
      output_channels(output_channels),
      mixer(std::make_unique<Mixer>(input_channels, output_channels)),
      rate(rate),
      processing_active(false),
      input_buffer(buffer_size * input_channels * sizeof(short)),
      output_buffer(buffer_size / (input_channels * sizeof(short)) * output_channels),
      current_input_frame(input_channels * sizeof(short), 0.0),
      current_output_frame(output_channels * sizeof(short), 0.0)
{
    // Initialize the level meters
    input_meter = std::make_unique<Meter>(rate, "input", input_channels);
    output_meter = std::make_unique<Meter>(rate, "output", output_channels);

    // Initialize the audio effects for each input channel
    for (int i = 0; i < input_channels; ++i)
    {
        input_volumes.emplace_back(std::make_unique<Gain>("input", i + 1));
        input_mutes.emplace_back(std::make_unique<Mute>("input", i + 1));
        input_equalizers.emplace_back(std::make_unique<Equalizer>(rate, "input", i + 1));
    }

    // Initialize the audio effects for each output channel
    for (int i = 0; i < output_channels; ++i)
    {
        output_volumes.emplace_back(std::make_unique<Gain>("output", i + 1));
        output_mutes.emplace_back(std::make_unique<Mute>("output", i + 1));
        output_equalizers.emplace_back(std::make_unique<Equalizer>(rate, "output", i + 1));
    }
}

AudioProcessor::~AudioProcessor()
{
    stop();
}

// Start and stop audio processing functions
void AudioProcessor::start()
{
    if (!processing_active)
    {
        processing_active = true;
        process();
        // processing_thread = std::thread(&AudioProcessor::process, this);
    }
}
void AudioProcessor::stop()
{
    if (processing_active)
    {
        processing_active = false;
        // processing_thread.join();
    }
}

// Main audio processing function. Reads audio data from the capture device, processes it, and writes it to the playback device.
void AudioProcessor::process()
{
    // Declare the alsa device that accesses the audio interface.
    AlsaDevice alsa_device(audio_interface, input_channels, output_channels, rate, format);

    // Start the alsa device.
    alsa_device.start();

    // Main audio processing loop.
    while (processing_active)
    {

        // Read audio data from the capture device into the buffer. If the read fails, print an error message and exit the loop.
        snd_pcm_sframes_t read_frames = alsa_device.read(input_buffer.data(), buffer_size / (input_channels * sizeof(short)));
        if (read_frames < 0)
        {
            std::cerr << "Failed to read from capture device: " << snd_strerror(read_frames) << std::endl;
            break;
        }

        // Loop to process each frame of audio data, where a frame is a set of one sample for each channel
        // The size of a frame can be calculated using the number of channels and the size of each sample,
        // which is 2 bytes in the case of 16 bit samples. So the frame size would be 2 * number of channels
        for (int frame = 0; frame < read_frames; ++frame)
        {

            // Process each input channel.
            for (int in_ch = 0; in_ch < input_channels; ++in_ch)
            {
                // Get the sample from the input buffer.
                short sample = ((short *)input_buffer.data())[frame * input_channels + in_ch];

                // Store the raw input sample in the input frame.
                current_input_frame[in_ch] = sample;
            }

            // Store input frame in input_meter before processing any effects
            input_meter->store(current_input_frame);

            // Process each input channel after storing the input frame.
            for (int in_ch = 0; in_ch < input_channels; ++in_ch)
            {
                // Get the sample from the current input frame.
                short sample = current_input_frame[in_ch];

                // Process the sample through the input equalizer.
                sample = input_equalizers[in_ch]->process(sample);
                // Process the sample through the input volume.
                sample = input_volumes[in_ch]->process(sample);
                // Process the sample through the input mute.
                sample = input_mutes[in_ch]->process(sample);

                // Store the processed input sample in the input frame.
                current_input_frame[in_ch] = sample;
            }

            // Mix input channels to output channels using the mixer object.
            current_output_frame = mixer->process(current_input_frame);

            // Process each output channel.
            for (int out_ch = 0; out_ch < output_channels; ++out_ch)
            {
                // Get the sample from the current output frame.
                output_buffer[frame * output_channels + out_ch] = current_output_frame[out_ch];
                // Process the mixed sample through the output equalizer.
                output_buffer[frame * output_channels + out_ch] = output_equalizers[out_ch]->process(output_buffer[frame * output_channels + out_ch]);
                // Process the mixed sample through the output volume.
                output_buffer[frame * output_channels + out_ch] = output_volumes[out_ch]->process(output_buffer[frame * output_channels + out_ch]);
                // Process the mixed sample through the output mute.
                output_buffer[frame * output_channels + out_ch] = output_mutes[out_ch]->process(output_buffer[frame * output_channels + out_ch]);

                // Store the output frame in the current_output_frame vector
                current_output_frame[out_ch] = output_buffer[frame * output_channels + out_ch];
            }

            // Store output frame in output_meter after processing all effects
            output_meter->store(current_output_frame);
        }

        // Write the processed audio data to the playback device. If the write fails, print an error message and exit the loop.
        snd_pcm_sframes_t write_frames = alsa_device.write(output_buffer.data(), read_frames);
        if (write_frames < 0)
        {
            std::cerr << "Failed to write to playback device: " << snd_strerror(write_frames) << std::endl;
            break;
        }
    }

    // Stop the alsa device at the end of the main loop
    alsa_device.stop();
}

#endif // AUDIO_PROCESSOR_H