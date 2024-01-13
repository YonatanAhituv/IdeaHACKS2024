from flask import Flask, render_template, url_for, flash, redirect, request, Response, session, jsonify
from threading import Lock
from flask_socketio import SocketIO, emit
import requests

# from werkzeug.utils import secure_filename

from flaskweb import app, socketio
thread = None
thread_lock = Lock()
# globals to keep track of achievements
achievement_dict = {"altitude": 500, "steps": 1000000, "acceleration": False, "humidity": False}

#route to home page
@app.route("/", methods=['GET', 'POST'])
@app.route("/index", methods=['GET', 'POST'])
def index():
    return render_template('index.html', async_mode=socketio.async_mode)

# get data from json file
@app.route("/data_json")
def data_json():
    dummy_data = [{
        # "acceloremeter": {
        #     "x": 0,
        #     "y": 0,
        #     "z": 0
        # },
        # "gyroscope": {
        #     "x": 0,
        #     "y": 0,
        #     "z": 0
        # },
        "altitude": 10,
        "temperature": 0,
        "humidity": 20,
        "pressure": 0,
        }
    ]
    return jsonify(dummy_data)

#route to achievements page
@app.route("/achievements", methods=['GET', 'POST'])
def achievements():
    if achievement_dict["altitude"] > 500:
        check1 = "checked"
    else:
        check1 = "unchecked"
    if achievement_dict["steps"] > 10000:
        check2 = "checked"
    else:
        check2 = "unchecked"
    if achievement_dict["acceleration"]:
        check3 = "checked"
    else:
        check3 = "unchecked"
    return render_template('achievements.html', title='Achievements', check1 = check1, check2 = check2, check3 = check3) # add variables for each achievement

# code to encyclopedia page
@app.route("/encyclopedia", methods=['GET', 'POST'])
def encyclopedia():
    # save image to computer and then display it on next encyclopedia page
    # use a list to store image names to display
    plant_array = []
    array_length = len(plant_array)
    return render_template('encyclopedia.html', title='Encyclopedia', plant_array = plant_array, array_length = array_length)
