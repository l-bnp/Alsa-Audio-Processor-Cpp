import React from "react";
import styles from "./FilterEnabled.module.css";

const FilterEnabled = ({ active, filter_id, onClick }) => {
  return (
    <div className={styles.container} onClick={onClick} data-active={active}>
      <span className={styles.text} data-active={active}>
        Filter {filter_id}
      </span>
    </div>
  );
};

export default FilterEnabled;
