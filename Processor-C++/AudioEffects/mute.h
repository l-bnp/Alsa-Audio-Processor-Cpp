// Mute.h
// Creates a Mute element that can be used to Mute an audio signal.

#ifndef MUTE_H
#define MUTE_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <mutex>
#include "../Utilities/event_manager.h"
#include "../Utilities/type_aliases.h"

class Mute
{
public:
    // Default constructor
    Mute() : Mute("", 0) {}

    // Constructor
    explicit Mute(const std::string &channel_type, unsigned int channel_number);

    // Destructor
    ~Mute();

    // Function to set the mute
    void set_mute(
        const std::string &channel_type, unsigned int channel_number, bool mute_bool,
        SetMuteCallbackType callback = [](const std::string &, const std::string &, unsigned int, bool) {});

    // Function to return the mute value
    void get_mute(
        const std::string &channel_type, unsigned int channel_number, SetMuteCallbackType callback = [](const std::string &, const std::string &, unsigned int, bool) {});

    // Function to process a single sample through all enabled filters
    short process(short sample);

private:
    double mute = 0.0;
    std::string channelType;
    unsigned int channelNumber;
    // EventManager function ID
    size_t event_manager_set_function_id_, event_manager_get_function_id_;
    std::mutex mute_mutex_;
};

// Constructor
Mute::Mute(const std::string &channel_type, unsigned int channel_number)
    : channelType(channel_type),
      channelNumber(channel_number)
{

    // Emit a get_mute event to get the current mute value from the database
    EventManager::getInstance().emitEvent<std::string, unsigned int, SetMuteCallbackType>(
        "get_database_mute", channelType, channelNumber,
        [this](const std::string &command_type, const std::string &channel_type, unsigned int channel_number, bool mute_bool)
        { this->set_mute(channel_type, channel_number, mute_bool); });

    // Register a listener to the set_mute event
    event_manager_set_function_id_ = EventManager::getInstance().on<const std::string &, unsigned int, bool, SetMuteCallbackType>(
        "set_mute", [this](const std::string &channel_type, unsigned int channel_number, bool mute_bool, SetMuteCallbackType callback)
        { this->set_mute(channel_type, channel_number, mute_bool, callback); });

    // Register a listener to the get_mute event
    event_manager_get_function_id_ = EventManager::getInstance().on<const std::string &, unsigned int, SetMuteCallbackType>(
        "get_mute", [this](const std::string &channel_type, unsigned int channel_number, SetMuteCallbackType callback)
        { this->get_mute(channel_type, channel_number, callback); });
}

// Destructor
Mute::~Mute()
{
    EventManager::getInstance().off("set_mute", event_manager_set_function_id_);
    EventManager::getInstance().off("get_mute", event_manager_get_function_id_);
}

// Function to set the mute
void Mute::set_mute(
    const std::string &channel_type, unsigned int channel_number, bool mute_bool,
    SetMuteCallbackType callback)
{
    // std::cout << "set_filter called, channelType: " << channelType << std::endl;
    // std::cout << "set_filter called, channelNumber: " << channelNumber << std::endl;

    if (channel_type == channelType && channel_number == channelNumber)
    {
        // lock the mutex
        std::lock_guard<std::mutex> lock(mute_mutex_);

        // set the mute value
        mute = mute_bool ? 0.0 : 1.0;

        // call the callback function
        callback("notify_mute", channel_type, channel_number, mute_bool);
    }
}

// Function to return the mute value
void Mute::get_mute(
    const std::string &channel_type, unsigned int channel_number,
    SetMuteCallbackType callback)
{
    // std::cout << "get_filter called, channelType: " << channelType << std::endl;
    // std::cout << "get_filter called, channelNumber: " << channelNumber << std::endl;

    if (channel_type == channelType && channel_number == channelNumber)
    {
        // lock the mutex
        std::lock_guard<std::mutex> lock(mute_mutex_);

        // call the callback function
        callback("notify_mute", channel_type, channel_number, mute == 0.0);
    }
}

// Function to process a single sample
short Mute::process(short sample)
{
    // lock the mutex
    std::lock_guard<std::mutex> lock(mute_mutex_);

    // apply the mute
    double out = sample * mute;

    // return the processed sample
    return static_cast<short>(out);
}

#endif // MUTE_H
