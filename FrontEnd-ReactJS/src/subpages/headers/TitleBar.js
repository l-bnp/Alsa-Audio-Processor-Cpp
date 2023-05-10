import React, { useEffect, useState } from "react";
import styles from "./TitleBar.module.css";

const TitleBar = (props) => {
  return (
    <div className={styles.titleBar}>
      <div className={styles.titleLabel}>{props.title}</div>
    </div>
  );
};

export default TitleBar;
