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
    enum Type
    {
        Lowpass,
        Highpass,
        Notch,
        Peaking,
        Invalid
    };
    // Enable filter functions
    bool isEnabled() const { return filterEnabled; }
    void setEnabled(bool enable) { filterEnabled = enable; }
    // Constructor
    explicit BiquadFilter(Type type, double sample_rate, double frequency, double q, double gain_db = 0.0);
    // Destructor
    ~BiquadFilter();
    // set_params function
    void set_params(Type type, double sample_rate, double frequency, double q, double gain_db = 0.0);
    // Process function
    short process(short sample);

private:
    // Enable filter variable
    bool filterEnabled = true;
    // Coefficients of the numerator and denominator of the transfer function
    double a0_, a1_, a2_, b0_, b1_, b2_;
    // State variables (delay line)
    double x1_ = 0, x2_ = 0, y1_ = 0, y2_ = 0;
};

// Constructor
BiquadFilter::BiquadFilter(Type type, double sample_rate, double frequency, double q, double gain_db)
{
    set_params(type, sample_rate, frequency, q, gain_db);
}

// Destructor
BiquadFilter::~BiquadFilter()
{
}

// Function to set up filter coefficients depending on filter type
void BiquadFilter::set_params(Type type, double sample_rate, double frequency, double q, double gain_db)
{
    // Calculate linear gain from gain in decibels
    double gain = std::pow(10, gain_db / 20.0);

    // Variables for filter coefficients
    double a0, a1, a2, b0, b1, b2;
    // Normalized angular frequency in radians
    double w0 = 2 * M_PI * frequency / sample_rate;
    // Alpha value, a parameter for computing filter coefficients
    double alpha = std::sin(w0) / (2 * q);
    // Cosine of the normalized angular frequency
    double cos_w0 = std::cos(w0);

    // The following formulas for b0, b1, b2, a0, a1, and a2 are derived from the
    // transfer function of a constant 0 dB peak gain bandpass filter
    // The bilinear transformation is a technique used to convert continuous-time (analog) filters to discrete-time (digital) filters.
    // It is done by substituting the Laplace transform variable s with a function of the discrete-time Fourier transform variable z.
    // The bilinear transform is defined as:
    // s = (2 * (z - 1)) / (T * (z + 1))
    // where T is the sampling period.
    switch (type)
    {
    case Lowpass:
    {
        // Analog transfer function H(s) = 1 / (s^2 + s/Q + 1)
        // Applying the bilinear transformation and solving for H(z), we get:
        // H(z) = (b0 + b1 * z^(-1) + b2 * z^(-2)) / (a0 + a1 * z^(-1) + a2 * z^(-2))
        double K = std::tan(M_PI * frequency / sample_rate);
        double norm = 1 / (1 + K / q + K * K);
        b0 = K * K * norm;
        b1 = 2 * b0;
        b2 = b0;
        a0 = 1;
        a1 = 2 * (K * K - 1) * norm;
        a2 = (1 - K / q + K * K) * norm;
        break;
    }
    case Highpass:
    {
        // Analog transfer function H(s) = s^2 / (s^2 + s/Q + 1)
        // Applying the bilinear transformation and solving for H(z), we get:
        // H(z) = (b0 + b1 * z^(-1) + b2 * z^(-2)) / (a0 + a1 * z^(-1) + a2 * z^(-2))
        double K = std::tan(M_PI * frequency / sample_rate);
        double norm = 1 / (1 + K / q + K * K);
        b0 = norm;
        b1 = -2 * b0;
        b2 = b0;
        a0 = 1;
        a1 = 2 * (K * K - 1) * norm;
        a2 = (1 - K / q + K * K) * norm;
        break;
    }
    case Notch:
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
        break;
    }
    case Peaking:
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
        break;
    }
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