# Start Instructions

Install dependencies:
```console
sudo apt-get install libasound2-dev
```

Compile:
```console
 g++ -std=c++17 -pthread -I/usr/local/include -L/usr/local/lib -o dsp-app main.cpp -lixwebsocket -lz -lcrypto -lssl -lasound
```

Run format:
```console
 sudo ./dsp-app -interface:<Interfae_Name> -inputs:<number_of_inputs> -outputs:<number_of_outputs> -rate:<sample_rate> -port:<network_control_server_port>
```

Run Example:
```console
 sudo ./dsp-app -interface:"plughw:CARD=PCH,DEV=0" -inputs:8 -outputs:8 -rate:44100 -port:3001
```

To control signal live over network (websocket) refer to [DSP Network Control.md](./DSP%20Network%20Control.md)

---
