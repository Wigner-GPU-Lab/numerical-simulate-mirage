# Numerical mirage simulator

This is a simple mirage simulator written in C++17. It uses the simplest possible 2D / 3D model. The Earth is either flat or round, both without any elevations. The ambient air temperature (above a transitional layer) is uniform and constant, the surface temperature is also constant. 

The 2D case is handled by the _eikonal_ command line application, which can be used to dump corrdinates of a ray.

The 3D case is handled by _main_ command line application. It simulates a pinhole camera watching a billboard.

## Installation
