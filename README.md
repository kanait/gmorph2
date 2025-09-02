# gmorph2

**gmorph2** is a research code for 3D morphing between two meshes with arbitrary connectivities.
This code implements the following paper:

Takashi Kanai, Hiromasa Suzuki, Fumihiko Kimura:
“Metamorphosis of Arbitrary Triangular Meshes,”
IEEE Computer Graphics and Applications, Vol. 20, No. 2, pp. 62–75, March/April 2000.

This code is provided **for research purposes only and without any support**.
I no longer remember the details, as I originally created it around 1997.

I have successfully compiled this code on Fedora 24 Linux and, more recently, on Ubuntu 24.04.
To compile the code, you need to install the following libraries:

For **Fedora** (via `dnf`):
- motif-devel
- mesa-libGL-devel
- mesa-libGLU-devel
- mesa-libGLw-devel
- libXpm-devel

For **Ubuntu** (via `apt`):
- libmotif-dev
- libgl1-mesa-dev
- libglu1-mesa-dev
- libglw1-mesa-dev
- libxpm-dev

After installing these libraries, simply run:

```bash
make
```

to create the executable file **gmorph2b8**.

Takashi Kanai


gmorph2 is a research code for 3D morphing between two meshes with arbitrary connectivies.
This code is an implementation of the following paper:

Takashi Kanai, Hiromasa Suzuki, Fumihiko Kimura:
"Metamorphosis of Arbitrary Triangular Meshes,"
IEEE Computer Graphics and Applications, Vol.20, No.2, pp.62-75, March/April 2000.

You can use this code only for research purpose *with no support*. I don't remember its contents because I created this code around 1997.

Now I've succeeded to compile this code on Fedora 24 linux and later on ubuntu 24.04.
To compile this code, you have to install the following library by using dnf (for Fedora):

- motif-devel
- mesa-libGL-devel
- mesa-libGLU-devel
- mesa-libGLw-devel
- libXpm-devel

or apt (for ubuntu): 

- libmotif-dev
- libgl1-mesa-dev
- libglu1-mesa-dev
- libglw1-mesa-dev
- libxpm-dev

After installing these libraries, simply

% make

to create an exacutable file (gmorph2b8).

Takashi Kanai



