import React from "react";
import styles from "./AudioPage.module.css";
import PortComponent from "./controls/PortComponent";
import GainComponent from "./controls/GainComponent";
import EqualizerComponent from "./controls/EqualizerComponent";
import MixerComponent from "./controls/MixerComponent";
import ConnectingLine from "./controls/ConnectingLine";

const AudioPage = () => {
  const renderConnectingLines = () => {
    const connectingLines = [];
    for (let i = 0; i < 8; i++) {
      connectingLines.push(<ConnectingLine key={i} />);
    }
    return connectingLines;
  };

  const renderEqualizerComponents = (channel_type) => {
    const equalizers = [];
    for (let channel_number = 1; channel_number <= 8; channel_number++) {
      equalizers.push(
        <EqualizerComponent
          key={`${channel_type}-${channel_number}`}
          channel_type={channel_type}
          channel_number={channel_number}
        />
      );
    }
    return equalizers;
  };

  const renderGainComponents = (channel_type) => {
    const gains = [];
    for (let channel_number = 1; channel_number <= 8; channel_number++) {
      gains.push(
        <GainComponent
          key={`${channel_type}-${channel_number}`}
          channel_type={channel_type}
          channel_number={channel_number}
        />
      );
    }
    return gains;
  };

  return (
    <div className={styles.audioPage}>
      {/* Control Name Headers */}
      <div className={styles.audioHeader}>
        <div className={styles.audioHeaderFlexBox}>
          <div className={styles.headerLabel}>Inputs</div>
          <div className={styles.headerLabel}>EQ. In.</div>
          <div className={styles.headerLabel}>Gain In.</div>
          <div className={styles.headerLabel}>Mixer</div>
          <div className={styles.headerLabel}>EQ Out.</div>
          <div className={styles.headerLabel}>Gain Out.</div>
          <div className={styles.headerLabel}>Outputs</div>
        </div>
      </div>

      <div className={styles.audioMainPart}>
        {/* Actual Control Buttons Area */}
        <div className={styles.audioControls}>
          {/* Inputs Block */}
          <div className={styles.componentColumn}>
            <PortComponent channel_type="input" />
          </div>
          <div className={styles.lineColumn}>{renderConnectingLines()}</div>
          {/* Input Equalizers */}
          <div className={styles.componentColumn}>
            {renderEqualizerComponents("input")}
          </div>
          <div className={styles.lineColumn}>{renderConnectingLines()}</div>
          {/* Input Gain */}
          <div className={styles.componentColumn}>
            {renderGainComponents("input")}
          </div>
          <div className={styles.lineColumn}>{renderConnectingLines()}</div>
          {/* Mixer */}
          <div className={styles.componentColumn}>
            <MixerComponent />
          </div>
          <div className={styles.lineColumn}>{renderConnectingLines()}</div>
          {/* Output Equalizers */}
          <div className={styles.componentColumn}>
            {renderEqualizerComponents("output")}
          </div>
          <div className={styles.lineColumn}>{renderConnectingLines()}</div>
          {/* Output Gain */}
          <div className={styles.componentColumn}>
            {renderGainComponents("output")}
          </div>
          <div className={styles.lineColumn}>{renderConnectingLines()}</div>
          {/* Outputs Block */}
          <div className={styles.componentColumn}>
            <PortComponent channel_type="output" />
          </div>
        </div>
      </div>
    </div>
  );
};

export default AudioPage;
