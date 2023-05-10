#ifndef TYPE_ALIASES_H
#define TYPE_ALIASES_H

#include <string>
#include <functional>
#include <vector>

using SetGainCallbackType = std::function<void(const std::string &, const std::string &, unsigned int, double)>;
using SetMuteCallbackType = std::function<void(const std::string &, const std::string &, unsigned int, bool)>;
using SetMixerCallbackType = std::function<void(const std::string &, unsigned int, unsigned int, bool)>;
using SetFilterCallbackType = std::function<void(const std::string &, const std::string &, unsigned int, unsigned int, bool, std::string, double, double, double)>;
using GetMeterCallbackType = std::function<void(const std::string &, const std::string &, const std::vector<double> &)>;

#endif // TYPE_ALIASES_H
