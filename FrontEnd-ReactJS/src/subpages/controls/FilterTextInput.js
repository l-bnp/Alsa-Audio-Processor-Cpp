import React, { useState, useRef, useEffect } from "react";
import styles from "./FilterTextInput.module.css";

const FilterTextInput = ({
  range,
  active,
  value,
  onChange,
  placeholder,
  dragExponential,
}) => {
  const [inputValue, setInputValue] = useState(value || "");
  const inputRef = useRef(null);

  useEffect(() => {
    setInputValue(value || "");
  }, [value]);

  const handleFocus = () => {
    inputRef.current.focus();
  };

  const handleChange = (event) => {
    const { value } = event.target;
    const regex = /^-?[0-9]*[.]?[0-9]*$/;
    if (regex.test(value)) {
      setInputValue(value);
    }
  };

  const handleKeyDown = (event) => {
    let newValue = event.target.value;

    if (newValue < range.min) {
      newValue = range.min;
    } else if (newValue > range.max) {
      newValue = range.max;
    } else if (newValue >= range.min && newValue <= range.max) {
      newValue = newValue;
    } else {
      newValue = range.default;
    }

    if (event.key === "Enter") {
      setInputValue(newValue);
      if (onChange) onChange(newValue);
    }
  };

  // Throttle function
  const throttle = (func, delay) => {
    let timeout;
    return function (...args) {
      if (!timeout) {
        timeout = setTimeout(() => {
          func.apply(this, args);
          timeout = null;
        }, delay);
      }
    };
  };

  const handleDragUpdate = (startY, startValue, event) => {
    const deltaY = startY - event.clientY;
    let newValue;

    if (dragExponential) {
      const ratio = deltaY / 200;
      newValue = startValue * Math.pow(range.max / range.min, ratio);
    } else {
      newValue = startValue + (deltaY / 200) * (range.max - range.min);
    }

    const clampedValue = Math.min(Math.max(newValue, range.min), range.max);

    setInputValue(clampedValue.toFixed(2));
    if (onChange) onChange(Number(clampedValue.toFixed(2)));
  };

  // New drag-related functions
  const handleMouseDown = (event) => {
    const startY = event.clientY;
    const startValue = parseFloat(inputValue || range.default);

    const handleMouseMove = throttle((event) => {
      handleDragUpdate(startY, startValue, event);
    }, 50);

    const handleMouseUp = () => {
      document.removeEventListener("mousemove", handleMouseMove);
      document.removeEventListener("mouseup", handleMouseUp);
    };

    document.addEventListener("mousemove", handleMouseMove);
    document.addEventListener("mouseup", handleMouseUp);
  };

  const handleTouchStart = (event) => {
    const startY = event.clientY;
    const startValue = parseFloat(inputValue);

    const handleTouchMove = throttle((event) => {
      handleDragUpdate(startY, startValue, event);
    }, 50);

    const handleTouchEnd = () => {
      document.removeEventListener("touchmove", handleTouchMove);
      document.removeEventListener("touchend", handleTouchEnd);
    };

    document.addEventListener("touchmove", handleTouchMove);
    document.addEventListener("touchend", handleTouchEnd);
  };

  return (
    <div
      className={styles.container}
      data-active={active}
      onClick={handleFocus}
    >
      <input
        ref={inputRef}
        className={styles.input}
        type="text"
        value={inputValue}
        onChange={handleChange}
        onKeyDown={handleKeyDown}
        onMouseDown={handleMouseDown}
        onTouchStart={handleTouchStart} // Add touch start event listener
        placeholder={placeholder}
      />
    </div>
  );
};

export default FilterTextInput;
