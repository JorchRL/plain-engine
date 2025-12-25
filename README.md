# Plain Engine - Advanced Racing Game Engine

A sophisticated, cross-platform racing game engine built with **C/C++** and composability in mind. Designed for incremental development from simple games to iRacing-level racing simulations.

## 🎯 Philosophy

- **Platform Independence**: Handmade Hero-style platform abstraction layer
- **Plain C Style**: Minimal C++ features, maximum portability and performance
- **Composable Architecture**: Modular systems that can be combined and extended
- **Game as Editor**: Media Molecule approach with hot-reloading and immediate-mode UI
- **Test-Driven Development**: Comprehensive testing with Google Test and headless testing
- **Performance First**: Optimized for 60 FPS on all target platforms

## 📁 Project Structure

```
plain-engine/
├── cpp/
│   ├── lib/
│   │   └── engine/
│   │       ├── include/
│   │       │   ├── platform/          # Platform abstraction layer
│   │       │   ├── physics/           # Physics system (Wassimulator-inspired)
│   │       │   ├── entity/            # Entity system (ECS)
│   │       │   ├── input/             # Input system (platform-agnostic)
│   │       │   ├── resources/         # Resource system (format-agnostic)
│   │       │   ├── rendering/         # Rendering system (multi-API)
│   │       │   ├── audio/             # Audio system (3D spatial)
│   │       │   ├── multiplayer/       # Multiplayer system
│   │       │   ├── testing/           # Testing framework (TDD)
│   │       │   ├── game.h             # Game base class
│   │       │   ├── run.h              # Engine run function
│   │       │   └── IGame.h            # Core interface definitions
│   │       └── src/
│   │           ├── platform/           # Platform implementations
│   │           ├── physics/            # Physics implementations
│   │           ├── entity/             # Entity implementations
│   │           ├── input/              # Input implementations
│   │           ├── resources/          # Resource implementations
│   │           ├── rendering/          # Rendering implementations
│   │           ├── audio/              # Audio implementations
│   │           ├── multiplayer/        # Multiplayer implementations
│   │           ├── testing/            # Test implementations
│   │           └── run.cpp             # Game loop implementation
│   ├── tests/                         # Comprehensive test suite
│   │   ├── unit/                   # Unit tests
│   │   ├── integration/             # Integration tests
│   │   ├── performance/             # Performance benchmarks
│   │   └── data/                   # Test data
│   ├── tools/                         # Development tools
│   │   ├── editor/                 # In-game editor
│   │   ├── asset_builder/           # Asset processing tools
│   │   └── debug/                  # Debug utilities
│   ├── src/                          # Game implementation
│   │   ├── game.cpp               # Main game implementation
│   │   ├── components/             # Game-specific components
│   │   ├── systems/                # Game-specific systems
│   │   └── assets/                 # Game assets
│   ├── CMakeLists.txt                # Build configuration
│   └── Makefile                      # Alternative build
├── SPEC_OVERVIEW.md                   # Executive specification overview
├── SPEC_PLATFORM.md                   # Platform layer specification
├── SPEC_PHYSICS.md                   # Physics system specification
├── SPEC_ENTITY.md                    # Entity system specification
├── SPEC_INPUT.md                     # Input system specification
├── SPEC_RESOURCES.md                 # Resource system specification
├── SPEC_RENDERING.md                 # Rendering system specification
├── SPEC_AUDIO.md                     # Audio system specification
├── SPEC_MULTIPLAYER.md               # Multiplayer system specification
├── SPEC_TESTING.md                   # Testing strategy specification
├── SPEC_ROADMAP.md                   # Implementation roadmap
└── README.md                         # This file
```

## 🚀 Quick Start

### Prerequisites
- C++20 compatible compiler
- CMake 3.25+ 
- Raylib (auto-fetched by CMake)

### Building and Running
```bash
# Using Make (recommended)
cd cpp
make
./build/debug/bin/pacman

# Or using CMake directly
cd cpp
mkdir build && cd build
cmake .. && make
./bin/pacman
```

## 🏗️ Architecture

### Engine Systems

The engine provides a comprehensive system architecture:

```cpp
// Core Foundation
Platform Layer      // Handmade Hero-style abstraction
Memory System       // Arena allocators, tracking
Threading System     // Job-based parallel processing
Testing Framework    // TDD with Google Test

// Core Engine Systems
Entity System        // Data-oriented ECS with hot-reloading
Physics System       // Wassimulator-inspired vehicle simulation
Resource System      // Format-agnostic asset management
Input System         // Platform-agnostic input with action mapping
Rendering System     // Multi-API rendering (OpenGL/Metal/DirectX/Vulkan)
Audio System         // Real-time 3D spatial audio

// Advanced Systems
Multiplayer System   // Authoritative server with client prediction
Editor System       // In-game editor with immediate-mode UI
Debug Tools         // Comprehensive debugging and profiling
```

### Core Interfaces

```cpp
// Game lifecycle
class Game : public IGame {
public:
    virtual void onStart() = 0;                    // Called once when game starts
    virtual void onUpdate(float deltaTime) = 0;     // Called every frame
    virtual void onRender() = 0;                  // Render game objects
    virtual bool wantsToQuit() const override { return false; }
};

// Component system
typedef struct Entity { uint32_t id, uint32_t generation; } Entity;
typedef struct Component { ComponentType type; void* data; } Component;

// Physics pipeline
typedef struct PhysicsStage { 
    const char* name; 
    void (*update)(PhysicsWorld* world, float deltaTime); 
} PhysicsStage;

// Rendering abstraction
typedef struct GraphicsContext {
    GraphicsAPI api;
    GraphicsCapabilities capabilities;
    void* platformContext;
} GraphicsContext;
```

### Example Racing Game

A simplified racing game showing core engine usage:

```cpp
class RacingGame : public engine::Game {
private:
    Entity playerVehicle;
    Entity track;
    VehicleComponent* vehicleComp;
    PhysicsWorld* physicsWorld;
    Renderer* renderer;
    
public:
    void onStart() override {
        // Create physics world
        physicsWorld = PhysicsWorld_Create(&physicsConfig, arena);
        
        // Create player vehicle
        playerVehicle = EntityManager_CreateEntity(entityManager);
        vehicleComp = EntityManager_AddComponent(entityManager, playerVehicle, 
                                              COMPONENT_VEHICLE, NULL);
        VehicleComponent_Create(physicsWorld, vehicleComp, "sports_car.json");
        
        // Create track
        track = EntityManager_CreateEntity(entityManager);
        TrackComponent* trackComp = EntityManager_AddComponent(entityManager, track, 
                                                        COMPONENT_TRACK, NULL);
        TrackComponent_Create(trackComp, "silverstone_track.json");
        
        // Setup rendering
        renderer = Renderer_Create(window, GRAPHICS_API_OPENGL);
        Renderer_AddRenderObject(renderer, &vehicleRenderObject);
        Renderer_AddRenderObject(renderer, &trackRenderObject);
    }
    
    void onUpdate(float deltaTime) override {
        // Update vehicle physics
        VehicleComponent_Update(vehicleComp, deltaTime);
        PhysicsWorld_Update(physicsWorld, deltaTime);
        
        // Update entity positions from physics
        Vec3 vehiclePos = PhysicsComponent_GetPosition(physicsWorld, vehicleComp->chassisBodyId);
        TransformComponent_SetPosition(vehicleTransform, vehiclePos);
        
        // Handle input
        const InputState* input = InputManager_GetState(inputManager);
        VehicleComponent_HandleInput(vehicleComp, input);
    }
    
    void onRender() override {
        Renderer_BeginFrame(renderer);
        Renderer_Render(renderer);
        Renderer_EndFrame(renderer);
    }
};
```

## 🚀 Getting Started

### Prerequisites
- **C++20** compatible compiler (GCC 10+, Clang 12+, MSVC 2022+)
- **CMake 3.25+** for build system
- **Google Test** for testing framework (auto-fetched)
- **Platform SDKs**: 
  - Windows: Windows 10 SDK
  - macOS: Xcode 14+
  - Linux: Mesa development libraries
- **Optional Graphics APIs**:
  - OpenGL 4.1+ (fallback)
  - Metal (macOS)
  - DirectX 11/12 (Windows)
  - Vulkan (optional)

### Building

```bash
# Clone repository
git clone <repository-url>
cd plain-engine

# Configure and build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# Run tests
ctest --parallel 4

# Run example game
./bin/racing_game
```

### Build Options
```bash
# Debug build with testing
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTING=On

# Release build
cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTING=Off

# Platform-specific builds
cmake .. -DTARGET_PLATFORM=Windows    # Windows
cmake .. -DTARGET_PLATFORM=macOS       # macOS
cmake .. -DTARGET_PLATFORM=Linux        # Linux
cmake .. -DTARGET_PLATFORM=WebAssembly  # Web (future)
```

### Testing
```bash
# Run all tests
make test

# Run specific test suites
./tests/unit_tests
./tests/integration_tests
./tests/performance_tests

# Generate coverage report (Debug builds)
make coverage
```

## 🎨 System APIs

### Rendering (Multi-API Abstraction)
```cpp
// Multi-API rendering (OpenGL/Metal/DirectX/Vulkan)
Renderer* renderer = Renderer_Create(window, preferredAPI);
Renderer_BeginFrame(renderer);
Renderer_Render(renderer);
Renderer_EndFrame(renderer);

// 2D rendering
Renderer2D_DrawSprite(renderer2D, &sprite);
Renderer2D_DrawText(renderer2D, &text);
Renderer2D_DrawDebugPhysics(renderer2D, physicsWorld);

// 3D rendering
Renderer_AddRenderObject(renderer, &renderObject);
Renderer_SetActiveCamera(renderer, cameraIndex);
Renderer_EnableShadows(renderer, true);
```

### Physics (Wassimulator-Inspired)
```cpp
// Composable physics pipeline
PhysicsWorld* world = PhysicsWorld_Create(&config, arena);
PhysicsWorld_AddStage(world, PhysicsStage_CreateRigidBody());
PhysicsWorld_AddStage(world, PhysicsStage_CreateCollision());
PhysicsWorld_AddStage(world, PhysicsStage_CreateVehicles());

// Vehicle simulation
VehicleComponent_Create(world, vehicle, "vehicle_config.json");
VehicleComponent_SetThrottle(vehicle, throttleInput);
VehicleComponent_SetSteering(vehicle, steeringInput);
```

### Audio (3D Spatial Audio)
```cpp
// Real-time audio with vehicle simulation
AudioContext* audio = AudioContext_Create(&audioConfig);
VehicleAudio* vehicleAudio = VehicleAudio_Create(audio, arena);
VehicleAudio_SetEngineState(vehicleAudio, rpm, throttle, load, gear);
VehicleAudio_SetPosition(vehicleAudio, vehiclePosition);
```

### Input (Platform-Agnostic)
```cpp
// Action-based input system
ActionSystem* actions = ActionSystem_Create(arena);
ActionSystem_AddKeyBinding(actions, "accelerate", KEY_W);
ActionSystem_AddGamepadBinding(actions, "brake", 0, GAMEPAD_LT);

// Vehicle-specific input handling
VehicleInputHandler* vehicleInput = VehicleInput_Create(arena);
VehicleInput_Update(vehicleInput, actionSystem);
const VehicleInputState* state = VehicleInput_GetState(vehicleInput);
```

## 📊 Core Features

### Engine Systems
- **Platform Layer**: Cross-platform abstraction (Windows/macOS/Linux + Mobile + Console)
- **Entity System**: Data-oriented ECS with hot-reloading and serialization
- **Physics System**: Wassimulator-inspired realistic vehicle simulation
- **Rendering System**: Multi-API support (OpenGL/Metal/DirectX/Vulkan)
- **Resource System**: Format-agnostic asset management with streaming
- **Input System**: Platform-agnostic input with action mapping and vehicle controls
- **Audio System**: Real-time 3D spatial audio with vehicle simulation
- **Multiplayer**: Authoritative server with client prediction and local multiplayer
- **Testing**: Comprehensive TDD framework with headless testing

### Game Features
- **Vehicle Simulation**: Engine torque curves, Pacejka tire models, weight transfer
- **Physics Pipeline**: Composable physics stages from simple to advanced
- **Multiplayer Racing**: iRacing-style ranked races with custom lobbies
- **Local Multiplayer**: Split-screen support with multiple gamepads
- **Editor Integration**: Media Molecule "game as editor" approach
- **Hot Reloading**: Real-time asset and code updates during development
- **Debug Tools**: Comprehensive debugging, profiling, and visualization tools

### Platform Support
- **Desktop**: Windows 10+, macOS 12+, Linux (Ubuntu 20.04+)
- **Mobile**: iOS 14+, Android 10+ (future)
- **Console**: Switch, PS5, Xbox Series X|S (future)
- **Web**: WebAssembly with WebGL/WebGPU (future)

### Performance
- **60 FPS**: Target frame rate on all platforms
- **Memory Efficient**: Arena allocators with minimal fragmentation
- **Multi-Threading**: Parallel processing where beneficial
- **Network Optimized**: Efficient state synchronization and prediction

## 📚 Documentation & Specifications

### System Specifications
- **[SPEC_OVERVIEW.md](SPEC_OVERVIEW.md)** - Executive specification overview
- **[SPEC_PLATFORM.md](SPEC_PLATFORM.md)** - Platform layer specification
- **[SPEC_PHYSICS.md](SPEC_PHYSICS.md)** - Physics system specification
- **[SPEC_ENTITY.md](SPEC_ENTITY.md)** - Entity system specification
- **[SPEC_INPUT.md](SPEC_INPUT.md)** - Input system specification
- **[SPEC_RESOURCES.md](SPEC_RESOURCES.md)** - Resource system specification
- **[SPEC_RENDERING.md](SPEC_RENDERING.md)** - Rendering system specification
- **[SPEC_AUDIO.md](SPEC_AUDIO.md)** - Audio system specification
- **[SPEC_MULTIPLAYER.md](SPEC_MULTIPLAYER.md)** - Multiplayer system specification
- **[SPEC_TESTING.md](SPEC_TESTING.md)** - Testing strategy specification
- **[SPEC_ROADMAP.md](SPEC_ROADMAP.md)** - Implementation roadmap

### Review Order
1. **Foundation**: SPEC_PLATFORM → SPEC_PHYSICS → SPEC_ENTITY → SPEC_TESTING
2. **Core Systems**: SPEC_INPUT → SPEC_RESOURCES → SPEC_RENDERING → SPEC_AUDIO
3. **Advanced**: SPEC_MULTIPLAYER → SPEC_ROADMAP
4. **Overview**: SPEC_OVERVIEW (single point of reference)

## 🚀 Development Roadmap

### Phase 1: Minimal Viable Game (Weeks 1-4)
- Simple top-down racing game with basic physics
- Foundation systems: platform, memory, entities, rendering
- Complete playable game with vehicle, track, UI

### Phase 2: Enhanced Racing Game (Weeks 5-8)
- Advanced vehicle physics with torque curves and tire models
- 3D rendering with lighting and effects
- Multiple vehicles, tracks, AI opponents

### Phase 3: Multiplayer Foundation (Weeks 9-12)
- Networked multiplayer with client prediction
- Lobby system and multiplayer races
- Network optimization and security

### Phase 4: Advanced Racing Features (Weeks 13-16)
- Vehicle customization, damage system, weather effects
- Local split-screen multiplayer
- In-game editor and debug tools

### Phase 5: Platform Expansion (Weeks 17-20)
- Mobile platforms with touch input and optimization
- Console platforms with controller support and certification
- Web platform with WebAssembly and browser features

## 🌐 Future Platform Support

### Web Deployment (Future)
```bash
# WebAssembly build (future)
emconfigure cmake ..
emmake make
```

### Mobile Platforms (Future)
- **iOS**: Metal rendering, touch input, iOS-specific optimizations
- **Android**: OpenGL ES/Vulkan, touch input, Android-specific features

### Console Platforms (Future)
- **Nintendo Switch**: Nintendo SDK integration, Joy-Con support
- **PlayStation 5**: PlayStation SDK, DualSense controller features
- **Xbox Series X|S**: Xbox SDK, Xbox controller features

## 🎮 Creating Your Game

### Quick Start
```cpp
#include "engine/include/game.h"
#include "engine/include/run.h"

class MyRacingGame : public engine::Game {
private:
    VehicleComponent* playerVehicle;
    
public:
    void onStart() override {
        // Create player vehicle
        Entity vehicle = EntityManager_CreateEntity(entityManager);
        playerVehicle = EntityManager_AddComponent(entityManager, vehicle, 
                                              COMPONENT_VEHICLE, NULL);
        VehicleComponent_Create(physicsWorld, playerVehicle, "my_car.json");
    }
    
    void onUpdate(float deltaTime) override {
        // Handle vehicle input
        const VehicleInputState* input = VehicleInput_GetState(vehicleInput);
        VehicleComponent_HandleInput(playerVehicle, input);
        
        // Update physics and vehicle simulation
        VehicleComponent_Update(playerVehicle, deltaTime);
        PhysicsWorld_Update(physicsWorld, deltaTime);
    }
    
    void onRender() override {
        // Render 3D scene
        Renderer_BeginFrame(renderer);
        Renderer_Render(renderer);
        
        // Render UI and debug information
        Renderer2D_Begin(renderer2D);
        Renderer2D_DrawText(renderer2D, "Speed: %.1f km/h", vehicleSpeed);
        Renderer2D_End(renderer2D);
        
        Renderer_EndFrame(renderer);
    }
    
    bool wantsToQuit() const override {
        return InputManager_IsKeyPressed(inputManager, KEY_ESCAPE);
    }
};

int main() {
    MyRacingGame game;
    
    GameConfig config;
    config.width = 1920;
    config.height = 1080;
    config.title = "My Racing Game";
    config.targetFPS = 60;
    
    return engine::run(game, config);
}
```

### Advanced Features
```cpp
// Hot reloading during development
void Game::onHotReload(const char* resourcePath) {
    if (strstr(resourcePath, ".json")) {
        VehicleComponent_ReloadConfig(playerVehicle, resourcePath);
    }
}

// Multiplayer integration
void Game::onPlayerConnected(uint32_t playerId) {
    Entity newVehicle = EntityManager_CreatePlayerVehicle(entityManager, playerId);
    MultiplayerManager_SpawnEntity(newVehicle, playerId);
}

// State snapshotting for debugging
void Game::saveState() {
    WorldState* state = EntityManager_CreateSnapshot(entityManager, frameArena);
    EntityManager_SaveSnapshotToFile(state, "debug_save.json");
}
```

## 📄 License & Credits

MIT License - see LICENSE file for details.

### Inspirations & References
- **Handmade Hero**: Platform layer architecture and philosophy
- **Wassimulator**: Vehicle physics simulation approach
- **Media Molecule**: "Game as Editor" design philosophy
- **iRacing**: Multiplayer and racing game design patterns
- **Entity Component System**: Data-oriented design patterns
- **Google Test**: Testing framework and methodology

### Third-Party Libraries
- **Google Test/Mock**: Testing framework (MIT license)
- **Raylib**: Initial prototyping and some platform code (MIT license)
- **Platform SDKs**: Native platform APIs (proprietary)
- **Compression Libraries**: Asset compression (various open source licenses)

---

*For detailed implementation guidance, see [SPEC_OVERVIEW.md](SPEC_OVERVIEW.md) and the comprehensive specification documents linked within.*

*Last Updated: 2025-12-25*
*Version: 2.0*
*Status: Ready for Implementation*