[TOC]

RetroFE is a cross-platform frontend designed for MAME cabinets/game centers/etc. with a focus on simplicity and customization.

#  Compiling and installing on Ubuntu Linux (10.04 or newer) #

## Install libraries ##
Install necessary dependencies:
	
	sudo apt-get install tortoisehg g++ cmake dos2unix zlib1g-dev libsdl2-2.0 libsdl2-mixer-2.0 libsdl2-image-2.0 libsdl2-ttf-2.0 \
                libsdl2-dev libsdl2-mixer-dev libsdl2-image-dev libsdl2-ttf-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
                libgstreamer-plugins-good1.0-dev gstreamer1.0-libav zlib1g-dev libglib2.0-0 libglib2.0-dev sqlite3

## Download and compile the source code ##
Download the source code:

	hg clone https://phulshof@bitbucket.org/phulshof/retrofe

Generate your gcc make files:

	cd retrofe
	cmake RetroFE/Source -BRetroFE/Build -DVERSION_MAJOR=0 -DVERSION_MINOR=0 -DVERSION_BUILD=0

Compile RetroFE and create a full environment: 

	cmake --build RetroFE/Build
	python Scripts/Package.py --os=linux --build=full

Copy your live RetroFE system to any folder of your choosing:
	cp -r Artifacts\linux\RetroFE /your/ideal/retrofe/path



# Compiling and installing on Windows #
** Visit the [RetroFE downloads](retrofe.com/download.php) page to download a precompiled version if you do not want to compile your own. **

## Install libraries ##
	Install Python 2.7
	Install sphinx with python
	Install visual studio 2012
	Install Microsoft Windows SDK for Windows 7 and .net Framework 4 http://www.microsoft.com/en-us/download/details.aspx?id=8279
	Install cmake
	Install tortoisehg
	Install 7zip
	Install gstreamer and gstreamer-devel to c:/gstreamer(x86, not 64 bit!) from http://gstreamer.freedesktop/org/data/pkg/windows/1.4.0

## Download and compile the source code ##

Download the source code

	hg clone https://phulshof@bitbucket.org/phulshof/retrofe


Setup Environment (to setup necessary variables and paths to compile in visual studio)

	cd retrofe
	RetroFE/Source/SetupEnvironment.bat   

Generate visual studio solution files

	cmake RetroFE/Source -BRetroFE/Build -DGSTREAMER_ROOT=C:/gstreamer/1.0/x86 -DVERSION_MAJOR=0 -DVERSION_MINOR=0 -DVERSION_BUILD=0
  
Compile RetroFE and create a full environment by running the following commands

	cmake --build RetroFE/Build --config Release
	python Scripts\Package.py --os=windows --build=full --gstreamer_path=D:/gstreamer/1.0/x86

Copy your live RetroFE system to any folder of your choosing. files can be found in Artifacts\windows\RetroFE


# Compiling and Installing on Raspberry Pi 2 (raspbian) #

** Due to performance, the Rasperry Pi 1 is not supported. **

Just run the following on your raspberry pi 2 (assuming you have an internet connection:

	bash <(curl -s https://bitbucket.org/phulshof/retrofe/raw/default/Scripts/Raspi2/install.sh)