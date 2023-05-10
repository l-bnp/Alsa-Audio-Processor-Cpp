import React from "react";
import styles from "./MixerComponent.module.css";
import { ReactComponent as ComponentIcon } from "../../icons/Icon-Mixer.svg";
import { useNavigate } from "react-router-dom";

const MixerComponent = (props) => {
  const navigate = useNavigate();

  const handleClick = () => {
    navigate("/mixer", {
      state: {},
    });
  };

  return (
    <div className={styles.mixerComponent} onClick={handleClick}>
      <ComponentIcon className={styles.iconClass} />
    </div>
  );
};

export default MixerComponent;
