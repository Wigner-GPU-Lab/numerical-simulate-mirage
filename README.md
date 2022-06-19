# Numerical mirage simulator

This is a simple mirage simulator written in C++17. It uses the simplest possible 2D / 3D model. The Earth is either flat or round, both without any elevations. The ambient air temperature (above a transitional layer) is uniform and constant, the surface temperature is also constant. 

The 2D case is handled by the _eikonal_ command line application, which can be used to dump coordinates of ray segments.

The 3D case is handled by _main_ command line application. It simulates a pinhole camera watching a billboard.

Both simulators can handle various physical models:

- water. Here both the water and the ambient temperatures should be between 0 and around 15 degrees Celsius.
- conventional asphalt TODO docs
- porous asphalt TODO docs

## Installation

Both apps have been developed for Linux, but some changes allow also Cygwin to be used. By default they compile only for Linux. I have developed them under Ubuntu 20.04. External library requirements are (development version required):

- libpthread
- libpng 1.6
- Eigen3 3.3.7
- png++

A the first three usually can be installed using the package manager. png++ might be missing (like in Cygwin), so download it from [here](https://www.nongnu.org/pngpp/). Further libraries are needed, but I have included them as git submodules:

- [CLIUtils/CLI11](https://github.com/CLIUtils/CLI11)
- [hauptmech/eigen-initializer_list](https://github.com/hauptmech/eigen-initializer_list)

Steps to download and compile:

1. `git clone --recurse-submodules https://gitlab.wigner.hu/bamer.balazs/numerical-simulate-mirage.git`
2. `cd numerical-simulate-mirage/`
3. `ln -s /usr/include/eigen3/ eigen3`
4. `ln -s /usr/include/png++/ png++`
5. `cmake .`
6. `make`

Steps 3 and 4 may need other directories to symlink to, for example when Eigen3 resides in an other system directory or png++ have been downloaded somewhere in the home.

## Drawing rays

_eikonal_ dumps only ray segment coordinates, so an other tool is needed for visualization. I have developed it with Octave or Matlab in mind, because they are partially compatible and are easy to use. By default it only writes the apparent mirror line direction (degrees from horizontal). To get the coordinates, one needs to run it with this option:

`./eikonal --silent false`

and it prints 4 vectors with the names `d`, `x`, `crity`, `mirry`. Here

- `x` is a set of common X coordinates for all other vectors holding Y coordinates.
- `d` contains the Y coordinates of the Earth surface (if not flat, which is default).
- `crity` is the critical ray, which is the steepest ray by the current settings which gets bent above the surface.
- `mirry` is the ray hitting the "mirror line", the horizontal line which seems to be the mirror axis between the normal image and the mirage.

so the plot `plot(x,d, x,crity, x,mirry)` looks like

![2D plot][2dplot]

_eikonal_ has some options, which 


[2dplot]: images/2dplot.png "2D plot"
