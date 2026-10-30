# VAST 2009 Explorer

This project was completed as a final take-home assignment for Visualization and Image Processing for Cyber Security at Wright State University.

## Build

Requirements:
- C++17 compiler
- CMake 3.20 or newer
- VTK 9 with OpenGL rendering and JPEG/PNG support

### Configure and build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### Run the interactive explorer

```sh
./build/vast-explorer --data .
```

### Run the analysis-only build

```sh
cmake -S . -B build-core -DVAST_BUILD_GUI=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build-core --parallel
./build-core/vast-analyze --data . --output reports
```

### Using a custom VTK installation

If VTK is not in the default system path, point CMake to it:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/path/to/vtk
```

or:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVTK_DIR=/path/to/lib/cmake/vtk-9.x
```
