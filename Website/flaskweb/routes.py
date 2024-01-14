from flask import Flask, render_template, url_for, flash, redirect, request, Response, session, jsonify
from threading import Lock
from flask_socketio import SocketIO, emit
import requests
import json
import base64
# from werkzeug.utils import secure_filename

from flaskweb import app, socketio
thread = None
thread_lock = Lock()
# globals to keep track of achievements
achievement_dict = {"altitude": 500, "steps": 1000000, "acceleration": False, "humidity": False}
plant_dict = {}
class Plant:
    def __init__(self, image_data):
        self.count = 1
        url = "https://plant.id/api/v3/identification"
        image_load = "data:image/jpg;base64" + image_data
        payload = json.dumps({
            "images": [
                image_load

            ],
            "latitude": 49.207,
            "longitude": 16.608,
            "similar_images": True
        })
        headers = {
            'Api-Key': 'v34KUh6vv80Rod8KXYzGwYP8tI4sza2g7flTbxJbRrkhWEPQtM',
            'Content-Type': 'application/json'

        }
        response = requests.request("POST", url, headers=headers, data=payload)
        self.image_url = response.json()["input"]["images"][0]
        self.name = response.json()["result"]["classification"]["suggestions"][0]["name"]
        self.valid = str(response.json()["result"]["is_plant"]["binary"])

    


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
    global achievement_dict
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

# receive picture and add to list
@app.route("/add_plant", methods=['GET', 'POST'])
def add_plant():
    global plant_dict
    # save image to computer and then display it on next encyclopedia page
    # use a list to store image names to display
    
    # get data from json and read into array
    # if request.method == 'POST':
    #     image = request.values.get('user')
    new_plant = Plant(image)
    if new_plant.valid == "true":
        if new_plant.name in plant_dict:
            plant_dict[new_plant.name].count += 1
        else:
            plant_dict[new_plant.name] = new_plant
    
    return new_plant.valid == "true"

# code to encyclopedia page
@app.route("/encyclopedia", methods=['GET', 'POST'])
def encyclopedia():
    global plant_dict
    # save image to computer and then display it on next encyclopedia page
    # use a list to store image names to display
    
    # get data from json and read into array

        
    num_plants = len(plant_dict)
    
    return render_template('encyclopedia.html', title='Encyclopedia', plant_dict = plant_dict, num_plants = num_plants)
