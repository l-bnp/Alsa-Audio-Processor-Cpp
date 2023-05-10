import React, { useState, useEffect, useContext, useRef } from "react";
import styles from "./EQComponent.module.css";
import EventManagerContext from "../../Utilities/EventManagerContext";
import { throttle } from "lodash";
const audioContext = new (window.AudioContext || window.webkitAudioContext)();
const filterCount = 16;
const biquadFilters = Array.from({ length: filterCount }, () =>
  audioContext.createBiquadFilter()
);

const EQComponent = ({ channel_type, channel_number }) => {
  const [pathData, setPathData] = useState("");
  const svgRef = useRef();
  const [enabledFilters, setEnabledFilters] = useState(
    Array(filterCount).fill(false)
  );
  const event_manager = useContext(EventManagerContext);
  const SVG_WIDTH = 1200;
  const SVG_HEIGHT = 300;
  const MIN_FREQUENCY = 20;
  const MAX_FREQUENCY = 20000;
  const MIN_GAIN = -20;
  const MAX_GAIN = 20;
  const MIN_Q = 0.1;
  const MAX_Q = 18.0;

  const eventListeningFunction = (
    channelType,
    channelNumber,
    filterId,
    filterEnabled,
    filterType,
    filterFrequency,
    filterq_factor,
    filterGain
  ) => {
    if (
      filterId >= 1 &&
      filterId <= 16 &&
      channelType === channel_type &&
      channelNumber === channel_number
    ) {
      const filterIndex = filterId - 1;
      biquadFilters[filterIndex].type = filterType;
      biquadFilters[filterIndex].frequency.value = filterFrequency;
      biquadFilters[filterIndex].Q.value = filterq_factor;
      biquadFilters[filterIndex].gain.value = filterGain;

      let newEnabledFilters = enabledFilters;
      newEnabledFilters[filterIndex] = filterEnabled;
      setEnabledFilters(newEnabledFilters);

      const pathData = updateFrequencyResponseCurve();
      setPathData(pathData);
    }
  };

  useEffect(() => {
    if (event_manager) {
      event_manager.on(`notify_filter`, eventListeningFunction);
    }
    return () => {
      event_manager.off(`notify_filter`, eventListeningFunction);
    };
  }, []);

  const generateAxisTicks = () => {
    const xTicks = [
      20, 30, 40, 50, 60, 70, 80, 90, 100, 200, 300, 400, 500, 600, 700, 800,
      900, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 20000,
    ];

    const yTicks = Array.from(
      { length: (MAX_GAIN - MIN_GAIN) / 5 + 1 },
      (_, i) => MIN_GAIN + i * 5
    );

    const xTickElements = xTicks.map((freq) => {
      const x = frequencyToSvgX(freq);
      const displayLabel = [100, 1000, 10000].includes(freq);
      return (
        <>
          <line
            x1={x}
            y1={0}
            x2={x}
            y2={SVG_HEIGHT}
            className={styles.gridLine}
          />
          <line
            x1={x}
            y1={SVG_HEIGHT}
            x2={x}
            y2={SVG_HEIGHT - 10}
            className={styles.tickLine}
          />
          {displayLabel && (
            <text
              x={x}
              y={SVG_HEIGHT - 12}
              textAnchor="middle"
              className={styles.axisLabel}
            >
              {freq}
            </text>
          )}
        </>
      );
    });

    const yTickElements = yTicks.map((gain) => {
      const y = gainToSvgY(gain);
      const displayLabel = [-15, -10, -5, 0, 5, 10, 15].includes(gain);
      const isZeroDb = gain === 0;
      return (
        <>
          <line
            x1={0}
            y1={y}
            x2={SVG_WIDTH}
            y2={y}
            className={isZeroDb ? styles.zeroDbGridLine : styles.gridLine}
          />
          <line x1={0} y1={y} x2={10} y2={y} className={styles.tickLine} />
          {displayLabel && (
            <text
              x={12}
              y={y}
              textAnchor="start"
              dominantBaseline="middle"
              className={styles.axisLabel}
            >
              {gain}
            </text>
          )}
        </>
      );
    });

    return (
      <>
        {xTickElements}
        {yTickElements}
      </>
    );
  };

  const frequencyToSvgX = (freq) => {
    const x =
      ((Math.log10(freq) - Math.log10(MIN_FREQUENCY)) /
        (Math.log10(MAX_FREQUENCY) - Math.log10(MIN_FREQUENCY))) *
      SVG_WIDTH;
    return x;
  };

  const gainToSvgY = (gain) => {
    const y = ((MAX_GAIN - gain) / (MAX_GAIN - MIN_GAIN)) * SVG_HEIGHT;
    return y;
  };

  const qFactorToSvgY = (qFactor) => {
    const y = ((MAX_Q - qFactor) / (MAX_Q - MIN_Q)) * SVG_HEIGHT;
    return y;
  };

  const svgXToFrequency = (x) => {
    const freq = Math.pow(
      10,
      (x / SVG_WIDTH) *
        (Math.log10(MAX_FREQUENCY) - Math.log10(MIN_FREQUENCY)) +
        Math.log10(MIN_FREQUENCY)
    );
    return freq;
  };

  const svgYToGain = (y) => {
    const gain = MAX_GAIN - (y / SVG_HEIGHT) * (MAX_GAIN - MIN_GAIN);
    return gain;
  };

  const svgYToQFactor = (y) => {
    const qFactor = MAX_Q - (y / SVG_HEIGHT) * (MAX_Q - MIN_Q);
    return qFactor;
  };

  const dragStart = (e, index) => {
    e.preventDefault();
    const moveListener = throttle((e) => dragMove(e, index), 50); // 50 ms
    const endListener = (e) => dragEnd(e, moveListener, endListener);

    if (e.type === "mousedown") {
      document.addEventListener("mousemove", moveListener);
      document.addEventListener("mouseup", endListener);
    } else if (e.type === "touchstart") {
      document.addEventListener("touchmove", moveListener);
      document.addEventListener("touchend", endListener);
    }
  };

  const dragEnd = (e, moveListener, endListener) => {
    if (e.type === "mouseup") {
      document.removeEventListener("mousemove", moveListener);
      document.removeEventListener("mouseup", endListener);
    } else if (e.type === "touchend") {
      document.removeEventListener("touchmove", moveListener);
      document.removeEventListener("touchend", endListener);
    }
  };

  const dragMove = (e, index) => {
    const svgBounds = svgRef.current.getBoundingClientRect();
    const clientX = e.clientX || (e.touches && e.touches[0].clientX) || 0;
    const clientY = e.clientY || (e.touches && e.touches[0].clientY) || 0;
    const mouseX = clientX - svgBounds.left;
    const mouseY = clientY - svgBounds.top;

    const clampedMouseX = Math.max(0, Math.min(SVG_WIDTH, mouseX));
    const clampedMouseY = Math.max(0, Math.min(SVG_HEIGHT, mouseY));

    const newCenterFrequency = svgXToFrequency(clampedMouseX);
    let newGain = biquadFilters[index].gain.value;
    let newQFactor = biquadFilters[index].Q.value;

    if (biquadFilters[index].type == "peaking") {
      newGain = svgYToGain(clampedMouseY);
    } else {
      newQFactor = svgYToQFactor(clampedMouseY);
    }

    // Send a "set_filter" event to the event manager
    event_manager.emitEvent(
      "set_filter",
      channel_type,
      channel_number,
      index + 1,
      enabledFilters[index],
      biquadFilters[index].type,
      newCenterFrequency,
      newQFactor,
      newGain
    );
  };

  const renderCircles = () => {
    return biquadFilters.map((filter, index) => {
      if (!enabledFilters[index]) return null;

      const filterId = index + 1;
      const freq = filter.frequency.value || 125 * filterId;
      const x = frequencyToSvgX(freq);
      let y;

      if (filter.type === "peaking") {
        y = gainToSvgY(filter.gain.value);
      } else {
        y = qFactorToSvgY(filter.Q.value);
      }

      return (
        <g key={filterId}>
          <circle
            onMouseDown={(e) => dragStart(e, index)}
            onTouchStart={(e) => dragStart(e, index)}
            cx={x}
            cy={y}
            r={7}
            className={styles.circle}
          />
          <text
            x={x}
            y={y}
            textAnchor="middle"
            dominantBaseline="middle"
            className={styles.circleLabel}
          >
            {filterId}
          </text>
        </g>
      );
    });
  };

  const updateFrequencyResponseCurve = () => {
    const numPoints = 1998;
    const curvePoints = [];

    // Create a Float32Array to store the frequency values
    const frequencies = new Float32Array(numPoints + 1);
    for (let i = 0; i <= numPoints; i++) {
      const freq = svgXToFrequency((i / numPoints) * SVG_WIDTH);
      frequencies[i] = freq;
    }

    // Get the frequency response of the filters
    const magResponseArray = biquadFilters.map(
      () => new Float32Array(frequencies.length)
    );
    const phaseResponseArray = biquadFilters.map(
      () => new Float32Array(frequencies.length)
    );

    //console.log(`MagResponseArray: ${magResponseArray}`);

    biquadFilters.forEach((filter, index) => {
      filter.getFrequencyResponse(
        frequencies,
        magResponseArray[index],
        phaseResponseArray[index]
      );
    });

    // Calculate the combined magnitude response considering only enabled filters
    const combinedMagResponse = magResponseArray[0].map((_, i) =>
      magResponseArray.reduce(
        (acc, magResponse, index) =>
          acc * (enabledFilters[index] ? magResponse[i] : 1),
        1
      )
    );

    //console.log(`CombinedMagResponse: ${combinedMagResponse}`);

    // Convert the magnitude response values to decibels
    const gainResponse = combinedMagResponse.map(
      (value) => 20 * Math.log10(value)
    );

    for (let i = 0; i < gainResponse.length; i++) {
      const x = frequencyToSvgX(frequencies[i]);
      const y = gainToSvgY(gainResponse[i]);
      curvePoints.push({ x, y });
    }

    // Render the curve using SVG path
    const pathData = curvePoints
      .map((point, i) =>
        i === 0 ? `M ${point.x} ${point.y}` : `L ${point.x} ${point.y}`
      )
      .join(" ");

    return pathData;
  };

  return (
    <div>
      <svg
        ref={svgRef}
        width={SVG_WIDTH}
        height={SVG_HEIGHT}
        className={styles.svg}
      >
        {/* Set background rectangle */}
        <rect width="100%" height="100%" className={styles.background} />

        {/* Render frequency and gain gridlines, ticks, and labels */}
        {generateAxisTicks()}

        {/* Frequency response curve with custom line color and size */}
        <path className={styles.frequencyResponseCurve} d={pathData} />

        {/* Render circles */}
        {renderCircles()}
      </svg>
    </div>
  );
};

export default EQComponent;
