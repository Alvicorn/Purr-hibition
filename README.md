# Purr-hibition

CMPT 433 Spring project.

Purr-hibition is a smart deterrent system designed to keep cats off countertops. The system uses a PIR motion sensor to detect movement, and upon detection, it activates a cat detection module using a webcam. If a cat is identified using a MobileNet SSD model via OpenCV, the system triggers deterrents: loud audio through a speaker and flashing LED lights. It also begins recording the incident and streams the live video feed to a remote web application. The web interface allows users to view the live feed, toggle recording, and activate/deactivate deterrents remotely.

## Dependencies
* aarch64-linux-gnu-gcc  cross compiler
* cmake
* python3
* miniconda on both the Host (VM) and the Target (BeagleY-AI) - make sure you install the correct architecture for your system! (aarch for BeagleY-AI)


## Run Instructions

One time setup: To install dependencies for our project, run the scripts provided in the repo: `host_dep_setup.sh` on the host and `target_dep_setup.sh` on the target

Below contains run instructions each time you want to run the application:

Target:

1. Run `purr_hibition` (built with gcc) under `/mnt/remote/myApps/project/purr-hibition`
2. Run: `conda activate purr-hibition`. Then run `python3 byai_camera.py` under `/mnt/remote/myApps/project/purr-hibition/python`


Host:

1. Run: `conda activate purr-hibition`. Then run `python3 video_streaming_server.py` under `~/cmpt433/work/project/Purr-hibition/as3-server`
2. Run: `node server.js` under `~/cmpt433/work/project/Purr-hibition/webapp`

Note: if `conda activate purr-hibition` is not working, try running `source ~/.bashrc` as this may fix it.
