// custom_websocket_server.h

#ifndef CUSTOM_WEBSOCKET_SERVER_H
#define CUSTOM_WEBSOCKET_SERVER_H

#include <iostream>
#include <stdexcept>
#include <memory>
#include <ixwebsocket/IXWebSocketServer.h>
#include "json.hpp"
#include "event_manager.h"
#include "type_aliases.h"

using json = nlohmann::json;

class CustomWebSocketServer
{
public:
    // Constructor
    explicit CustomWebSocketServer(int port);

private:
    // Server
    ix::WebSocketServer _server;
    // On message received function
    void onMessageReceived(std::shared_ptr<ix::WebSocket> webSocket, const std::string &message);
    // Broadcast to all clients message function
    void broadcastMessage(const std::string &message);
    // Response command functions
    void broadcastFailedResponse(const std::string &error_type, const std::string &error_message);
    void broadcastGainResponse(const std::string &command_type, const std::string &channel_type, unsigned int channel_number, double gain_db);
    void broadcastMuteResponse(const std::string &command_type, const std::string &channel_type, unsigned int channel_number, bool mute);
    void broadcastMixerResponse(const std::string &command_type, unsigned int input_channel, unsigned int output_channel, bool route);
    void broadcastFilterResponse(const std::string &command_type, const std::string &channel_type, unsigned int channel_number, unsigned int filter_id,
                                 bool filter_enabled, std::string filter_type, double center_frequency, double q_factor, double gain_db);
    void broadcastSignalAmplitudes(const std::string &command_type, const std::string &channel_type, const std::vector<double> &amplitudes_db);
};

CustomWebSocketServer::CustomWebSocketServer(int port)
    : _server(port, "0.0.0.0")
{

    _server.setOnConnectionCallback(
        [this](std::weak_ptr<ix::WebSocket> webSocketWeak, std::shared_ptr<ix::ConnectionState> connectionState)
        {
            auto webSocket = webSocketWeak.lock();
            if (!webSocket)
            {
                return;
            }
            webSocket->setOnMessageCallback(
                [this, webSocket](const ix::WebSocketMessagePtr &msg)
                {
                    if (msg->type == ix::WebSocketMessageType::Message)
                    {
                        onMessageReceived(webSocket, msg->str);
                    }
                });
        });

    auto res = _server.listen();
    if (!res.first)
    {
        // Error handling
        // return 1;
    }

    // Per message deflate connection is enabled by default. It can be disabled
    // which might be helpful when running on low power devices such as a Rasbery Pi
    _server.disablePerMessageDeflate();

    // Run the server in the background. Server can be stoped by calling server.stop()
    _server.start();

    // Block until server.stop() is called.
    //_server.wait();
}

void CustomWebSocketServer::onMessageReceived(std::shared_ptr<ix::WebSocket> webSocket, const std::string &message)
{
    // std::cout << "New command message received: " << message << std::endl;

    try
    {
        auto commandJson = json::parse(message);

        if (commandJson.find("command_type") != commandJson.end())
        {
            std::string command_type = commandJson["command_type"];

            if (command_type == "set_gain")
            {
                EventManager::getInstance().emitEvent<const std::string &, unsigned int, double, SetGainCallbackType>(
                    commandJson.at("command_type").get<std::string>(), commandJson.at("channel_type").get<std::string>(),
                    commandJson.at("channel_number").get<unsigned int>(), commandJson.at("gain_db").get<double>(),
                    [this](const std::string &command_type, const std::string &channel_type, unsigned int channel_number, double gain_db)
                    { this->broadcastGainResponse(command_type, channel_type, channel_number, gain_db); });
                return;
            }
            else if (command_type == "set_mute")
            {
                EventManager::getInstance().emitEvent<const std::string &, unsigned int, bool, SetMuteCallbackType>(
                    commandJson.at("command_type").get<std::string>(), commandJson.at("channel_type").get<std::string>(),
                    commandJson.at("channel_number").get<unsigned int>(), commandJson.at("mute").get<bool>(),
                    [this](const std::string &command_type, const std::string &channel_type, unsigned int channel_number, bool mute)
                    { this->broadcastMuteResponse(command_type, channel_type, channel_number, mute); });
                return;
            }
            else if (command_type == "set_mixer")
            {
                EventManager::getInstance().emitEvent<unsigned int, unsigned int, bool, SetMixerCallbackType>(
                    commandJson.at("command_type").get<std::string>(), commandJson.at("input_channel").get<unsigned int>(),
                    commandJson.at("output_channel").get<unsigned int>(), commandJson.at("mix").get<bool>(),
                    [this](const std::string &command_type, unsigned int input_channel, unsigned int output_channel, bool route)
                    { this->broadcastMixerResponse(command_type, input_channel, output_channel, route); });
                return;
            }
            else if (command_type == "set_filter")
            {
                EventManager::getInstance().emitEvent<const std::string &, unsigned int, unsigned int, bool, std::string, double, double, double, SetFilterCallbackType>(
                    commandJson.at("command_type").get<std::string>(), commandJson.at("channel_type").get<std::string>(),
                    commandJson.at("channel_number").get<unsigned int>(), commandJson.at("filter_id").get<unsigned int>(),
                    commandJson.at("filter_enabled").get<bool>(), commandJson.at("filter_type").get<std::string>(),
                    commandJson.at("center_frequency").get<double>(), commandJson.at("q_factor").get<double>(),
                    commandJson.at("gain_db").get<double>(),
                    [this](const std::string &command_type, const std::string &channel_type, unsigned int channel_number, unsigned int filter_id,
                           bool filter_enabled, std::string filter_type, double center_frequency, double q_factor, double gain_db)
                    { this->broadcastFilterResponse(command_type, channel_type, channel_number, filter_id, filter_enabled, filter_type, center_frequency, q_factor, gain_db); });
                return;
            }
            else if (command_type == "get_gain")
            {
                EventManager::getInstance().emitEvent<const std::string &, unsigned int, SetGainCallbackType>(
                    commandJson.at("command_type").get<std::string>(), commandJson.at("channel_type").get<std::string>(),
                    commandJson.at("channel_number").get<unsigned int>(),
                    [this](const std::string &command_type, const std::string &channel_type, unsigned int channel_number, double gain_db)
                    { this->broadcastGainResponse(command_type, channel_type, channel_number, gain_db); });
                return;
            }
            else if (command_type == "get_mute")
            {
                EventManager::getInstance().emitEvent<const std::string &, unsigned int, SetMuteCallbackType>(
                    commandJson.at("command_type").get<std::string>(), commandJson.at("channel_type").get<std::string>(),
                    commandJson.at("channel_number").get<unsigned int>(),
                    [this](const std::string &command_type, const std::string &channel_type, unsigned int channel_number, bool mute)
                    { this->broadcastMuteResponse(command_type, channel_type, channel_number, mute); });
                return;
            }
            else if (command_type == "get_mixer")
            {
                EventManager::getInstance().emitEvent<unsigned int, unsigned int, SetMixerCallbackType>(
                    commandJson.at("command_type").get<std::string>(), commandJson.at("input_channel").get<unsigned int>(),
                    commandJson.at("output_channel").get<unsigned int>(),
                    [this](const std::string &command_type, unsigned int input_channel, unsigned int output_channel, bool mix)
                    { this->broadcastMixerResponse(command_type, input_channel, output_channel, mix); });
                return;
            }
            else if (command_type == "get_filter")
            {
                EventManager::getInstance().emitEvent<const std::string &, unsigned int, unsigned int, SetFilterCallbackType>(
                    commandJson.at("command_type").get<std::string>(), commandJson.at("channel_type").get<std::string>(),
                    commandJson.at("channel_number").get<unsigned int>(), commandJson.at("filter_id").get<unsigned int>(),
                    [this](const std::string &command_type, const std::string &channel_type, unsigned int channel_number, unsigned int filter_id,
                           bool filter_enabled, std::string filter_type, double center_frequency, double q_factor, double gain_db)
                    { this->broadcastFilterResponse(command_type, channel_type, channel_number, filter_id, filter_enabled, filter_type, center_frequency, q_factor, gain_db); });
                return;
            }
            else if (command_type == "get_meter")
            {
                EventManager::getInstance().emitEvent<const std::string &, GetMeterCallbackType>(
                    commandJson.at("command_type").get<std::string>(), commandJson.at("channel_type").get<std::string>(),
                    [this](const std::string &command_type, const std::string &channel_type, const std::vector<double> &amplitudes_db)
                    { this->broadcastSignalAmplitudes(command_type, channel_type, amplitudes_db); });

                return;
            }
            else
            {
                broadcastFailedResponse(std::string("unknown_command"), std::string("fail"));
                std::cerr << "Error: Unknown command_type '" << command_type << "'" << std::endl;
                return;
            }
        }
        else
        {
            broadcastFailedResponse(std::string("missing_command"), std::string("fail"));
            std::cerr << "Error: Invalid message format, missing 'command_type' field" << std::endl;
            return;
        }
    }
    catch (const json::exception &e)
    {
        broadcastFailedResponse(std::string("parse_error"), std::string(e.what()));
        std::cerr << "Error: Failed to parse JSON message:  " << e.what() << std::endl;
    }
}

void CustomWebSocketServer::broadcastMessage(const std::string &message)
{
    // Get a list of all connected clients
    auto clients = _server.getClients();

    // Iterate over each client and send the message
    for (const auto &client : clients)
    {
        client->send(message);
    }
}

void CustomWebSocketServer::broadcastFailedResponse(const std::string &error_type, const std::string &error_message)
{
    json responseJson;
    responseJson["error_type"] = error_type;
    responseJson["error_message"] = error_message;
    broadcastMessage(responseJson.dump());
}

// Response command functions
void CustomWebSocketServer::broadcastGainResponse(const std::string &command_type, const std::string &channel_type, unsigned int channel_number, double gain_db)
{
    json responseJson;
    responseJson["command_type"] = command_type;
    responseJson["channel_type"] = channel_type;
    responseJson["channel_number"] = channel_number;
    responseJson["gain_db"] = gain_db;
    broadcastMessage(responseJson.dump());
}

void CustomWebSocketServer::broadcastMuteResponse(const std::string &command_type, const std::string &channel_type, unsigned int channel_number, bool mute)
{
    json responseJson;
    responseJson["command_type"] = command_type;
    responseJson["channel_type"] = channel_type;
    responseJson["channel_number"] = channel_number;
    responseJson["mute"] = mute;
    broadcastMessage(responseJson.dump());
}

void CustomWebSocketServer::broadcastMixerResponse(const std::string &command_type, unsigned int input_channel, unsigned int output_channel, bool route)
{
    json responseJson;
    responseJson["command_type"] = command_type;
    responseJson["input_channel"] = input_channel;
    responseJson["output_channel"] = output_channel;
    responseJson["mix"] = route;
    broadcastMessage(responseJson.dump());
}

void CustomWebSocketServer::broadcastFilterResponse(const std::string &command_type, const std::string &channel_type, unsigned int channel_number, unsigned int filter_id,
                                                    bool filter_enabled, std::string filter_type, double center_frequency, double q_factor, double gain_db)
{
    json responseJson;
    responseJson["command_type"] = command_type;
    responseJson["channel_type"] = channel_type;
    responseJson["channel_number"] = channel_number;
    responseJson["filter_id"] = filter_id;
    responseJson["filter_enabled"] = filter_enabled;
    responseJson["filter_type"] = filter_type;
    responseJson["center_frequency"] = center_frequency;
    responseJson["q_factor"] = q_factor;
    responseJson["gain_db"] = gain_db;
    broadcastMessage(responseJson.dump());
}

void CustomWebSocketServer::broadcastSignalAmplitudes(const std::string &command_type, const std::string &channel_type, const std::vector<double> &amplitudes_db)
{
    json responseJson;
    responseJson["command_type"] = command_type;
    responseJson["channel_type"] = channel_type;
    responseJson["amplitudes_db"] = amplitudes_db;
    broadcastMessage(responseJson.dump());
}

#endif // CUSTOM_WEBSOCKET_SERVER_H