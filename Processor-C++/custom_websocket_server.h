// custom_websocket_server.h

#ifndef CUSTOM_WEBSOCKET_SERVER_H
#define CUSTOM_WEBSOCKET_SERVER_H

#include <iostream>
#include <stdexcept>
#include <memory>
#include <ixwebsocket/IXWebSocketServer.h>
#include "json.hpp"
#include "submodules/Event-Manager-Cpp/event_manager.h"

using json = nlohmann::json;

class CustomWebSocketServer
{
public:
    // Constructor
    explicit CustomWebSocketServer(int port, EventManager &event_manager);

private:
    // Server
    ix::WebSocketServer _server;
    // Event manager
    EventManager &event_manager;
    // On message received function
    void onMessageReceived(std::shared_ptr<ix::WebSocket> webSocket, const std::string &message);
    // Broadcast to all clients message function
    void broadcastMessage(const std::string &message);
    // Response command function
    void broadcastCommandResponse(const std::string &responseTo, const std::string &commandStatus);
    // Broadcast volume function
    void broadcastSignalAmplitudes(const std::string &commandStatus, const std::string &channelType, const std::vector<int> &volumes);
};

CustomWebSocketServer::CustomWebSocketServer(int port, EventManager &event_manager)
    : _server(port, "0.0.0.0"), event_manager(event_manager)
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

// Response command function
void CustomWebSocketServer::broadcastCommandResponse(const std::string &responseTo, const std::string &commandStatus)
{
    json responseJson;
    responseJson["response_to"] = responseTo;
    responseJson["command_status"] = commandStatus;
    broadcastMessage(responseJson.dump());
}

// Broadcast volume function
void CustomWebSocketServer::broadcastSignalAmplitudes(const std::string &commandStatus, const std::string &channelType, const std::vector<int> &volumes)
{
    json responseJson;
    responseJson["response_to"] = "get_signal_amplitudes";
    responseJson["command_status"] = commandStatus;
    responseJson["channel_type"] = channelType;
    responseJson["volumes"] = volumes;
    broadcastMessage(responseJson.dump());
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

            if (command_type == "set_volume")
            {
                event_manager.emitEvent<const std::string &, unsigned int, int, std::function<void(const std::string &, const std::string &)>>(
                    commandJson.at("command_type").get<std::string>(), commandJson.at("channel_type").get<std::string>(),
                    commandJson.at("channel_number").get<unsigned int>(), commandJson.at("volume_db").get<int>(),
                    [this](const std::string &responseTo, const std::string &commandStatus)
                    { this->broadcastCommandResponse(responseTo, commandStatus); });
                return;
            }
            else if (command_type == "set_mute")
            {
                event_manager.emitEvent<const std::string &, unsigned int, bool, std::function<void(const std::string &, const std::string &)>>(
                    commandJson.at("command_type").get<std::string>(), commandJson.at("channel_type").get<std::string>(),
                    commandJson.at("channel_number").get<unsigned int>(), commandJson.at("mute").get<bool>(),
                    [this](const std::string &responseTo, const std::string &commandStatus)
                    { this->broadcastCommandResponse(responseTo, commandStatus); });
                return;
            }
            else if (command_type == "set_routing")
            {
                event_manager.emitEvent<unsigned int, unsigned int, bool, std::function<void(const std::string &, const std::string &)>>(
                    commandJson.at("command_type").get<std::string>(), commandJson.at("input_channel").get<unsigned int>(),
                    commandJson.at("output_channel").get<unsigned int>(), commandJson.at("route").get<bool>(),
                    [this](const std::string &responseTo, const std::string &commandStatus)
                    { this->broadcastCommandResponse(responseTo, commandStatus); });
                return;
            }
            else if (command_type == "set_filter")
            {
                event_manager.emitEvent<const std::string &, unsigned int, unsigned int, bool, std::string, double, double, double,
                                        std::function<void(const std::string &, const std::string &)>>(
                    commandJson.at("command_type").get<std::string>(), commandJson.at("channel_type").get<std::string>(),
                    commandJson.at("channel_number").get<unsigned int>(), commandJson.at("filter_id").get<unsigned int>(),
                    commandJson.at("filter_enabled").get<bool>(), commandJson.at("filter_type").get<std::string>(),
                    commandJson.at("center_frequency").get<double>(), commandJson.at("q_factor").get<double>(), commandJson.at("gain_db").get<double>(),
                    [this](const std::string &responseTo, const std::string &commandStatus)
                    { this->broadcastCommandResponse(responseTo, commandStatus); });
                return;
            }
            else if (command_type == "get_signal_amplitudes")
            {
                event_manager.emitEvent<const std::string &, std::function<void(const std::string &, const std::string &, const std::vector<int> &)>>(
                    commandJson.at("command_type").get<std::string>(), commandJson.at("channel_type").get<std::string>(),
                    [this](const std::string &commandStatus, const std::string &channelType, const std::vector<int> &volumes)
                    { this->broadcastSignalAmplitudes(commandStatus, channelType, volumes); });

                return;
            }
            else
            {
                broadcastCommandResponse(std::string("unknown_command"), std::string("fail"));
                std::cerr << "Error: Unknown command_type '" << command_type << "'" << std::endl;
                return;
            }
        }
        else
        {
            broadcastCommandResponse(std::string("missing_command"), std::string("fail"));
            std::cerr << "Error: Invalid message format, missing 'command_type' field" << std::endl;
            return;
        }
    }
    catch (const json::exception &e)
    {
        broadcastCommandResponse(std::string("parse_error"), std::string(e.what()));
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

#endif // CUSTOM_WEBSOCKET_SERVER_H