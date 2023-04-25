# Network Control

The network control of the audio processor is done through sending specific JSON formated commands. The connection is established via websocket to the host IP address and to the port specified in the program's command line arguments. For more info on how to run the program, refer to the [Running The Program.md](./Running%20The%20Program.md) file.

--- 

The format for the JSON commands to be sent to the websocket server is described below. All the commands should include at least a `command_type` field, specifying the desired command. For each `command_type` there will be a different number and type of extra arguments required.

## Value Types and Ranges:

- command_type: string ("set_volume", "set_mute", "set_routing", "set_filter", "get_signal_amplitudes")

### command_type: "set_volume"
- channel_type: string ("input", "output")
- channel_number: unsigned int (1 - 16)
- volume_db: int (-60 - 0)


### command_type: "set_mute"
- channel_type: string ("input", "output")
- channel_number: unsigned int (1 - 16)
- mute: bool (false, true)


### command_type: "set_routing"
- input_channel: unsigned int (1 - 16)
- output_channel: unsigned int (1 - 16)
- route: bool (false, true)


### command_type: "set_filter"
- channel_type: string ("input", "output")
- channel_number: unsigned int (1 - 16)
- filter_id: unsigned int (1 - 16)
- filter_enabled: bool (false, true)
- filter_type: string ("highpass", "lowpass", "notch", "peaking")
- center_frequency: double (20.0 - 20000.0)
- q_factor: double (0.1 - 10.0)
- gain_db: double (-60 - 20)


### command_type: "get_signal_amplitudes".
- channel_type: string ("input", "output")

---

## Examples:

### Set Volume
Command:
  ```json
  {
    "command_type":"set_volume",
    "channel_type":"output",
    "channel_number":1,
    "volume_db":-9
  }
  ```

Response:
  ```json
  {
    "response_to":"set_volume",
    "command_status":"success"
  }
  ```

### Mute Channel
Command:
  ```json
  {
    "command_type":"set_mute",
    "channel_type":"input",
    "channel_number":1,
    "mute":false
  }
  ```

Response:
  ```json
  {
    "response_to":"set_mute",
    "command_status":"success"
  }
  ```

### Route Input to Output
Command:
  ```json
  {
    "command_type":"set_routing",
    "input_channel":3,
    "output_channel":1,
    "route":false
  }
  ```

Response:
  ```json
  {
    "response_to":"set_routing",
    "command_status":"success"
  }
  ```

### Set Filter
Command:
  ```json
  {
    "command_type":"set_filter",
    "channel_type":"output",
    "channel_number":1,
    "filter_id":1,
    "filter_enabled":true,
    "filter_type":"highpass",
    "center_frequency":700.0,
    "q_factor":0.7,
    "gain_db":0
  }
  ```

Response:
  ```json
  {
    "response_to":"set_filter",
    "command_status":"success"
  }
  ```

### Get All Volume Levels in dB
Command:
  ```json
  {
    "command_type":"get_signal_amplitudes",
    "channel_type":"input"
  }
  ```

Response:
  ```json
  {
    "response_to":"get_signal_amplitudes",
    "command_status":"success",
    "volumes":[-72,-60,-82,-68,-56,-90,-84,-57]
  }
  ```


