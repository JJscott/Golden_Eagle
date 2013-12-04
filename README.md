Golden Eagle
============

Instructions 


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

- Or for x64, substitute:

```bat
cmake .. -G "Visual Studio 12 Win64"
```

- You now have a VS2013 solution
- In VS set 'game' as the startup project (right click on 'game' -> Set as StartUp Project.
