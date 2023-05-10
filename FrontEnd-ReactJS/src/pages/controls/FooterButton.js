import React from "react";
import styles from "./FooterButton.module.css";
import { useNavigate, useLocation } from "react-router-dom";

import { ReactComponent as HomeOffIcon } from "../../icons/Icon-Home-Off.svg";
import { ReactComponent as HomeOnIcon } from "../../icons/Icon-Home-On.svg";
import { ReactComponent as ConfigOffIcon } from "../../icons/Icon-Config-Off.svg";
import { ReactComponent as ConfigOnIcon } from "../../icons//Icon-Config-On.svg";

const FooterButton = ({ btnType }) => {
  const navigate = useNavigate();
  const location = useLocation();

  const offIcons = {
    home: <HomeOffIcon className={styles.buttonIcon} />,
    config: <ConfigOffIcon className={styles.buttonIcon} />,
  };
  const onIcons = {
    home: <HomeOnIcon className={styles.buttonIcon} />,
    config: <ConfigOnIcon className={styles.buttonIcon} />,
  };
  const activePath = {
    home: "/",
    config: "/config",
  };

  const isActive = location.pathname === activePath[btnType];

  let ButtonIcon;

  if (isActive) {
    ButtonIcon = onIcons[btnType];
  } else {
    ButtonIcon = offIcons[btnType];
  }

  const handleClick = () => {
    navigate(activePath[btnType]);
  };

  return (
    <button
      className={styles.footerButton}
      onClick={() => handleClick()}
      data-active={isActive}
    >
      {ButtonIcon}
    </button>
  );
};

export default FooterButton;
