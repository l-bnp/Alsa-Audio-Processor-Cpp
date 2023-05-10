import React, { useEffect, useContext } from "react";
import { useLocation } from "react-router-dom";
import styles from "./EqualizerSubpage.module.css";
import FilterParameters from "./controls/FilterParameters";
import TitleBar from "./headers/TitleBar";
import EventManagerContext from "../Utilities/EventManagerContext";
import EQComponent from "./controls/EQComponent";

const NUMBER_OF_FILTERS = 16;

const EqualizerSubpage = (props) => {
  const location = useLocation();
  const event_manager = useContext(EventManagerContext);
  const { channel_type, channel_number } = location.state;

  useEffect(() => {
    const emitGetFilter = () => {
      // Emit the "get_filter" event for each filter
      for (let i = 1; i <= NUMBER_OF_FILTERS; i++) {
        event_manager.emitEvent("get_filter", channel_type, channel_number, i);
      }
    };
    emitGetFilter();
    return () => {};
  }, []);

  function capitalizeFirstLetter(string) {
    return string.charAt(0).toUpperCase() + string.slice(1);
  }

  const generateGrid = (initialIndex) => {
    const elements = [];

    const headers = [
      "Filter Number",
      "Type",
      "Frequency (Hz)",
      "Q Factor",
      "Gain (dB)",
    ];

    const headerRow = (
      <div className={styles.headerRow}>
        {headers.map((header, index) => (
          <div key={`header-${index}`} className={styles.header}>
            {header}
          </div>
        ))}
      </div>
    );

    elements.push(headerRow);

    for (let i = initialIndex; i < NUMBER_OF_FILTERS / 2 + initialIndex; i++) {
      elements.push(
        <div key={`row-${i}`}>
          <FilterParameters
            filter_id={i}
            channel_type={channel_type}
            channel_number={channel_number}
          />
        </div>
      );
    }

    return elements;
  };

  return (
    <div className={styles.equalizerSubpage}>
      <TitleBar
        title={capitalizeFirstLetter(channel_type) + " " + channel_number}
      />
      <div className={styles.equalizerSubpageContent}>
        <div className={styles.eqresponse}>
          <EQComponent
            channel_type={channel_type}
            channel_number={channel_number}
          />
        </div>
        <div className={styles.matrixes}>
          <div className={styles.matrix}>{generateGrid(1)}</div>
          <div className={styles.matrix}>{generateGrid(9)}</div>
        </div>
      </div>
    </div>
  );
};

export default EqualizerSubpage;
