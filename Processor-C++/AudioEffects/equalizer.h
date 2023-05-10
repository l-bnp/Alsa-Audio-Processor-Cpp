// equalizer.h
// Creates a vector of filters and processes each sample through all enabled filters

#ifndef EQUALIZER_H
#define EQUALIZER_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <mutex>
#include "biquad_filter.h"
#include "../Utilities/event_manager.h"
#include "../Utilities/type_aliases.h"

class Equalizer
{
public:
    // Default constructor
    Equalizer() : Equalizer(0.0, "", 0) {}

    // Constructor
    explicit Equalizer(double sample_rate, const std::string &channel_type, unsigned int channel_number);

    // Destructor
    ~Equalizer();

    // Function to set the parameters of a BiquadFilter instance and enable/disable it
    void set_filter(
        const std::string &channel_type, unsigned int channel_number, unsigned int filter_id, bool is_enabled,
        std::string filter_type, double center_frequency, double q_factor, double gain_db, SetFilterCallbackType callback = [](const std::string &, const std::string &, unsigned int, unsigned int, bool, std::string, double, double, double) {});

    // Function to return the parameters of a BiquadFilter instance
    void get_filter(
        const std::string &channel_type, unsigned int channel_number, unsigned int filter_id,
        SetFilterCallbackType callback = [](const std::string &, const std::string &, unsigned int, unsigned int, bool, std::string, double, double, double) {});

    // Function to process a single sample through all enabled filters
    short process(short sample);

private:
    // Map of BiquadFilter instances, indexed by ID
    std::map<unsigned int, BiquadFilter> enabled_filters_;
    std::map<unsigned int, BiquadFilter> disabled_filters_;
    // Sampling rate of the audio signal
    double sample_rate_;
    std::string channelType;
    unsigned int channelNumber;
    unsigned int MAX_FILTERS;
    // EventManager function ID
    size_t event_manager_set_function_id_, event_manager_get_function_id_;
    std::mutex filters_mutex_;
};

// Constructor
Equalizer::Equalizer(double sample_rate, const std::string &channel_type, unsigned int channel_number)
    : sample_rate_(sample_rate),
      channelType(channel_type),
      channelNumber(channel_number),
      MAX_FILTERS(16)
{

    // Emit a get filter event for each filter in the array to syncronize the filter settings from the database when the server starts
    for (int j = 0; j < MAX_FILTERS; ++j)
    {
        EventManager::getInstance().emitEvent<std::string, unsigned int, unsigned int, SetFilterCallbackType>(
            "get_database_filter", channel_type, channel_number, j + 1,
            [this](const std::string &command_type, const std::string &channel_type, unsigned int channel_number, unsigned int filter_id, bool is_enabled,
                   const std::string &filter_type, double center_frequency, double q_factor, double gain_db)
            { this->set_filter(channel_type, channel_number, filter_id, is_enabled, filter_type, center_frequency, q_factor, gain_db); });
    }

    // Register a listener for the "set_filter" event
    event_manager_set_function_id_ = EventManager::getInstance().on<const std::string &, unsigned int, unsigned int, bool, std::string, double, double, double, SetFilterCallbackType>(
        "set_filter", [this](const std::string &channel_type, unsigned int channel_number, unsigned int filter_id, bool is_enabled,
                             std::string filter_type, double center_frequency, double q_factor, double gain_db, SetFilterCallbackType callback)
        { this->set_filter(channel_type, channel_number, filter_id, is_enabled, filter_type, center_frequency, q_factor, gain_db, callback); });

    // Register a listener for the "get_filter" event
    event_manager_get_function_id_ = EventManager::getInstance().on<const std::string &, unsigned int, unsigned int, SetFilterCallbackType>(
        "get_filter", [this](const std::string &channel_type, unsigned int channel_number, unsigned int filter_id, SetFilterCallbackType callback)
        { this->get_filter(channel_type, channel_number, filter_id, callback); });
}

// Destructor
Equalizer::~Equalizer()
{
    EventManager::getInstance().off("set_filter", event_manager_set_function_id_);
    EventManager::getInstance().off("get_filter", event_manager_get_function_id_);
}

// Function to set the parameters of a BiquadFilter instance and enable/disable it
void Equalizer::set_filter(const std::string &channel_type, unsigned int channel_number, unsigned int filter_id, bool is_enabled,
                           std::string filter_type, double center_frequency, double q_factor, double gain_db, SetFilterCallbackType callback)
{
    // std::cout << "set_filter called, channelType: " << channelType << std::endl;
    // std::cout << "set_filter called, channelNumber: " << channelNumber << std::endl;

    if (channel_type == channelType && channel_number == channelNumber)
    {
        // lock the mutex
        std::lock_guard<std::mutex> lock(filters_mutex_);

        // Find the filter with the given ID, if it exists
        auto disabled_filter_iter = disabled_filters_.find(filter_id);
        auto enabled_filter_iter = enabled_filters_.find(filter_id);

        // Pointer to the filter
        BiquadFilter *filter_ptr = nullptr;

        // If the filter exists in one of the maps, set the pointer to it
        if (disabled_filter_iter != disabled_filters_.end())
        {
            filter_ptr = &(disabled_filter_iter->second);
        }
        else if (enabled_filter_iter != enabled_filters_.end())
        {
            filter_ptr = &(enabled_filter_iter->second);
        }

        // If the filter doesn't exist, add a new one to the map
        if (filter_ptr == nullptr)
        {
            BiquadFilter new_filter(filter_type, sample_rate_, center_frequency, q_factor, gain_db);
            if (is_enabled)
            {
                enabled_filters_.emplace(filter_id, new_filter);
            }
            else
            {
                disabled_filters_.emplace(filter_id, new_filter);
            }
        }
        // If the filter already exists, update its parameters
        else
        {
            filter_ptr->set_params(filter_type, sample_rate_, center_frequency, q_factor, gain_db);

            // Enable or disable the filter
            if (is_enabled)
            {
                enabled_filters_.emplace(filter_id, *filter_ptr);
                if (disabled_filter_iter != disabled_filters_.end())
                {
                    disabled_filters_.erase(disabled_filter_iter);
                }
            }
            else
            {
                disabled_filters_.emplace(filter_id, *filter_ptr);
                if (enabled_filter_iter != enabled_filters_.end())
                {
                    enabled_filters_.erase(enabled_filter_iter);
                }
            }
        }
        callback("notify_filter", channel_type, channel_number, filter_id, is_enabled, filter_type, center_frequency, q_factor, gain_db);
    }
}

// Function to get the parameters of a BiquadFilter instance
void Equalizer::get_filter(const std::string &channel_type, unsigned int channel_number, unsigned int filter_id, SetFilterCallbackType callback)
{
    if (channel_type == channelType && channel_number == channelNumber)
    {
        // lock the mutex
        std::lock_guard<std::mutex> lock(filters_mutex_);

        // Find the filter with the given ID, if it exists
        auto disabled_filter_iter = disabled_filters_.find(filter_id);
        auto enabled_filter_iter = enabled_filters_.find(filter_id);

        // Pointer to the filter
        BiquadFilter *filter_ptr = nullptr;
        bool is_enabled = false;

        // If the filter exists in one of the maps, set the pointer to it
        if (disabled_filter_iter != disabled_filters_.end())
        {
            filter_ptr = &(disabled_filter_iter->second);
        }
        else if (enabled_filter_iter != enabled_filters_.end())
        {
            filter_ptr = &(enabled_filter_iter->second);
            is_enabled = true;
        }

        // If the filter doesn't exist, notify a filter with the default parameters
        if (filter_ptr == nullptr)
        {
            callback("notify_filter", channel_type, channel_number, filter_id, false, "peaking", 1000, 0.707, 0);
        }
        else
        {
            callback("notify_filter", channel_type, channel_number, filter_id, is_enabled, filter_ptr->get_filter_type(), filter_ptr->get_center_frequency(),
                     filter_ptr->get_q_factor(), filter_ptr->get_gain_db());
        }
    }
}

// Function to process a single sample through all enabled filters
short Equalizer::process(short sample)
{
    // lock the mutex
    std::lock_guard<std::mutex> lock(filters_mutex_);

    // Sample to be processed
    double out = sample;
    for (auto &pair : enabled_filters_)
    {
        // Process the sample through the enabled filter
        out = pair.second.process(out);
    }

    return static_cast<short>(out);
}

#endif // EQUALIZER_H
