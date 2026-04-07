# gmorph2

**gmorph2** is a research code for 3D morphing between two meshes with arbitrary connectivities.
This code implements the following paper:

Takashi Kanai, Hiromasa Suzuki, Fumihiko Kimura:
“Metamorphosis of Arbitrary Triangular Meshes,”
IEEE Computer Graphics and Applications, Vol. 20, No. 2, pp. 62–75, March/April 2000.

This code is provided **for research purposes only and without any support**.
Originally, it was developed as code that runs on SGI workstations around 1997-1998.
This code was created earlier than the [Windows version](https://github.com/kanait/gmorph); the Windows version was later developed based on this code.
Recently, I updated the code to compile and run on Ubuntu 24.04, and fixed a few minor bugs that had been present since then.
Compared with the original 1997-era code, the harmonic-map sparse solver has also been replaced from `linbcg` to an Eigen-based BiCGSTAB implementation for licensing reasons.

## GUI: Qt 6 and `-gui`

The interactive GUI is implemented with **Qt 6** (`Widgets`, `OpenGL`, `OpenGLWidgets`): run `gmorph2b8 -gui` to open the Qt main window and OpenGL views.

As of the current tree, the legacy **Motif / X11** UI has been removed from the build: **you do not need OpenMotif or X11 development packages** to compile or run the Qt GUI.

## CLI-only target: `gmorph2b8_cli`

In addition to the default `gmorph2b8` target, CMake can build an extra executable **`gmorph2b8_cli`** (no Qt, no OpenGL) for batch processing only.

## Compilation

### Ubuntu / Debian

To compile the code, install the following libraries (via `apt`):

- cmake
- a C/C++ toolchain (`build-essential`)
- Qt 6 development packages (at least `qt6-base-dev`; OpenGL modules may be packaged separately depending on distro)
- OpenGL + GLU development packages (`libgl1-mesa-dev`, `libglu1-mesa-dev`)
- Eigen (`libeigen3-dev`)

Example:

```bash
sudo apt update
sudo apt install -y \
  build-essential cmake \
  qt6-base-dev \
  libgl1-mesa-dev libglu1-mesa-dev \
  libeigen3-dev
```

### macOS (Homebrew)

On a Mac, install **Qt 6**, **Eigen**, and **CMake**:

```bash
brew install cmake qt eigen
```

XQuartz / OpenMotif are **not required** for the current Qt GUI build.

### Build with CMake

To create the executable **gmorph2b8** (Qt GUI + CLI):

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

(On Linux you can use `make` instead of `cmake --build .` if you use the default Makefile generator.)

To also build the CLI-only executable **gmorph2b8_cli**:

```bash
cmake -S . -B build -DGMORPH_BUILD_CLI_ONLY_TARGET=ON
cmake --build build
```

## Usage

The `gmorph2b8` executable supports both command-line and GUI use.
Morphing PPD generation has been verified for the following GMH files under `data`.
(Figure numbers are those in the paper cited above.)

- bunny-tiger/bunny-tiger#1_SP001.gmh (Bunny's head - Tiger's head, rough corres., Figure 12(a))
- bunny-tiger/bunny-tiger_SP001.gmh (Bunny's head - Tiger's head, fine corres., Figure 12(b))
- Delorean-Porsche/Delorean-Porsche_SP.gmh (Delorean - Porsche, Figure 13)
- star-pai/spf1_6nSP0002.gmh (Star - Pai, Figure 14)
- torus-bottle/torus-bottle_SP008.gmh (Torus - Bottle, Figure 15)

To generate a morphing PPD from the command line, go to the folder that contains the GMH file and run:

```bash
../../build/gmorph2b8 in.gmh out.ppd
```

Basic CLI syntax:
`gmorph2b8 [options] <in.gmh> <out.ppd>`

This will generate the PPD file.
To start the GUI, use the following command:

```bash
../../build/gmorph2b8 -gui
```

This will start the GUI.

<p align="center">
  <img alt="gmorph2b8" src="./gmorph2b8.png" width="70%" />
</p>

### Command-line options (selected)

- `-div <n>`: morphing division number (default: 100; also affects the animation smoothness in GUI mode).
- `-smooth`: enable smooth shading.
- `-enh_disp`: enable enhanced display mode.
- `-rec <name>`: record morphing meshes as Wavefront OBJ files like `<name>001.obj`, `<name>002.obj`, ... (frame count follows `-div`).
- `-mphtosgi`: save morphing results to SGI image files.
- `-spath <sublength> <out.gmh>`: make shortest-path mode graph (CLI-only).

## Authors

- **[Takashi Kanai](https://graphics.c.u-tokyo.ac.jp/hp/en/)** - The University of Tokyo

## License

This software is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
