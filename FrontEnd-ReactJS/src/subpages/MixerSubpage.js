import React from "react";
import { useLocation } from "react-router-dom";
import TitleBar from "./headers/TitleBar";
import styles from "./MixerSubpage.module.css";
import GeneralCheckboxButton from "./controls/GeneralCheckboxButton";

const MixerSubpage = () => {
  const location = useLocation();

  const generateMatrix = () => {
    const elements = [];

    // Add the top-left empty header
    elements.push(<div key="top-left-header" className={styles.header}></div>);

    // Add the top input headers
    for (let input_channel = 1; input_channel <= 8; input_channel++) {
      elements.push(
        <div key={`input-header-${input_channel}`} className={styles.header}>
          In {input_channel}
        </div>
      );
    }

    for (let output_channel = 1; output_channel <= 8; output_channel++) {
      // Add the left output headers
      elements.push(
        <div key={`output-header-${output_channel}`} className={styles.header}>
          Out {output_channel}
        </div>
      );

      for (let input_channel = 1; input_channel <= 8; input_channel++) {
        elements.push(
          <GeneralCheckboxButton
            key={`output-${output_channel}-input-${input_channel}`}
            box_type="mixer"
            output_channel={output_channel}
            input_channel={input_channel}
          />
        );
      }
    }
    return elements;
  };

  return (
    <div className={styles.mixerSubpage}>
      <TitleBar title="Mixer" />
      <div className={styles.subpageContent}>
        <div className={styles.matrix}>{generateMatrix()}</div>
      </div>
    </div>
  );
};

export default MixerSubpage;
