
import cv2
from flask import Flask, render_template, Response
import numpy as np
import threading
import time
import pyaudio


app = Flask(__name__)
@app.route('/')
def index():
    return render_template('index.html', my_list=[0,1,2])

@app.route('/video_feed0')
def video_feed0():
    return Response(get_color_rs(), mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/video_feed1')
def video_feed1():
    return Response(get_depth_rs(), mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/video_feed2')
def video_feed2():
    return Response(get_infra_rs(), mimetype='multipart/x-mixed-replace; boundary=frame')


FORMAT = pyaudio.paInt16
CHANNELS = 2
RATE = 44100
CHUNK = 1024
RECORD_SECONDS = 1

def genHeader(sampleRate, bitsPerSample, channels):
    datasize = 2000*10**6
    o = bytes("RIFF",'ascii')                                               # (4byte) Marks file as RIFF
    o += (datasize + 36).to_bytes(4,'little')                               # (4byte) File size in bytes excluding this and RIFF marker
    o += bytes("WAVE",'ascii')                                              # (4byte) File type
    o += bytes("fmt ",'ascii')                                              # (4byte) Format Chunk Marker
    o += (16).to_bytes(4,'little')                                          # (4byte) Length of above format data
    o += (1).to_bytes(2,'little')                                           # (2byte) Format type (1 - PCM)
    o += (channels).to_bytes(2,'little')                                    # (2byte)
    o += (sampleRate).to_bytes(4,'little')                                  # (4byte)
    o += (sampleRate * channels * bitsPerSample // 8).to_bytes(4,'little')  # (4byte)
    o += (channels * bitsPerSample // 8).to_bytes(2,'little')               # (2byte)
    o += (bitsPerSample).to_bytes(2,'little')                               # (2byte)
    o += bytes("data",'ascii')                                              # (4byte) Data Chunk Marker
    o += (datasize).to_bytes(4,'little')                                    # (4byte) Data size in bytes
    return o

@app.route('/audio')
def audio():
    # start Recording
    def sound():

        CHUNK = 1024
        sampleRate = 44100
        bitsPerSample = 16
        channels = 2
        wav_header = genHeader(sampleRate, bitsPerSample, channels)

        stream = audio1.open(format=FORMAT, channels=CHANNELS,
                        rate=RATE, input=True,input_device_index=11,
                        frames_per_buffer=CHUNK)
        print("recording...")
        #frames = []
        first_run = True
        while True:
           if first_run:
               data = wav_header + stream.read(CHUNK)
               first_run = False
           else:
               data = stream.read(CHUNK)
           yield(data)

    return Response(sound())

capture = None
depth = None
infra = None
color = None
pipeline = None
lock = threading.Lock()

def init():

    global audio1
    audio1 = pyaudio.PyAudio()

    import pyrealsense2 as rs
    pipeline = rs.pipeline()
    context = rs.context()
    #camera_data = find_cameras(context)
    config = rs.config()
    config.enable_stream(rs.stream.color, width=1920, height=1080, format=rs.format.rgb8, framerate=30)
    config.enable_stream(rs.stream.depth, width=320, height=240, format=rs.format.z16, framerate=30)
    config.enable_stream(rs.stream.infrared, width=320, height=240, format=rs.format.y8, framerate=30)

    conf = pipeline.start(config)

    depth_sensor = conf.get_device().first_depth_sensor()
    color_sensor = conf.get_device().first_color_sensor()

    depth_sensor.set_option(rs.option.min_distance, 0)


    dep = conf.get_stream(rs.stream.depth, 0).as_video_stream_profile()
    col = conf.get_stream(rs.stream.color, 0).as_video_stream_profile()

    global capture, color, depth, infra, lock

    while True:
        capture = pipeline.wait_for_frames()
        
        depth = capture.get_depth_frame()
        color = capture.get_color_frame()
        infra = capture.get_infrared_frame()



def get_color_rs():
    global capture, color, lock

    while True:
        #with lock:
            if capture:
                col = np.asarray(color.get_data())
                colDown = cv2.resize(col, (320, 240))
                ret, buffer = cv2.imencode('.jpg', colDown)
                frame = buffer.tobytes()
                yield (b'--frame\r\n'
                    b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')
                time.sleep(0.02)

def get_infra_rs():
    global capture, infra, lock

    while True:
        #with lock:
            if capture:
                ret, buffer = cv2.imencode('.jpg', cv2.pyrDown(np.asarray(infra.get_data())))
                frame = buffer.tobytes()
                yield (b'--frame\r\n'
                    b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')
                time.sleep(0.02)

def get_depth_rs():
    global capture, depth, lock

    while True:
        #with lock:
            if capture:
                ret, buffer = cv2.imencode('.jpg', cv2.pyrDown(np.asarray(depth.get_data())) / 31.0)
                frame = buffer.tobytes()
                yield (b'--frame\r\n'
                    b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')
                time.sleep(0.02)

def gen_frames():
    cap = cv2.VideoCapture(0)

    while True:
        success, frame = cap.read()  # read the camera frame
        if not success:
            break
        else:
            ret, buffer = cv2.imencode('.jpg', frame)
            frame = buffer.tobytes()
            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')  # concat frame one by one and show result

if __name__ == "__main__":
    t = threading.Thread(target=init)
    t.daemon = True
    t.start()
    app.run(debug=True, host='0.0.0.0', threaded=True, use_reloader=False)

    
