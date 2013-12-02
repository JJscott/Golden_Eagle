Golden Eagle
============

CMake stuff is there, I don't know howto use in on Windows, but if someone would try and document exactly how to do it here, that'd be create.

Building
========

@Neo: shouldn't we be building in a './build/' subdirectory instead of in source? - Ben

Linux
-----

(Assuming cloned to directory named "Golden_Eagle").

```bash
cd Golden_Eagle
cmake .
make
./bin/Golden

```

Windows
-------

- Install cmake for Windows (win32 installer from http://www.cmake.org/cmake/resources/software.html)
- Add cmake to your path if it doesn't
- from cmd.exe:
```bat
cd Golden_Eagle
cmake . -G "Visual Studio 12"
```
- you now have a VS2013 solution
