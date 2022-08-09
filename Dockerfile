FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive

RUN     apt-get update -y \
        && apt-get install -y ssh \
        build-essential \
        g++ \
        cmake \
        python3-dev \
        python3-pip \
        nano \
	doxygen \
	graphviz \
        && apt-get clean -y

COPY ./ /home/SPACE4AI/
RUN pip3 install -r /home/SPACE4AI/requirements.txt

WORKDIR /home/SPACE4AI

RUN mkdir build && cd build && cmake .. && make 

ENTRYPOINT ["/bin/bash"]
