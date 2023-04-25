# State Management

### Install and Configure Database (MySQL)


Add the MySQL APT repository to your system:
```console
wget https://dev.mysql.com/get/mysql-apt-config_0.8.17-1_all.deb
sudo dpkg -i mysql-apt-config_0.8.17-1_all.deb
sudo apt update
```

Install And Configure MySQL:
```console
sudo apt install mysql-server
sudo systemctl enable mysql
sudo systemctl start mysql
mysql -u root -p
CREATE DATABASE audio_processor;
```

Install MySQL C++ Connector:
```console
sudo apt install libmysqlcppconn-dev
```

Find MySQL C++ Connector Include Path:
```console
find /usr -iname "mysqlx"
find /usr -iname "xdevapi.h"
```

Add Flags To include The Path and Compilation Command,
Considering that the found path is `/usr/local/include/mysqlx` and the command is `-lmysqlcppconn8`
```console
g++ -std=c++17 -pthread -I/usr/local/include -I/usr/local/include/mysqlx -L/usr/local/lib -o motsucDSP main.cpp -lixwebsocket -lz -lcrypto -lssl -lasound -lmysqlcppconn8
```

* The SCHEME should be already created inside MySQL
* Remember to set correct password

--- 

# Database Parameter Format

Uses MYSQL database with the following specification:

## Table 

- Settings
    ```
    Database: audio_processor
    Table: audio_parameters
    ```

- Columns
    ```
    parameter_name: VARCHAR(50) PRIMARY KEY
    parameter_int_value: INT DEFAULT 0
    parameter_double_value: DOUBLE DEFAULT 0
    parameter_str_value: VARCHAR(50) DEFAULT ''
    ```

## Entries

- Volumes
    ```
    General format:
    <channel_type>_volume_<channel_number>

    Example:
    "input_volume_1": int
    ```

- Mutes
    ```
    General format:
    <channel_type>_mute_<channel_number>

    Example:
    "input_mute_1": int
    ```

- Routings
    ```
    General format:
    <channel_type>_volume_<channel_number>

    Example:
    "routing_3_1": int
    ```

- Filters
    ```
    General format:
    <channel_type>_filter_<channel_number>_<filter_number>_<parameter>

    Example:
    "input_filter_1_1_center_frequency": double
    "input_filter_1_1_enabled": int
    "input_filter_1_1_filter_type": string
    "input_filter_1_1_gain_db": double
    "input_filter_1_1_q_factor": double
    ```

---

## Example table:

| parameter_name                     | parameter_int_value | parameter_double_value | parameter_str_value |
|------------------------------------|---------------------|------------------------|---------------------|
| input_filter_1_1_center_frequency  |                   0 |                    300 |                     |
| input_filter_1_1_enabled           |                   1 |                      0 |                     |
| input_filter_1_1_filter_type       |                   0 |                      0 | highpass            |
| input_filter_1_1_gain_db           |                   0 |                      0 |                     |
| input_filter_1_1_q_factor          |                   0 |                    0.7 |                     |
| input_mute_1                       |                   0 |                      0 |                     |
| input_mute_2                       |                   1 |                      0 |                     |
| input_volume_1                     |                   0 |                      0 |                     |
| input_volume_12                    |                 -12 |                      0 |                     |
| output_filter_1_1_center_frequency |                   0 |                   1000 |                     |
| output_filter_1_1_enabled          |                   1 |                      0 |                     |
| output_filter_1_1_filter_type      |                   0 |                      0 | peaking             |
| output_filter_1_1_gain_db          |                   0 |                      6 |                     |
| output_filter_1_1_q_factor         |                   0 |                      1 |                     |
| output_volume_1                    |                  -3 |                      0 |                     |
| routing_1_1                        |                   1 |                      0 |                     |
| routing_1_2                        |                   1 |                      0 |                     |
| routing_3_1                        |                   0 |                      0 |                     |