class EventManager {
  constructor() {
    // Listeners object
    this.listeners = {};
  }

  // Add event listener
  on(eventName, callback) {
    if (!this.listeners[eventName]) {
      this.listeners[eventName] = [];
    }
    this.listeners[eventName].push(callback);
  }

  // Remove specific listener
  off(eventName, callback) {
    if (this.listeners[eventName]) {
      this.listeners[eventName] = this.listeners[eventName].filter(
        (cb) => cb !== callback
      );
    }
  }

  // Remove all listebers for a given event, or all listeners at all, if no eventName is given
  removeAllListeners(eventName) {
    if (eventName) {
      delete this.listeners[eventName];
    } else {
      this.listeners = {};
    }
  }

  emitEvent(eventName, ...args) {
    if (this.listeners[eventName]) {
      this.listeners[eventName].forEach((callback) => {
        callback(...args);
      });
    }
  }
}

export default EventManager;
