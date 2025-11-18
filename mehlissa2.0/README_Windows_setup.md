# MEHLISSA 2.0: Setup in Windows

## General Information

The setup of MEHLISSA 2.0 under Windows can be done with __WSL or__ using __MSYS2 and MinGW__.
In our experience, using WSL is a bit easier, especially if you don't already work with MSYS2 and MinGW.
We have encountered some problems with using the WSL toolchain in a running MSYS2 and MinGW setup, though.
So, if you already have them installed and do not want to break your current system, we recommend following the second setup option.
In the following, we will first explain the setup using WSL and then the setup via MSYS2 and MinGW.
In both approaches, you will need to have the __MEHLISSA repository downloaded and unpacked__ on your local machine and __Visual Studio Code installed__ on your PC.

## Setup using WSL

To get MEHLISSA 2.0 up and running on your Windows machine with WSL, you will have to perform the following steps.

1. __Install WSL__: Open a shell with admin rights and run `wsl --install`.\
   Restart your computer after installing WSL successfully.
2. __Install__ the __WSL extension__ in Visual Studio Code
3. __Connect to WSL__: In the lower left corner, click on the green arrow symbol and choose 'Connect to WSL'.
4. __Open__ the __MEHLISSA 2.0 code__ in the WSL environment.\
   Go to File → Open Folder and specify the path in your mounted Windows system.\
   An example path could be `/mnt/c/Users/Name/Documents/MEHLISSA/mehlissa2.0`.
5. __Open__ a __terminal__ and run the following command to __compile__ MEHLISSA 2.0
   ```
   cd mehlissa2.0/src
   cmake .
   make clean all
   ```
6. __Run MEHLISSA 2.0__ from the terminal
   ```
   ./bin/MehlissaCancer [command line arguments]
   ```
__Note__: In some cases, you will have to install additional packages in your WSL environment. In our experience, these can include `cmake`, `g++`, and boost.
You can install all three via apt.
For boost, please also check the requirements listed [in the Boost documentation](https://www.boost.org/doc/user-guide/getting-started.html).

## Setup using MSYS2 and MinGW

To get MEHLISSA 2.0 up and running on your Windows machine with MSYS2 and MinGW, you will have to perform the following steps.
    
1. __Install__ the __C/C++ extension__ in Visual Studio Code.
2. Download and install __[MSYS2](https://www.msys2.org/)__.
3. __Open__ the __MSYS2 console__ and run
   ```
    $ pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain
   ```
   Accept the `default=all` configuration. In a second step, run
   ```
    $ pacman -S mingw-w64-x86_64-gcc
   ```
4. __Add MinGW to__ your __PATH__.\
   The default path should be `C:\msys64\mingw64\bin`.\
   Check that gcc/g++/gdb were installed correctly, for example, by running `gcc --version` in a new terminal.
5. __Install [Chocolatey](https://chocolatey.org/install)__ and use it to __install make__
   ```
    choco install make
   ```
    The command must be run from an admistrator shell.
6. __Install [cmake](https://cmake.org/download/)__.\
   Set MinGW as the compiler output by adding the system environment variable `CMAKE_GENERATOR` with the value `MinGW Makefiles`.
7. __Open__ the __MSYS2 console__ and __install Boost__.
   ```
    $ pacman -S mingw-w64-x86_64-boost
   ```
8. __Configure__ MEHLISSA 2.0's __`CMakeLists.txt`__\
   Add the boost install directory to the `CMAKE_PREFIX_PATH` and update the include path in the target_include_directories to your boost root directory.\
   Don't forget to escape the backslashes.\
   For example, `set(CMAKE_PREFIX_PATH "C:\\msys64\\mingw64\\lib\\cmake\\Boost-1.89.0")` and `target_include_directories(MehlissaCancer PUBLIC "C:\\msys64\\mingw64\\lib\\cmake\\Boost-1.89.0")`.
9. __Create__ the __output directories__ `bin` and `output` in the folder `mehlissa2.0`.
10. __Open Visual Studio Code__ in __`mehlissa2.0/src`__ and, in a terminal, run
    ```
    cmake .
    make clean all
    ```
    In case of linking problems, check that make uses mingw and not ucrt.
    If necessary remove ucrt via MSYS2 with\
    `$ pacman -R mingw-w64-ucrt-x86_64-gcc`.
12. __Run MEHLISSA 2.0__ from the terminal
    ```
    ./bin/MehlissaCancer [command line arguments]
    ```
