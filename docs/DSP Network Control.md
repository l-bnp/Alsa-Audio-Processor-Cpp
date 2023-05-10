# Network Control

The network control of the audio processor is done through sending specific JSON formated commands. The connection is established via websocket to the host IP address and to the port specified in the program's command line arguments. For more info on how to run the program, refer to the [Running The Program.md](./Running%20The%20Program.md) file.

--- 

All JSON shall contain a `command_type` property with string value and additional properties, for each command type. The available commands are the following:

| Command          | Command Arguments                                                     | Response            | Response Arguments                                                |
|------------------|-----------------------------------------------------------------------|---------------------|-------------------------------------------------------------------|
| set_gain         | - command_type: string<br>- channel_type: string<br>- channel_number: unsigned int<br>- gain_db: double | notify_gain,<br>set_gain_failed | - command_type: string<br>- channel_type: string<br>- channel_number: unsigned int<br>- gain_db: double         |
| get_gain         | - command_type: string<br>- channel_type: string<br>- channel_number: unsigned int | notify_gain,<br>get_gain_failed | - command_type: string<br>- channel_type: string<br>- channel_number: unsigned int<br>- gain_db: double         |
| set_mute         | - command_type: string<br>- channel_type: string<br>- channel_number: unsigned int<br>- mute: bool       | notify_mute,<br>set_mute_failed   | - command_type: string<br>- channel_type: string<br>- channel_number: unsigned int<br>- mute: bool             |
| get_mute         | - command_type: string<br>- channel_type: string<br>- channel_number: unsigned int | notify_mute,<br>get_mute_failed   | - command_type: string<br>- channel_type: string<br>- channel_number: unsigned int<br>- mute: bool             |
| set_mixer        | - command_type: string<br>- input_channel: unsigned int<br>- output_channel: unsigned int<br>- mix: bool | notify_mixer,<br>set_mixer_failed | - command_type: string<br>- input_channel: unsigned int<br>- output_channel: unsigned int<br>- mix: bool |
| get_mixer        | - command_type: string<br>- input_channel: unsigned int<br>- output_channel: unsigned int | notify_mixer,<br>get_mixer_failed | - command_type: string<br>- input_channel: unsigned int<br>- output_channel: unsigned int<br>- mix: bool |
| set_filter       | - command_type: string<br>- channel_type: string<br>- channel_number: unsigned int<br>- filter_id: unsigned int<br>- filter_enabled: bool<br>- filter_type: string<br>- center_frequency: double<br>- q_factor: double<br>- gain_db: double | notify_filter,<br>set_filter_failed | - command_type: string<br>- channel_type: string<br>- channel_number: unsigned int<br>- filter_id: unsigned int<br>- filter_enabled: bool<br>- filter_type: string<br>- center_frequency: double<br>- q_factor: double<br>- gain_db: double |
| get_filter       | - command_type: string<br>- channel_type: string<br>- channel_number: unsigned int<br>- filter_id: unsigned int | notify_filter,<br>get_filter_failed | - command_type: string<br>- channel_type: string<br>- channel_number: unsigned int<br>- filter_id: unsigned int<br>- filter_enabled: bool<br>- filter_type: string<br>- center_frequency: double<br>- q_factor: double<br>- gain_db: double |
| get_meter | - command_type: string<br>- channel_type: string | notify_meter,<br>get_meter_failed | - command_type: string<br>- channel_type: string<br>- amplitudes_db: array<double> |


--- 

# Command Descriptions:

## Set Gain

Sets the value of a channel's volume. Should specify if its an input or output channel, the channel number and the desired volume level in dBFS.

#### Command:
- command_type: string ("set_gain")
- channel_type: string ("input", "output")
- channel_number: unsigned int (1 - 16)
- gain_db: double (-60.0 - 0.0)

#### Response:
- command_type: string ("notify_gain", "set_gain_failed")
- channel_type: string ("input", "output")
- channel_number: unsigned int (1 - 16)
- gain_db: double (-60.0 - 0.0)


## Get Gain

Asks for the value of a channel's volume. Should specify if its an input or output channel and the channel number. Gets a return value of volume in dBFS.

#### Command:
- command_type: string ("get_gain")
- channel_type: string ("input", "output")
- channel_number: unsigned int (1 - 16)

#### Response:
- command_type: string ("notify_gain", "get_gain_failed")
- channel_type: string ("input", "output")
- channel_number: unsigned int (1 - 16)
- gain_db: double (-60.0 - 0.0)


## Set Mute

Sets the value of a channel's mute. Should specify if its an input or output channel, the channel number and the desired mute state, either true or false.

#### Command:
- command_type: string ("set_mute")
- channel_type: string ("input", "output")
- channel_number: unsigned int (1 - 16)
- mute: bool (false, true)

#### Response:
- command_type: string ("notify_mute", "set_mute_failed")
- channel_type: string ("input", "output")
- channel_number: unsigned int (1 - 16)
- mute: bool (false, true)


## Get Mute

Asks for the value of a channel's mute. Should specify if its an input or output channel and the channel number. Gets a return value specifying the mute state of the channel, either true or false.

#### Command:
- command_type: string ("get_mute")
- channel_type: string ("input", "output")
- channel_number: unsigned int (1 - 16)

#### Response:
- command_type: string ("notify_mute", "get_mute_failed")
- channel_type: string ("input", "output")
- channel_number: unsigned int (1 - 16)
- mute: bool (false, true)


## Set Mixer

Sets the routing of any input channel to any output channel as true or false. Should specify the input channel number, the output channel number, and the desired mix state, either true or false.

#### Command:
- command_type: string ("set_mixer")
- input_channel: unsigned int (1 - 16)
- output_channel: unsigned int (1 - 16)
- mix: bool (false, true)

#### Response:
- command_type: string ("notify_mixer", "set_mixer_failed")
- input_channel: unsigned int (1 - 16)
- output_channel: unsigned int (1 - 16)
- mix: bool (false, true)


## Get Mixer

Asks for the routing of an input channel to an output channel. Should specify the input channel number and the output channel number. Get returned the mix state, either true or false.

#### Command:
- command_type: string ("get_mixer")
- input_channel: unsigned int (1 - 16)
- output_channel: unsigned int (1 - 16)

#### Response:
- command_type: string ("notify_mixer", "get_mixer_failed")
- input_channel: unsigned int (1 - 16)
- output_channel: unsigned int (1 - 16)
- mix: bool (false, true)


## Set Filter

Sets a filter for a channel. Should specify if its an input or output channel, the channel number, the filter number (id) and the filter parameters: If its enabled or disabled, the filter type, the center frequency in Hz, the Q factor and the gain in dBFS.

#### Command:
- command_type: string ("set_filter")
- channel_type: string ("input", "output")
- channel_number: unsigned int (1 - 16)
- filter_id: unsigned int (1 - 16)
- filter_enabled: bool (false, true)
- filter_type: string ("highpass", "lowpass", "notch", "peaking")
- center_frequency: double (20.0 - 20000.0)
- q_factor: double (0.1 - 10.0)
- gain_db: double (-60 - 20)

#### Response:
- command_type: string ("notify_filter", "set_filter_failed")
- channel_type: string ("input", "output")
- channel_number: unsigned int (1 - 16)
- filter_id: unsigned int (1 - 16)
- filter_enabled: bool (false, true)
- filter_type: string ("highpass", "lowpass", "notch", "peaking")
- center_frequency: double (20.0 - 20000.0)
- q_factor: double (0.1 - 10.0)
- gain_db: double (-60 - 20)


## Get Filter

Asks for a specific filter state for a given channel channel. Should specify if its an input or output channel, the channel number and the filter number (id). Gets as returned value the filter parameters: If its enabled or disabled, the filter type, the center frequency in Hz, the Q factor and the gain in dBFS.

#### Command:
- command_type: string ("get_filter")
- channel_type: string ("input", "output")
- channel_number: unsigned int (1 - 16)
- filter_id: unsigned int (1 - 16)

#### Response:
- command_type: string ("notify_filter", "get_filter_failed")
- channel_type: string ("input", "output")
- channel_number: unsigned int (1 - 16)
- filter_id: unsigned int (1 - 16)
- filter_enabled: bool (false, true)
- filter_type: string ("highpass", "lowpass", "notch", "peaking")
- center_frequency: double (20.0 - 20000.0)
- q_factor: double (0.1 - 10.0)
- gain_db: double (-60 - 20)


## Get Signal Amplitudes

Asks for the current amplitudes of either all input channel or all output channels. Should specify only if its requiring input or output levels. Gets an array with the amplitudes in dBFS as a return value.

#### Command:
- command_type: string ("get_meter")
- channel_type: string ("input", "output")

#### Response:
- command_type: string ("notify_meter", "get_meter_failed")
- channel_type: string ("input", "output")
- amplitudes_db: array\<double\>


---

# Examples:


## Set Gain

#### Command:
  ```json
  {
    "command_type":"set_gain",
    "channel_type":"output",
    "channel_number":1,
    "gain_db":-20.0
  }
  ```

#### Success Response:
  ```json
  {
    "command_type":"notify_gain",
    "channel_type":"output",
    "channel_number":1,
    "gain_db":-20.0
  }
  ```
  
#### Fail Response:
  ```json
  {
    "command_type":"set_gain_failed",
    "channel_type":"output",
    "channel_number":1,
    "gain_db":-20.0
  }
  ```

## Get Gain

#### Command:
  ```json
  {
    "command_type":"get_gain",
    "channel_type":"input",
    "channel_number":2
  }
  ```

#### Success Response:
  ```json
  {
    "command_type":"notify_gain",
    "channel_type":"input",
    "channel_number":2,
    "gain_db":-9.0
  }
  ```

#### Fail Response:
  ```json
  {
    "command_type":"get_gain_failed",
    "channel_type":"input",
    "channel_number":2,
    "gain_db":0.0
  }
  ```

## Set Mute

#### Command:
  ```json
  {
    "command_type":"set_mute",
    "channel_type":"output",
    "channel_number":1,
    "mute":false
  }
  ```

#### Success Response:
  ```json
  {
    "command_type":"notify_mute",
    "channel_type":"input",
    "channel_number":1,
    "mute":false
  }
  ```

#### Fail Response:
  ```json
  {
    "command_type":"set_mute_failed",
    "channel_type":"input",
    "channel_number":1,
    "mute":false
  }
  ```


## Get Mute

#### Command:
  ```json
  {
    "command_type":"get_mute",
    "channel_type":"input",
    "channel_number":1
  }
  ```

#### Success Response:
  ```json
  {
    "command_type":"notify_mute",
    "channel_type":"input",
    "channel_number":1,
    "mute":false
  }
  ```

#### Fail Response:
  ```json
  {
    "command_type":"get_mute_failed",
    "channel_type":"input",
    "channel_number":1,
    "mute":false
  }
  ```


## Set Mixer

#### Command:
  ```json
  {
    "command_type":"set_mixer",
    "input_channel":1,
    "output_channel":1,
    "mix":true
  }
  ```

#### Success Response:
  ```json
  {
    "command_type":"notify_mixer",
    "input_channel":3,
    "output_channel":1,
    "mix":false
  }
  ```

#### Fail Response:
  ```json
  {
    "command_type":"set_mixer_failed",
    "input_channel":3,
    "output_channel":1,
    "mix":false
  }
  ```

## Get Mixer

#### Command:
  ```json
  {
    "command_type":"get_mixer",
    "input_channel":1,
    "output_channel":1
  }
  ```

#### Success Response:
  ```json
  {
    "command_type":"notify_mixer",
    "input_channel":3,
    "output_channel":1,
    "mix":false
  }
  ```

#### Fail Response:
  ```json
  {
    "command_type":"get_mixer_failed",
    "input_channel":3,
    "output_channel":1,
    "mix":false
  }
  ```

## Set Filter

#### Command:
  ```json
  {
    "command_type":"set_filter",
    "channel_type":"input",
    "channel_number":1,
    "filter_id":2,
    "filter_enabled":false,
    "filter_type":"lowpass",
    "center_frequency":700.0,
    "q_factor":0.7,
    "gain_db":0
  }
  ```

#### Success Response:
  ```json
  {
    "command_type":"notify_filter",
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

#### Fail Response:
  ```json
  {
    "command_type":"set_filter_failed",
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


## Get Filter

#### Command:
  ```json
  {
    "command_type":"get_filter",
    "channel_type":"input",
    "channel_number":1,
    "filter_id":2
  }
  ```

#### Success Response:
  ```json
  {
    "command_type":"notify_filter",
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

#### Fail Response:
  ```json
  {
    "command_type":"get_filter_failed",
    "channel_type":"output",
    "channel_number":1,
    "filter_id":1,
    "filter_enabled":false,
    "filter_type":"peaking",
    "center_frequency":0.0,
    "q_factor":0.0,
    "gain_db":0
  }
  ```

## Get Signal Amplitudes

#### Command:
  ```json
  {
    "command_type":"get_meter",
    "channel_type":"input"
  }
  ```

#### Success Response:
  ```json
  {
    "command_type":"notify_meter",
    "channel_type":"input",
    "amplitudes_db":[-72.0,-60.0,-82.0,-68.0,-56.0,-90.0,-84.0,-57.0]
  }
  ```

#### Fail Response:
  ```json
  {
    "command_type":"get_meter_failed",
    "channel_type":"input",
    "amplitudes_db":[]
  }
  ```


