## Cantor

Cantor is a KDE application aiming to provide a nice interface
for doing mathematics and scientific computing. It doesn't implement
its own computation logic, but instead is built around different
backends.

## Available Backends

- Julia programming language: https://julialang.org/
- KAlgebra for calculation and plotting: https://edu.kde.org/kalgebra/
- Lua programming language: https://www.lua.org/
- Maxima computer algebra system: https://maxima.sourceforge.net/
- Octave for numerical computation: https://gnu.org/software/octave/
- Python 2 programming language: https://www.python.org/
- Python 3 programming language: https://www.python.org/
- Qalculate desktop calculator: https://qalculate.github.io/
- R project for statistical computing: https://www.r-project.org/
- Sage mathematics software: https://www.sagemath.org/
- Scilab for numerical computation: https://www.scilab.org/

## How To Build and Install Cantor

To build and install Cantor, download the source archive from

- https://github.com/KDE/cantor/releases

and extract it, then follow the steps below:

```
cd cantor
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install -DCMAKE_BUILD_TYPE=RELEASE
make
make install  # or su -c 'make install'
```

If `-DCMAKE_INSTALL_PREFIX` is not used, Cantor will be installed in
default cmake install directory (`/usr/local/` usually).

To uninstall the project:
```
make uninstall  # or su -c 'make uninstall'
```
