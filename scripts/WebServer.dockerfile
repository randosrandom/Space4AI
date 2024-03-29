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

# Cambiare questo Dockerfile per aggiungere ML library

FROM python:3.9

RUN apt-get update -y 

ARG DEBIAN_FRONTEND=noninteractive TZ="Europe/Rome"

#REQUIREMENTS
RUN pip3 install \
	Flask \
	pacsltk \
	gunicorn

WORKDIR /home