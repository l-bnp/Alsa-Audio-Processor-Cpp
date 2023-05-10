// biquad_filter.h
// A BiquadFilter is a type of digital signal processing filter that uses a second-order recursive algorithm
// to filter signals. It is a versatile filter that can be used for a variety of filtering tasks, including
// lowpass, highpass, bandpass, notch, allpass, and peaking. The filter works by processing each sample of
// an input signal and producing a corresponding output sample based on the input and previous samples.
// The coefficients of the filter determine the filter's cutoff frequency, resonance, and gain.

#ifndef BIQUAD_FILTER_H
#define BIQUAD_FILTER_H

#include <cmath>
#include <vector>

class BiquadFilter
{
public:
    // Constructor
    explicit BiquadFilter(std::string filter_type, double sample_rate, double center_frequency, double q_factor, double gain_db);
    // Destructor
    ~BiquadFilter();
    // set_params function
    void set_params(std::string filter_type, double sample_rate, double center_frequency, double q_factor, double gain_db);
    // Process function
    short process(short sample);
    // Functions to return filter parameters
    std::string get_filter_type() const { return filter_type_; }
    double get_sample_rate() const { return sample_rate_; }
    double get_center_frequency() const { return center_frequency_; }
    double get_q_factor() const { return q_factor_; }
    double get_gain_db() const { return gain_db_; }

private:
    // Filter parameters
    std::string filter_type_ = "peaking";
    double sample_rate_ = 44100;
    double center_frequency_ = 1000;
    double q_factor_ = 0.707;
    double gain_db_ = 0;

    // Coefficients of the numerator and denominator of the transfer function
    double a0_, a1_, a2_, b0_, b1_, b2_;
    // State variables (delay line)
    double x1_ = 0, x2_ = 0, y1_ = 0, y2_ = 0;
};

// Constructor
BiquadFilter::BiquadFilter(std::string filter_type, double sample_rate, double center_frequency, double q_factor, double gain_db)
{
    set_params(filter_type, sample_rate, center_frequency, q_factor, gain_db);
}

// Destructor
BiquadFilter::~BiquadFilter()
{
}

// Function to set up filter coefficients depending on filter type
void BiquadFilter::set_params(std::string filter_type, double sample_rate, double center_frequency, double q_factor, double gain_db)
{
    // Set filter parameters
    filter_type_ = filter_type;
    sample_rate_ = sample_rate;
    center_frequency_ = center_frequency;
    q_factor_ = q_factor;
    gain_db_ = gain_db;

    // Calculate linear gain from gain in decibels
    double gain = std::pow(10, gain_db / 20.0);

    // Variables for filter coefficients
    double a0, a1, a2, b0, b1, b2;
    // Normalized angular frequency in radians
    double w0 = 2 * M_PI * center_frequency / sample_rate;
    // Alpha value, a parameter for computing filter coefficients
    double alpha = std::sin(w0) / (2 * q_factor);
    // Cosine of the normalized angular frequency
    double cos_w0 = std::cos(w0);

    // The following formulas for b0, b1, b2, a0, a1, and a2 are derived from the
    // transfer function of a constant 0 dB peak gain bandpass filter
    // The bilinear transformation is a technique used to convert continuous-time (analog) filters to discrete-time (digital) filters.
    // It is done by substituting the Laplace transform variable s with a function of the discrete-time Fourier transform variable z.
    // The bilinear transform is defined as:
    // s = (2 * (z - 1)) / (T * (z + 1))
    // where T is the sampling period.
    if (filter_type == "lowpass")
    {
        // Analog transfer function H(s) = 1 / (s^2 + s/Q + 1)
        // Applying the bilinear transformation and solving for H(z), we get:
        // H(z) = (b0 + b1 * z^(-1) + b2 * z^(-2)) / (a0 + a1 * z^(-1) + a2 * z^(-2))
        double K = std::tan(M_PI * center_frequency / sample_rate);
        double norm = 1 / (1 + K / q_factor + K * K);
        b0 = K * K * norm;
        b1 = 2 * b0;
        b2 = b0;
        a0 = 1;
        a1 = 2 * (K * K - 1) * norm;
        a2 = (1 - K / q_factor + K * K) * norm;
    }
    else if (filter_type == "highpass")
    {
        // Analog transfer function H(s) = s^2 / (s^2 + s/Q + 1)
        // Applying the bilinear transformation and solving for H(z), we get:
        // H(z) = (b0 + b1 * z^(-1) + b2 * z^(-2)) / (a0 + a1 * z^(-1) + a2 * z^(-2))
        double K = std::tan(M_PI * center_frequency / sample_rate);
        double norm = 1 / (1 + K / q_factor + K * K);
        b0 = norm;
        b1 = -2 * b0;
        b2 = b0;
        a0 = 1;
        a1 = 2 * (K * K - 1) * norm;
        a2 = (1 - K / q_factor + K * K) * norm;
    }
    else if (filter_type == "notch")
    {
        // H(s) = (s^2 + 1) / (s^2 + s/Q + 1)
        // Applying the bilinear transformation and solving for H(z), we get:
        // H(z) = (b0 + b1 * z^(-1) + b2 * z^(-2)) / (a0 + a1 * z^(-1) + a2 * z^(-2))
        b0 = 1;
        b1 = -2 * cos_w0;
        b2 = 1;
        a0 = 1 + alpha;
        a1 = -2 * cos_w0;
        a2 = 1 - alpha;
    }
    else if (filter_type == "peaking")
    {
        double a = std::sqrt(gain);
        double alpha_a = alpha * a;
        double alpha_d = alpha / a;

        b0 = 1 + alpha_a;
        b1 = -2 * cos_w0;
        b2 = 1 - alpha_a;
        a0 = 1 + alpha_d;
        a1 = -2 * cos_w0;
        a2 = 1 - alpha_d;
    }

    // Normalize filter coefficients
    double norm = 1 / a0;
    a0_ = 1;
    a1_ = a1 * norm;
    a2_ = a2 * norm;
    b0_ = b0 * norm;
    b1_ = b1 * norm;
    b2_ = b2 * norm;
}

// Process function for a single sample
short BiquadFilter::process(short sample)
{
    // Convert input sample to a double
    double in = sample;

    // Calculate filter output
    // difference equation y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2],
    // where y[n] is the output sample, x[n] is the input sample, and a and b are the filter coefficients.
    double out = b0_ * in + b1_ * x1_ + b2_ * x2_ - a1_ * y1_ - a2_ * y2_;

    // Update the delay line (state variables) by shifting the previous
    // input and output samples to the right and storing the current input and output samples
    x2_ = x1_;
    x1_ = in;
    y2_ = y1_;
    y1_ = out;

    // Convert the output to a short and return it
    return static_cast<short>(out);
}

#endif // BIQUAD_FILTER_H