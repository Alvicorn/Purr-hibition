#!/bin/bash

# Setup the target (BYAI)

CONDA_ENV="purr-hibition"


# Function to check if a program is installed
is_installed() {
  command -v $1 >/dev/null 2>&1
}


# Check if Python is installed
if is_installed python3; then
  echo "Python is already installed."
else
  echo "Python not found. Installing Python..."
  sudo apt update
  sudo apt install -y python3 python3-pip python3-venv
fi

echo "Upgrading pip..."
python3 -m pip install \
  --upgrade \
  pip \
  --break-system-packages \
  --no-warn-script-location

sudo apt install -y ffmpeg v4l-utils xvfb

# Check if Mambaforce is installed
if is_installed mamba; then
  echo "Mambaforge is already installed."
else
  echo "Mambaforge not found. Installing Mambaforge..."
  wget https://github.com/conda-forge/miniforge/releases/download/24.3.0-0/Mambaforge-24.3.0-0-Linux-aarch64.sh
  bash Mambaforge-24.3.0-0-Linux-aarch64.sh -b

  source ~/mambaforge/etc/profile.d/conda.sh
  conda init bash
  rm Mambaforge-24.3.0-0-Linux-aarch64.sh
fi

source ~/mambaforge/etc/profile.d/conda.sh


# Check if the Conda environment exists for $CONDA_ENV
if conda info --envs | grep -q $CONDA_ENV; then
  echo "Conda environment $CONDA_ENV already exists."
else
  echo "Creating a Conda environment named $CONDA_ENV with Python 3.11..."
  conda create --name $CONDA_ENV python=3.11 -y
fi
conda activate $CONDA_ENV

# Install the dependencies for the $CONDA_ENV
echo "Installing Python dependencies"
pip install \
  numpy==1.26.4 \
  opencv-python-headless \
  websockets \
  aiortc \
  aiohttp

echo "Installation complete!"
