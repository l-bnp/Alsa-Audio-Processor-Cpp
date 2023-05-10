# Events 

These are the events handled by the internal event manager in the C++ audio processor program.  The table shows the classes that emit and listen to each event:

| Event Name                | Event Emitter Classes                  | Event Listener Classes                 | 
|---------------------------|----------------------------------------|----------------------------------------|
| set_gain                  | CustomWebSocketServer                  | Gain, Database                         |
| get_gain                  | CustomWebSocketServer                  | Gain                                   |
| set_mute                  | CustomWebSocketServer                  | Mute, Database                         |
| get_mute                  | CustomWebSocketServer                  | Mute                                   |
| set_mixer                 | CustomWebSocketServer                  | Mixer, Database                        |
| get_mixer                 | CustomWebSocketServer                  | Mixer                                  |
| set_filter                | CustomWebSocketServer                  | Equalizer, Database                    |
| get_filter                | CustomWebSocketServer                  | Equalizer                              |
| get_meter                 | CustomWebSocketServer                  | Meter                                  |
| get_database_gain         | Gain                                   | Database                               |
| get_database_mute         | Mute                                   | Database                               |
| get_database_mixer        | Mixer                                  | Database                               |
| get_database_filter       | Equalizer                              | Database                               |
