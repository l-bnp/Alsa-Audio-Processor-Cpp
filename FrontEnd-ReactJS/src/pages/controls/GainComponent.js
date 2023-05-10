import React from "react";
import styles from "./GainComponent.module.css";
import { ReactComponent as ComponentIcon } from "../../icons/Icon-Gain.svg";
import { useNavigate } from "react-router-dom";

const GainComponent = (props) => {
  const navigate = useNavigate();

  const handleClick = () => {
    navigate("/gain", {
      state: {
        channel_type: props.channel_type,
        channel_number: props.channel_number,
      },
    });
  };

  return (
    <div className={styles.gainComponent} onClick={handleClick}>
      <ComponentIcon className={styles.iconClass} />
    </div>
  );
};

export default GainComponent;
