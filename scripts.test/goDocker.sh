#!/bin/bash

cd ..
docker build -t trippy --build-arg USER=$USER --build-arg UID=$UID . &&
docker run -it --name=trippy_exp -v $(pwd):/data trippy
docker rm trippy_exp
cd scripts.test
