Golden Eagle
============

CMake stuff is there, I don't know howto use in on Windows, but if someone would try and document exactly how to do it here, that'd be create.

Building
========

Do all building in Golden_Eagle/build so it doesnt pollute the source tree.

@Neo: clean this up when cmake actually works? How do we get GLFW as a nice dependency?

Assuming cloned to directory named "Golden_Eagle".

Linux
-----

```bash
cd Golden_Eagle
mkdir build
cd build
cmake ..
make
./???/Golden

```

Windows
-------

- Install cmake for Windows (win32 installer from http://www.cmake.org/cmake/resources/software.html)
- Add cmake to your path if it doesn't
- from cmd.exe:

```bat
cd Golden_Eagle
mkdir build
cd build
cmake .. -G "Visual Studio 12"
```

- you now have a VS2013 solution
