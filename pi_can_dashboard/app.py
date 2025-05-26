from flask import Flask, render_template, jsonify
from threading import Thread
import can
from collections import deque
import csv
import os

app = Flask(__name__)
buffer = deque(maxlen=100)

# Label mapping
ID_LABELS = {
    "0x321": "STM32 Test",
    "0x110": "High Beam",
    "0x120": "Battery Warning",
    "0x130": "Crash Trigger",
    "0x140": "Temperature Sensor",
    "0x150": "Blinker"
}

# Ensure logs folder exists
LOG_PATH = "logs/can_log.csv"
os.makedirs("logs", exist_ok=True)

# Create log file with header if it doesn't exist
if not os.path.exists(LOG_PATH):
    with open(LOG_PATH, "w", newline='') as f:
        writer = csv.writer(f)
        writer.writerow(["timestamp", "id", "label", "data"])

def log_to_csv(msg):
    with open(LOG_PATH, "a", newline='') as f:
        writer = csv.writer(f)
        writer.writerow([msg["timestamp"], msg["id"], ID_LABELS.get(msg["id"], "Unknown"), msg["data"]])

def can_listener():
    bus = can.interface.Bus(channel='can0', bustype='socketcan')
    while True:
        msg = bus.recv()
        entry = {
            "id": hex(msg.arbitration_id),
            "data": msg.data.hex(),
            "timestamp": msg.timestamp
        }
        buffer.append(entry)
        log_to_csv(entry)

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/api/can")
def api_can():
    def label_msg(msg):
        return {
            "id": msg["id"],
            "label": ID_LABELS.get(msg["id"], "Unknown"),
            "data": msg["data"],
            "timestamp": msg["timestamp"]
        }
    return jsonify([label_msg(m) for m in buffer])

if __name__ == "__main__":
    Thread(target=can_listener, daemon=True).start()
    app.run(host="0.0.0.0", port=5000)
