import json
import asyncio
from fastapi import FastAPI
from fastapi import Request
from fastapi import WebSocket
from fastapi.templating import Jinja2Templates
import numpy as np
import cv2
import threading

app = FastAPI()
templates = Jinja2Templates(directory="templates")
global frame, success

#cap = cv2.VideoCapture(0)
#cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
#cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)

import pyrealsense2.pyrealsense2 as rs

global pipeline

pipeline = rs.pipeline()
context = rs.context()
    #camera_data = find_cameras(context)
config = rs.config()
#config.enable_stream(rs.stream.color, width=1920, height=1080, format=rs.format.rgb8, framerate=30)
config.enable_stream(rs.stream.depth, width=1024, height=768, format=rs.format.z16, framerate=30)
config.enable_stream(rs.stream.infrared, width=1024, height=768, format=rs.format.y8, framerate=30)

conf = pipeline.start(config)

depth_sensor = conf.get_device().first_depth_sensor()
#color_sensor = conf.get_device().first_color_sensor()

depth_sensor.set_option(rs.option.min_distance, 0)



@app.get("/")
def read_root(request: Request):
    return templates.TemplateResponse("index.html", {"request": request})

@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()

    global frame
    global success

    global pipeline, capture, depth, infra

    minScale = 0
    maxScale = 1

    while True:
        #success, frame = cap.read()  # read the camera frame

        capture = pipeline.wait_for_frames()
            
        depth = capture.get_depth_frame()
        infra = capture.get_infrared_frame()

        #if (1):
        #    image_array = np.asarray(infra.get_data())
        #else:
        #    image_array = np.asarray(cv2.pyrDown(np.asarray(infra.get_data())))



        #frame4 = cv2.cvtColor(cv2.pyrDown(np.asarray(infra.get_data())), cv2.COLOR_GRAY2RGBA)
        #image_array = np.asarray(frame4[:,:])
        ir_array = cv2.pyrDown(np.asarray(infra.get_data()))
        dep_array = ((cv2.pyrDown(np.asarray(depth.get_data())) - float(minScale)) / (float(maxScale) - float(minScale))).astype('uint8')

        ir_array[0] = 0
        dep_array[0] = 1

            #await asyncio.sleep(0.1)
        await websocket.send_bytes(ir_array.tobytes())
        await websocket.send_bytes(dep_array.tobytes())
        #data = await websocket.receive_text()
        #minScale, maxScale = data.split()




