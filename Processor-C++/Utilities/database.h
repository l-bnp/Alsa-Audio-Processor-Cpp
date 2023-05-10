// database.h
#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <mysqlx/xdevapi.h>
#include <vector>
#include "event_manager.h"
#include "type_aliases.h"

class Database
{
public:
    Database(const std::string &host, int port, const std::string &user, const std::string &password, const std::string &schema);

private:
    mysqlx::Session session;
    mysqlx::Schema schema;
    std::string tableName = "audio_parameters";
    void setGain(
        const std::string &channel_type, unsigned int channel_number, double volume_db,
        SetGainCallbackType callback = [](const std::string &, const std::string &, unsigned int, double) {});
    void setMute(
        const std::string &channel_type, unsigned int channel_number, bool mute,
        SetMuteCallbackType callback = [](const std::string &, const std::string &, unsigned int, bool) {});
    void setMixer(
        unsigned int input_channel_number, unsigned int output_channel_number, bool route,
        SetMixerCallbackType callback = [](const std::string &, unsigned int, unsigned int, bool) {});
    void setFilter(
        const std::string &channel_type, unsigned int channel_number, unsigned int filter_id, bool isEnabled,
        std::string filter_type_str, double center_frequency, double q_factor, double gain_db,
        SetFilterCallbackType callback = [](const std::string &, const std::string &, unsigned int, unsigned int, bool, std::string, double, double, double) {});
    void getGain(const std::string &channel_type, unsigned int channel_number, SetGainCallbackType callback);
    void getMute(const std::string &channel_type, unsigned int channel_number, SetMuteCallbackType callback);
    void getMixer(unsigned int input_channel_number, unsigned int output_channel_number, SetMixerCallbackType callback);
    void getFilter(const std::string &channel_type, unsigned int channel_number, unsigned int filter_id, SetFilterCallbackType callback);
};

Database::Database(const std::string &host, int port, const std::string &user, const std::string &password, const std::string &schemaName)
    : session(host, port, user, password),
      schema(session.getSchema(schemaName))
{
    // Create table if it doesn't exist
    session.sql("CREATE TABLE IF NOT EXISTS " + schemaName + "." + tableName + " ("
                                                                               "parameter_name VARCHAR(50) PRIMARY KEY,"
                                                                               "parameter_int_value INT DEFAULT 0,"
                                                                               "parameter_double_value DOUBLE DEFAULT 0,"
                                                                               "parameter_str_value VARCHAR(50) DEFAULT '')")
        .execute();

    EventManager::getInstance().on<std::string, unsigned int, SetGainCallbackType>(
        "get_database_gain", [this](const std::string &channel_type, unsigned int channel_number, SetGainCallbackType callback)
        { this->getGain(channel_type, channel_number, callback); });

    EventManager::getInstance().on<std::string, unsigned int, SetMuteCallbackType>(
        "get_database_mute", [this](const std::string &channel_type, unsigned int channel_number, SetMuteCallbackType callback)
        { this->getMute(channel_type, channel_number, callback); });

    EventManager::getInstance().on<unsigned int, unsigned int, SetMixerCallbackType>(
        "get_database_mixer", [this](unsigned int input_channel_number, unsigned int output_channel_number, SetMixerCallbackType callback)
        { this->getMixer(input_channel_number, output_channel_number, callback); });
    EventManager::getInstance().on<std::string, unsigned int, unsigned int, SetFilterCallbackType>(
        "get_database_filter", [this](const std::string &channel_type, unsigned int channel_number, unsigned int filter_id, SetFilterCallbackType callback)
        { this->getFilter(channel_type, channel_number, filter_id, callback); });

    EventManager::getInstance().on<const std::string &, unsigned int, double, SetGainCallbackType>(
        "set_gain", [this](const std::string &channel_type, unsigned int channel_number, double volume_db, SetGainCallbackType callback)
        { this->setGain(channel_type, channel_number, volume_db); });

    EventManager::getInstance().on<const std::string &, unsigned int, bool, SetMuteCallbackType>(
        "set_mute", [this](const std::string &channel_type, unsigned int channel_number, bool mute, SetMuteCallbackType callback)
        { this->setMute(channel_type, channel_number, mute); });

    EventManager::getInstance().on<unsigned int, unsigned int, bool, SetMixerCallbackType>(
        "set_mixer", [this](unsigned int input_channel_number, unsigned int output_channel_number, bool route, SetMixerCallbackType callback)
        { this->setMixer(input_channel_number, output_channel_number, route); });

    EventManager::getInstance().on<const std::string &, unsigned int, unsigned int, bool, std::string, double, double, double, SetFilterCallbackType>(
        "set_filter", [this](const std::string &channel_type, unsigned int channel_number, unsigned int filter_id, bool isEnabled,
                             std::string filter_type_str, double center_frequency, double q_factor, double gain_db, SetFilterCallbackType callback)
        { this->setFilter(channel_type, channel_number, filter_id, isEnabled, filter_type_str, center_frequency, q_factor, gain_db); });
}

void Database::setGain(
    const std::string &channel_type, unsigned int channel_number, double volume_db,
    SetGainCallbackType callback)
{
    std::string parameter_name = channel_type + "_volume_" + std::to_string(channel_number);
    mysqlx::Table table = schema.getTable(tableName);
    table.remove().where("parameter_name = :name").bind("name", parameter_name).execute();

    table.insert("parameter_name", "parameter_double_value").values(parameter_name, volume_db).execute();
}

void Database::getGain(const std::string &channel_type, unsigned int channel_number, SetGainCallbackType callback)
{
    std::string parameter_name = channel_type + "_volume_" + std::to_string(channel_number);
    mysqlx::Table table = schema.getTable(tableName);
    mysqlx::RowResult result = table.select("parameter_double_value").where("parameter_name = :name").bind("name", parameter_name).execute();

    double volume_db = -60.0;

    std::string command_type = "get_gain_failed";

    if (mysqlx::Row row = result.fetchOne())
    {
        command_type = "notify_gain";
        volume_db = static_cast<double>(row[0]);
    }

    callback(command_type, channel_type, channel_number, volume_db);
}

void Database::setMute(
    const std::string &channel_type, unsigned int channel_number, bool mute,
    SetMuteCallbackType callback)
{
    std::string parameter_name = channel_type + "_mute_" + std::to_string(channel_number);
    mysqlx::Table table = schema.getTable(tableName);
    table.remove().where("parameter_name = :name").bind("name", parameter_name).execute();
    table.insert("parameter_name", "parameter_int_value").values(parameter_name, mute ? 1.0f : 0.0f).execute();
}

void Database::getMute(const std::string &channel_type, unsigned int channel_number, SetMuteCallbackType callback)
{
    std::string parameter_name = channel_type + "_mute_" + std::to_string(channel_number);
    mysqlx::Table table = schema.getTable(tableName);
    mysqlx::RowResult result = table.select("parameter_int_value").where("parameter_name = :name").bind("name", parameter_name).execute();

    // Default value if it's not found in the database
    bool mute = true;

    std::string command_type = "get_mute_failed";

    if (mysqlx::Row row = result.fetchOne())
    {
        command_type = "notify_mute";
        mute = static_cast<bool>(row[0]);
    }

    callback(command_type, channel_type, channel_number, mute);
}

void Database::setMixer(
    unsigned int input_channel_number, unsigned int output_channel_number, bool route,
    SetMixerCallbackType callback)
{
    std::string parameter_name = "routing_" + std::to_string(input_channel_number) + "_" + std::to_string(output_channel_number);
    mysqlx::Table table = schema.getTable(tableName);
    table.remove().where("parameter_name = :name").bind("name", parameter_name).execute();
    table.insert("parameter_name", "parameter_int_value").values(parameter_name, route ? 1.0f : 0.0f).execute();
}

void Database::getMixer(unsigned int input_channel_number, unsigned int output_channel_number, SetMixerCallbackType callback)
{
    std::string parameter_name = "routing_" + std::to_string(input_channel_number) + "_" + std::to_string(output_channel_number);
    mysqlx::Table table = schema.getTable(tableName);
    mysqlx::RowResult result = table.select("parameter_int_value").where("parameter_name = :name").bind("name", parameter_name).execute();

    // Default value
    bool route = false;

    std::string command_type = "get_mixer_failed";

    if (mysqlx::Row row = result.fetchOne())
    {
        command_type = "notify_mixer";
        route = static_cast<bool>(row[0]);
    }

    callback(command_type, input_channel_number, output_channel_number, route);
}

void Database::setFilter(
    const std::string &channel_type, unsigned int channel_number, unsigned int filter_id, bool isEnabled,
    std::string filter_type_str, double center_frequency, double q_factor, double gain_db,
    SetFilterCallbackType callback)
{
    std::string parameter_prefix = channel_type + "_filter_" + std::to_string(channel_number) + "_" + std::to_string(filter_id) + "_";
    mysqlx::Table table = schema.getTable(tableName);

    // Helper function to update a single filter parameter
    auto updateFilterParameterInt = [&](const std::string &name, int value)
    {
        std::string parameter_name = parameter_prefix + name;
        table.remove().where("parameter_name = :name").bind("name", parameter_name).execute();
        table.insert("parameter_name", "parameter_int_value").values(parameter_name, value).execute();
    };

    // Helper function to update a single filter parameter
    auto updateFilterParameterDouble = [&](const std::string &name, double value)
    {
        std::string parameter_name = parameter_prefix + name;
        table.remove().where("parameter_name = :name").bind("name", parameter_name).execute();
        table.insert("parameter_name", "parameter_double_value").values(parameter_name, value).execute();
    };

    // Helper function to update a single filter parameter (string)
    auto updateFilterParameterStr = [&](const std::string &name, const std::string &value)
    {
        std::string parameter_name = parameter_prefix + name;
        table.remove().where("parameter_name = :name").bind("name", parameter_name).execute();
        table.insert("parameter_name", "parameter_str_value").values(parameter_name, value).execute();
    };

    updateFilterParameterInt("enabled", isEnabled ? 1.0 : 0.0);
    updateFilterParameterDouble("center_frequency", center_frequency);
    updateFilterParameterDouble("q_factor", q_factor);
    updateFilterParameterDouble("gain_db", gain_db);
    updateFilterParameterStr("filter_type", filter_type_str);
}

void Database::getFilter(const std::string &channel_type, unsigned int channel_number, unsigned int filter_id, SetFilterCallbackType callback)
{
    auto fetchFilterParameterInt = [&](const std::string &parameter_suffix, bool &parameterNotFound) -> int
    {
        std::string parameter_name = channel_type + "_filter_" + std::to_string(channel_number) + "_" + std::to_string(filter_id) + "_" + parameter_suffix;
        mysqlx::Table table = schema.getTable(tableName);
        mysqlx::RowResult result = table.select("parameter_int_value").where("parameter_name = :name").bind("name", parameter_name).execute();

        if (mysqlx::Row row = result.fetchOne())
        {
            return static_cast<int>(row[0]);
        }
        parameterNotFound = true;
        return 0.0; // Default value
    };

    auto fetchFilterParameterDouble = [&](const std::string &parameter_suffix, bool &parameterNotFound) -> double
    {
        std::string parameter_name = channel_type + "_filter_" + std::to_string(channel_number) + "_" + std::to_string(filter_id) + "_" + parameter_suffix;
        mysqlx::Table table = schema.getTable(tableName);
        mysqlx::RowResult result = table.select("parameter_double_value").where("parameter_name = :name").bind("name", parameter_name).execute();

        if (mysqlx::Row row = result.fetchOne())
        {
            return static_cast<double>(row[0]);
        }
        parameterNotFound = true;
        return 0.0; // Default value
    };

    auto fetchFilterParameterStr = [&](const std::string &parameter_suffix, bool &parameterNotFound) -> std::string
    {
        std::string parameter_name = channel_type + "_filter_" + std::to_string(channel_number) + "_" + std::to_string(filter_id) + "_" + parameter_suffix;
        mysqlx::Table table = schema.getTable(tableName);
        mysqlx::RowResult result = table.select("parameter_str_value").where("parameter_name = :name").bind("name", parameter_name).execute();

        // std::cout << "Looking for parameter_name: " << parameter_name << std::endl;

        if (mysqlx::Row row = result.fetchOne())
        {
            std::string value = static_cast<std::string>(row[0]);
            // std::cout << "Found parameter value: " << value << std::endl;
            return value;
        }

        parameterNotFound = true;
        // std::cout << "Parameter not found, using default value" << std::endl;
        return ""; // Default value
    };

    bool anyParameterNotFound = false;

    bool isEnabled = fetchFilterParameterInt("enabled", anyParameterNotFound) != 0;

    double center_frequency = fetchFilterParameterDouble("center_frequency", anyParameterNotFound);
    if (center_frequency == 0.0)
    {
        center_frequency = 1000.0;
    }

    double q_factor = fetchFilterParameterDouble("q_factor", anyParameterNotFound);
    if (q_factor == 0.0)
    {
        q_factor = 0.707;
    }

    double gain_db = fetchFilterParameterDouble("gain_db", anyParameterNotFound);

    std::string filter_type_str = fetchFilterParameterStr("filter_type", anyParameterNotFound);
    if (filter_type_str.empty())
    {
        filter_type_str = "peaking";
    }

    std::string command_type = "notify_filter";

    if (anyParameterNotFound)
    {
        command_type = "get_filter_failed";
        // return;
    }

    callback(command_type, channel_type, channel_number, filter_id, isEnabled, filter_type_str, center_frequency, q_factor, gain_db);
}

#endif // DATABASE_H
