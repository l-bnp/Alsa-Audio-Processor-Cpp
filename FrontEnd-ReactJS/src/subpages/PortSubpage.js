import React, { useEffect, useContext } from "react";
import { useLocation } from "react-router-dom";
import TitleBar from "./headers/TitleBar";
import styles from "./PortSubpage.module.css";
import AmplitudeMeter from "./controls/AmplitudeMeter";
import EventManagerContext from "../Utilities/EventManagerContext";

const PortSubpage = () => {
  const location = useLocation();
  const { channel_type } = location.state;
  const event_manager = useContext(EventManagerContext);

  useEffect(() => {
    const emitSignalAmplitudes = () => {
      // Emit the "get_meter" event with the channel_type
      event_manager.emitEvent("get_meter", channel_type);
    };

    // Set up an interval to emit the event every 0.1 seconds
    const intervalId = setInterval(emitSignalAmplitudes, 10);

    // Return a cleanup function that clears the interval when the component gets unmounted
    return () => {
      clearInterval(intervalId);
    };
  }, [channel_type]); // Only re-run the effect if channel_type changes

  function capitalizeFirstLetter(string) {
    return string.charAt(0).toUpperCase() + string.slice(1);
  }

  const renderHeaderRow = () => {
    const headers = [];

    headers.push(
      <div key={`header-init}`} className={styles.header}>
        {""}
      </div>
    );

    for (let channel_number = 1; channel_number <= 8; channel_number++) {
      headers.push(
        <div key={`header-${channel_number}`} className={styles.header}>
          {channel_type === "input"
            ? `In ${channel_number}`
            : `Out ${channel_number}`}
        </div>
      );
    }

    return headers;
  };

  const renderSignalLevelLabels = () => {
    const labels = [];
    for (let level = 0; level >= -60; level -= 10) {
      labels.push(
        <div key={`level-${level}`} className={styles.signalLevelLabel}>
          {level}
        </div>
      );
    }
    return labels;
  };

  const renderAmplitudeMeters = () => {
    const meters = [];

    for (let channel_number = 1; channel_number <= 8; channel_number++) {
      meters.push(
        <div className={styles.meter}>
          <AmplitudeMeter
            key={`amplitude-meter-${channel_number}`}
            channel_type={channel_type}
            channel_number={channel_number}
          />
        </div>
      );
    }

    return meters;
  };

  return (
    <div className={styles.portSubpage}>
      <TitleBar title={capitalizeFirstLetter(channel_type) + " Levels"} />
      <div className={styles.subpageContent}>
        <div className={styles.matrix}>
          {renderHeaderRow()}
          <div className={styles.signalLevelLabels}>
            {renderSignalLevelLabels()}
          </div>
          {renderAmplitudeMeters()}
        </div>
      </div>
    </div>
  );
};

export default PortSubpage;
