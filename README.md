# README #

RetroFE is a cross-platform frontend designed for MAME cabinets/game centers/etc. with a focus on simplicity and customization.

##  Compiling and installing on Ubuntu Linux (10.04 or newer) ##
===============================
### Install libraries ###
Install necessary dependencies:
	
	sudo apt-get install tortoisehg g++ cmake dos2unix zlib1g-dev libsdl2-2.0 libsdl2-mixer-2.0 libsdl2-image-2.0 libsdl2-ttf-2.0 \
                libsdl2-dev libsdl2-mixer-dev libsdl2-image-dev libsdl2-ttf-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
                libgstreamer-plugins-good1.0-dev zlib1g-dev libglib2.0-0 libglib2.0-dev sqlite3

### Download and compile the source code ###
1. Download the source code:

   hg clone https://bitbucket.org/teamretro/retrofe

2. Generate your gcc make files:

  cd retrofe
  cmake RetroFE/Source -BRetroFE/Build -DVERSION_MAJOR=0 -DVERSION_MINOR=0 -DVERSION_BUILD=0

3. Compile RetroFE and create a full environment: 

  cmake --build RetroFE/Build
  python Scripts/Package.py --os=linux --build=full

### Download and compile the source code ###
cp -r Artifacts\linux\RetroFE /your/ideal/retrofe/path



## Compiling and installing on Windows ## 
** Visit the [RetroFE downloads](retrofe.com/download.php) page to download a precompiled version if you do not want to compile your own. **

### Install libraries ###
  Install Python 2.7
  Install sphinx with python
  Install visual studio 2012
  Install Microsoft Windows SDK for Windows 7 and .net Framework 4 http://www.microsoft.com/en-us/download/details.aspx?id=8279
  Install cmake
  Install tortoisehg
  Install 7zip
  Install gstreamer and gstreamer-devel to c:/gstreamer(x86, not 64 bit!) from http://gstreamer.freedesktop/org/data/pkg/windows/1.4.0

### Download and compile the source code ###
1. Open a command prompt 
2. Download the source code:

   hg clone https://bitbucket.org/teamretro/retrofe

3. Setup Environment (to setup necessary variables and paths to compile in visual studio)

   cd retrofe
   RetroFE/Source/SetupEnvironment.bat   

4. Generate visual studio solution files:

  cmake RetroFE/Source -BRetroFE/Build -DGSTREAMER_ROOT=C:/gstreamer/1.0/x86 -DVERSION_MAJOR=0 -DVERSION_MINOR=0 -DVERSION_BUILD=0
  
5. Compile RetroFE and create a full environment by running the following commands: 

  cmake --build RetroFE/Build --config Release
  python Scripts\Package.py --os=windows --build=full --gstreamer_path=D:/gstreamer/1.0/x86

6. Copy your live RetroFE system to any folder of your choosing. files can be found in Artifacts\windows\RetroFE


### Other build options ###
After compiling your code you can also build different environments:

1. To copy just the layouts to Artifacts/windows/RetroFE, run 
  python Scripts\Package.py --os=windows --build=layout --gstreamer_path=D:/gstreamer/1.0/x86
  or
  python Scripts\Package.py --os=linux --build=layout
  
2. To copy just retrofe.exe to Artifacts/windows/RetroFE, run 
  python Scripts\Package.py --os=windows --build=engine --gstreamer_path=D:/gstreamer/1.0/x86
  or
  python Scripts\Package.py --os=linux --build=engine

3. To copy just the third party dlls and retrofe to Artifacts/windows/RetroFE, run 
  python Scripts\Package.py --os=windows --build=core --gstreamer_path=D:/gstreamer/1.0/x86
  or
  python Scripts\Package.py --os=linux --build=core

4. To copy a whole live installation Artifacts/windows/RetroFE, run 
  python Scripts\Package.py --os=windows --build=full --gstreamer_path=D:/gstreamer/1.0/x86
  or
  python Scripts\Package.py --os=linux --build=full
  
  
## Compiling and Installing on Raspberry Pi (raspbian) ##
### Install libraries ###
Install necessary dependencies:

	sudo apt-get install tortoisehg g++ cmake dos2unix

### Download and compile the source code ###
1. Download the source code:

   hg clone https://bitbucket.org/teamretro/retrofe

2. Compile SDL2:
   sudo Scripts/Raspi/install_sdl2_rpi.sh
   
3. Generate your gcc make files:
  cd retrofe
  cmake RetroFE/Source -BRetroFE/Build -DVERSION_MAJOR=0 -DVERSION_MINOR=0 -DVERSION_BUILD=0

4. Compile RetroFE and create a full environment: 

  cmake --build RetroFE/Build
  python Scripts/Package.py --os=linux --build=full

### Download and compile the source code ###
cp -r Artifacts\linux\RetroFE /your/ideal/retrofe/path
