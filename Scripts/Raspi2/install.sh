#!/bin/bash

# --------------------------------------------------------
# Installs RetroFE and SDL2 from source on the Raspberry Pi 2 (Raspbian)
#
# Run from the web:
#   bash <(curl -s https://bitbucket.org/teamretro/retrofe/raw/default/Scripts/Raspi2/install.sh)
# --------------------------------------------------------

bold="\033[1m"
normal="\033[0m"

# Welcome message
echo -e "\n${bold}This script will download and install RetroFE including SDL2 from source."
echo -e "It will take 30-70 mins to complete on the Raspberry Pi 2 depending on number of updates and upgrades required.${normal}"

# Prompt to continue
read -p "  Continue? (y/n) " ans
  if [[ $ans != "y" ]]; then
    echo -e "\nQuitting...\n"
    exit
  fi
  echo -e "\n\n${bold}Would you like to Update and Upgrade Raspbian?${normal}?"
  read -p " (y/n) " upans
  echo -e "${bold}Compile and install SDL2?${normal}?"
  read -p " (y/n) " sdlans
  
# Time the install process
START_TIME=$SECONDS

  if [[ $upans = "y" ]]; then

    # Update Rasbian
    echo -e "\n\n${bold}Updating...${normal}"
    sudo apt-get update -y

    # Upgrade Rasbian
    echo -e "\n\n${bold}Upgrading...${normal}"
    sudo apt-get upgrade -y
  
  fi



# Install basic dependencies not needed for compiling SDL2
echo -e "\n\n${bold}Installing Round 1 Dependencies...${normal}"
sudo apt-get install -y tortoisehg g++ cmake dos2unix

# *********************************************************************
  if [[ $sdlans = "y" ]]; then

# Compile SDL2 Libs
  echo -e "\n\n${bold}Preparing SDL2 Libraries...${normal}"

# Setting up SDL2 variables
  url="http://www.libsdl.org"

  # sdl="SDL2-2.0.3"
  # sdl_url="${url}/release/${sdl}.tar.gz"
  # sdl_config="--host=armv7l-raspberry-linux-gnueabihf --disable-pulseaudio --disable-esd --disable-video-mir --disable-video-wayland --disable-video-x11 --disable-video-opengl"

  image="SDL2_image-2.0.0"
  image_url="${url}/projects/SDL_image/release/${image}.tar.gz"
  image_config=""

  mixer="SDL2_mixer-2.0.0"
  mixer_url="${url}/projects/SDL_mixer/release/${mixer}.tar.gz"
  mixer_config=""

  ttf="SDL2_ttf-2.0.12"
  ttf_url="${url}/projects/SDL_ttf/release/${ttf}.tar.gz"
  ttf_config=""

  get () {
    cd /tmp
    wget -N $1
    tar -xzvf $2.tar.gz
    cd $2
    ./configure $3 $4 $5 $6 $7 $8 $9 && make -j 4 && sudo make install
    rm /tmp/$2.tar.gz
    rm -rf /tmp/$2
  }

  echo -e "\n\n${bold}Installing Round 2 Dependencies...${normal}"
  sudo apt-get install -y libglib2.0-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libsdl2* automake build-essential libasound2-dev libdbus-1-dev libfreeimage-dev libfreetype6-dev libjpeg8-dev libmpeg3-dev libopenal-dev libpango1.0-dev libsndfile-dev libtiff4-dev libudev-dev libvorbis-dev libwebp-dev 
  echo -e "\n\n${bold}Beginning SDL2 Core build...${normal}"
# get ${sdl_url} ${sdl} ${sdl_config}
# Needs to be fixed so its part of the loop
    cd /tmp
    wget -N http://www.libsdl.org/release/SDL2-2.0.3.tar.gz
    tar -xzvf SDL2-2.0.3.tar.gz
    cd SDL2-2.0.3
    ./configure --host=armv7l-raspberry-linux-gnueabihf --disable-pulseaudio --disable-esd --disable-video-mir --disable-video-wayland --disable-video-x11 --disable-video-opengl
    make -j 4
    sudo make install
    rm /tmp/SDL2-2.0.3.tar.gz
    rm -rf /tmp/SDL2-2.0.3
  echo -e "\n\n${bold}Beginning SDL2 Image build...${normal}"
  get ${image_url} ${image} ${image_config}
  echo -e "\n\n${bold}Beginning SDL2 Mixer build...${normal}"
  get ${mixer_url} ${mixer} ${mixer_config}
  echo -e "\n\n${bold}Beginning SDL2 TTF build...${normal}"
  get ${ttf_url} ${ttf} ${ttf_config}
  echo -e "\n\n${bold}SDL2 Libraries Complete....${normal}"
# ****************************************************************************************
 fi
 
# Download RetroFE source
echo -e "\n\n${bold}Downloading RetroFE...${normal}"
hg clone --debug -v https://bitbucket.org/teamretro/retrofe

  cd retrofe
# Generate Make files for RetroFE
  echo -e "\n\n${bold}Generating make files...${normal}"
  cmake RetroFE/Source -BRetroFE/Build -DVERSION_MAJOR=0 -DVERSION_MINOR=0 -DVERSION_BUILD=0
# Compile RetroFE
  echo -e "\n\n${bold}Compiling RetroFE and creating a full environment...${normal}"
  cmake --build RetroFE/Build
# Install Basic folders and menu
  python Scripts/Package.py --os=linux --build=full
# Print the time elapsed
  ELAPSED_TIME=$(($SECONDS - $START_TIME))
  echo -e "\n${bold}Finished in $(($ELAPSED_TIME/60/60)) hr, $(($ELAPSED_TIME/60%60)) min, and $(($ELAPSED_TIME%60)) sec${normal}\n"
#Final Message
  echo -e "Type ${bold}./Artifacts/linux/RetroFE/retrofe${normal} to start..."