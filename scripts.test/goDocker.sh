#!/bin/bash

cd ..
sudo docker build -t trippy --build-arg USER=$USER --build-arg UID=$UID . &&
sudo docker run -it --name=trippy_exp -v $(pwd):/data trippy
sudo docker rm trippy_exp
cd scripts.test
