import json
import asyncio
from fastapi import FastAPI
from fastapi import Request
from fastapi import WebSocket
from fastapi.templating import Jinja2Templates
import numpy as np
import cv2
import threading

import pyaudio


app = FastAPI()
templates = Jinja2Templates(directory="templates")
global device
global depth, infra, color

#cap = cv2.VideoCapture(0)
#cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
#cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)

import k4a



@app.on_event("startup")
def startup_event():
    global device
    global lock
    global frameNumber
    global audio1
    global stream

    
    frameNumber = 0

    device = k4a.Device.open()
    if device is None:
        exit(-1)

    lock = threading.Lock()

    device_config = k4a.DeviceConfiguration(
        color_format=k4a.EImageFormat.COLOR_BGRA32,
        color_resolution=k4a.EColorResolution.RES_720P,
        depth_mode=k4a.EDepthMode.WFOV_2X2BINNED,
        camera_fps=k4a.EFramesPerSecond.FPS_30,
        synchronized_images_only=True,
        depth_delay_off_color_usec=0,
        wired_sync_mode=k4a.EWiredSyncMode.STANDALONE,
        subordinate_delay_off_master_usec=0,
        disable_streaming_indicator=True)

    status = device.start_cameras(device_config)
    if status != k4a.EStatus.SUCCEEDED:
        exit(-1)

    thread = threading.Thread(target=get_frames, args=())
    thread.daemon = True
    thread.start()

@app.on_event("shutdown")
def shutdown_event():
    global device


    device.stop_cameras()
    device.close()

def get_frames():
    global device
    global lock, frameNumber
    global depth, infra, color
    while True:
        capture = device.get_capture(-1)
        frameNumber += 1
        #print("here")

        #lock.acquire()
        depth = np.copy(capture.depth.data)
        infra = np.copy(capture.ir.data)
        color = np.copy(capture.color.data)
        #lock.release()
        #print("therer")



@app.get("/")
def read_root(request: Request):
    return templates.TemplateResponse("index_k4a.html", {"request": request})

@app.websocket("/wsCol")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    global color
    while True:
        col_array = np.rot90(cv2.resize(color, (320,240)).astype('uint8'), -1)
        await websocket.send_bytes(col_array.tobytes())
        await asyncio.sleep(0.01)

@app.websocket("/wsDep")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    global depth
    while True:
        dep_array = ((infra - float(minScale)) / (float(maxScale) - float(minScale))).astype('uint8')

        await websocket.send_bytes(dep_array.tobytes())
        await asyncio.sleep(0.01)


@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()

    global device
    global lock
    global depth, infra, color
    global frameNumber

    global minScale, maxScale

    minScale = 0
    maxScale = 1

    previousFrameNumber = 0

    while True:

        #lock.acquire()
        #frame4 = cv2.cvtColor(np.asarray(infra), cv2.COLOR_GRAY2RGBA)
        #image_array = np.asarray(color[0:200,0:200])
        #lock.release()
        if previousFrameNumber < frameNumber:
            previousFrameNumber = frameNumber
            #dep_array = ((cv2.resize(infra, (160,144)) - float(minScale)) / (float(maxScale) - float(minScale))).astype('uint8')
            dep_array = ((infra - float(minScale)) / (float(maxScale) - float(minScale))).astype('uint8')

            await websocket.send_bytes(dep_array.tobytes())
            await asyncio.sleep(0.01)

        else:
            await asyncio.sleep(0.01)




@app.websocket("/ws1")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    global minScale, maxScale
    while True:

        data = await websocket.receive_text()
        minScale, maxScale = data.split()

