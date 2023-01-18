# Copyright 2021 AI-SPRINT

#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at

#      http://www.apache.org/licenses/LICENSE-2.0

#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

from flask import Flask, request
from pacsltk import perfmodel
import json

app = Flask(__name__)

@app.route("/pacsltk", methods=['GET', 'POST'])
def pacsltk_predictor():

	data_raw = request.get_data().decode('utf-8')
	data_json = json.loads(data_raw)

	props1, _ = perfmodel.get_sls_warm_count_dist(data_json["arrival_rate"], data_json["warm_service_time"], data_json["cold_service_time"], data_json["idle_time_before_kill"])
	
	return str(props1["avg_resp_time"]), 200 # (time, error_code)

@app.route("/corePredictor", methods=['GET', 'POST'])
def core_predictor():
	pass