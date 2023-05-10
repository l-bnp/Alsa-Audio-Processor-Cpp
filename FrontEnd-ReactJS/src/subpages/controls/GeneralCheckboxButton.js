import React, { useState, useEffect, useContext } from "react";
import styles from "./GeneralCheckboxButton.module.css";
import EventManagerContext from "../../Utilities/EventManagerContext";

const GeneralCheckboxButton = (props) => {
  const [active, setActive] = useState(false);
  const event_manager = useContext(EventManagerContext);
  const box_type = props.box_type;

  let updateFunction = function (channel_type, channel_number, mute_state) {
    if (
      channel_type === props.channel_type &&
      channel_number === props.channel_number
    ) {
      setActive(mute_state);
    }
  };

  let stateRequestFunction = function () {
    event_manager.emitEvent(
      `get_${box_type}`,
      props.channel_type,
      props.channel_number
    );
  };

  let handleClickFunction = function () {
    event_manager.emitEvent(
      `set_${box_type}`,
      props.channel_type,
      props.channel_number,
      !active
    );
  };

  if (box_type == "mixer") {
    updateFunction = function (input_channel, output_channel, mixer_state) {
      if (
        input_channel === props.input_channel &&
        output_channel === props.output_channel
      ) {
        setActive(mixer_state);
      }
    };

    stateRequestFunction = function () {
      event_manager.emitEvent(
        `get_${box_type}`,
        props.input_channel,
        props.output_channel
      );
    };

    handleClickFunction = function () {
      event_manager.emitEvent(
        `set_${box_type}`,
        props.input_channel,
        props.output_channel,
        !active
      );
    };
  }

  useEffect(() => {
    if (event_manager) {
      event_manager.on(`notify_${box_type}`, updateFunction);
      stateRequestFunction();
    }
    return () => {
      event_manager.off(`notify_${box_type}`, updateFunction);
    };
  }, []);

  const handleClick = () => {
    handleClickFunction();
  };

  return (
    <div
      className={styles.checkboxButton}
      onClick={handleClick}
      data-active={active}
    ></div>
  );
};

export default GeneralCheckboxButton;
