
import cv2
from flask import Flask, render_template, Response
import numpy as np
import threading
import time
import pyaudio
import serial
from serial.serialutil import PARITY_NONE, STOPBITS_ONE


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

@app.route('/video_feed3')
def video_feed3():
    return Response(get_thermal(), mimetype='multipart/x-mixed-replace; boundary=frame')


def genHeader(sampleRate, bitsPerSample, channels, samples):
    datasize = samples * channels * bitsPerSample // 8
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

        p = pyaudio.PyAudio()
        info = p.get_host_api_info_by_index(0)
        numdevices = info.get('deviceCount')
        for i in range(0, numdevices):
            if (p.get_device_info_by_host_api_device_index(0, i).get('maxInputChannels')) > 0:
                print("Input Device id ", i, " - ", p.get_device_info_by_host_api_device_index(0, i).get('name'))

        FORMAT = pyaudio.paInt16
        CHANNELS = 1
        RATE = 44100

        CHUNK = 4096
        bitsPerSample = 16
        wav_header = genHeader(RATE, bitsPerSample, CHANNELS, 200000)

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
              # print("reading")
           yield(data)

    return Response(sound())

capture = None
depth = None
infra = None
color = None
pipeline = None
lock = threading.Lock()

def init():

    # global audio1
    # audio1 = pyaudio.PyAudio()

    # global ser
    # ser = serial.Serial()
    # ser.baudrate = 2000000
    # ser.port = '/dev/ttyUSB0'
    # ser.bytesize=8
    # ser.stopbits=STOPBITS_ONE
    # ser.parity=PARITY_NONE
    # ser.open()

    import pyrealsense2.pyrealsense2 as rs
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

    global capture, color, depth, infra, lock, thermal

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
                ret, buffer = cv2.imencode('.jpg', cv2.cvtColor(colDown, cv2.COLOR_BGR2RGB))
                frame = buffer.tobytes()
                yield (b'--frame\r\n'
                    b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')
                time.sleep(0.02)

def get_infra_rs():
    global capture, infra, lock

    while True:
        #with lock:
            if capture:
                ret, buffer = cv2.imencode('.jpg', np.asarray(infra.get_data()))
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

def get_thermal():
    global ser

    img = np.zeros([32, 24])


    while True:
        #print("reading thermal")
        thermal = ser.readline()

        imgList = list(thermal)
        img = np.array(imgList[0:768])
        imgMat = np.reshape(img, (24, 32))
        ret, buffer = cv2.imencode('.jpg', imgMat)
        frame = buffer.tobytes()
        yield (b'--frame\r\n'
            b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')

        #with lock:
        #decoded_bytes = int(thermal[0:len(thermal)-2].decode("utf-8"))
        #print(decoded_bytes)
        #print(list(thermal))
        #time.sleep(0.1)


def gen_frames():
    cap = cv2.VideoCapture(0)

    while True:
        success, frame = cap.read()  # read the camera frame
        if not success:
            break
        else:
            ret, buffer = cv2.imencode('.jpg', cv2.cvtColor(frame, cv2.COLOR_BGR2RGB))
            frame = buffer.tobytes()
            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')  # concat frame one by one and show result

if __name__ == "__main__":
    t = threading.Thread(target=init)
    t.daemon = True
    t.start()
    app.run(debug=True, host='0.0.0.0', threaded=True, use_reloader=False)

    
