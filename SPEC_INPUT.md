# Input System Specification

## Overview

The Input System provides a unified, platform-agnostic interface for handling all forms of user input. Designed with plain C style, emphasizing modularity and extensibility for different input types and devices. Supports keyboard, mouse, gamepad, touch, and custom input devices.

## Design Philosophy

- **Platform Independence**: Abstract platform-specific input behind common interface
- **Event-Driven**: Event-based architecture with polling options
- **Composable Input Actions**: Raw input can be composed into logical actions
- **Plain C Style**: Simple structures, function pointers for polymorphism
- **Configurable**: Input mappings can be loaded from configuration files
- **Device-Agnostic**: Support for any input device type

## Core Architecture

### 1. Input Event System

```c
// Input event types
typedef enum {
    EVENT_KEY_DOWN,
    EVENT_KEY_UP,
    EVENT_KEY_REPEAT,
    EVENT_MOUSE_DOWN,
    EVENT_MOUSE_UP,
    EVENT_MOUSE_MOVE,
    EVENT_MOUSE_WHEEL,
    EVENT_GAMEPAD_CONNECT,
    EVENT_GAMEPAD_DISCONNECT,
    EVENT_GAMEPAD_BUTTON_DOWN,
    EVENT_GAMEPAD_BUTTON_UP,
    EVENT_GAMEPAD_AXIS_MOVE,
    EVENT_TOUCH_DOWN,
    EVENT_TOUCH_UP,
    EVENT_TOUCH_MOVE,
    EVENT_TEXT_INPUT,
    EVENT_DEVICE_ADDED,
    EVENT_DEVICE_REMOVED,
    EVENT_CUSTOM
} InputEventType;

// Input codes (platform-independent)
typedef enum {
    // Keyboard
    KEY_UNKNOWN = 0,
    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M,
    KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
    KEY_SPACE, KEY_ENTER, KEY_ESCAPE, KEY_TAB, KEY_BACKSPACE, KEY_DELETE,
    KEY_LSHIFT, KEY_RSHIFT, KEY_LCTRL, KEY_RCTRL, KEY_LALT, KEY_RALT,
    KEY_COUNT
} KeyCode;

// Mouse buttons
typedef enum {
    MOUSE_LEFT,
    MOUSE_RIGHT,
    MOUSE_MIDDLE,
    MOUSE_X1,
    MOUSE_X2,
    MOUSE_COUNT
} MouseButton;

// Gamepad buttons
typedef enum {
    GAMEPAD_A,
    GAMEPAD_B,
    GAMEPAD_X,
    GAMEPAD_Y,
    GAMEPAD_LB,
    GAMEPAD_RB,
    GAMEPAD_LT,
    GAMEPAD_RT,
    GAMEPAD_LS,
    GAMEPAD_RS,
    GAMEPAD_DPAD_UP,
    GAMEPAD_DPAD_DOWN,
    GAMEPAD_DPAD_LEFT,
    GAMEPAD_DPAD_RIGHT,
    GAMEPAD_START,
    GAMEPAD_SELECT,
    GAMEPAD_HOME,
    GAMEPAD_COUNT
} GamepadButton;

// Gamepad axes
typedef enum {
    GAMEPAD_AXIS_LEFT_X,
    GAMEPAD_AXIS_LEFT_Y,
    GAMEPAD_AXIS_RIGHT_X,
    GAMEPAD_AXIS_RIGHT_Y,
    GAMEPAD_AXIS_LEFT_TRIGGER,
    GAMEPAD_AXIS_RIGHT_TRIGGER,
    GAMEPAD_AXIS_COUNT
} GamepadAxis;

// Input event structure
typedef struct {
    InputEventType type;
    uint32_t timestamp;
    uint32_t deviceId;
    
    union {
        struct {
            KeyCode key;
            bool repeat;
        } keyboard;
        
        struct {
            float x, y;
            int deltaX, deltaY;
            float wheel;
            MouseButton button;
        } mouse;
        
        struct {
            GamepadButton button;
            bool isPressed;
            float value;
        } gamepadButton;
        
        struct {
            GamepadAxis axis;
            float value;
            float deltaValue;
        } gamepadAxis;
        
        struct {
            uint32_t fingerId;
            float x, y;
            float pressure;
        } touch;
        
        struct {
            const char* text;
            uint32_t length;
        } text;
        
        struct {
            uint32_t customType;
            void* customData;
        } custom;
    };
} InputEvent;

// Event callback function
typedef void (*InputEventCallback)(const InputEvent* event, void* userData);
```

### 2. Input Manager

```c
// Input device information
typedef struct {
    uint32_t deviceId;
    const char* name;
    const char* manufacturer;
    bool isConnected;
    uint32_t type;
} InputDevice;

// Input manager state
typedef struct {
    // Raw device state
    bool keys[KEY_COUNT];
    bool mouseButtons[MOUSE_COUNT];
    Vec2 mousePosition;
    Vec2 mouseDelta;
    float mouseWheel;
    
    // Gamepad state
    struct {
        bool isConnected;
        bool buttons[GAMEPAD_COUNT];
        float axes[GAMEPAD_AXIS_COUNT];
        float vibration[2];  // Left and right vibration motors
    } gamepads[4];
    
    // Touch state
    struct {
        bool active;
        Vec2 position;
        float pressure;
    } touches[10];
    
    // Event system
    InputEvent* eventQueue;
    uint32_t eventCount;
    uint32_t eventCapacity;
    uint32_t eventHead;
    uint32_t eventTail;
    
    // Event callbacks
    InputEventCallback* callbacks;
    void** callbackUserData;
    uint32_t callbackCount;
    
    // Device management
    InputDevice* devices;
    uint32_t deviceCount;
    
    // Configuration
    bool enableEvents;
    bool enablePolling;
    float deadzone;
    float sensitivity;
    
    // Memory
    MemoryArena* arena;
} InputManager;

// Input manager API
InputManager* InputManager_Create(MemoryArena* arena);
void InputManager_Destroy(InputManager* manager);
void InputManager_Update(InputManager* manager);

// Event handling
void InputManager_AddEventListener(InputManager* manager, InputEventCallback callback, void* userData);
void InputManager_RemoveEventListener(InputManager* manager, InputEventCallback callback);
void InputManager_ProcessEvents(InputManager* manager);

// Raw input polling
bool InputManager_IsKeyPressed(InputManager* manager, KeyCode key);
bool InputManager_IsKeyJustPressed(InputManager* manager, KeyCode key);
bool InputManager_IsKeyJustReleased(InputManager* manager, KeyCode key);

bool InputManager_IsMouseButtonPressed(InputManager* manager, MouseButton button);
bool InputManager_IsMouseButtonJustPressed(InputManager* manager, MouseButton button);
bool InputManager_IsMouseButtonJustReleased(InputManager* manager, MouseButton button);
Vec2 InputManager_GetMousePosition(InputManager* manager);
Vec2 InputManager_GetMouseDelta(InputManager* manager);
float InputManager_GetMouseWheel(InputManager* manager);

bool InputManager_IsGamepadConnected(InputManager* manager, uint32_t gamepadIndex);
bool InputManager_IsGamepadButtonPressed(InputManager* manager, uint32_t gamepadIndex, GamepadButton button);
bool InputManager_IsGamepadButtonJustPressed(InputManager* manager, uint32_t gamepadIndex, GamepadButton button);
float InputManager_GetGamepadAxis(InputManager* manager, uint32_t gamepadIndex, GamepadAxis axis);

// Device management
uint32_t InputManager_GetDeviceCount(InputManager* manager);
const InputDevice* InputManager_GetDevice(InputManager* manager, uint32_t index);
void InputManager_SetVibration(InputManager* manager, uint32_t gamepadIndex, float leftMotor, float rightMotor);
```

### 3. Input Action System

```c
// Input action types
typedef enum {
    ACTION_NONE,
    ACTION_BUTTON,
    ACTION_AXIS,
    ACTION_VECTOR,
    ACTION_GESTURE,
    ACTION_SEQUENCE
} ActionType;

// Input action binding
typedef struct {
    const char* name;
    ActionType type;
    bool isPressed;
    bool wasPressed;
    float value;
    Vec2 vector;
    
    // Configuration
    float deadzone;
    float sensitivity;
    bool invert;
    float scale;
    
    // Raw input bindings
    struct {
        enum { BINDING_NONE, BINDING_KEY, BINDING_MOUSE, BINDING_GAMEPAD_BUTTON, BINDING_GAMEPAD_AXIS } type;
        union {
            KeyCode key;
            MouseButton mouseButton;
            struct { uint32_t gamepadIndex; GamepadButton button; } gamepadButton;
            struct { uint32_t gamepadIndex; GamepadAxis axis; float direction; } gamepadAxis;
        } data;
        float modifier;  // For axis bindings
    } bindings[8];
    uint32_t bindingCount;
} InputAction;

// Action system
typedef struct {
    InputAction* actions;
    uint32_t actionCount;
    uint32_t actionCapacity;
    
    // Context system
    const char* currentContext;
    
    // State tracking
    uint32_t currentFrame;
    uint32_t* lastPressedFrame;
    
    // Memory
    MemoryArena* arena;
} ActionSystem;

// Action system API
ActionSystem* ActionSystem_Create(MemoryArena* arena);
void ActionSystem_Destroy(ActionSystem* system);
void ActionSystem_Update(ActionSystem* system, InputManager* inputManager);

// Action management
InputAction* ActionSystem_CreateAction(ActionSystem* system, const char* name, ActionType type);
void ActionSystem_RemoveAction(ActionSystem* system, const char* name);
InputAction* ActionSystem_GetAction(ActionSystem* system, const char* name);

// Action queries
bool ActionSystem_IsActionPressed(ActionSystem* system, const char* name);
bool ActionSystem_IsActionJustPressed(ActionSystem* system, const char* name);
bool ActionSystem_IsActionJustReleased(ActionSystem* system, const char* name);
float ActionSystem_GetActionValue(ActionSystem* system, const char* name);
Vec2 ActionSystem_GetActionVector(ActionSystem* system, const char* name);

// Binding configuration
void ActionSystem_AddKeyBinding(ActionSystem* system, const char* actionName, KeyCode key);
void ActionSystem_AddMouseBinding(ActionSystem* system, const char* actionName, MouseButton button);
void ActionSystem_AddGamepadBinding(ActionSystem* system, const char* actionName, uint32_t gamepadIndex, GamepadButton button);
void ActionSystem_AddGamepadAxisBinding(ActionSystem* system, const char* actionName, uint32_t gamepadIndex, GamepadAxis axis, float direction);

// Context system
void ActionSystem_SetContext(ActionSystem* system, const char* context);
const char* ActionSystem_GetContext(ActionSystem* system);
void ActionSystem_SaveContext(ActionSystem* system, const char* context, const char* filename);
void ActionSystem_LoadContext(ActionSystem* system, const char* context, const char* filename);
```

### 4. Input Context System

```c
// Input context for different game situations
typedef struct {
    const char* name;
    
    // Action mappings for this context
    struct {
        const char* actionName;
        InputAction action;
    }* actionMappings;
    uint32_t actionMappingCount;
    
    // Context priority (higher numbers override lower)
    int priority;
    
    // Context transitions
    struct {
        const char* fromContext;
        const char* triggerAction;
    }* transitions;
    uint32_t transitionCount;
} InputContext;

// Context manager
typedef struct {
    InputContext* contexts;
    uint32_t contextCount;
    uint32_t contextCapacity;
    
    // Active context stack
    const char** contextStack;
    uint32_t stackDepth;
    uint32_t maxStackDepth;
    
    // Default context
    const char* defaultContext;
    
    // Memory
    MemoryArena* arena;
} ContextManager;

// Context manager API
ContextManager* ContextManager_Create(MemoryArena* arena);
void ContextManager_Destroy(ContextManager* manager);
void ContextManager_Update(ContextManager* manager, ActionSystem* actionSystem);

// Context management
InputContext* ContextManager_CreateContext(ContextManager* manager, const char* name);
void ContextManager_RemoveContext(ContextManager* manager, const char* name);
InputContext* ContextManager_GetContext(ContextManager* manager, const char* name);

// Context stack
void ContextManager_PushContext(ContextManager* manager, const char* context);
void ContextManager_PopContext(ContextManager* manager);
void ContextManager_SetContext(ContextManager* manager, const char* context);
const char* ContextManager_GetCurrentContext(ContextManager* manager);

// Configuration
void ContextManager_SaveConfiguration(ContextManager* manager, const char* filename);
void ContextManager_LoadConfiguration(ContextManager* manager, const char* filename);
```

### 5. Vehicle-Specific Input Handling

```c
// Vehicle input state
typedef struct {
    // Primary controls
    float throttle;
    float brake;
    float steering;
    float clutch;
    
    // Secondary controls
    int gearUp;
    int gearDown;
    int gearIndex;
    bool handbrake;
    bool headlights;
    bool horn;
    
    // Advanced controls
    bool abs;
    bool tractionControl;
    bool stabilityControl;
    
    // Camera controls
    float cameraLookX;
    float cameraLookY;
    bool cameraZoomIn;
    bool cameraZoomOut;
    int cameraView;
    
    // UI controls
    bool pause;
    bool menu;
    bool chat;
    bool map;
} VehicleInputState;

// Vehicle input configuration
typedef struct {
    float throttleSensitivity;
    float brakeSensitivity;
    float steeringSensitivity;
    float steeringDeadzone;
    bool invertSteering;
    bool invertLook;
    
    // Force feedback settings
    bool enableForceFeedback;
    float forceFeedbackStrength;
    float wheelRange;
    
    // Camera settings
    float lookSensitivity;
    bool invertLookY;
    
    // Accessibility
    bool autoTransmission;
    float assistLevel;  // 0 = no assist, 1 = full assist
} VehicleInputConfig;

// Vehicle input handler
typedef struct {
    VehicleInputState state;
    VehicleInputState previousState;
    VehicleInputConfig config;
    
    // Action bindings
    const char* throttleAction;
    const char* brakeAction;
    const char* steeringAction;
    const char* clutchAction;
    const char* gearUpAction;
    const char* gearDownAction;
    
    // Wheel support
    bool hasWheel;
    uint32_t wheelDeviceId;
    float wheelRange;
    
    // Force feedback
    void* forceFeedbackDevice;
    float currentForce;
    float targetForce;
} VehicleInputHandler;

// Vehicle input API
VehicleInputHandler* VehicleInput_Create(MemoryArena* arena);
void VehicleInput_Destroy(VehicleInputHandler* handler);
void VehicleInput_Update(VehicleInputHandler* handler, ActionSystem* actionSystem);

// Input state
const VehicleInputState* VehicleInput_GetState(const VehicleInputHandler* handler);
const VehicleInputState* VehicleInput_GetPreviousState(const VehicleInputHandler* handler);
bool VehicleInput_WasJustPressed(const VehicleInputHandler* handler, const char* actionName);
float VehicleInput_GetInputDelta(const VehicleInputHandler* handler, const char* actionName);

// Configuration
void VehicleInput_LoadConfig(VehicleInputHandler* handler, const char* filename);
void VehicleInput_SaveConfig(VehicleInputHandler* handler, const char* filename);
void VehicleInput_SetConfig(VehicleInputHandler* handler, const VehicleInputConfig* config);
```

### 6. Input Configuration Files

```json
{
    "actions": {
        "accelerate": {
            "type": "axis",
            "bindings": [
                {"type": "key", "data": "W"},
                {"type": "gamepad_axis", "data": {"gamepadIndex": 0, "axis": "RIGHT_TRIGGER"}},
                {"type": "mouse", "data": "RIGHT"}
            ],
            "sensitivity": 1.0,
            "deadzone": 0.1
        },
        "brake": {
            "type": "axis",
            "bindings": [
                {"type": "key", "data": "S"},
                {"type": "gamepad_axis", "data": {"gamepadIndex": 0, "axis": "LEFT_TRIGGER"}}
            ],
            "sensitivity": 1.0,
            "deadzone": 0.1
        },
        "steering": {
            "type": "axis",
            "bindings": [
                {"type": "key", "data": "LEFT", "modifier": -1.0},
                {"type": "key", "data": "RIGHT", "modifier": 1.0},
                {"type": "gamepad_axis", "data": {"gamepadIndex": 0, "axis": "LEFT_X"}}
            ],
            "sensitivity": 1.0,
            "deadzone": 0.15
        },
        "handbrake": {
            "type": "button",
            "bindings": [
                {"type": "key", "data": "SPACE"},
                {"type": "gamepad_button", "data": {"gamepadIndex": 0, "button": "RB"}}
            ]
        }
    },
    "contexts": {
        "gameplay": {
            "priority": 1,
            "actions": ["accelerate", "brake", "steering", "handbrake", "menu", "pause"]
        },
        "menu": {
            "priority": 2,
            "actions": ["menu_up", "menu_down", "menu_select", "menu_back"]
        },
        "chat": {
            "priority": 3,
            "actions": ["chat_text", "chat_send", "chat_cancel"]
        }
    },
    "vehicle": {
        "throttleSensitivity": 1.0,
        "brakeSensitivity": 1.0,
        "steeringSensitivity": 0.8,
        "steeringDeadzone": 0.15,
        "invertSteering": false,
        "enableForceFeedback": true,
        "forceFeedbackStrength": 0.7,
        "wheelRange": 900,
        "autoTransmission": false,
        "assistLevel": 0.2
    }
}
```

## Implementation Roadmap

### Phase 1: Core Input System (Week 1)
- [ ] Basic input event system
- [ ] Keyboard and mouse support
- [ ] Simple action system
- [ ] Event queue management
- [ ] Platform integration

### Phase 2: Gamepad Support (Week 2)
- [ ] Gamepad detection and management
- [ ] Button and axis input
- [ ] Vibration support
- [ ] Multiple gamepad support
- [ ] Gamepad configuration

### Phase 3: Advanced Action System (Week 3)
- [ ] Complex action bindings
- [ ] Context system
- [ ] Action state tracking
- [ ] Configuration loading/saving
- [ ] Action remapping

### Phase 4: Touch and Mobile Support (Week 4)
- [ ] Touch input handling
- [ ] Gesture recognition
- [ ] Virtual gamepad
- [ ] Mobile-specific UI input
- [ ] Touch configuration

### Phase 5: Vehicle Input Specialization (Week 5)
- [ ] Vehicle input handlers
- [ ] Steering wheel support
- [ ] Force feedback integration
- [ ] Advanced vehicle controls
- [ ] Accessibility features

## Integration Points

### With Entity System
- Input components for entities
- Vehicle component input integration
- Camera entity input control
- UI entity input handling

### With Physics System
- Force feedback integration
- Realistic steering response
- Input-to-force mapping
- Physics-based input response

### With Multiplayer System
- Input prediction
- Input validation
- Input compression
- Client-side prediction

### With Rendering System
- UI input visualization
- Cursor rendering
- Input debugging overlay
- Touch visualization

## Performance Considerations

### Event Processing
- Lock-free event queue for thread safety
- Batch event processing
- Event filtering and prioritization
- Minimal memory allocations

### Input Polling
- Efficient device state caching
- Delta-based change detection
- Optimized gamepad polling
- Touch input pooling

### Memory Usage
- Arena allocation for events
- Fixed-size buffers for input
- Compact event storage
- Efficient string storage

## Testing Strategy

### Unit Tests
- Event generation and processing
- Action binding correctness
- Context switching behavior
- Configuration parsing

### Integration Tests
- Multi-device input handling
- Platform-specific behavior
- Real-time input performance
- Input lag measurement

### Usability Tests
- Input responsiveness
- Configuration UI usability
- Accessibility feature testing
- Cross-platform consistency

## Security Considerations

- Input validation and sanitization
- Rate limiting for input events
- Prevention of input-based exploits
- Secure configuration file parsing
- Protection against malicious input devices