# Numerical mirage simulator

This is a simple mirage simulator written in C++17. It uses the simplest possible 2D / 3D model. The Earth is either flat or round, both without any elevations. The ambient air temperature (above a transitional layer) is uniform and constant, the surface temperature is also constant. The simulation involves solving a differential equation, for which a Runge--Kutta method is used, implemented by GNU Scientific Library. TODO include the article when published.

The 2D case is handled by the _eikonal_ command line application, which can be used to dump coordinates of ray segments.

The 3D case is handled by _main_ command line application. It simulates a pinhole camera watching a billboard.

Both simulators can handle various physical models:

- water. Here both the water and the ambient temperatures should be between 0 and around 15 degrees Celsius.
- conventional asphalt TODO docs
- porous asphalt TODO docs

## Installation

Both apps have been developed for Linux, but some changes allow also Cygwin to be used. By default they compile only for Linux. We have developed them under Ubuntu 20.04. External library requirements are (development version required):

- libpthread
- libpng 1.6
- Eigen3 3.3.7
- GNU Scientific Library 2.5
- png++

A the first four usually can be installed using the package manager. png++ might be missing (like in Cygwin), so download it from [here](https://www.nongnu.org/pngpp/). Further libraries are needed, but we have included them as git submodules:

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

_eikonal_ dumps only ray segment coordinates, so an other tool is needed for visualization. We have developed it with Octave or Matlab in mind, because they are partially compatible and are easy to use. By default it only writes the apparent mirror line direction (degrees from horizontal). To get the coordinates, one needs to run it with this option:

`./eikonal --silent false`

and leaving everything else on default values, it prints 4 vectors with the names `d`, `x`, `crity`, `mirry`. Here

- `x` is a set of common X coordinates for all other vectors holding Y coordinates.
- `d` contains the Y coordinates of the Earth surface (if not flat, which is default).
- `crity` is the critical ray, which is the steepest ray by the current settings which gets bent above the surface.
- `mirry` is the ray hitting the "mirror line", the horizontal line which seems to be the mirror axis between the normal image and the mirage.

so the plot `plot(x,d, x,crity, x,mirry)` looks like

![2D plot][2dplot]

_eikonal_ has some options, which affect the

- differential equation solver algorithm
- number of line segments
- simulated physics
- simulated geometry like distances, Earth form and radius.

The first two sorts of options have proven default values, so it is enough to deal with the physics and geometry options. Here, when `--dir` is not specified, the critical ray angle is computed and used. Please refer the help for more information:

`./eikonal --help`

### Iterations

We have provided a bash script to let _eikonal_ be used in an automated manner:

```bash
rm iterated.txt
bash iterateEikonal.sh <start> <diff> <count> <parameterToIterate> [rest of params to be passed to main]
```

The script accepts 4 parameters:

- start value
- difference to add to the start value each time
- iteration count
- iterated parameter name, together with `--` like `--tempAmb`

All other options will be appended as is to the called _eikonal_ command line. Output goes into iterated.txt.


## Rendering images

_main_ can be used to render images. Again, it is a pure command line app which uses PNG images for input and output. It has two inputs:

- the billboard contents, which defaults to be an RCA monoscope image.
- the water to copy below the rendered image.

The rays are traced individually, with possible subsampling for each pixel to produce better output. The invocation

`./main --resolution 665 --subsample 3` produces this image:

![Rendered subsample 3][rendered-subsample3]

As shown, by default no water is copied below the mirage. _main_ has these sorts of options:

- differential equation solver algorithm
- simulated physics
- simulated geometry like distances, Earth form and radius.
- rendering options including resolution, subsampling, marks and borders
- file names to use for input and output

The first sort of options have proven default values, so it is enough to deal with the rest. Please refer the help for more information:

`./main --help`

### Iterations

We have provided an other bash script to let _main_ be used in an automated manner:

`bash iterateMain.sh <start> <diff> <count> <parameterToIterate> [rest of params to be passed to main]`

The script accepts 4 parameters:

- start value
- difference to add to the start value each time
- iteration count
- iterated parameter name, together with `--` like `--tempAmb`

All other options will be appended as is to the called _main_ command line. Please do not specify output name, it will be `series<n>.png`


[2dplot]: images/2dplot.png "2D plot"
[rendered-subsample3]: images/rendered-subsample3.png "Rendered image using --subsample 3"
