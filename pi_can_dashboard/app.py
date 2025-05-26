from flask import Flask, render_template, jsonify
from threading import Thread
import can
from collections import deque

app = Flask(__name__)
buffer = deque(maxlen=100)

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
    return jsonify(list(buffer))

if __name__ == "__main__":
    Thread(target=can_listener, daemon=True).start()
    app.run(host="0.0.0.0", port=5000)
