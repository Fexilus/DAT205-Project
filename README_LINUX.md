# Untested instructions for linux

## Description
The labs are built with the build software CMake, which you will need to install.

You will also need to install the necessary third-party libraries needed. 
The libraries that you need are SDL2, GLEW and glm.
You will also need to have an implementation of OpenGL installed on your system e.g. Mesa. 

Install CMake and these libraries with your package manager of choice:
```shell
apt-get install ...
yum install ...
pacman -S ...
...
```

If you are missing any necessary libraries CMake will complain when you are trying to build.

Now to building. Change directory to the root directory containing the labs.

Make a directory called 'build' and enter it:
``` shell
mkdir build
cd build
```

Prepare the build files by typing:
``` shell
cmake ..
```
or
``` shell
cmake DCMAKE_BUILD_TYPE=Release ..
```

for a debug vs release build.

Now you can build by simply typing:
``` shell
make
```

in the same directory. 
