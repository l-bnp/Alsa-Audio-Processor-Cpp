import React from "react";
import styles from "./PortComponent.module.css";
import { useNavigate } from "react-router-dom";

const PortComponent = (props) => {
  const navigate = useNavigate();

  const handleClick = () => {
    navigate("/port", {
      state: {
        channel_type: props.channel_type,
      },
    });
  };

  let portLabel = "";

  if (props.channel_type == "input") {
    portLabel = "In";
  } else if (props.channel_type == "output") {
    portLabel = "Out";
  }

  return (
    <div className={styles.portComponent} onClick={handleClick}>
      <div className={styles.portLabel}>{portLabel} 1</div>
      <div className={styles.portLabel}>{portLabel} 2</div>
      <div className={styles.portLabel}>{portLabel} 3</div>
      <div className={styles.portLabel}>{portLabel} 4</div>
      <div className={styles.portLabel}>{portLabel} 5</div>
      <div className={styles.portLabel}>{portLabel} 6</div>
      <div className={styles.portLabel}>{portLabel} 7</div>
      <div className={styles.portLabel}>{portLabel} 8</div>
    </div>
  );
};

export default PortComponent;
