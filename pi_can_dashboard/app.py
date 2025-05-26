from flask import Flask, render_template, jsonify
from threading import Thread
import can
from collections import deque

app = Flask(__name__)
buffer = deque(maxlen=100)

# CAN ID → Label mappings
ID_LABELS = {
    "0x321": "STM32 Test",
    "0x110": "High Beam",
    "0x120": "Battery Warning",
    "0x130": "Crash Trigger",
    "0x140": "Temperature Sensor",
    "0x150": "Blinker"
}

def can_listener():
    bus = can.interface.Bus(channel='can0', bustype='socketcan')
    while True:
        msg = bus.recv()
        buffer.append({
            "id": hex(msg.arbitration_id),
            "data": msg.data.hex(),
            "timestamp": msg.timestamp
        })

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
