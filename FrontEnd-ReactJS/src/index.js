import React from "react";
import ReactDOM from "react-dom";
import "./index.css";
import App from "./App";
import "./fonts/fonts.css";
import "./fonts/Roboto-Regular.ttf";
import "./fonts/Roboto-Bold.ttf";

const rootElement = document.getElementById("root");

ReactDOM.render(
  <React.StrictMode>
    <App className="appclass"></App>
  </React.StrictMode>,
  rootElement
);
