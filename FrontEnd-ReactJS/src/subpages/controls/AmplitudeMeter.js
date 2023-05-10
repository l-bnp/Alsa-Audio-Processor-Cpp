// AmplitudeMeter.js
import React, { useState, useEffect, useContext } from "react";
import styles from "./AmplitudeMeter.module.css";
import EventManagerContext from "../../Utilities/EventManagerContext";

const LOWER_BOUND = -60.0;
const UPPER_BOUND = 0.0;

const AmplitudeMeter = ({ channel_type, channel_number }) => {
  const [value, setValue] = useState(-80.0);
  const [localMaxBuffer, setLocalMaxBuffer] = useState([]);
  const [localMax, setLocalMax] = useState(-80);
  const [maxValue, setMaxValue] = useState(-80);
  const event_manager = useContext(EventManagerContext);

  const eventListeningFunction = (channelType, amplitudes_db) => {
    if (channelType === channel_type) {
      const amplitudeValue = amplitudes_db[channel_number - 1];
      let amplitude =
        amplitudeValue === null
          ? LOWER_BOUND
          : Math.max(Math.min(amplitudeValue, UPPER_BOUND), LOWER_BOUND);
      setValue(amplitude);

      // Update local maximum buffer
      setLocalMaxBuffer((prevBuffer) => {
        let newBuffer = [...prevBuffer, amplitude];
        if (newBuffer.length > 100) {
          newBuffer.shift();
        }
        return newBuffer;
      });
    }
  };

  useEffect(() => {
    if (event_manager) {
      event_manager.on(`notify_meter`, eventListeningFunction);
    }
    return () => {
      event_manager.off(`notify_meter`, eventListeningFunction);
    };
  }, []);

  useEffect(() => {
    const meanAmplitude =
      localMaxBuffer.reduce((acc, cur) => acc + cur, 0) / localMaxBuffer.length;
    setLocalMax(Math.max(meanAmplitude, value));

    // Update maxValue
    setMaxValue(Math.max(...localMaxBuffer));
  }, [localMaxBuffer, value]);

  const fillPercentage =
    ((value - LOWER_BOUND) / (UPPER_BOUND - LOWER_BOUND)) * 100;
  const localMaxPercentage =
    ((localMax - LOWER_BOUND) / (UPPER_BOUND - LOWER_BOUND)) * 100;
  const maxLinePercentage =
    ((maxValue - LOWER_BOUND) / (UPPER_BOUND - LOWER_BOUND)) * 100;

  return (
    <div className={styles.componentContainer}>
      <div className={styles.meterContainer}>
        <div
          className={styles.localMaxBar}
          style={{ height: `${localMaxPercentage}%` }}
        ></div>
        <div
          className={styles.maxLine}
          style={{ height: `${maxLinePercentage}%` }}
        ></div>
        <div
          className={styles.fill}
          style={{ height: `${fillPercentage}%` }}
        ></div>
      </div>
      <div className={styles.valueContainer}>{Math.round(value)}</div>
    </div>
  );
};

export default AmplitudeMeter;
