// main.cpp

#include <iostream>
#include <sstream>
#include "audio_processor.h"
#include "custom_websocket_server.h"
#include "event_manager.h"

// Function to parse an unsigned integer argument from a command line argument string
bool parse_uint_arg(const char *arg, const std::string &flag, unsigned int &value)
{
    std::string arg_str(arg);
    // Check if the argument string contains the specified flag
    if (arg_str.find(flag) == 0)
    {
        // Extract the integer value after the flag
        std::istringstream iss(arg_str.substr(flag.length()));
        iss >> value;
        return true;
    }
    return false;
}

// Function to parse a string argument from a command line argument string
bool parse_string_arg(const char *arg, const std::string &flag, std::string &value)
{
    std::string arg_str(arg);
    // Check if the argument string contains the specified flag
    if (arg_str.find(flag) == 0)
    {
        // Extract the string value after the flag
        value = arg_str.substr(flag.length());
        return true;
    }
    return false;
}

int main(int argc, char *argv[])
{
    // Check if the correct number of arguments was provided
    if (argc != 6)
    {
        std::cerr << "Usage: sudo " << argv[0] << " -interface:<audio_interface_name> -inputs:<input_number> -outputs:<output_number> -rate:<sample_rate> -port:<server_port>" << std::endl;
        return 1;
    }

    // Declare variables to store command line argument values
    std::string audio_interface;
    unsigned int input_channels, output_channels, rate, port;

    // Parse command line arguments and store values in variables
    for (int i = 1; i < argc; ++i)
    {
        if (!parse_string_arg(argv[i], "-interface:", audio_interface) &&
            !parse_uint_arg(argv[i], "-inputs:", input_channels) &&
            !parse_uint_arg(argv[i], "-outputs:", output_channels) &&
            !parse_uint_arg(argv[i], "-rate:", rate) &&
            !parse_uint_arg(argv[i], "-port:", port))
        {
            // If an invalid option was provided, display usage instructions and exit
            std::cerr << "Invalid option: " << argv[i] << std::endl;
            std::cerr << "Usage: sudo " << argv[0] << " -interface:<audio_interface_name> -inputs:<input_number> -outputs:<output_number> -rate:<sample_rate> -port:<server_port>" << std::endl;
            return 1;
        }
    }

    // Convert audio_interface string to const char* for use with ALSA
    const char *audio_interface_cstr = audio_interface.c_str();

    // Restore audio configuration from asound.state file.
    // Ensures that the audio interface restarts with the correct configuration after a reboot or disconnection
    // This file needs to be saved after the first time the audio interface is configured
    system("alsactl --file ~/.config/asound.state restore");

    // Create EventManager object
    EventManager event_manager;

    // Create AudioProcessor object with the specified audio interface, input and output channels, and sample rate
    AudioProcessor audioProcessor(audio_interface_cstr, input_channels, output_channels, rate, event_manager);

    // Create CustomWebSocketServer object with the specified port and audioProcessor object
    CustomWebSocketServer webSocketServer(port, event_manager);

    // Start audio processing
    audioProcessor.start();

    return 0;
}
