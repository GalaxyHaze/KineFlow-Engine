<div align="center">

```
██╗  ██╗██╗███╗   ██╗███████╗███████╗██╗      ██████╗ ██╗    ██╗
██║ ██╔╝██║████╗  ██║██╔════╝██╔════╝██║     ██╔═══██╗██║    ██║
█████╔╝ ██║██╔██╗ ██║█████╗  █████╗  ██║     ██║   ██║██║ █╗ ██║
██╔═██╗ ██║██║╚██╗██║██╔══╝  ██╔══╝  ██║     ██║   ██║██║███╗██║
██║  ██╗██║██║ ╚████║███████╗██║     ███████╗╚██████╔╝╚███╔███╔╝
╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝╚══════╝╚═╝     ╚══════╝ ╚═════╝  ╚══╝╚══╝
```

**A C++ game framework where the right code is the easy code.**

[![License: MIT](https://img.shields.io/badge/License--MIT-blue?style=flat-square)](LICENSE)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-orange?style=flat-square&logo=c%2B%2B)](https://en.cppreference.com)
[![CMake](https://img.shields.io/badge/CMake-3.25+-green?style=flat-square&logo=cmake)](https://cmake.org)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey?style=flat-square)]()

</div>

---

## What is KineFlow?

KineFlow is a **C++ game framework** built around one principle: the pit of success. The good path should always be the obvious one. You don't configure a build system, you don't manage a game loop, and you don't call `new` freely. You write scenes — KineFlow handles the rest.

It is not an engine with an editor. It is not a library you configure by hand. It is an opinionated **C++ ecosystem** where you express *what* your game does, not *how* the infrastructure holds it together.

> KineFlow is also a living proof of concept for the design philosophy behind [Kalidous-Lang](https://github.com/GalaxyHaze/kalidous-lang) — showing that making the right path frictionless changes how software gets written.

---

## Core Philosophy

| Principle | What it means in practice |
|---|---|
| **Pit of success** | Doing things correctly is the path of least resistance |
| **Intent over infrastructure** | You declare scenes, transitions, and resources — never touch CMake |
| **Auditable by default** | The allocator owns everything; nothing is hidden from the runtime |
| **Complexity is opt-in** | A game loop, a scene, and a return value. That's the whole API surface to start |

---

## Getting Started

KineFlow is a **GitHub template**. No installer, no CLI required at this stage — clone it and you already have a working C++ project.

```bash
# 1. Use this repo as a template on GitHub, or clone directly
git clone https://github.com/GalaxyHaze/kineflow my-game
cd my-game

# 2. Configure your project
nano kineflow.toml

# 3. Build & run
cmake -B build -G Ninja
cmake --build build
./build/my-game
```

---

## Configuration

The only file you ever touch is `kineflow.toml`. CMake is an implementation detail — it reads this file and configures itself.

```toml
[project]
name    = "my-game"
version = "0.1.0"

[runtime]
fps_cap    = 60
vsync      = true
fixed_step = false   # true → onUpdate receives a fixed dt

[window]
title  = "My Game"
width  = 1280
height = 720

[features]
audio      = true
physics    = false
networking = false
```

Toggle features on or off. KineFlow will include or exclude the relevant dependencies automatically — no `#ifdef` sprawl in your code.

---

## The Scene System

Every screen in your game is a `Scene`. You override `work()`, and you return the next `Scene*`. Control flow **is** the return value.

```cpp
#include <kineflow/scene.hpp>

class MainMenu : public Scene {
public:
    Scene* work() {
        // your menu logic here

        if (user_clicked_play)
            return allocator.new<GameScene>();

        if (user_clicked_quit)
            return nullptr;   // nullptr = exit, destructor handles cleanup
    }
};
```

```cpp
#include <kineflow/scene.hpp>
#include "MainMenu.hpp"

class GameScene : public Scene {
    Player* player;

public:
    Scene* work() {
        player = allocator.new<Player>(/* args... */);

        // offload AI updates — doesn't block the frame
        .parallel([&] { ai.update(dt); });

        // stream next level in the background
        .async([&] { assets.stream("world_2"); });

        if (player_died)
            return allocator.new<MainMenu>();

        return nullptr;
    }
};
```

There is no `int main()` to write. The entry point is already in the framework. You only create scenes.

---

## Concurrency Primitives

Exposed directly from the `Scene` parent — no setup, no thread handles.

| Primitive | Behaviour |
|---|---|
| `.async(fn)` | Runs `fn` asynchronously; does not block the current frame |
| `.parallel(fn)` | Runs `fn` alongside `work()` within the same frame |
| `.daemon(fn)` | Runs for the lifetime of the engine, independent of any scene |

`.daemon` is the right place for background music, auto-save, analytics — anything that outlives a single scene.

---

## The Allocator

There is no raw `new`. Every heap allocation goes through the scene's arena allocator.

```cpp
// allocate a Player with constructor arguments
Player* p = allocator.new<Player>(x, y, health);

// allocate a batch — returns a span
auto enemies = allocator.new_array<Enemy>(32);
```

The runtime owns all memory. This means:
- **No leaks** — everything tied to a scene is freed when the scene exits
- **No dangling pointers** across scene transitions
- **Full auditability** — the engine knows every live allocation at any point in time
- **Future-proof** — a debug inspector can enumerate all objects in any scene without instrumentation

---

## Project Structure

After cloning the template, your project looks like this:

```
my-game/
├── kineflow.toml          ← your only configuration file
├── src/
│   └── scenes/
│       ├── MainMenu.hpp
│       └── GameScene.hpp
├── assets/
│   ├── textures/
│   └── audio/
└── CMakeLists.txt         ← generated & managed by KineFlow, do not edit
```

---

## Roadmap

KineFlow is designed to grow one meaningful layer at a time. Each phase is independently useful.

- [x] **Phase 1** — Template repo: CMake pre-configured, TOML-driven, scene system
- [ ] **Phase 2** — Asset pipeline: texture, audio, and font loading with automatic caching
- [ ] **Phase 3** — Boilerplate CLI: `kineflow new <scene>`, `kineflow build`, `kineflow run`
- [ ] **Phase 4** — Debug inspector: real-time scene graph viewer and entity inspector via ImGui

---

## Why not Godot / Unity / Unreal?

Those are great tools for their use cases. KineFlow exists to answer a different question:

> What does a C++ game framework look like when the *framework itself enforces good practices*, rather than documenting them?

KineFlow is also a teaching tool. It demonstrates concretely why a language like [Kalidous](https://github.com/GalaxyHaze/kalidous-lang) is designed the way it is — ownership semantics, controlled allocation, explicit flow — applied to something you can run and play.

---

## License

MIT — see [LICENSE](LICENSE) for details.

---

<div align="center">

*Built with intent. No boilerplate. No excuses.*

</div>