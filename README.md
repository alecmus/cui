# cui
cui framework, for rapid gui application development in C++ on Windows

## ABOUT THE LIBRARY
The cui framework is a library designed for the rapid development of modern, efficient and easy to maintain C++ applications with a Graphical User Interface (gui). It is part of the liblec libraries (https://github.com/alecmus/liblec).

## PREBUILT BINARIES
Prebuild binaries of the library can be found under releases: https://github.com/alecmus/cui/releases.

## BUILDING
Create a folder '\liblec' and clone the repository into it such that it resides in 'liblec\cui'. Open the Microsoft Visual Studio Solution file liblec\cui\cui.sln. Select Build -> Batch Build, then select the desired configurations of the given four:
1. Debug x86
2. Relese x86 (32 bit Release Build)
3. Debug x64
4. Release x64 (64 bit Release Build)

Build.

Three folders will be created in the \liblec directory, namely bin, lib and include. Below is a description of these subdirectories.

1. bin - contains the binary files. The following files will be created:

File            | Description
--------------- | ------------------------------------
cui32.dll    | 32 bit release build
cui64.dll    | 64 bit release build
cui32d.dll   | 32 bit debug build
cui64d.dll   | 64 bit debug build

2. lib - contains the static library files that accompany the dlls. The files are named after the respective dlls.
3. include - contains the include files

## LINKING TO THE LIBRARY

### Microsoft Visual Studio
Open your project's properties and for All Configurations and All Platforms set the following:
1. C/C++ -> General -> Additional Include Directories -> Edit -> New Line ... add \liblec\include
2. Linker -> General -> Additional Library Directories -> Edit -> New Line ... add \liblec\lib
3. Debugging -> Environment -> Edit ... add PATH=\liblec\bin;PATH%

Now you can use the required functions by calling #include <liblec/cui/...>

Build.

## DEPLOYING YOUR APPLICATION
If it's a 32 bit build you will need to deploy it with cui32.dll in the same folder. If it's a 64 bit build use the cui64.dll.