// gain.h
// Creates a gain element that can be used to increase or decrease the gain of an audio signal.

#ifndef GAIN_H
#define GAIN_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <mutex>
#include "../Utilities/event_manager.h"
#include "../Utilities/type_aliases.h"

class Gain
{
public:
    // Default constructor
    Gain() : Gain("", 0) {}

    // Constructor
    explicit Gain(const std::string &channel_type, unsigned int channel_number);

    // Destructor
    ~Gain();

    // Function to set the gain
    void set_gain(
        const std::string &channel_type, unsigned int channel_number, double gain_db,
        SetGainCallbackType callback = [](const std::string &, const std::string &, unsigned int, double) {});

    // Function to return the current gain value
    void get_gain(
        const std::string &channel_type, unsigned int channel_number,
        SetGainCallbackType callback = [](const std::string &, const std::string &, unsigned int, double) {});

    // Function to process a single sample through all enabled filters
    short process(short sample);

private:
    double gain = 0.0;
    std::string channelType;
    unsigned int channelNumber;
    // EventManager function ID
    size_t event_manager_set_function_id_, event_manager_get_function_id_;
    std::mutex gain_mutex_;
};

// Constructor
Gain::Gain(const std::string &channel_type, unsigned int channel_number)
    : channelType(channel_type),
      channelNumber(channel_number)
{

    // Emit get_database_gain event to get the gain from the database
    EventManager::getInstance().emitEvent<std::string, unsigned int, SetGainCallbackType>(
        "get_database_gain", channelType, channelNumber,
        [this](const std::string &command_type, const std::string &channel_type, unsigned int channel_number, double gain_db)
        { this->set_gain(channel_type, channel_number, gain_db); });

    // Register callback for set_gain event
    event_manager_set_function_id_ = EventManager::getInstance().on<const std::string &, unsigned int, double, SetGainCallbackType>(
        "set_gain", [this](const std::string &channel_type, unsigned int channel_number, double gain_db, SetGainCallbackType callback)
        { this->set_gain(channel_type, channel_number, gain_db, callback); });

    // Register callback for get_gain event
    event_manager_get_function_id_ = EventManager::getInstance().on<std::string, unsigned int, SetGainCallbackType>(
        "get_gain", [this](const std::string &channel_type, unsigned int channel_number, SetGainCallbackType callback)
        { this->get_gain(channel_type, channel_number, callback); });
}

// Destructor
Gain::~Gain()
{
    EventManager::getInstance().off("set_gain", event_manager_set_function_id_);
    EventManager::getInstance().off("get_gain", event_manager_get_function_id_);
}

// Function to set the gain
void Gain::set_gain(
    const std::string &channel_type, unsigned int channel_number, double gain_db,
    SetGainCallbackType callback)
{
    // std::cout << "set_filter called, channelType: " << channelType << std::endl;
    // std::cout << "set_filter called, channelNumber: " << channelNumber << std::endl;

    if (channel_type == channelType && channel_number == channelNumber)
    {
        // lock mutex
        std::lock_guard<std::mutex> lock(gain_mutex_);

        // convert gain_db to linear gain
        gain = std::pow(10, gain_db / 20.0);

        // clamp gain between 0.0 and 1.0
        gain = std::max(0.0, std::min(1.0, gain));

        // execute callback
        callback("notify_gain", channel_type, channel_number, gain_db);
    }
}

// Function to return the current gain value
void Gain::get_gain(
    const std::string &channel_type, unsigned int channel_number,
    SetGainCallbackType callback)
{
    // std::cout << "get_gain called, channelType: " << channelType << std::endl;
    // std::cout << "get_gain called, channelNumber: " << channelNumber << std::endl;

    if (channel_type == channelType && channel_number == channelNumber)
    {
        // lock mutex
        std::lock_guard<std::mutex> lock(gain_mutex_);

        // execute callback
        callback("notify_gain", channel_type, channel_number, 20.0 * std::log10(gain));
    }
}

// Function to process a single sample
short Gain::process(short sample)
{
    // lock mutex
    std::lock_guard<std::mutex> lock(gain_mutex_);

    // apply gain
    double out = sample * gain;

    // Return the processed sample
    return static_cast<short>(out);
}

#endif // GAIN_H
