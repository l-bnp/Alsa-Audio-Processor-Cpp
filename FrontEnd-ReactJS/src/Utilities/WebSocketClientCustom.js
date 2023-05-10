class WebSocketClientCustom {
  constructor(serverAddress, event_manager) {
    this.socketState = "disconnected";

    this.event_manager = event_manager;
    // Websocket
    this.serverAddress = serverAddress;
    this.connectSocket();

    // Transmission Queue
    this.transmissionQueue = [];

    this.startQueueTimer();

    // Register Event listeners
    this.registerEventManagerListeners();
  }

  startQueueTimer() {
    //this.stopQueueTimer();
    this.queueTimer = setInterval(() => {
      this.checkTxQueue();
    }, 1000);
  }

  stopQueueTimer() {
    clearInterval(this.queueTimer);
  }

  checkTxQueue() {
    this.stopQueueTimer();
    if (this.socket.readyState == WebSocket.OPEN) {
      this.socketState = "connected";
      while (this.transmissionQueue.length > 0) {
        this.socket.send(this.transmissionQueue[0]);
        // Remove first element from array
        this.transmissionQueue.shift();
      }
    } else {
      this.socketState = "disconnected";
      if (this.socket.readyState == WebSocket.CLOSED) {
        this.connectSocket();
      }
    }
    this.event_manager.emitEvent("notify_ws_state", this.socketState);
    this.startQueueTimer();
  }

  handleSocketOpen = (event) => {
    // this.socket.send("Hello Server!");
  };

  handleSocketClose = (event) => {};

  handleSocketMessage = (event) => {
    // console.log("Message from server ", event.data);
    this.newMessage(event.data);
  };

  // Reconnect to socket and add event handlers
  connectSocket() {
    this.socket = new WebSocket(this.serverAddress);

    // Connection opened
    this.socket.addEventListener("open", this.handleSocketOpen);

    // Connection closed
    this.socket.addEventListener("close", this.handleSocketClose);

    // Listen for messages
    this.socket.addEventListener("message", this.handleSocketMessage);
  }

  disconnectSocket() {
    if (this.socket) {
      this.socket.removeEventListener("open", this.handleSocketOpen);
      this.socket.removeEventListener("close", this.handleSocketClose);
      this.socket.removeEventListener("message", this.handleSocketMessage);
      this.socket.close();
    }
  }

  changeAddress(newAddress) {
    this.stopQueueTimer();
    this.disconnectSocket();
    this.serverAddress = newAddress;
    this.connectSocket();
    this.startQueueTimer();
  }

  addToTransmissionQueue(message) {
    // console.log("Adding to transmission queue: ", message);
    this.transmissionQueue.push(message);
    this.checkTxQueue();
  }

  sendToServer(...args) {
    const messageObject = this.buildMessageObject(...args);
    const message = JSON.stringify(messageObject);
    this.addToTransmissionQueue(message);
  }

  buildMessageObject(command_type, ...args) {
    if (!command_type) {
      throw new Error("A command_type argument is required");
    }

    switch (command_type) {
      case "set_gain":
        if (args.length !== 3) {
          throw new Error("set_gain requires 3 arguments");
        }
        return {
          command_type,
          channel_type: args[0],
          channel_number: args[1],
          gain_db: args[2],
        };

      case "get_gain":
        if (args.length !== 2) {
          throw new Error("get_gain requires 2 arguments");
        }
        return {
          command_type,
          channel_type: args[0],
          channel_number: args[1],
        };

      case "set_mute":
        if (args.length !== 3) {
          throw new Error("set_mute requires 3 arguments");
        }
        return {
          command_type,
          channel_type: args[0],
          channel_number: args[1],
          mute: args[2],
        };

      case "get_mute":
        if (args.length !== 2) {
          throw new Error("get_mute requires 2 arguments");
        }
        return {
          command_type,
          channel_type: args[0],
          channel_number: args[1],
        };

      case "set_mixer":
        if (args.length !== 3) {
          throw new Error("set_mixer requires 3 arguments");
        }
        return {
          command_type,
          input_channel: args[0],
          output_channel: args[1],
          mix: args[2],
        };

      case "get_mixer":
        if (args.length !== 2) {
          throw new Error("get_mixer requires 2 arguments");
        }
        return {
          command_type,
          input_channel: args[0],
          output_channel: args[1],
        };

      case "set_filter":
        if (args.length !== 8) {
          throw new Error("set_filter requires 8 arguments");
        }
        return {
          command_type,
          channel_type: args[0],
          channel_number: args[1],
          filter_id: args[2],
          filter_enabled: args[3],
          filter_type: args[4],
          center_frequency: args[5],
          q_factor: args[6],
          gain_db: args[7],
        };

      case "get_filter":
        if (args.length !== 3) {
          throw new Error("get_filter requires 3 arguments");
        }
        return {
          command_type,
          channel_type: args[0],
          channel_number: args[1],
          filter_id: args[2],
        };

      case "get_meter":
        if (args.length !== 1) {
          throw new Error("get_meter requires 1 argument");
        }
        return {
          command_type,
          channel_type: args[0],
        };

      default:
        throw new Error("Invalid command type");
    }
  }

  newMessage(message) {
    // console.log("newMessage ", message);
    const messageObject = JSON.parse(message);
    if (messageObject.command_type === "notify_gain") {
      this.event_manager.emitEvent(
        "notify_gain",
        messageObject.channel_type,
        messageObject.channel_number,
        messageObject.gain_db
      );
    } else if (messageObject.command_type === "notify_mute") {
      this.event_manager.emitEvent(
        "notify_mute",
        messageObject.channel_type,
        messageObject.channel_number,
        messageObject.mute
      );
    } else if (messageObject.command_type === "notify_mixer") {
      this.event_manager.emitEvent(
        "notify_mixer",
        messageObject.input_channel,
        messageObject.output_channel,
        messageObject.mix
      );
    } else if (messageObject.command_type === "notify_filter") {
      this.event_manager.emitEvent(
        "notify_filter",
        messageObject.channel_type,
        messageObject.channel_number,
        messageObject.filter_id,
        messageObject.filter_enabled,
        messageObject.filter_type,
        messageObject.center_frequency,
        messageObject.q_factor,
        messageObject.gain_db
      );
    } else if (messageObject.command_type === "notify_meter") {
      this.event_manager.emitEvent(
        "notify_meter",
        messageObject.channel_type,
        messageObject.amplitudes_db
      );
    } else {
      console.log("Unknown message type: " + messageObject.command_type);
    }
  }

  // Event listener in the event manager
  registerEventManagerListeners() {
    // Register event listeners

    this.event_manager.on("set_ws_address", (ws_address) => {
      this.changeAddress(ws_address);
    });

    this.event_manager.on("get_ws_address", () => {
      this.event_manager.emitEvent("notify_ws_address", this.serverAddress);
    });

    this.event_manager.on("get_ws_state", () => {
      this.event_manager.emitEvent("notify_ws_state", this.socketState);
    });

    this.event_manager.on(
      "set_gain",
      (channel_type, channel_number, gain_db) => {
        this.sendToServer("set_gain", channel_type, channel_number, gain_db);
      }
    );

    this.event_manager.on("get_gain", (channel_type, channel_number) => {
      this.sendToServer("get_gain", channel_type, channel_number);
    });

    this.event_manager.on(
      "set_mute",
      (channel_type, channel_number, mute_state) => {
        this.sendToServer("set_mute", channel_type, channel_number, mute_state);
      }
    );

    this.event_manager.on("get_mute", (channel_type, channel_number) => {
      this.sendToServer("get_mute", channel_type, channel_number);
    });

    this.event_manager.on(
      "set_mixer",
      (input_channel, output_channel, mix_state) => {
        this.sendToServer(
          "set_mixer",
          input_channel,
          output_channel,
          mix_state
        );
      }
    );

    this.event_manager.on("get_mixer", (input_channel, output_channel) => {
      this.sendToServer("get_mixer", input_channel, output_channel);
    });

    this.event_manager.on(
      "get_filter",
      (channel_type, channel_number, filter_id) => {
        this.sendToServer(
          "get_filter",
          channel_type,
          channel_number,
          filter_id
        );
      }
    );

    this.event_manager.on(
      "set_filter",
      (
        channel_type,
        channel_number,
        filter_id,
        filter_enabled,
        filter_type,
        center_frequency,
        q_factor,
        gain_db
      ) => {
        this.sendToServer(
          "set_filter",
          channel_type,
          channel_number,
          filter_id,
          filter_enabled,
          filter_type,
          center_frequency,
          q_factor,
          gain_db
        );
      }
    );

    this.event_manager.on("get_meter", (channel_type) => {
      this.sendToServer("get_meter", channel_type);
    });
  }
}

export default WebSocketClientCustom;
