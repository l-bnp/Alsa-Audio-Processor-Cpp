import React, { useEffect, useState } from "react";
import styles from "./Header.module.css";
import ConnectionSign from "../controls/ConnectionSign";

const Header = () => {
  const [time, setTime] = useState("");
  const [date, setDate] = useState("");

  useEffect(() => {
    const updateDateTime = () => {
      const now = new Date();
      const hours = String(now.getHours()).padStart(2, "0");
      const minutes = String(now.getMinutes()).padStart(2, "0");
      const day = String(now.getDate()).padStart(2, "0");
      const month = String(now.getMonth() + 1).padStart(2, "0");
      const year = now.getFullYear();

      setTime(`${hours}:${minutes}`);
      setDate(`${day}/${month}/${year}`);
    };

    updateDateTime();
    const timer = setInterval(updateDateTime, 1000);

    return () => {
      clearInterval(timer);
    };
  }, []);

  return (
    <header className={styles.header}>
      <div className={styles.leftText}>
        <div className={styles.connectionSign}>
          <ConnectionSign />{" "}
        </div>
        <div className={styles.ambientLabel}>Audio Processor</div>
      </div>
      <div className={styles.time}>{time}</div>
      <div className={styles.date}>{date}</div>
    </header>
  );
};

export default Header;
