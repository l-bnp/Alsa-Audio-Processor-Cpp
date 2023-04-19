// audio_processor.h
// Main audio processor class. This class is responsible for reading audio data from the ALSA device, processing it, and writing it back to the ALSA device.
// It also provides functions to set the volume and mute of each channel,
// as well as to set the routing of the input channels to the output channels and to set the filters of each channel.

#ifndef AUDIO_PROCESSOR_H
#define AUDIO_PROCESSOR_H

#include <iostream>
#include <vector>
#include <thread>
#include <cmath>
#include <algorithm> // for std::clamp
#include "alsa_device.h"
#include "biquad_filter.h"
#include "equalizer.h"
#include "event_manager.h"

class AudioProcessor
{
public:
    // Constructor
    AudioProcessor(const char *audio_interface, unsigned int input_channels, unsigned int output_channels, unsigned int rate, EventManager &event_manager);
    // Destructor
    ~AudioProcessor();
    // Start and stop audio processing functions
    void start();
    void stop();
    // Parameter setting functions to be accessed by other classes
    bool setVolume(const std::string &channel_type, unsigned int channel_number, int volume_db);
    bool setMute(const std::string &channel_type, unsigned int channel_number, bool mute);
    bool setRouting(unsigned int input_channel_number, unsigned int output_channel_number, bool route);
    bool setFilter(const std::string &channel_type, unsigned int channel_number, std::string filter_id, bool isEnabled,
                   std::string filter_type_str, double center_frequency, double q_factor, double gain_db);
    int getAmplitude(const std::string &channel_type, unsigned int channel) const;
    void getVolumes(const std::string &channel_type) const;

private:
    // Constants
    const int MIN_VOL = -60;
    const double MIN_CENTER_FREQUENCY = 20.0;
    const double MAX_CENTER_FREQUENCY = 20000.0;
    const double MIN_Q_FACTOR = 0.1;
    const double MAX_Q_FACTOR = 10.0;
    const double MAX_GAIN_DB = 20.0;
    const unsigned int rms_buffer_size = 10000;
    // Event manager
    EventManager &event_manager;
    // Audio parameters
    const char *audio_interface;
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
    unsigned int rate;
    unsigned int input_channels;
    unsigned int output_channels;
    const int buffer_size = 2048;
    // Siganl buffers
    std::vector<char> input_buffer;
    std::vector<short> mixed_samples;
    std::vector<short> output_buffer;
    bool processing_active = false;
    // Level metering variables
    std::vector<std::vector<short>> input_samples_buffer;
    std::vector<std::vector<short>> output_samples_buffer;
    // Signal processing variables
    std::vector<float> input_mutes;
    std::vector<float> output_mutes;
    std::vector<float> input_volumes;
    std::vector<float> output_volumes;
    std::vector<std::vector<float>> mixing_matrix;
    std::vector<Equalizer> input_equalizers;
    std::vector<Equalizer> output_equalizers;
    // Audio processing function
    void process();
    // Parameter value Error printing function
    void handleError(const std::string &function_name) const;
    // Parameter value check function
    bool isValidChannel(const std::string &channel_type, unsigned int channel_number, unsigned int input_channels, unsigned int output_channels) const;
    bool isValidFrequency(double frequency) const;
    bool isValidQFactor(double q_factor) const;
    bool isValidGainDb(double gain_db) const;
    // String to Biquad filter type function
    BiquadFilter::Type stringToFilterType(const std::string &filter_type_str) const;
};

// Constructor and destructor
AudioProcessor::AudioProcessor(const char *audio_interface, unsigned int input_channels, unsigned int output_channels, unsigned int rate, EventManager &event_manager)
    : audio_interface(audio_interface),
      input_channels(input_channels),
      output_channels(output_channels),
      rate(rate),
      event_manager(event_manager),
      input_mutes(input_channels, 1.0f),
      output_mutes(output_channels, 1.0f),
      input_volumes(input_channels, 1.0f),
      output_volumes(output_channels, 1.0f),
      mixing_matrix(input_channels, std::vector<float>(output_channels, 1.0f)),
      processing_active(false),
      input_buffer(buffer_size * input_channels * sizeof(short)),
      output_buffer(buffer_size / (input_channels * sizeof(short)) * output_channels),
      mixed_samples(output_channels)
{
    input_samples_buffer.resize(input_channels, std::vector<short>(rms_buffer_size, 0));
    output_samples_buffer.resize(output_channels, std::vector<short>(rms_buffer_size, 0));
    for (int i = 0; i < input_channels; ++i)
    {
        input_equalizers.emplace_back(rate);
    }
    for (int i = 0; i < output_channels; ++i)
    {
        output_equalizers.emplace_back(rate);
    }

    // Register events to the event manager
    event_manager.on<const std::string &, unsigned int, int>("set_volume", [this](const std::string &channel_type, unsigned int channel_number, int volume_db)
                                                             { this->setVolume(channel_type, channel_number, volume_db); });
    event_manager.on<const std::string &, unsigned int, bool>("set_mute", [this](const std::string &channel_type, unsigned int channel_number, bool mute)
                                                              { this->setMute(channel_type, channel_number, mute); });
    event_manager.on<unsigned int, unsigned int, bool>("set_routing", [this](unsigned int input_channel_number, unsigned int output_channel_number, bool route)
                                                       { this->setRouting(input_channel_number, output_channel_number, route); });
    event_manager.on<const std::string &, unsigned int, std::string, bool, std::string, double, double, double>("set_filter", [this](const std::string &channel_type, unsigned int channel_number, std::string filter_id, bool isEnabled, std::string filter_type_str, double center_frequency, double q_factor, double gain_db)
                                                                                                                { this->setFilter(channel_type, channel_number, filter_id, isEnabled, filter_type_str, center_frequency, q_factor, gain_db); });
    event_manager.on<const std::string &>("get_volumes", [this](const std::string &channel_type)
                                          { this->getVolumes(channel_type); });
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

    // Variable to keep track of the current position in the RMS buffer.
    // The RMS buffer is used to calculate the RMS value of the audio signal.
    // Used to get signal amplitudes for the level meters.
    unsigned int current_rms_buffer_pos = 0;

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
            // Reset the mixed_samples vector to zero for the current frame.
            // IMPORTANT! This is necessary because the mixed_samples vector is used to store the sum of the samples from each input channel.
            // If the mixed_samples vector is not reset to zero, the sum of the samples from each input channel will be added to the previous sum,
            // which will result in an incorrect output.
            std::fill(mixed_samples.begin(), mixed_samples.end(), 0);

            // If the current RMS buffer position reaches the buffer size, reset the position.
            if (current_rms_buffer_pos >= rms_buffer_size)
            {
                current_rms_buffer_pos = 0;
                // std::cout << std::endl << getAmplitude(false, 1);
            }

            // Process each input channel.
            for (int in_ch = 0; in_ch < input_channels; ++in_ch)
            {
                // Get the sample from the input buffer.
                short sample = ((short *)input_buffer.data())[frame * input_channels + in_ch];
                // Store the processed sample in the input samples buffer for level metering.
                input_samples_buffer[in_ch][current_rms_buffer_pos] = sample;

                // Process the sample through the input equalizer.
                sample = static_cast<short>(input_equalizers[in_ch].process(sample));
                // Control input volume by applying input mute and input volume factors.
                sample = static_cast<short>(sample * input_mutes[in_ch] * input_volumes[in_ch]);

                // Mix input channels to output channels according to the mixing matrix.
                for (int out_ch = 0; out_ch < output_channels; ++out_ch)
                {
                    mixed_samples[out_ch] += static_cast<short>(sample * mixing_matrix[in_ch][out_ch]);
                }
            }

            // Process each output channel.
            for (int out_ch = 0; out_ch < output_channels; ++out_ch)
            {
                // Process the mixed sample through the output equalizer.
                output_buffer[frame * output_channels + out_ch] = static_cast<short>(output_equalizers[out_ch].process(mixed_samples[out_ch]));
                // Control output volume by applying output mute and output volume factors.
                output_buffer[frame * output_channels + out_ch] = static_cast<short>(output_buffer[frame * output_channels + out_ch] * output_mutes[out_ch] * output_volumes[out_ch]);
                // Store the processed sample in the output samples buffer for level metering.
                output_samples_buffer[out_ch][current_rms_buffer_pos] = output_buffer[frame * output_channels + out_ch];
            }

            // Increment the current RMS buffer position.
            current_rms_buffer_pos++;
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

// Functions that check the parameter values for the set functions
void AudioProcessor::handleError(const std::string &function_name) const
{
    std::cout << "Error with " << function_name << " parameters!" << std::endl;
}
bool AudioProcessor::isValidChannel(const std::string &channel_type, unsigned int channel_number, unsigned int input_channels, unsigned int output_channels) const
{
    if (channel_type == "input")
    {
        return channel_number > 0 && channel_number <= input_channels;
    }
    else if (channel_type == "output")
    {
        return channel_number > 0 && channel_number <= output_channels;
    }
    return false;
}
bool AudioProcessor::isValidFrequency(double frequency) const
{
    return frequency >= MIN_CENTER_FREQUENCY && frequency <= MAX_CENTER_FREQUENCY;
}
bool AudioProcessor::isValidQFactor(double q_factor) const
{
    return q_factor >= MIN_Q_FACTOR && q_factor <= MAX_Q_FACTOR;
}
bool AudioProcessor::isValidGainDb(double gain_db) const
{
    return gain_db <= MAX_GAIN_DB;
}
// Convert a string to a BiquadFilter::Type
BiquadFilter::Type AudioProcessor::stringToFilterType(const std::string &filter_type_str) const
{
    if (filter_type_str == "highpass")
    {
        return BiquadFilter::Type::Highpass;
    }
    else if (filter_type_str == "lowpass")
    {
        return BiquadFilter::Type::Lowpass;
    }
    else if (filter_type_str == "notch")
    {
        return BiquadFilter::Type::Notch;
    }
    else if (filter_type_str == "peaking")
    {
        return BiquadFilter::Type::Peaking;
    }
    else
    {
        return BiquadFilter::Type::Invalid;
    }
}

// Functions to set (change) audio processing values
bool AudioProcessor::setVolume(const std::string &channel_type, unsigned int channel_number, int volume_db)
{
    if (!isValidChannel(channel_type, channel_number, input_channels, output_channels))
    {
        handleError("setVolume");
        return false;
    }

    float volume = std::pow(10, volume_db / 20.0);
    if (channel_type == "input")
    {
        input_volumes[channel_number - 1] = volume;
    }
    else if (channel_type == "output")
    {
        output_volumes[channel_number - 1] = volume;
    }

    return true;
}

bool AudioProcessor::setMute(const std::string &channel_type, unsigned int channel_number, bool mute)
{
    if (!isValidChannel(channel_type, channel_number, input_channels, output_channels))
    {
        handleError("setMute");
        return false;
    }

    float mute_vol = mute ? 0.0f : 1.0f;
    if (channel_type == "input")
    {
        input_mutes[channel_number - 1] = mute_vol;
    }
    else if (channel_type == "output")
    {
        output_mutes[channel_number - 1] = mute_vol;
    }

    return true;
}

bool AudioProcessor::setRouting(unsigned int input_channel_number, unsigned int output_channel_number, bool route)
{
    if (!isValidChannel("input", input_channel_number, input_channels, output_channels) ||
        !isValidChannel("output", output_channel_number, input_channels, output_channels))
    {
        handleError("setRouting");
        return false;
    }

    float route_vol = route ? 1.0f : 0.0f;
    mixing_matrix[input_channel_number - 1][output_channel_number - 1] = route_vol;
    return true;
}

bool AudioProcessor::setFilter(const std::string &channel_type, unsigned int channel_number, std::string filter_id, bool isEnabled,
                               std::string filter_type_str, double center_frequency, double q_factor, double gain_db)
{
    BiquadFilter::Type filter_type = stringToFilterType(filter_type_str);
    if (filter_type == BiquadFilter::Type::Invalid ||
        !isValidChannel(channel_type, channel_number, input_channels, output_channels) ||
        !isValidFrequency(center_frequency) ||
        !isValidQFactor(q_factor) ||
        !isValidGainDb(gain_db))
    {
        handleError("setFilter");
        return false;
    }

    if (channel_type == "input")
    {
        input_equalizers[channel_number - 1].set_filter(filter_id, isEnabled, filter_type, center_frequency, q_factor, gain_db);
    }
    else if (channel_type == "output")
    {
        output_equalizers[channel_number - 1].set_filter(filter_id, isEnabled, filter_type, center_frequency, q_factor, gain_db);
    }

    return true;
}

// Functions to get audio input or output real time amplitudes
int AudioProcessor::getAmplitude(const std::string &channel_type, unsigned int channel) const
{
    if (!isValidChannel(channel_type, channel, input_channels, output_channels))
    {
        handleError("getAmplitude");
        return -666;
    }

    const std::vector<short> &samples_buffer = channel_type == "input" ? input_samples_buffer[channel - 1] : output_samples_buffer[channel - 1];
    float sum = 0.0f;

    for (short sample : samples_buffer)
    {
        float normalized_sample = static_cast<float>(sample) / 14000.0f;
        sum += normalized_sample * normalized_sample;
    }

    double amplitude_linear = std::clamp(static_cast<double>(std::sqrt(sum / static_cast<double>(rms_buffer_size))), 0.0, 1.0);
    int amplitude_db = static_cast<int>(20 * std::log10(amplitude_linear));

    return amplitude_db;
}

//
void AudioProcessor::getVolumes(const std::string &channel_type) const
{
    std::string command_status = "fail";
    std::vector<int> volumes;
    if (channel_type == "input")
    {
        for (unsigned int i = 1; i <= input_channels; i++)
        {
            volumes.push_back(getAmplitude(channel_type, i));
        }
        command_status = "success";
    }
    else if (channel_type == "output")
    {
        for (unsigned int i = 1; i <= output_channels; i++)
        {
            volumes.push_back(getAmplitude(channel_type, i));
        }
        command_status = "success";
    }
    event_manager.emitEvent("broadcast_volumes", command_status, channel_type, volumes);
}

#endif // AUDIO_PROCESSOR_H