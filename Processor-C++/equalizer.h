// equalizer.h
// Creates a vector of filters and processes each sample through all enabled filters

#ifndef EQUALIZER_H
#define EQUALIZER_H

#include <iostream>
#include <vector>
#include <map>
#include "biquad_filter.h" // Make sure to include the BiquadFilter class header

class Equalizer
{
public:
    // Constructor
    explicit Equalizer(double sample_rate);
    // Function to set the parameters of a BiquadFilter instance and enable/disable it
    void set_filter(unsigned int filter_id, bool enableFilter, BiquadFilter::Type type, double center_frequency, double q, double gain_db = 0.0);
    // Function to process a single sample through all enabled filters
    short process(short sample);

private:
    // Map of BiquadFilter instances, indexed by ID
    std::map<unsigned int, BiquadFilter> enabled_filters_;
    std::map<unsigned int, BiquadFilter> disabled_filters_;
    // Sampling rate of the audio signal
    double sample_rate_;
};

Equalizer::Equalizer(double sample_rate)
    : sample_rate_(sample_rate)
{
}

// Function to set the parameters of a BiquadFilter instance and enable/disable it
void Equalizer::set_filter(unsigned int filter_id, bool enableFilter, BiquadFilter::Type type, double center_frequency, double q, double gain_db)
{
    // Find the filter with the given ID, if it exists
    auto disabled_filter_iter = disabled_filters_.find(filter_id);
    auto enabled_filter_iter = enabled_filters_.find(filter_id);

    BiquadFilter *filter_ptr = nullptr;

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
        BiquadFilter new_filter(type, sample_rate_, center_frequency, q, gain_db);
        if (enableFilter)
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
        filter_ptr->set_params(type, sample_rate_, center_frequency, q, gain_db);

        // Enable or disable the filter
        if (enableFilter)
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
}

// Function to process a single sample through all enabled filters
short Equalizer::process(short sample)
{
    double out = sample;
    for (auto &pair : enabled_filters_)
    {
        // Process the sample through the enabled filter
        out = pair.second.process(out);
    }
    return static_cast<short>(out);
}

#endif // EQUALIZER_H
