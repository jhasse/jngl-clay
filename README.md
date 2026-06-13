# jngl-clay

A [Clay](https://github.com/nicbarker/clay) renderer for [JNGL](https://github.com/jhasse/jngl).

Clay is a high-performance, immediate-mode UI layout library written in C. JNGL ("Just a
Neat Game Library") is an easy-to-use C++ game library. This project wires the two together so
you can build Clay user interfaces and have them drawn through JNGL's renderer — rectangles,
rounded rectangles, text, images, borders, and scissor/clipping are all supported.

The integration with Clay is done via [ClayMan](https://github.com/TimothyHoytBSME/ClayMan),
a C++ wrapper around Clay.

## Features

- Drop-in JNGL renderer for Clay's render command array (`drawClayCommands`)
- Text measurement callback backed by JNGL fonts (`measureText`)
- Renders rectangles, rounded rectangles, text, images, and borders (including rounded
  corners via arcs)
- Scissor / clipping support
- Optional overlay-color rendering (requires a newer Clay than the one ClayMan ships with)

## Repository layout

| Path | Description |
| --- | --- |
| `include/clay_renderer_jngl.hpp` | The renderer — `measureText` and `drawClayCommands`. This is the core of the project. |
| `src/main.cpp` | Minimal example using ClayMan: a centered label, image, and a clickable button. |
| `src/full-example.cpp` | A larger ClayMan-based example. |
| `src/transitions.cpp` | Example using Clay directly (without ClayMan). |
| `data/` | Sample assets used by the examples. |
| `jngl/`, `clay/`, `ClayMan/` | Git submodules for the dependencies. |

## Building

This project uses CMake and pulls its dependencies in as git submodules, so clone
recursively:

```bash
git clone --recursive https://github.com/jhasse/jngl-clay.git
cd jngl-clay
```

If you already cloned without `--recursive`:

```bash
git submodule update --init --recursive
```

Then configure and build:

```bash
cmake -Bbuild -GNinja
ninja -Cbuild
```

You may need JNGL's system dependencies installed first — see the
[JNGL build instructions](jngl/README.md) for the packages required on your platform.

### Running the examples

```bash
build/jngl-clay      # minimal example (src/main.cpp)
build/full-example   # larger ClayMan example (src/full-example.cpp)
build/transitions    # raw-Clay example (src/transitions.cpp)
```

## Usage

Include the renderer header, supply `measureText` to Clay/ClayMan, and call
`drawClayCommands` with the render command array Clay produces each frame:

```cpp
#include <clayman.hpp>
#include <clay_renderer_jngl.hpp>
#include <jngl/init.hpp>

class MyScene : public jngl::Scene {
    ClayMan clayMan{ width, height, measureText, nullptr };
    Clay_RenderCommandArray renderCommands{};

    void step() override {
        // ... update Clay state and build your layout ...
        renderCommands = clayMan.endLayout();
    }

    void draw() const override {
        drawClayCommands(renderCommands);
    }
};
```

See `src/main.cpp` for a complete, runnable example.
