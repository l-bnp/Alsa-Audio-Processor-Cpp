import React from "react";
import styles from "./Footer.module.css";
import FooterButton from "../controls/FooterButton";

const Footer = () => {
  return (
    <footer className={styles.footer}>
      <div className={styles.centerButtons}>
        <FooterButton btnType="home" />
      </div>
      <div className={styles.rightButtons}>
        <FooterButton btnType="config" />
      </div>
    </footer>
  );
};

export default Footer;
