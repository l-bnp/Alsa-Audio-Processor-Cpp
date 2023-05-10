// ConnectingLine.js
import React from "react";
import styles from "./ConnectingLine.module.css";

const ConnectingLine = () => {
  return (
    <div className={styles.container}>
      <div className={styles.leftCircle}></div>
      <div className={styles.connectingLine}></div>
      <div className={styles.rightCircle}></div>
    </div>
  );
};

export default ConnectingLine;
