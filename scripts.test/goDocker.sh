#!/bin/bash

cd ..
docker stop trippy_exp
docker rm trippy_exp
docker build -t trippy --build-arg USER=$USER --build-arg UID=$UID . &&
docker run -it --name=trippy_exp -v $(pwd):/data trippy
docker stop trippy_exp
cd scripts.test
