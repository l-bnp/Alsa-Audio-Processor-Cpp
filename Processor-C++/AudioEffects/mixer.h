// Mixer.h
// Creates a Mixer element that can be used to Mix inputs to outputs.

#ifndef MIXER_H
#define MIXER_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <mutex>
#include "../Utilities/event_manager.h"
#include "../Utilities/type_aliases.h"

class Mixer
{
public:
    // Default constructor
    Mixer() : Mixer(0, 0) {}

    // Constructor
    explicit Mixer(unsigned int input_channels, unsigned int output_channels);

    // Destructor
    ~Mixer();

    // Function to set the mixer routing. If mix_bool is true, the input channel will be mixed to the output channel.
    void set_mixer(
        unsigned int input_channel_number, unsigned int output_channel_number, bool mix_bool,
        SetMixerCallbackType callback = [](const std::string &, unsigned int, unsigned int, bool) {});

    // Function to return the mixing_matrix_ value for a given input and output channel
    void get_mixer(unsigned int input_channel_number, unsigned int output_channel_number, SetMixerCallbackType callback);

    // Function to process a single sample through the mixer
    std::vector<short> process(const std::vector<short> &input_frame);

private:
    std::vector<std::vector<float>> mixing_matrix_;
    unsigned int input_channels_;
    unsigned int output_channels_;
    std::vector<short> output_frame_;
    // EventManager function ID
    size_t event_manager_set_function_id_, event_manager_get_function_id_;
    std::mutex mixer_mutex_;
};

// Constructor
Mixer::Mixer(unsigned int input_channels, unsigned int output_channels)
    : input_channels_(input_channels), output_channels_(output_channels),
      mixing_matrix_(input_channels, std::vector<float>(output_channels, 0.0f)),
      output_frame_(output_channels, 0)
{
    for (int i = 0; i < input_channels_; ++i)
    {
        for (int j = 0; j < output_channels_; ++j)
        {
            EventManager::getInstance().emitEvent<unsigned int, unsigned int, SetMixerCallbackType>(
                "get_database_mixer", i + 1, j + 1,
                [this](const std::string &command_type, unsigned int input_channel_number, unsigned int output_channel_number, bool mix_bool)
                { this->set_mixer(input_channel_number, output_channel_number, mix_bool); });
        }
    }

    // Register the set_mixer function with the corresponding event
    event_manager_set_function_id_ = EventManager::getInstance().on<unsigned int, unsigned int, bool, SetMixerCallbackType>(
        "set_mixer", [this](unsigned int input_channel_number, unsigned int output_channel_number, bool mix_bool, SetMixerCallbackType callback)
        { this->set_mixer(input_channel_number, output_channel_number, mix_bool, callback); });

    // Register the get_mixer function with the corresponding event
    event_manager_get_function_id_ = EventManager::getInstance().on<unsigned int, unsigned int, SetMixerCallbackType>(
        "get_mixer", [this](unsigned int input_channel_number, unsigned int output_channel_number, SetMixerCallbackType callback)
        { this->get_mixer(input_channel_number, output_channel_number, callback); });
}

// Destructor
Mixer::~Mixer()
{
    EventManager::getInstance().off("set_mixer", event_manager_set_function_id_);
    EventManager::getInstance().off("get_mixer", event_manager_get_function_id_);
}

// Function to set the mixer routing. If mix_bool is true, the input channel will be mixed to the output channel.
void Mixer::set_mixer(unsigned int input_channel_number, unsigned int output_channel_number, bool mix_bool,
                      SetMixerCallbackType callback)
{
    if (input_channel_number <= input_channels_ && output_channel_number <= output_channels_)
    {
        // Lock the mixer_mutex_ to prevent the mixing_matrix_ from being modified while it is being read.
        std::lock_guard<std::mutex> lock(mixer_mutex_);
        // If mix_bool is true, set the mixing_matrix_ value to 1.0f. Otherwise, set the mixing_matrix_ value to 0.0f.
        float mix = mix_bool ? 1.0f : 0.0f;
        mixing_matrix_[input_channel_number - 1][output_channel_number - 1] = mix;
        // Execute callback function
        callback("notify_mixer", input_channel_number, output_channel_number, mix_bool);
    }
}

// Function to return the mixing_matrix_ value for a given input and output channel
void Mixer::get_mixer(unsigned int input_channel_number, unsigned int output_channel_number, SetMixerCallbackType callback)
{
    if (input_channel_number <= input_channels_ && output_channel_number <= output_channels_)
    {
        // Lock the mixer_mutex_ to prevent the mixing_matrix_ from being modified while it is being read.
        std::lock_guard<std::mutex> lock(mixer_mutex_);
        // Execute callback function
        callback("notify_mixer", input_channel_number, output_channel_number, mixing_matrix_[input_channel_number - 1][output_channel_number - 1] ? true : false);
    }
}

// Function to process a frame of one sample of each input channel through the mixer
std::vector<short> Mixer::process(const std::vector<short> &input_frame)
{
    // Lock the mixer_mutex_ to prevent the mixing_matrix_ from being modified while it is being read.
    std::lock_guard<std::mutex> lock(mixer_mutex_);

    // Reset the mixed_samples vector to zero for the current frame.
    // IMPORTANT! This is necessary because the mixed_samples vector is used to store the sum of the samples from each input channel.
    // If the mixed_samples vector is not reset to zero, the sum of the samples from each input channel will be added to the previous sum,
    // which will result in an incorrect output.
    std::fill(output_frame_.begin(), output_frame_.end(), 0);

    // Multiply each input channel sample by the corresponding mixing_matrix_ value and add the result to the output_frame_ vector.
    for (unsigned int in_ch = 0; in_ch < input_channels_; ++in_ch)
    {
        for (unsigned int out_ch = 0; out_ch < output_channels_; ++out_ch)
        {
            output_frame_[out_ch] +=
                static_cast<short>(input_frame[in_ch] * mixing_matrix_[in_ch][out_ch]);
        }
    }

    return output_frame_;
}

#endif // MIXER_H
