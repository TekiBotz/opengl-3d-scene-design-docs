# OpenGL 3D Scene

An interactive 3D scene rendered against an OpenGL 3.3 core context: a textured telescope on a tripod
sitting on a ground plane. You fly around it with the mouse and keyboard and switch between
perspective and orthographic projection. Written in C++ for the Visual Studio 2022 toolchain, with
GLFW for the window and input, GLAD for GL function loading, GLM for matrix math, and stb_image for
texture decode.

## Tech stack

| Piece | Version | Role | In the repo? |
|-------|---------|------|--------------|
| C++ / MSVC | v143 toolset (VS 2022) | language + compiler | project files only |
| OpenGL | 3.3 core context; `#version 330` shaders | rendering API | — |
| GLAD | 0.1.x loader, generated for gl 4.3 core, no extensions | GL function loader | yes — `glad.c` plus `include/glad/glad.h` and `include/KHR/khrplatform.h` |
| GLFW | 3.x, `lib-vc2019` build | window, input, GL context | no, you supply `glfw3.lib` and the headers |
| GLM | any recent 0.9.9+ | vectors + matrices (header-only) | no, you supply the headers |
| stb_image | v2.28 | JPEG decode for textures | yes (`stb_image.h`) |

## Repository layout

```
opengl-3d-scene-design-docs/
├── Telescope.sln                  VS 2022 solution
├── Telescope/
│   ├── Source.cpp                 scene geometry, render loop, GLFW input callbacks
│   ├── camera.h                   fly-camera class, adapted from LearnOpenGL
│   ├── glad.c                     GLAD loader source
│   ├── include/
│   │   ├── glad/glad.h            GLAD header (vendored, gl 4.3 core)
│   │   └── KHR/khrplatform.h      Khronos platform header GLAD depends on
│   ├── stb_image.h                stb_image v2.28
│   ├── telescopeLeg.jpeg          texture for the legs and the tube
│   ├── blackPlastic.jpeg          texture for the tripod feet
│   ├── carpet.jpeg                committed but never loaded
│   ├── Telescope.vcxproj          only the Debug|Win32 configuration is set up
│   └── Telescope.vcxproj.filters
├── LICENSE
└── README.md
```

## What the scene contains

Everything is built from four primitive meshes, each reused several times with different model
matrices:

- **Legs** — a textured cube, drawn three times, scaled long and thin and rotated out into a tripod.
- **Feet** — an indexed pyramid, drawn three times at the base of each leg.
- **Tube, eyepiece, finder scope** — a 9-vertex wedge drawn six times at 60° increments around its
  axis, which sweeps out a hexagonal prism. The same mesh at three different scales and positions
  makes the main tube plus the two smaller cylinders.
- **Ground plane** — an indexed box flattened on one axis.

Legs and tube sample `telescopeLeg.jpeg`; the feet sample `blackPlastic.jpeg`. There is no lighting
model. The fragment shader samples the bound texture and writes it straight to the framebuffer.

## Controls

| Input | Action |
|-------|--------|
| W / A / S / D | move forward / left / back / right |
| Q / E | move up / down |
| Left mouse button + drag | look around |
| Scroll wheel | zoom (changes perspective FOV) |
| P | toggle orthographic vs perspective projection |
| Esc | quit |

The scene starts in orthographic projection.

## Building

This is a Visual Studio project, so it builds on Windows with MSVC. There is no CMake setup.

### 1. Install Visual Studio 2022

Include the "Desktop development with C++" workload (v143 toolset, Windows 10/11 SDK).

### 2. Get the dependencies

GLAD and stb_image are vendored, so you only need to supply **GLM** and **GLFW**. `Telescope.vcxproj`
looks for them under `C:\OpenGL`:

```
C:\OpenGL\
├── glm\                        GLM headers, so <glm/glm.hpp> resolves
└── GLFW\
    ├── include\                <GLFW/glfw3.h>
    └── lib-vc2019\glfw3.lib
```

GLM is header-only; download the release zip and drop the `glm` folder in place. For GLFW, use the
precompiled Windows binaries and point at the `lib-vc2019` variant, which the project links as
`glfw3.lib`. If you keep the libraries elsewhere, edit the paths in **Telescope → Properties →
VC++ Directories** instead of recreating `C:\OpenGL`.

The GLAD headers in `Telescope/include` were generated with glad 0.1.36 for gl 4.3 core (no
extensions) and check out against the committed `glad.c` (0.1.33), which targets the same symbol set.
To regenerate them, use <https://glad.dav1d.de> or `pip install glad` with:

```
glad --profile core --api gl=4.3 --generator c --spec gl --extensions "" --out-path glad_out
```

then copy `glad_out/include/glad/glad.h` and `glad_out/include/KHR/khrplatform.h` into
`Telescope/include` (and `glad_out/src/glad.c` over `Telescope/glad.c` if you want a matched pair).

### 3. Build and run

1. Open `Telescope.sln`.
2. Set the configuration to **Debug** and the platform to **x86**. The x64, ARM64, and Release
   configurations exist but have no include or library paths, so they will not build.
3. Press F5.

Textures load by relative path (`stbi_load("telescopeLeg.jpeg", ...)`), so the working directory has
to be the `Telescope` project folder. That is the default when you run from Visual Studio. Running
`Telescope.exe` straight out of `Debug\` will fail to find the JPEGs.

## Known issues

- Only `Debug|Win32` is configured. The other five configurations are the untouched VS template and
  will not compile.
- The GLFW context is requested as OpenGL 3.3, but the vendored GLAD headers are generated for 4.3.
  The 3.3 subset is all that gets used, so this is harmless; regenerate at gl 3.3 if you want them to
  match exactly.
- `carpet.jpeg` is committed but never loaded.
- `P` has no debounce, so holding the key flips the projection every frame instead of once per press.
- The ground plane's vertex layout has no texture coordinates and no texture is bound when it draws,
  so it renders flat black rather than textured.
- `Source.cpp` builds its geometry inline as large float arrays with no shader-loading or mesh
  abstraction. It works but there's a lot of repetition between the leg, peg, and cylinder draw loops.

## License

MIT, see [LICENSE](LICENSE).
