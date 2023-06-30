# Burnback-3d

Analysis of 3D burn surfaces for solid propellant rockets using tetrahedra based Time Marching Method as an alternative of the Level Set Method. Key features:

- Combustion time computation with tetrahedra meshes for solid propellant rockets
- Supports per node configuration of the recession speed
- Supports isotropic and anisotropic propellants
- Graphical interface to evaluate the results
- Import and export data of any mesh formats with a python script

If you only need a 2D analysis, see [burnback-qt](https://codeberg.org/iff/burnback-qt).

Built binaries for Windows and MacOS can be found at [releases](https://github.com/iffse/burnback-3d/releases). For Linux is advisable to compile from source as Qt has no compatibility across different distributions (binaries built with Ubuntu CI didn't work on my Arch Linux). It is also possible to run the Windows binary through Wine, with minor flickers.

Supports both light and dark theme. Should use accordingly to your system theme. If you want dark theme, and it isn't, add `QT_QUICK_CONTROLS_MATERIAL_THEME=Dark` to your environment variables.

![burnback-3d interface](img/interface.png)

The contour visualization is only for preview purposes. For a much more detailed visualization, try [ParaView](https://www.paraview.org/):

![result visualization in paraview](img/paraview.png)

## Usage

First, you will need a mesh in order to use the program for analysis. For instance, [Gmsh](https://gmsh.info/) is an open source meshing software that can generate 2D and 3D finite element mesh.

Once you have the mesh, you will have to convert it into a Json file with tetrahedra based information. A python script that converts Gmsh mesh file to this format using [meshio](https://github.com/nschloe/meshio) can be found at tools directory: [mesh_convert.py](./tools/mesh_convert.py). The script is optimized for speed and can convert a mesh with 200K nodes in seconds. If you are using another meshing tool, feel free to edit it (probable you will only need to change the cell names).

When using Gmsh and the script, you can define boundary conditions with physical groups with the following naming conventions in surfaces:

- `inlet 0`: The boundary is an inlet, where the propellant starts to burn. The number after the condition is the initial condition applied in that boundary.
- `outlet`: Used for boundaries where the combustion ends, like the shell of the container.
- `symmetry 0 0`: Used to indicate that a boundary defines a symmetry. Numbers after the condition is the angle of the normal vector to the symmetry plane, initially has the direction of `x`. The vector is rotated first in the `z` direction, and later in the `y` direction.
- `condition`: Placeholder for conditions that should be changed later in Burnback GUI (will be treated as outlet by default).

In volumes, you can define recession velocities:

- `recession 1 [0.5 0.2 45 30 25]` (optional): Used to indicate the recession velocity of a node, defaults to 1. When more than 1 number is specified, the velocity is considered to be anisotropic: First 3 numbers are the recession speed to the `x`, `y`, and `z` axis respectively, and the last 3 numbers are the rotation angles in degree with respect to axis `x`, `y`, and `z` respectively.

Everything after the names above will be added to a description field.

Example files of Gmsh can be found at [examples/gmsh](./examples/gmsh). The commands to be executed to obtain the Json file are:
```shell
gmsh -3 mesh.geo
python mesh_convert.py mesh.msh
```

If you want to use the exported results to another format other than Json (for instance `.CGNS`, or `.dat` for TecPlot/ParaView, etc.) you can use the [result_convert.py](./tools/result_convert.py) script. Usage is:
```shell
python result_convert.py result.json output.extension
```
You can provide only the extension name for the output file. In this case the name is inferred from the input file.

## Compiling

Can be compiled by either using command line or using the QtCreator. Binaries should be found at `<Project Dir>/target/debug|release`. When building with QtCreator, `<Project Dir>` equals to where the build location is set.

Qt modules dependencies:

- qt-3d
- qt-charts
- qt-declaratives
- qt-quickcontrols2

WARNING: Do not compile with Qt 5.15.2. The 3D model importer is broken for that version.

### Using command line

The project can be easily built with the `Makefile`. The `Makefile` is written with multiplatform compilation in mind, it should work with Linux, macOS, or Microsoft Windows:

- `make run`: Build the debug binary and run
	- `make run-sanitizer`: Build the debug binary with sanitizer and run
- `make debug`: Build the debug binary
- `make release`: Build the release binary

### Using QtCreator

Open `burnback-3d.pro` with QtCreator, set your compiling options if needed, and runs directly by clicking the play button at bottom-left.

