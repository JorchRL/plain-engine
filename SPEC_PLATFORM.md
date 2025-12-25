# Platform Layer Specification

## Overview

The Platform Layer provides the foundation for cross-platform development, abstracting platform-specific functionality behind a consistent C-style API. Inspired by Handmade Hero's approach, it handles low-level concerns while remaining simple and composable.

## Design Philosophy

- **Platform Independence**: Abstract all platform-specific code
- **C Style**: Plain C functions, minimal C++ features
- **Minimal Dependencies**: Self-contained, no external libraries
- **Composable**: Modular design allowing selective implementation
- **Performance First**: Direct platform API access when needed

## Core Components

### 1. Platform Abstraction Interface

#### Data Structures

```c
// Platform handles - opaque pointers for encapsulation
typedef struct PlatformWindow* PlatformWindow;
typedef struct PlatformThread* PlatformThread;
typedef struct PlatformMutex* PlatformMutex;
typedef struct PlatformFile* PlatformFile;
typedef struct PlatformTimer* PlatformTimer;

// Platform info structures
typedef struct {
    int width, height;
    bool fullscreen;
    bool resizable;
    const char* title;
} WindowConfig;

typedef struct {
    int major, minor, patch;
    const char* name;
    const char* renderer;
} PlatformInfo;

typedef struct {
    int buttonCount;
    int axisCount;
    bool isConnected;
    const char* name;
} GameControllerInfo;

// Memory management
typedef struct {
    void* (*alloc)(size_t size);
    void* (*realloc)(void* ptr, size_t size);
    void (*free)(void* ptr);
    size_t totalAllocated;
    size_t peakUsage;
} MemoryArena;

// Platform time
typedef struct {
    uint64_t frequency;
    uint64_t startTime;
} PlatformTime;
```

#### Core API

```c
// Platform initialization
bool Platform_Init(MemoryArena* arena);
void Platform_Shutdown(void);
PlatformInfo Platform_GetInfo(void);

// Window management
PlatformWindow Platform_CreateWindow(const WindowConfig* config);
void Platform_DestroyWindow(PlatformWindow window);
void Platform_GetWindowSize(PlatformWindow window, int* width, int* height);
void Platform_SetWindowSize(PlatformWindow window, int width, int height);
bool Platform_ShouldClose(PlatformWindow window);
void Platform_SwapBuffers(PlatformWindow window);

// Input management (abstracted)
typedef struct {
    bool keys[512];              // Keyboard state
    bool mouseButtons[8];         // Mouse button state
    int mouseX, mouseY;           // Mouse position
    int mouseDeltaX, mouseDeltaY;  // Mouse delta
    float mouseWheel;
    GameControllerInfo controllers[4];  // Gamepad state
} InputState;

void Platform_UpdateInput(InputState* input);
bool Platform_IsKeyPressed(int key);
bool Platform_IsMouseButtonPressed(int button);

// Time management
PlatformTime Platform_InitTime(void);
float Platform_GetDeltaTime(void);
uint64_t Platform_GetTimeMicroseconds(void);
void Platform_Sleep(uint32_t milliseconds);

// Threading
PlatformThread Platform_CreateThread(void (*entry)(void*), void* data);
void Platform_JoinThread(PlatformThread thread);
void Platform_DetachThread(PlatformThread thread);
void Platform_ExitThread(void);

PlatformMutex Platform_CreateMutex(void);
void Platform_DestroyMutex(PlatformMutex mutex);
void Platform_LockMutex(PlatformMutex mutex);
void Platform_UnlockMutex(PlatformMutex mutex);

// File I/O
PlatformFile Platform_OpenFile(const char* path, const char* mode);
void Platform_CloseFile(PlatformFile file);
size_t Platform_ReadFile(PlatformFile file, void* buffer, size_t size);
size_t Platform_WriteFile(PlatformFile file, const void* buffer, size_t size);
bool Platform_FileExists(const char* path);
uint64_t Platform_GetFileSize(const char* path);
time_t Platform_GetFileModifiedTime(const char* path);

// Dynamic library loading
typedef struct PlatformLib* PlatformLib;
PlatformLib Platform_LoadLibrary(const char* path);
void* Platform_GetProcAddress(PlatformLib lib, const char* name);
void Platform_FreeLibrary(PlatformLib lib);

// Debug/logging
void Platform_Log(const char* level, const char* message, ...);
void Platform_Assert(bool condition, const char* message);
void Platform_Breakpoint(void);

// Hot reloading support
typedef struct {
    const char* watchedPath;
    void (*callback)(const char* path, void* userData);
    void* userData;
} FileWatchHandle;

FileWatchHandle Platform_WatchFile(const char* path, void (*callback)(const char*, void*), void* userData);
void Platform_UnwatchFile(FileWatchHandle handle);
void Platform_UpdateFileWatchers(void);
```

### 2. Memory Management System

#### Arena Allocator

```c
// Simple memory arena for bulk allocations
typedef struct MemoryArena {
    uint8_t* base;
    size_t used;
    size_t capacity;
    struct MemoryArena* next;  // For linked arenas
} MemoryArena;

// Arena API
MemoryArena* Platform_CreateArena(size_t capacity);
void Platform_DestroyArena(MemoryArena* arena);
void* Platform_ArenaAlloc(MemoryArena* arena, size_t size);
void* Platform_ArenaRealloc(MemoryArena* arena, void* ptr, size_t size);
void Platform_ArenaReset(MemoryArena* arena);
void Platform_ArenaClear(MemoryArena* arena);

// Helper macros for arena allocation
#define ArenaPushStruct(arena, type) (type*)Platform_ArenaAlloc(arena, sizeof(type))
#define ArenaPushArray(arena, count, type) (type*)Platform_ArenaAlloc(arena, (count) * sizeof(type))
#define ArenaPushString(arena, str) Platform_ArenaPushString(arena, str)
```

#### Memory Tracking

```c
typedef struct {
    const char* tag;
    size_t size;
    void* ptr;
    uint64_t timestamp;
} AllocationInfo;

typedef struct {
    AllocationInfo* allocations;
    uint32_t count;
    uint32_t capacity;
    size_t totalAllocated;
    size_t peakUsage;
} MemoryTracker;

MemoryTracker* Platform_CreateMemoryTracker(MemoryArena* arena);
void Platform_TrackAllocation(MemoryTracker* tracker, void* ptr, size_t size, const char* tag);
void Platform_TrackFree(MemoryTracker* tracker, void* ptr);
void Platform_DumpMemoryUsage(MemoryTracker* tracker);
```

### 3. Platform-Specific Implementations

#### Windows Implementation (`platform_win32.c`)

```c
// Windows-specific includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <fileapi.h>
#include <synchapi.h>
#include <profileapi.h>

// Internal Windows structures
typedef struct {
    HWND hwnd;
    HDC hdc;
    HGLRC hglrc;
    bool shouldClose;
} Win32Window;

typedef struct {
    HANDLE handle;
    DWORD threadId;
} Win32Thread;

typedef struct {
    HANDLE handle;
} Win32Mutex;

typedef struct {
    HANDLE handle;
} Win32File;

typedef struct {
    LARGE_INTEGER frequency;
    LARGE_INTEGER startTime;
} Win32Time;

// Windows-specific implementation functions
PlatformWindow Win32_CreateWindow(const WindowConfig* config);
void Win32_UpdateInput(InputState* input);
void Win32_ProcessMessages(void);
```

#### macOS Implementation (`platform_macos.m`)

```c
// macOS-specific includes
#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

// Internal macOS structures
@interface PlatformWindowDelegate : NSObject <NSWindowDelegate>
@end

@interface PlatformView : NSView
@end

typedef struct {
    NSWindow* nsWindow;
    NSView* nsView;
    bool shouldClose;
} MacOSWindow;

// macOS-specific implementation functions
PlatformWindow MacOS_CreateWindow(const WindowConfig* config);
void MacOS_UpdateInput(InputState* input);
void MacOS_ProcessEvents(void);
```

### 4. Platform Features Detection

#### Capabilities System

```c
typedef enum {
    PLATFORM_FEATURE_OPENGL,
    PLATFORM_FEATURE_METAL,
    PLATFORM_FEATURE_DIRECTX,
    PLATFORM_FEATURE_VULKAN,
    PLATFORM_FEATURE_TOUCH,
    PLATFORM_FEATURE_GAMEPAD,
    PLATFORM_FEATURE_VIBRATION,
    PLATFORM_FEATURE_MICROPHONE,
    PLATFORM_FEATURE_CAMERA,
    PLATFORM_FEATURE_FILE_WATCHING,
    PLATFORM_FEATURE_HOT_RELOADING,
    PLATFORM_FEATURE_COUNT
} PlatformFeature;

typedef struct {
    bool available;
    const char* version;
    const char* notes;
} FeatureInfo;

FeatureInfo Platform_CheckFeature(PlatformFeature feature);
bool Platform_SupportsFeature(PlatformFeature feature);
```

#### Performance Profiling

```c
typedef struct {
    const char* name;
    uint64_t startTime;
    uint64_t totalTime;
    uint64_t callCount;
    uint64_t minTime;
    uint64_t maxTime;
} ProfileEntry;

typedef struct {
    ProfileEntry* entries;
    uint32_t count;
    uint32_t capacity;
    bool enabled;
} ProfileData;

void Platform_BeginProfile(ProfileData* profile, const char* name);
void Platform_EndProfile(ProfileData* profile, const char* name);
void Platform_DumpProfile(ProfileData* profile);

// Profile macros
#define ProfileBegin(profile, name) \
    if ((profile)->enabled) Platform_BeginProfile(profile, name)

#define ProfileEnd(profile, name) \
    if ((profile)->enabled) Platform_EndProfile(profile, name)

#define ProfileScope(profile, name) \
    ProfileBegin(profile, name); \
    defer(ProfileEnd(profile, name))
```

## Implementation Checklist

### Phase 1: Core Foundation
- [ ] Define platform data structures
- [ ] Implement memory arena system
- [ ] Create basic window management
- [ ] Add simple input handling
- [ ] Implement time management

### Phase 2: Cross-Platform Support
- [ ] Windows implementation
- [ ] macOS implementation  
- [ ] Linux implementation
- [ ] Platform detection and selection
- [ ] Feature capability system

### Phase 3: Advanced Features
- [ ] Threading support
- [ ] File I/O abstraction
- [ ] Dynamic library loading
- [ ] Hot reloading file watching
- [ ] Performance profiling
- [ ] Memory tracking

### Phase 4: Platform Extensions
- [ ] Mobile platform support (iOS, Android)
- [ ] Console platform support (Switch, PS5, Xbox)
- [ ] Touch input handling
- [ ] Gamepad vibration support
- [ ] Platform-specific optimizations

## Integration Points

### With Engine Systems
- **Memory System**: Provides arena allocators to all systems
- **Input System**: Uses platform input state
- **Resource System**: Uses platform file I/O
- **Audio System**: Uses platform threading and time
- **Physics System**: Uses platform threading for parallel processing
- **Rendering System**: Uses platform window management

### With Game Code
- **Main Loop**: Uses platform time and input
- **Hot Reloading**: Uses platform file watching
- **Debug Tools**: Uses platform logging and profiling
- **Configuration**: Uses platform file I/O for settings

## Testing Strategy

### Unit Tests
- Memory arena allocation/deallocation
- File I/O operations
- Threading synchronization
- Time measurement accuracy
- String operations

### Integration Tests
- Window creation and management
- Input state consistency
- Platform feature detection
- Cross-platform behavior parity

### Platform-Specific Tests
- Each platform implementation tested individually
- Performance benchmarks per platform
- Memory leak detection
- Error handling scenarios

## Performance Considerations

### Memory
- Arena allocation eliminates fragmentation
- Minimal allocation overhead
- Stack-based allocation when possible
- Memory pool for small frequent allocations

### Threading
- Lock-free data structures where possible
- Thread-local storage for performance
- Minimal synchronization overhead
- Work-stealing queue for parallel tasks

### I/O
- Asynchronous file operations
- Buffered I/O for better performance
- Memory-mapped files for large assets
- Compression support for reduced I/O

## Security Considerations

- Input validation for all file operations
- Safe string operations
- Buffer overflow protection
- Memory access validation
- Platform-specific security features

## Future Extensions

### Additional Platforms
- WebAssembly support
- Embedded systems
- VR/AR platforms
- Cloud gaming platforms

### Advanced Features
- Distributed computing
- GPU compute abstraction
- Network acceleration
- Advanced debugging tools

## Dependencies

### Required
- C99 compiler (no C++ dependencies)
- Platform SDKs (Windows SDK, Xcode, etc.)

### Optional
- OpenGL/Metal/DirectX headers
- Platform-specific optimization libraries
- Debugging and profiling tools