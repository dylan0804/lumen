# Lumen

<img width="640" height="395" alt="demo" src="https://github.com/user-attachments/assets/026cb4dc-2461-4eb4-a7e4-98700ab2ca5c" />

A small OpenGL renderer written in C++ as a weekend project. With a textured floor, a lit capsule, and a color-cycling light orbiting the scene, with Phong lighting (ambient/diffuse/specular)

## Build

```
mkdir build && cd build
cmake ..<img width="640" height="395" alt="demo" src="https://github.com/user-attachments/assets/d142df49-f30a-4cac-a6f7-194de35dccb4" />

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
