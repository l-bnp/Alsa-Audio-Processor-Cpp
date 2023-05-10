// ConfigSubpage.js

import React, { useState, useEffect } from "react";
import TitleBar from "./headers/TitleBar";
import ApplyButton from "./controls/ApplyButton";
import styles from "./ConfigSubpage.module.css";
import EventManagerContext from "../Utilities/EventManagerContext";

const ConfigSubpage = (props) => {
  const event_manager = React.useContext(EventManagerContext);
  const [address, setAddress] = useState("");
  const [websocketState, setWebsocketState] = useState("disconnected");

  useEffect(() => {
    const handleAddressChange = (newAddress) => {
      setAddress(newAddress);
    };

    const handleWebsocketStateChange = (newState) => {
      setWebsocketState(newState);
    };

    if (event_manager) {
      event_manager.on("notify_ws_address", handleAddressChange);
      event_manager.on("notify_ws_state", handleWebsocketStateChange);
      event_manager.emitEvent("get_ws_address");
      event_manager.emitEvent("get_ws_state");
    }

    return () => {
      if (event_manager) {
        event_manager.off("notify_ws_address", handleAddressChange);
        event_manager.off("notify_ws_state", handleWebsocketStateChange);
      }
    };
  }, []);

  const handleApplyClick = () => {
    event_manager.emitEvent("set_ws_address", address);
  };

  const handleAddressChange = (e) => {
    setAddress(e.target.value);
  };

  function capitalizeFirstLetter(string) {
    return string.charAt(0).toUpperCase() + string.slice(1);
  }

  return (
    <div className={styles.configSubpage}>
      <TitleBar title="Configuration" />
      <div className={styles.configSubpageContent}>
        <div className={styles.controlName}>
          Audio Processor Server Websocket
        </div>
        <input
          type="text"
          value={address}
          onChange={handleAddressChange}
          placeholder="Enter WebSocket Address"
        />
        <div className={styles.button}>
          <ApplyButton onClick={handleApplyClick} active={true} />
        </div>
        <div className={styles.websocketState}>
          <span>WebSocket State:</span>
          <span
            className={styles.websocketStateText}
            data-active={websocketState === "connected"}
          >
            {capitalizeFirstLetter(websocketState)}
          </span>
        </div>
      </div>
    </div>
  );
};

export default ConfigSubpage;
