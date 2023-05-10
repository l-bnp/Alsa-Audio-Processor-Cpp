// meter.h
// Creates a Meter element that can be used to measure the amplitude of an audio signal.

#ifndef METER_H
#define METER_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <mutex>
#include "../Utilities/event_manager.h"
#include "../Utilities/type_aliases.h"

class Meter
{
public:
    // Default constructor
    Meter() : Meter(44100, "", 0) {}

    // Constructor
    explicit Meter(double sample_rate, const std::string &channel_type, unsigned int channel_count);

    // Destructor
    ~Meter();

    // Function to get the amplitude of a single channel
    double get_channel_amplitude_db(unsigned int channel_number);

    // Function to get the amplitude of all channels
    void get_meter(const std::string &channel_type, GetMeterCallbackType callback);

    // Function to store a single sample
    void store(const std::vector<short> &frame);

private:
    std::string channel_type_;
    unsigned int channel_count_;
    std::vector<std::vector<short>> frame_buffer_;
    // EventManager function ID
    size_t event_manager_function_id_;
    std::mutex meter_mutex_;
    unsigned int buffer_size_;
    double sample_rate_;
    unsigned int current_frame_buffer_pos = 0;
    const float DBFS_CONSTANT = 14000.0f;
};

// Constructor
Meter::Meter(double sample_rate, const std::string &channel_type, unsigned int channel_count)
    : sample_rate_(sample_rate), channel_type_(channel_type), channel_count_(channel_count),
      buffer_size_(static_cast<unsigned int>(sample_rate * 0.1)),
      frame_buffer_(channel_count, std::vector<short>(static_cast<unsigned int>(sample_rate * 0.1), 0))
{
    EventManager::getInstance().on<const std::string &, GetMeterCallbackType>(
        "get_meter", [this](const std::string &channel_type, GetMeterCallbackType callback)
        { this->get_meter(channel_type, callback); });
}

// Destructor
Meter::~Meter()
{
    EventManager::getInstance().off("get_meter", event_manager_function_id_);
}

// Function to store a single sample
double Meter::get_channel_amplitude_db(unsigned int channel_number)
{
    // Lock the mutex
    std::lock_guard<std::mutex> lock(meter_mutex_);

    // Get the buffer for the channel
    const std::vector<short> &samples_buffer = frame_buffer_[channel_number];

    // Initialize the sum as 0
    double sum = 0.0;

    // Calculate the sum of the squares of the samples. Divide by a constant to normalize the samples to the actual audio interface 0 dbFS level.
    for (short sample : samples_buffer)
    {
        double normalized_sample = static_cast<double>(sample) / DBFS_CONSTANT;
        sum += normalized_sample * normalized_sample;
    }

    // Calculate the root mean square of the samples and clamp it between 0 and 1
    double amplitude_linear = std::clamp(static_cast<double>(std::sqrt(sum / static_cast<double>(buffer_size_))), 0.0, 1.0);

    // Convert the amplitude to decibels
    double amplitude_db = static_cast<double>(20 * std::log10(amplitude_linear));

    return amplitude_db;
}

// Function to get the amplitude of all channels
void Meter::get_meter(const std::string &channel_type, GetMeterCallbackType callback)
{
    if (channel_type == channel_type_)
    {
        std::vector<double> amplitudes;
        // Get the amplitude of each channel and store it in the vector
        for (unsigned int i = 0; i < channel_count_; i++)
        {
            amplitudes.push_back(get_channel_amplitude_db(i));
        }
        // Call the callback function with the vector of amplitudes
        callback("notify_meter", channel_type, amplitudes);
    }
}

// Function to store a single frame
void Meter::store(const std::vector<short> &frame)
{
    // Lock the mutex
    std::lock_guard<std::mutex> lock(meter_mutex_);

    // Reset the position in the buffer if it has reached the end
    if (current_frame_buffer_pos >= buffer_size_)
    {
        current_frame_buffer_pos = 0;
    }

    // Store the received frame in the buffer
    for (unsigned int i = 0; i < channel_count_; i++)
    {
        frame_buffer_[i][current_frame_buffer_pos] = frame[i];
    }

    // Increment the position in the buffer
    current_frame_buffer_pos++;
}

#endif // METER_H
