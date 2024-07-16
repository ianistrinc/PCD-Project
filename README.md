
# PCD-Project

## Overview

This project consists of a server and three types of clients designed for image processing via TCP socket connections. Each client has distinct functionalities: an admin client, a normal client, and a Python interface client. The server processes images based on the paths and types of processing provided by the clients.

## Server

The server is implemented in `server.cpp`. It handles connections from clients, processes images based on the provided paths, and returns the processed images.

## Clients

### Admin Client (`clienta.cpp`)

- Connects to the server.
- Sends messages to the server.
- Destroys all connections to the server.

### Normal Client (`client1.cpp`)

- Connects to the server.
- Sends an image path and processing type to the server.
- Receives and displays the processed image.

### Python Interface Client (`client2.py`)

- Connects to the server.
- Sends an image path and processing type from the UI.
- Receives and displays the processed image using Python libraries.

## Image Processing Modules

- `canny.cpp`: Applies Gaussian blur to reduce noise and uses the Canny algorithm to detect edges.
- `contur.cpp`: Converts the image to grayscale and applies erosion to highlight contours.
- `rotate.cpp`: Finds contours, rotates objects within these contours by 180 degrees, and replaces them in the original image.
