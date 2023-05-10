// FilterApplyButton.js
import React, { useState } from "react";
import styles from "./ApplyButton.module.css";

const FilterApplyButton = ({ active, onClick }) => {
  const [pressed, setPressed] = useState(false);

  const handleMouseDown = () => {
    if (active) {
      setPressed(true);
    }
  };

  const handleMouseUp = () => {
    if (active) {
      setPressed(false);
      onClick();
    }
  };

  const handleClick = () => {
    if (active) {
      onClick();
    }
  };

  const handleTouchStart = () => {
    if (active) {
      setPressed(true);
    }
  };

  const handleTouchEnd = () => {
    if (active) {
      setPressed(false);
      onClick();
    }
  };

  return (
    <div
      className={styles.container}
      onMouseDown={handleMouseDown}
      onMouseUp={handleMouseUp}
      onMouseLeave={handleMouseUp}
      onTouchStart={handleTouchStart}
      onTouchEnd={handleTouchEnd}
      data-active={active}
      data-pressed={pressed}
    >
      <span className={styles.text} data-pressed={pressed}>
        {active ? "Apply" : "Applied"}
      </span>
    </div>
  );
};

export default FilterApplyButton;
