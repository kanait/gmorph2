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

The interactive GUI is implemented with **Qt 6** (`Widgets`, `OpenGL`, `OpenGLWidgets`): run `gmorph2b8 -gui` to open the Qt main window and OpenGL views. The original **Motif / X11** sources are still compiled and linked into the same binary (shared data structures and callbacks), so you still need OpenMotif and X11 development packages at build time even if you only use the Qt interface. On the command line, morphing works without displaying a Qt window.

## Compilation

### Ubuntu / Debian

To compile the code, install the following libraries (via `apt`):

- libmotif-dev
- libgl1-mesa-dev
- libglu1-mesa-dev
- libglw1-mesa-dev
- libeigen3-dev
- libxpm-dev

You may also need additional libraries required by the packages above (for example: `libx11-dev`, `libxext-dev`, `libxt-dev`, `libxi-dev`, `libsm-dev`, `libice-dev`).

### macOS (Homebrew)

On a Mac, install **X11 client libraries**, **OpenMotif**, **XQuartz** (for X11 headers/libraries, `GLw`, and a `libGL` that exposes **GLX** — Apple’s `OpenGL.framework` does not provide `glX*`, which this tree still uses from the legacy GL code path), plus **Qt 6** and **Eigen** for the current build:

```bash
brew install libx11 libxext libxt libxi libsm libice libxpm openmotif
brew install qt eigen
brew install --cask xquartz
```

After installing the XQuartz cask, complete its installer if prompted; a **logout/login** (or reboot) is often needed before `/opt/X11` and the X11 environment are picked up reliably. Ensure `pkg-config` can see the `.pc` files (CMake prepends common paths such as `/opt/homebrew/lib/pkgconfig` or `/usr/local/lib/pkgconfig` and `/opt/X11/lib/pkgconfig`, depending on your machine).

If `libGL` with GLX is not found, install Mesa as an alternative to satisfy linking: `brew install mesa` (see the CMake configuration messages).

### Build with CMake

To create the executable **gmorph2b8**:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

(On Linux you can use `make` instead of `cmake --build .` if you use the default Makefile generator.)

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
To start the GUI, use the following command. Note that the GUI requires an X11/GLX environment (an active X server).

```bash
../../build/gmorph2b8 -gui
```

This will start the GUI.

<p align="center">
  <img alt="gmorph2b8" src="./gmorph2b8.png" width="70%" />
</p>

### Command-line options (selected)
* `-div <n>`: morphing division number (default: 100; also affects the animation smoothness in GUI mode).
* `-smooth`: enable smooth shading.
* `-enh_disp`: enable enhanced display mode.
* `-rec <name>`: record morphing meshes as Wavefront OBJ files like `<name>001.obj`, `<name>002.obj`, ... (frame count follows `-div`).
* `-mphtosgi`: save morphing results to SGI image files.
* `-spath <sublength> <out.gmh>`: make shortest-path mode graph (CLI-only).

## Authors

* **[Takashi Kanai](https://graphics.c.u-tokyo.ac.jp/hp/en/)** - The University of Tokyo

## License

This software is licensed under the MIT License - see the [LICENSE](LICENSE) file for details. 
