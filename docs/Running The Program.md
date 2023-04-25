# Start Instructions

- Install dependencies:
    ```console
    sudo apt update
    sudo apt install cmake g++ libssl-dev libasound2-dev
    ```
- Download [C++ Event Manager](https://github.com/l-bnp/Event-Manager-Cpp.git) to the [`submodules/Event-Manager-Cpp`](../Processor-C%2B%2B/submodules/Event-Manager-Cpp/) folder.
- Download and install [C++ IXWebSockets](https://github.com/machinezone/IXWebSocket.git).

- Compile:
    ```console
    g++ -std=c++17 -pthread -I/usr/local/include -I/usr/include/mysql-cppconn-8 -L/usr/local/lib -o dsp-app main.cpp -lixwebsocket -lz -lcrypto -lssl -lasound -lmysqlcppconn8
    ```

- Run format:
    ```console
    sudo ./dsp-app -interface:<Interfae_Name> -inputs:<number_of_inputs> -outputs:<number_of_outputs> -rate:<sample_rate> -port:<network_control_server_port>
    ```

- Run Example:
    ```console
    sudo ./dsp-app -interface:"plughw:CARD=PCH,DEV=0" -inputs:8 -outputs:8 -rate:44100 -port:3001
    ```

* To install, configure and interact with the parameter managing database (MySQL) refer to [State Managing.md](./State%20Managing.md)
* To control signal live over network (websocket) refer to [DSP Network Control.md](./DSP%20Network%20Control.md).

---