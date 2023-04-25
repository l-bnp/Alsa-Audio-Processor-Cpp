// database.h
#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <mysqlx/xdevapi.h>
#include <vector>
#include "event_manager.h"

class Database
{
public:
    Database(const std::string &host, int port, const std::string &user, const std::string &password, const std::string &schema, EventManager &event_manager);

private:
    mysqlx::Session session;
    mysqlx::Schema schema;
    std::string tableName = "audio_parameters";
    // Event manager
    EventManager &event_manager;
    void setVolume(const std::string &channel_type, unsigned int channel_number, int volume);
    void getVolume(const std::string &channel_type, unsigned int channel_number, std::function<void(std::string, unsigned int, int)> callback);
    void setMute(const std::string &channel_type, unsigned int channel_number, bool mute);
    void getMute(const std::string &channel_type, unsigned int channel_number, std::function<void(std::string, unsigned int, bool)> callback);
    void setRouting(unsigned int input_channel_number, unsigned int output_channel_number, bool route);
    void getRouting(unsigned int input_channel_number, unsigned int output_channel_number, std::function<void(unsigned int, unsigned int, bool)> callback);
    void setFilter(const std::string &channel_type, unsigned int channel_number, unsigned int filter_id, bool isEnabled, std::string filter_type_str, double center_frequency, double q_factor, double gain_db);
    void getFilter(const std::string &channel_type, unsigned int channel_number, unsigned int filter_id, std::function<void(std::string, unsigned int, unsigned int, bool, std::string, double, double, double)> callback);
};

Database::Database(const std::string &host, int port, const std::string &user, const std::string &password, const std::string &schemaName, EventManager &event_manager)
    : session(host, port, user, password),
      schema(session.getSchema(schemaName)),
      event_manager(event_manager)
{
    // Create table if it doesn't exist
    session.sql("CREATE TABLE IF NOT EXISTS " + schemaName + "." + tableName + " ("
                                                                               "parameter_name VARCHAR(50) PRIMARY KEY,"
                                                                               "parameter_int_value INT DEFAULT 0,"
                                                                               "parameter_double_value DOUBLE DEFAULT 0,"
                                                                               "parameter_str_value VARCHAR(50) DEFAULT '')")
        .execute();

    event_manager.on<std::string, unsigned int, std::function<void(std::string, unsigned int, int)>>("get_volume", [this](const std::string &channel_type, unsigned int channel_number, std::function<void(std::string, unsigned int, int)> callback)
                                                                                                     { this->getVolume(channel_type, channel_number, callback); });

    event_manager.on<std::string, unsigned int, std::function<void(std::string, unsigned int, bool)>>("get_mute", [this](const std::string &channel_type, unsigned int channel_number, std::function<void(std::string, unsigned int, bool)> callback)
                                                                                                      { this->getMute(channel_type, channel_number, callback); });

    event_manager.on<unsigned int, unsigned int, std::function<void(unsigned int, unsigned int, bool)>>("get_routing", [this](unsigned int input_channel_number, unsigned int output_channel_number, std::function<void(unsigned int, unsigned int, bool)> callback)
                                                                                                        { this->getRouting(input_channel_number, output_channel_number, callback); });
    event_manager.on<std::string, unsigned int, unsigned int, std::function<void(std::string, unsigned int, unsigned int, bool, std::string, double, double, double)>>("get_filter", [this](const std::string &channel_type, unsigned int channel_number, unsigned int filter_id, std::function<void(std::string, unsigned int, unsigned int, bool, std::string, double, double, double)> callback)
                                                                                                                                                                       { this->getFilter(channel_type, channel_number, filter_id, callback); });

    event_manager.on<const std::string &, unsigned int, int>("set_volume", [this](const std::string &channel_type, unsigned int channel_number, int volume_db)
                                                             { this->setVolume(channel_type, channel_number, volume_db); });

    event_manager.on<const std::string &, unsigned int, bool>("set_mute", [this](const std::string &channel_type, unsigned int channel_number, bool mute)
                                                              { this->setMute(channel_type, channel_number, mute); });

    event_manager.on<unsigned int, unsigned int, bool>("set_routing", [this](unsigned int input_channel_number, unsigned int output_channel_number, bool route)
                                                       { this->setRouting(input_channel_number, output_channel_number, route); });

    event_manager.on<const std::string &, unsigned int, unsigned int, bool, std::string, double, double, double>("set_filter", [this](const std::string &channel_type, unsigned int channel_number, unsigned int filter_id, bool isEnabled, std::string filter_type_str, double center_frequency, double q_factor, double gain_db)
                                                                                                                 { this->setFilter(channel_type, channel_number, filter_id, isEnabled, filter_type_str, center_frequency, q_factor, gain_db); });
}

void Database::setVolume(const std::string &channel_type, unsigned int channel_number, int volume_db)
{
    std::string parameter_name = channel_type + "_volume_" + std::to_string(channel_number);
    mysqlx::Table table = schema.getTable(tableName);
    table.remove().where("parameter_name = :name").bind("name", parameter_name).execute();

    table.insert("parameter_name", "parameter_int_value").values(parameter_name, volume_db).execute();
}

void Database::getVolume(const std::string &channel_type, unsigned int channel_number, std::function<void(std::string, unsigned int, int)> callback)
{
    std::string parameter_name = channel_type + "_volume_" + std::to_string(channel_number);
    mysqlx::Table table = schema.getTable(tableName);
    mysqlx::RowResult result = table.select("parameter_int_value").where("parameter_name = :name").bind("name", parameter_name).execute();

    int volume_db = 0;

    if (mysqlx::Row row = result.fetchOne())
    {
        volume_db = static_cast<int>(row[0]);
    }

    callback(channel_type, channel_number, volume_db);
}

void Database::setMute(const std::string &channel_type, unsigned int channel_number, bool mute)
{
    std::string parameter_name = channel_type + "_mute_" + std::to_string(channel_number);
    mysqlx::Table table = schema.getTable(tableName);
    table.remove().where("parameter_name = :name").bind("name", parameter_name).execute();
    table.insert("parameter_name", "parameter_int_value").values(parameter_name, mute ? 1.0f : 0.0f).execute();
}

void Database::getMute(const std::string &channel_type, unsigned int channel_number, std::function<void(std::string, unsigned int, bool)> callback)
{
    std::string parameter_name = channel_type + "_mute_" + std::to_string(channel_number);
    mysqlx::Table table = schema.getTable(tableName);
    mysqlx::RowResult result = table.select("parameter_int_value").where("parameter_name = :name").bind("name", parameter_name).execute();

    // Default value if it's not found in the database
    bool mute = false;

    if (mysqlx::Row row = result.fetchOne())
    {
        mute = static_cast<bool>(row[0]);
    }

    callback(channel_type, channel_number, mute);
}

void Database::setRouting(unsigned int input_channel_number, unsigned int output_channel_number, bool route)
{
    std::string parameter_name = "routing_" + std::to_string(input_channel_number) + "_" + std::to_string(output_channel_number);
    mysqlx::Table table = schema.getTable(tableName);
    table.remove().where("parameter_name = :name").bind("name", parameter_name).execute();
    table.insert("parameter_name", "parameter_int_value").values(parameter_name, route ? 1.0f : 0.0f).execute();
}

void Database::getRouting(unsigned int input_channel_number, unsigned int output_channel_number, std::function<void(unsigned int, unsigned int, bool)> callback)
{
    std::string parameter_name = "routing_" + std::to_string(input_channel_number) + "_" + std::to_string(output_channel_number);
    mysqlx::Table table = schema.getTable(tableName);
    mysqlx::RowResult result = table.select("parameter_int_value").where("parameter_name = :name").bind("name", parameter_name).execute();

    // Default value
    bool route = true;
    if (mysqlx::Row row = result.fetchOne())
    {
        route = static_cast<bool>(row[0]);
    }

    callback(input_channel_number, output_channel_number, route);
}

void Database::setFilter(const std::string &channel_type, unsigned int channel_number, unsigned int filter_id, bool isEnabled, std::string filter_type_str, double center_frequency, double q_factor, double gain_db)
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

void Database::getFilter(const std::string &channel_type, unsigned int channel_number, unsigned int filter_id, std::function<void(std::string, unsigned int, unsigned int, bool, std::string, double, double, double)> callback)
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
        center_frequency = 1000;
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

    /* if (anyParameterNotFound)
    {
        return;
    } */

    callback(channel_type, channel_number, filter_id, isEnabled, filter_type_str, center_frequency, q_factor, gain_db);
}

#endif // DATABASE_H
