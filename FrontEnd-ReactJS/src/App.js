import React from "react";
import Header from "./pages/headers/Header";
import Footer from "./pages/headers/Footer";
import AudioPage from "./pages/AudioPage";
import GainSubpage from "./subpages/GainSubpage";
import EqualizerSubpage from "./subpages/EqualizerSubpage";
import PortSubpage from "./subpages/PortSubpage";
import MixerSubpage from "./subpages/MixerSubpage";
import ConfigSubpage from "./subpages/ConfigSubpage";
import styles from "./App.module.css";
import { BrowserRouter as Router, Route, Routes } from "react-router-dom";
import EventManagerContext from "./Utilities/EventManagerContext";
import EventManager from "./Utilities/EventManager";
import WebSocketClientCustom from "./Utilities/WebSocketClientCustom";

const event_manager = new EventManager();
const websocketClient = new WebSocketClientCustom(
  "ws://192.168.17.91:3001",
  event_manager
);

function App() {
  return (
    <EventManagerContext.Provider value={event_manager}>
      <Router>
        <div className={styles.appContainer}>
          <Header />
          <main>
            <Routes>
              <Route path="/" element={<AudioPage />} />
              <Route path="/gain" element={<GainSubpage />} />
              <Route path="/equalizer" element={<EqualizerSubpage />} />
              <Route path="/port" element={<PortSubpage />} />
              <Route path="/mixer" element={<MixerSubpage />} />
              <Route path="/config" element={<ConfigSubpage />} />
            </Routes>
          </main>
          <Footer />
        </div>
      </Router>
    </EventManagerContext.Provider>
  );
}

export default App;
