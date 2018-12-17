# ubuntu 18.04
FROM ubuntu:bionic   

LABEL mantainer="d.galaktionov@udc.es"

# Install essentials
RUN apt-get update && apt-get install -y \
	sudo \
	cmake \
	gcc-multilib \
	g++-multilib \
	libboost-all-dev \
	p7zip-full \
	screen \
	zstd \
	libzstd-dev

# Get arguments to create user with the same name and UID than in the host
ARG USER
ARG UID

# Create a user, give it sudo permissions and make it owner of /data
RUN useradd -u $UID $USER
RUN adduser $USER sudo
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
RUN mkdir /data
RUN chown -R $USER:$USER /data
USER $USER
WORKDIR /data
