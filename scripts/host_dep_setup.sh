#!/bin/bash

# Setup the target

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
python3 -m pip install
  --upgrade \
  pip  \
  --break-system-packages \
  --no-warn-script-location


# Check if conda is installed
if is_installed conda; then
  echo "Conda is already installed."
else
  echo "Install conda before continuing"
  exit 1
fi


# Check if the Conda environment exists for $CONDA_ENV
if conda info --envs | grep -q $CONDA_ENV; then
  echo "Conda environment $CONDA_ENV already exists."
else
  echo "Creating a Conda environment named $CONDA_ENV with Python 3.9..."
  conda create --name $CONDA_ENV python=3.9 -y
fi
conda activate $CONDA_ENV

# Install the dependencies for the $CONDA_ENV
echo "Installing Python dependencies"
pip install \
  opencv-python \
  websockets

echo "Installation complete!"
