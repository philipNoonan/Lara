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


def callback(in_data, frame_count, time_info, status):
    global audio_data, audio_ready
    audio_data = np.fromstring(in_data, dtype=np.float32)
    audio_data = np.reshape(audio_data, (1024, 7))
    audio_ready = True
    return (audio_data, pyaudio.paContinue)

@app.on_event("startup")
def startup_event():
    global device
    global lock
    global frameNumber
    global audio1
    global stream

    audio1 = pyaudio.PyAudio()
    FORMAT = pyaudio.paFloat32
    CHANNELS = 7
    RATE = 48000
    CHUNK = 2048

    stream = audio1.open(format=FORMAT, channels=CHANNELS,
        rate=RATE, input=True,input_device_index=11,
        frames_per_buffer=CHUNK)

    frameNumber = 0



@app.on_event("shutdown")
def shutdown_event():
    global audio1, stream

    stream.stop_stream()
    stream.close()
    audio1.terminate()



@app.get("/")
def read_root(request: Request):
    return templates.TemplateResponse("index_k4a_audio.html", {"request": request})





@app.websocket("/wsAudio")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()

    global stream

    global audio_data

    # since you know the sampling rate, duration, and all that stuff, you should be able to properly schedule the playback of chunks in the browser
    # https://github.com/kmoskwiak/node-tcp-streaming-server/blob/master/client/js/app.js

    
    #test_arr = np.ones(48000).astype('int32')

    while True:
        
        data = stream.read(2048, exception_on_overflow = False)

        audio_data = np.fromstring(data, dtype=np.float32)
        audio_data = np.reshape(audio_data, (2048, 7))
        #print(len(data))
        
        await websocket.send_bytes((1000.0* audio_data[:, 0]).tobytes())
        #await asyncio.sleep(0.001)




