import React, { useState, useEffect, useRef } from "react";
import styles from "./CustomSelect.module.css";
import { ReactComponent as DownArrowIcon } from "../../icons/Icon-Arrow-Dropdown-2.svg";

const CustomSelect = ({ active, options, value, onChange }) => {
  const [open, setOpen] = useState(false);
  const [dropUp, setDropUp] = useState(false);
  const containerRef = useRef(null);

  useEffect(() => {
    const handleClickOutside = (e) => {
      if (containerRef.current && !containerRef.current.contains(e.target)) {
        setOpen(false);
      }
    };

    window.addEventListener("mousedown", handleClickOutside);
    return () => {
      window.removeEventListener("mousedown", handleClickOutside);
    };
  }, []);

  useEffect(() => {
    const handleResize = () => {
      const rect = containerRef.current.getBoundingClientRect();
      const windowHeight = window.innerHeight;
      const dropDownHeight = options.length * 50; // 50px is the approximate height of an option, adjust accordingly

      setDropUp(rect.y + dropDownHeight > windowHeight);
    };

    window.addEventListener("resize", handleResize);
    handleResize();

    return () => {
      window.removeEventListener("resize", handleResize);
    };
  }, [containerRef, options.length]);

  const handleClick = () => {
    setOpen(!open);
  };

  const handleOptionClick = (option) => {
    onChange(option.value);
    setOpen(false);
  };

  const selectedOption = options.find((option) => option.value === value);

  const renderOption = (option, index) => {
    const isSelected = value === option.value;
    return (
      <div
        key={index}
        className={`${styles.option} ${
          isSelected ? styles.selectedOption : ""
        }`}
        onClick={() => handleOptionClick(option)}
      >
        {option.label}
      </div>
    );
  };

  return (
    <div ref={containerRef} data-active={active} className={styles.container}>
      <div className={styles.selected} onClick={handleClick}>
        {selectedOption.label}
      </div>
      {open && (
        <div className={`${styles.options} ${dropUp ? styles.dropUp : ""}`}>
          {options.map(renderOption)}
        </div>
      )}
      <DownArrowIcon className={styles.arrow} fill="currentColor" />
    </div>
  );
};

export default CustomSelect;
