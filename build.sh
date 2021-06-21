#!/bin/sh
g++ -o Project -lGL -lGLEW -lglfw main.cpp Window.cpp Shader.cpp Camera.cpp Mesh.cpp Model3D.cpp stb_image.cpp tiny_obj_loader.cpp SkyBox.cpp
