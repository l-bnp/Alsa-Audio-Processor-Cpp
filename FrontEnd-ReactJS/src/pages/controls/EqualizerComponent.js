import React from "react";
import styles from "./EqualizerComponent.module.css";
import { ReactComponent as ComponentIcon } from "../../icons/Icon-EQ.svg";
import { useNavigate } from "react-router-dom";

const EqualizerComponent = (props) => {
  const navigate = useNavigate();

  const handleClick = () => {
    navigate("/equalizer", {
      state: {
        channel_type: props.channel_type,
        channel_number: props.channel_number,
      },
    });
  };

  return (
    <div className={styles.equalizerComponent} onClick={handleClick}>
      <ComponentIcon className={styles.iconClass} />
    </div>
  );
};

export default EqualizerComponent;
