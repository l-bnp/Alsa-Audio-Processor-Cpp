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
    explicit Equalizer(double sample_rate)
        : sample_rate_(sample_rate)
    {
    }

    // Function to set the parameters of a BiquadFilter instance and enable/disable it
    void set_filter(std::string filter_id, bool enableFilter, BiquadFilter::Type type, double center_frequency, double q, double gain_db = 0.0)
    {
        // Find the filter with the given ID, if it exists
        auto filter_iter = filters_.find(filter_id);

        // If the filter doesn't exist, add a new one to the map
        if (filter_iter == filters_.end())
        {
            filters_.emplace(filter_id, BiquadFilter(type, sample_rate_, center_frequency, q, gain_db));
        }
        // If the filter already exists, update its parameters
        else
        {
            filter_iter->second.set_params(type, sample_rate_, center_frequency, q, gain_db);
        }

        // Enable or disable the filter
        filter_iter->second.setEnabled(enableFilter);
    }

    // Function to process a single sample through all enabled filters
    short process(short sample)
    {
        double out = sample;
        for (auto &pair : filters_)
        {
            // If the filter is enabled, process the sample through it
            if (pair.second.isEnabled())
            {
                out = pair.second.process(out);
            }
        }
        return static_cast<short>(out);
    }

private:
    // Map of BiquadFilter instances, indexed by ID
    std::map<std::string, BiquadFilter> filters_;
    // Sampling rate of the audio signal
    double sample_rate_;
};

#endif // EQUALIZER_H
