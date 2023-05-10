import React, { useState, useEffect, useContext } from "react";
import styles from "./ConnectionSign.module.css";
import EventManagerContext from "../../Utilities/EventManagerContext";

function ConnectionSign(props) {
  const event_manager = useContext(EventManagerContext);
  const [currentState, setState] = useState("disconnected");

  useEffect(() => {
    const handleWebsocketStateChange = (newState) => {
      setState(newState);
    };

    if (event_manager) {
      event_manager.on("notify_ws_state", handleWebsocketStateChange);
      event_manager.emitEvent("get_ws_state");
    }

    return () => {
      if (event_manager) {
        event_manager.off("notify_ws_state", handleWebsocketStateChange);
      }
    };
  }, []);

  const returnRows = [];

  returnRows.push(
    <div
      className={styles.connectionSign}
      data-active={currentState == "connected"}
    ></div>
  );

  return <>{returnRows}</>;
}

export default ConnectionSign;
