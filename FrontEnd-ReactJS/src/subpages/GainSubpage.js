import React from "react";
import { useLocation } from "react-router-dom";
import styles from "./GainSubpage.module.css";
import TitleBar from "./headers/TitleBar";
import GeneralSlider from "./controls/GeneralSlider";
import GeneralCheckboxButton from "./controls/GeneralCheckboxButton";

const GainSubpage = (props) => {
  const location = useLocation();
  const { channel_type, channel_number } = location.state;

  function capitalizeFirstLetter(string) {
    return string.charAt(0).toUpperCase() + string.slice(1);
  }

  return (
    <div className={styles.gainSubpage}>
      <TitleBar
        title={capitalizeFirstLetter(channel_type) + " " + channel_number}
      />
      <div className={styles.gainSubpageContent}>
        <div className={styles.controlNamesDiv}>
          <div className={styles.controlName}>Mute</div>
          <div className={styles.controlName}>Gain</div>
        </div>
        <div className={styles.controlsDiv}>
          <div className={styles.control}>
            <GeneralCheckboxButton
              channel_type={channel_type}
              channel_number={channel_number}
              box_type="mute"
            />
          </div>
          <div className={styles.control}>
            <GeneralSlider
              channel_type={channel_type}
              channel_number={channel_number}
            />
          </div>
        </div>
      </div>
    </div>
  );
};

export default GainSubpage;
