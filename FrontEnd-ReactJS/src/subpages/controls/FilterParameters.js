import React, { useState, useEffect, useContext } from "react";
import styles from "./FilterParameters.module.css";
import FilterEnabled from "./FilterEnabled";
import FilterTextInput from "./FilterTextInput";
import EventManagerContext from "../../Utilities/EventManagerContext";
import CustomSelect from "./CustomSelect";

const FilterParameters = ({ channel_type, channel_number, filter_id }) => {
  const [enabled, setEnabled] = useState(false);
  const [type, setType] = useState("peaking");
  const [centerFrequency, setCenterFrequency] = useState(1000);
  const [qFactor, setQFactor] = useState(0.707);
  const [gainDb, setGainDb] = useState(0);
  const [lastNotifiedParameters, setLastNotifiedParameters] = useState({
    enabled: false,
    type: "peaking",
    centerFrequency: 1000,
    qFactor: 0.707,
    gainDb: 0,
  });
  const event_manager = useContext(EventManagerContext);

  const eventListeningFunction = (
    channelType,
    channelNumber,
    filterId,
    filterEnabled,
    filterType,
    filterFrequency,
    filterQFactor,
    filterGain
  ) => {
    if (
      filterId === filter_id &&
      channelType === channel_type &&
      channelNumber === channel_number
    ) {
      setLastNotifiedParameters({
        enabled: filterEnabled,
        type: filterType,
        centerFrequency: filterFrequency.toFixed(0),
        qFactor: filterQFactor.toFixed(2),
        gainDb: filterGain.toFixed(1),
      });
      //
      setEnabled(filterEnabled);
      setType(filterType);
      setCenterFrequency(parseFloat(filterFrequency.toFixed(0)));
      setQFactor(parseFloat(filterQFactor.toFixed(2)));
      setGainDb(parseFloat(filterGain.toFixed(1)));
    }
  };

  useEffect(() => {
    if (event_manager) {
      event_manager.on(`notify_filter`, eventListeningFunction);
    }
    return () => {
      event_manager.off(`notify_filter`, eventListeningFunction);
    };
  }, []);

  const handleEnabledClick = () => {
    const newEnabled = !enabled;
    setEnabled(newEnabled);
    sendFilterParameters(newEnabled, type, centerFrequency, qFactor, gainDb);
  };

  const handleTypeChange = (newType) => {
    setType(newType);
    sendFilterParameters(enabled, newType, centerFrequency, qFactor, gainDb);
  };

  const handleCenterFrequencyChange = (value) => {
    const newCenterFrequency = parseFloat(value);
    setCenterFrequency(newCenterFrequency);
    sendFilterParameters(enabled, type, newCenterFrequency, qFactor, gainDb);
  };

  const handleQFactorChange = (value) => {
    const newQFactor = parseFloat(value);
    setQFactor(newQFactor);
    sendFilterParameters(enabled, type, centerFrequency, newQFactor, gainDb);
  };

  const handleGainDbChange = (value) => {
    const newGainDb = parseFloat(value);
    setGainDb(newGainDb);
    sendFilterParameters(enabled, type, centerFrequency, qFactor, newGainDb);
  };

  const sendFilterParameters = (
    enabled,
    type,
    centerFrequency,
    qFactor,
    gainDb
  ) => {
    event_manager.emitEvent(
      "set_filter",
      channel_type,
      channel_number,
      filter_id,
      enabled,
      type,
      centerFrequency,
      qFactor,
      gainDb
    );
  };

  return (
    <div className={styles.filterParameters}>
      <div className={styles.filterParameter}>
        <FilterEnabled
          active={enabled}
          filter_id={filter_id}
          onClick={handleEnabledClick}
        />
      </div>
      <div className={styles.filterParameter}>
        <CustomSelect
          active={enabled ? true : false}
          options={[
            { label: "Low Pass", value: "lowpass" },
            { label: "Peaking", value: "peaking" },
            { label: "Notch", value: "notch" },
            { label: "High Pass", value: "highpass" },
          ]}
          value={type}
          onChange={handleTypeChange}
        />
      </div>
      <div className={styles.filterParameter}>
        <FilterTextInput
          range={{ min: 20, max: 20000, default: 1000 }}
          active={enabled ? true : false}
          value={centerFrequency}
          onChange={handleCenterFrequencyChange}
          placeholder="20 to 20000"
          dragExponential={true}
        />
      </div>
      <div className={styles.filterParameter}>
        <FilterTextInput
          range={{ min: 0.1, max: 18.0, default: 0.707 }}
          active={enabled ? true : false}
          value={qFactor}
          onChange={handleQFactorChange}
          placeholder="0.1 to 18.0"
          dragExponential={false}
        />
      </div>
      <div className={styles.filterParameter}>
        <FilterTextInput
          range={{ min: -20.0, max: 20.0, default: 0 }}
          active={type === "peaking" && enabled ? true : false}
          value={gainDb}
          onChange={handleGainDbChange}
          placeholder="-20.0 to 20.0"
          dragExponential={false}
        />
      </div>
    </div>
  );
};

export default FilterParameters;
