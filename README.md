# Lumen

A small OpenGL renderer written in C++ as a weekend project. With a textured floor, a lit capsule, and a color-cycling light orbiting the scene, with Phong lighting (ambient/diffuse/specular)

## Build

```
mkdir build && cd build
cmake ..
cmake --build .
```

## Run

Run from the project root (not from inside `build/`), since shaders/textures are loaded by relative path:

```
./build/app
```

## Controls

- `W` / `A` / `S` / `D` — move
- Mouse — look around
- `Esc` — quit
