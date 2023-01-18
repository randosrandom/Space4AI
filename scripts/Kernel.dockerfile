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


FROM ubuntu:20.04

RUN apt-get update -y 

ARG DEBIAN_FRONTEND=noninteractive TZ="Europe/Rome"

RUN     apt install -y \
        build-essential \
        cmake \
        doxygen \
        g++ \
        git \
        graphviz \
        libssl-dev \
        nano \
        python3-dev \
        python3-pip \
        openssl \
        ssh \
        && apt-get clean -y

WORKDIR /home/SPACE4AI


