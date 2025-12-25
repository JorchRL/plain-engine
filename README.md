# Plain Engine - C++ Game Engine

A minimal, cross-platform game engine built with **C++** and **Raylib**. No external graphics libraries beyond Raylib's lightweight framework.

## 🎯 Philosophy

- **Minimal Dependencies**: Uses only Raylib for graphics and input
- **Cross-Platform**: Works on Windows, macOS, and Linux
- **Clean Architecture**: Simple, modular engine design
- **Performance First**: Direct Raylib API usage for maximum performance

## 📁 Project Structure

```
plain-engine/
├── cpp/
│   ├── lib/
│   │   └── engine/
│   │       ├── include/
│   │       │   ├── game.h      # Game base class
│   │       │   ├── run.h       # Engine run function
│   │       │   └── IGame.h     # Core interface definitions
│   │       └── src/
│   │           └── run.cpp     # Game loop implementation
│   ├── src/
│   │   └── main.cpp            # Example game implementation
│   ├── CMakeLists.txt          # Build configuration
│   └── Makefile              # Alternative build
└── README.md
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

### Core Interface

The engine provides a clean lifecycle API:

```cpp
class Game : public IGame {
public:
    virtual void onStart() = 0;           // Called once when game starts
    virtual void onUpdate(float deltaTime) = 0; // Called every frame
    virtual void onRender() = 0;         // Render game objects
    virtual bool wantsToQuit() const override { return false; }
};
```

### Engine Components

- **IGame**: Pure virtual interface defining game lifecycle
- **Game**: Abstract base class with engine-specific functionality  
- **run()**: Main game loop function handling window, input, and timing

### Example Game

The included bouncing ball demo shows basic usage:

```cpp
class MyGame : public engine::Game {
private:
    float ballX = 400.0f, ballY = 225.0f;
    float ballVelocityX = 200.0f, ballVelocityY = 150.0f;
    
public:
    void onStart() override {
        std::cout << "Game started!\n";
    }
    
    void onUpdate(float deltaTime) override {
        ballX += ballVelocityX * deltaTime;
        ballY += ballVelocityY * deltaTime;
        // Bounce off walls...
    }
    
    void onRender() override {
        DrawCircle(ballX, ballY, 20, RED);
    }
};
```

## 🔧 Development

### Build Commands
```bash
make           # Debug build
make release   # Release build  
make clean     # Clean build files
```

### Configuration Options
```cpp
GameConfig config;
config.width = 800;
config.height = 450;
config.title = "My Game";
config.targetFPS = 60;
return engine::run(game, config);
```

## 🎨 Rendering APIs

The engine uses Raylib's drawing primitives:

```cpp
// Shapes
DrawCircle(x, y, radius, RED);
DrawRectangle(x, y, width, height, BLUE);
DrawLine(startX, startY, endX, endY, GREEN);

// Text
DrawText("Hello World", x, y, fontSize, WHITE);
```

## 📊 Features

- **Game Loop**: Frame-based updates with delta time
- **Input Handling**: Keyboard and mouse via Raylib
- **Window Management**: Resizable windows, FPS control
- **Asset Loading**: Raylib's built-in asset management
- **Audio**: Raylib's audio system (if needed)

## 🔌 Future Web Support

For web deployment, the engine can be compiled to WebAssembly using Emscripten:

```bash
# Future: WASM build
emconfigure cmake ..
emmake make
```

## 🤝 Creating Your Own Game

1. Include the engine headers
2. Create a class inheriting from `engine::Game`
3. Implement the lifecycle methods
4. Call `engine::run()` with your game instance

## 📄 License

MIT License - see LICENSE file for details.