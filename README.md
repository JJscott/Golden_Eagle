Golden Eagle
============

CMake stuff is there, I don't know howto use in on Windows, but if someone would try and document exactly how to do it here, that'd be create.

Building
========

Assuming cloned to directory named "Golden_Eagle".

Linux
-----

```bash
cd Golden_Eagle
mkdir build
cd build
cmake ..
make
./bin/Golden

```

Windows
-------

- Install cmake for Windows (win32 installer from http://www.cmake.org/cmake/resources/software.html)
- Add cmake to your path if it doesn't
- From cmd.exe:

```bat
cd Golden_Eagle
mkdir build
cd build
cmake .. -G "Visual Studio 12"
```

- You now have a VS2013 solution

- You will need to select the correct startup project, by right clicking and choosing set as startup project in the context menu.
