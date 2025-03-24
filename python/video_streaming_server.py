# References: https://medium.com/@tauseefahmad12/object-detection-using-mobilenet-ssd-e75b177567ee

import base64
from datetime import datetime as dt
import os
from queue import Queue

import cv2
from flask import Flask, render_template
from flask_sock import Sock
import numpy as np

from log_config import serial_logger


CONFIDENCE_THRESHOLD = 0.5
VIDEO_DIR = f"{os.path.expanduser('~')}/Videos"

log = serial_logger()
app = Flask(__name__)
sock = Sock(app)

prototxt = "MobileNetSSD/MobileNetSSD_deploy.prototxt"
caffe_model = "MobileNetSSD/MobileNetSSD_deploy.caffemodel"

net = cv2.dnn.readNetFromCaffe(prototxt, caffe_model)
classNames = {
    0: "background",
    1: "aeroplane",
    2: "bicycle",
    3: "bird",
    4: "boat",
    5: "bottle",
    6: "bus",
    7: "car",
    8: "cat",
    9: "chair",
    10: "cow",
    11: "diningtable",
    12: "dog",
    13: "horse",
    14: "motorbike",
    15: "person",
    16: "pottedplant",
    17: "sheep",
    18: "sofa",
    19: "train",
    20: "tvmonitor",
}


frame_queue = Queue()

####################
# HELPER FUNCTIONS #
####################


class VideoRecorder:
    """
    Manage the recording of frames recieved from a video streamer.
    """

    def __init__(self):
        self.fourcc = cv2.VideoWriter_fourcc(*"MJPG")

        recording_dir = f"{VIDEO_DIR}/purr-hibition"
        if not os.path.exists(recording_dir):
            os.makedirs(recording_dir)

        self.out = cv2.VideoWriter(
            f"{recording_dir}/purr-hibition-{dt.now().timestamp()}.avi",
            self.fourcc,
            15.0,
            (640, 480),
        )
        log.info("Video recorder initialized")

    def __del__(self):
        self.out.release()
        log.info("Video recorder resources released")

    def record_video(self, frame: np.ndarray):
        self.out.write(frame)


def encode_frame(frame: np.ndarray):
    """
    Encode a frame with base64.

    Args:
        frame (np.ndarray): A single frame from a camera capture.
    """
    _, buffer = cv2.imencode(".jpg", frame)
    return base64.b64encode(buffer).decode("utf-8")


def process_frame(frame: np.ndarray):
    """
    Analyze the frame and identify any object found by
    adding a bounding box around the object, the name of
    the object and the confidence level of the decteaction.

    Args:
        frame (np.ndarray): A single frame from a camera capture.
    """
    width = frame.shape[1]
    height = frame.shape[0]

    blob = cv2.dnn.blobFromImage(
        frame,
        scalefactor=1 / 127.5,
        size=(300, 300),
        mean=(127.5, 127.5, 127.5),
        swapRB=True,
        crop=False,
    )
    net.setInput(blob)
    detections = net.forward()

    for i in range(detections.shape[2]):
        confidence = detections[0, 0, i, 2]
        if confidence > CONFIDENCE_THRESHOLD:
            class_id = int(detections[0, 0, i, 1])

            # scale to the frame
            x_top_left = int(detections[0, 0, i, 3] * width)
            y_top_left = int(detections[0, 0, i, 4] * height)
            x_bottom_right = int(detections[0, 0, i, 5] * width)
            y_bottom_right = int(detections[0, 0, i, 6] * height)

            # draw bounding box around the detected object
            cv2.rectangle(
                frame,
                (x_top_left, y_top_left),
                (x_bottom_right, y_bottom_right),
                (0, 255, 0),
            )

            if class_id in classNames:
                label = f"{classNames[class_id]}: {confidence:.2f}"
                (w, h), t = cv2.getTextSize(label, cv2.FONT_HERSHEY_SIMPLEX, 0.5, 1)
                y_top_left = max(y_top_left, h)
                cv2.rectangle(
                    frame,
                    (x_top_left, y_top_left - h),
                    (x_top_left + w, y_top_left + t),
                    (0, 0, 0),
                    cv2.FILLED,
                )
                cv2.putText(
                    frame,
                    label,
                    (x_top_left, y_top_left),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.5,
                    (0, 255, 0),
                )


####################
# SERVER ENDPOINTS #
####################


@app.route("/")
def index():
    return render_template("index.html")


@sock.route("/video-feed")
def video_sender(ws):
    """
    Websocket route for a client to send video frames to.
    """
    log.info("Video Sender Client connected")
    recorder = VideoRecorder()
    while True:
        try:
            data = ws.receive()
            if data is None:
                break

            # Decode base64 data into an image
            frame_data = base64.b64decode(data)
            nparr = np.frombuffer(frame_data, np.uint8)
            frame = cv2.imdecode(nparr, cv2.IMREAD_COLOR)

            process_frame(frame)
            recorder.record_video(frame)
            frame_queue.put(frame)

        except Exception as e:
            log.warning(f"Error in video sender: {e}")
            break

    log.info("Video Sender Client disconnected")


@sock.route("/live-feed")
def video_viewer(ws):
    """
    Websocket route for a client to get video frames from.
    """
    log.info("Video Viewer Client connected")

    while True:
        try:
            if not frame_queue.empty():
                frame = frame_queue.get()
                frame_base64 = encode_frame(frame)
                ws.send(frame_base64)
        except Exception as e:
            log.error(f"Error in video viewer: {e}")
            break

    log.info("Video Viewer Client disconnected")


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8080, debug=True)
