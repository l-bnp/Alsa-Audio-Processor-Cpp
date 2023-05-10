import React, { useState, useRef, useEffect, useContext } from "react";
import styles from "./GeneralSlider.module.css";
import EventManagerContext from "../../Utilities/EventManagerContext";

// VOLUME_MAX_VALUE SHOULD BE <= 0
const SLIDER_MIN_VALUE = 0.0;
const SLIDER_MAX_VALUE = 100.0;
const VOLUME_MIN_VALUE = -60.0;
const VOLUME_MAX_VALUE = 0.0;

const GeneralSlider = (props) => {
  const [sliderValue, setSliderValue] = useState(SLIDER_MIN_VALUE);
  const [volumeValue, setVolumeValue] = useState(VOLUME_MIN_VALUE);
  const sliderTrack = useRef(null);
  const sliderHandle = useRef(null);
  const sliderFill = useRef(null);
  const event_manager = useContext(EventManagerContext);

  const convertSliderToVolume = (sliderValue) => {
    // scale sliderValue between 0 and 100
    let sliderPercentage =
      ((sliderValue - SLIDER_MIN_VALUE) * 100) /
      (SLIDER_MAX_VALUE - SLIDER_MIN_VALUE);

    // clamp sliderpercentage between 0 and 100
    sliderPercentage = Math.max(Math.min(sliderPercentage, 100), 0);

    // A = VOLUME_MIN_VALUE; A != 0
    // B = (1/100) * log(VOLUME_MAX_VALUE/VOLUME_MIN_VALUE); B != 0
    // y = (A)*exp((B)*(x))
    // volume_db = (VOLUME_MIN_VALUE)*exp((1/100) * log(VOLUME_MAX_VALUE/VOLUME_MIN_VALUE)*(sliderPercentage))
    const volume_db =
      (VOLUME_MIN_VALUE - 1.0) *
      Math.exp(
        (1 / 100) *
          Math.log((VOLUME_MAX_VALUE - 1.0) / (VOLUME_MIN_VALUE - 1.0)) *
          sliderPercentage
      );

    // clamp between min and max
    /* volume_db = Math.max(
      Math.min(volume_db, VOLUME_MAX_VALUE),
      VOLUME_MIN_VALUE
    ); */

    return volume_db + 1;
  };

  //

  const convertVolumeToSlider = (volume_db) => {
    // clamp between min and max
    volume_db = Math.max(
      Math.min(volume_db, VOLUME_MAX_VALUE),
      VOLUME_MIN_VALUE
    );

    // A = VOLUME_MIN_VALUE; A != 0
    // B = (1/100) * log(VOLUME_MAX_VALUE/VOLUME_MIN_VALUE); B != 0
    // y = log(x/(A))/(B)
    // sliderValue = log(volume_db/(VOLUME_MIN_VALUE))/((1/100) * log(VOLUME_MAX_VALUE/VOLUME_MIN_VALUE))
    let sliderValue =
      Math.log((volume_db - 1) / (VOLUME_MIN_VALUE - 1)) /
      ((1 / 100) * Math.log((VOLUME_MAX_VALUE - 1) / (VOLUME_MIN_VALUE - 1)));

    // scale sliderValue from the [0, 100] range to the [SLIDER_MIN_VALUE, SLIDER_MAX_VALUE] range
    sliderValue =
      SLIDER_MIN_VALUE +
      (sliderValue * (SLIDER_MAX_VALUE - SLIDER_MIN_VALUE)) / 100;

    // clamp sliderValue between min and max
    /* sliderValue = Math.max(
      Math.min(sliderValue, SLIDER_MAX_VALUE),
      SLIDER_MIN_VALUE
    ); */

    return sliderValue;
  };

  useEffect(() => {
    updateSlider(sliderValue);
  }, [sliderValue]);

  useEffect(() => {
    const handleNotifyVolume = (channel_type, channel_number, volume_db) => {
      if (
        channel_type === props.channel_type &&
        channel_number === props.channel_number
      ) {
        volume_db = Math.max(
          Math.min(volume_db, VOLUME_MAX_VALUE),
          VOLUME_MIN_VALUE
        );
        setVolumeValue(volume_db);
        setSliderValue(convertVolumeToSlider(volume_db));
      }
    };

    if (event_manager) {
      event_manager.on(`notify_gain`, handleNotifyVolume);
      event_manager.emitEvent(
        `get_gain`,
        props.channel_type,
        props.channel_number
      );
    }

    return () => {
      if (event_manager) {
        event_manager.off(`notify_gain`, handleNotifyVolume);
      }
    };
  }, []);

  const updateSlider = (newValue) => {
    const sliderHeight = sliderTrack.current.offsetHeight;
    const handleHeight = sliderHandle.current.offsetHeight;
    const maxTop = sliderHeight - handleHeight;
    const percentage =
      (newValue - SLIDER_MIN_VALUE) / (SLIDER_MAX_VALUE - SLIDER_MIN_VALUE);
    const top = (1 - percentage) * maxTop;

    sliderFill.current.style.height = `${percentage * 100}%`;
    sliderHandle.current.style.top = `${top}px`;
  };

  const handleStart = (event) => {
    if (event.type === "mousedown") {
      window.addEventListener("mousemove", handleMove);
      window.addEventListener("mouseup", handleEnd);
    } else {
      window.addEventListener("touchmove", handleMove);
      window.addEventListener("touchend", handleEnd);
    }
  };

  const handleMove = (event) => {
    event.preventDefault();
    const clientY = event.clientY || event.touches[0].clientY;
    const rect = sliderTrack.current.getBoundingClientRect();
    const sliderHeight = sliderTrack.current.offsetHeight;
    const handleHeight = sliderHandle.current.offsetHeight;

    let percentage =
      (sliderHeight - (clientY - rect.top) - handleHeight / 2) /
      (sliderHeight - handleHeight);

    percentage = Math.max(0, Math.min(1, percentage.toFixed(4))); // Clamp percentage between 0 and 1

    let newValue =
      SLIDER_MIN_VALUE + percentage * (SLIDER_MAX_VALUE - SLIDER_MIN_VALUE);

    // setValue(newValue);
    event_manager.emitEvent(
      `set_gain`,
      props.channel_type,
      props.channel_number,
      convertSliderToVolume(newValue)
    );
  };

  const handleEnd = (event) => {
    if (event.type === "mouseup") {
      window.removeEventListener("mousemove", handleMove);
      window.removeEventListener("mouseup", handleEnd);
    } else {
      window.removeEventListener("touchmove", handleMove);
      window.removeEventListener("touchend", handleEnd);
    }
  };

  return (
    <div className={styles.componentContainer}>
      <div className={styles.sliderContainer}>
        <div ref={sliderTrack} className={styles.sliderTrack}>
          <div ref={sliderFill} className={styles.sliderFill}></div>
          <div
            ref={sliderHandle}
            className={styles.sliderHandle}
            onMouseDown={handleStart}
            onTouchStart={handleStart}
          ></div>
        </div>
      </div>
      <div className={styles.valueContainer}>{Math.round(volumeValue)}</div>
    </div>
  );
};

export default GeneralSlider;
