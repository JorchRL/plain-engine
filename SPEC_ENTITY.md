# Entity System Specification

## Overview

The Entity System provides a data-oriented, component-based architecture for game objects. Designed for maximum composability and cache efficiency, using plain C style with minimal C++ features. Supports hot-reloading, serialization, and efficient queries for game logic.

## Design Philosophy

- **Data-Oriented**: Components stored in contiguous arrays for cache efficiency
- **Component-Based**: Composable behavior through component combination
- **Plain C Style**: Simple structures, function pointers for polymorphism
- **Hot Reloading**: Component definitions can be reloaded at runtime
- **Serialization**: Complete entity state snapshotting for debugging and networking
- **Query-Driven**: Efficient component iteration and filtering

## Core Architecture

### 1. Component System Foundation

```c
// Component type definitions
typedef uint32_t ComponentType;
typedef uint32_t ComponentID;

// Maximum components per entity and total components
#define MAX_COMPONENTS_PER_ENTITY 32
#define MAX_COMPONENT_TYPES 256
#define MAX_ENTITIES 10000

// Component mask for efficient filtering
typedef uint32_t ComponentMask;
typedef uint64_t ExtendedComponentMask;  // For more than 32 component types

// Entity identifier (handles reused entity IDs)
typedef struct {
    uint32_t id;        // Entity ID
    uint32_t generation; // Generation to detect reused IDs
} Entity;

// Component descriptor for registration
typedef struct {
    ComponentType type;
    const char* name;
    size_t size;
    
    // Component lifecycle functions
    void (*init)(void* component, Entity entity);
    void (*destroy)(void* component, Entity entity);
    void (*serialize)(const void* component, char* buffer, size_t* size);
    void (*deserialize)(void* component, const char* buffer, size_t size);
    
    // Network serialization
    void (*netSerialize)(const void* component, char* buffer, size_t* size);
    void (*netDeserialize)(void* component, const char* buffer, size_t size);
    
    // Debug drawing
    void (*debugDraw)(const void* component, Entity entity);
    
    // Hot reloading support
    bool (*needsUpdate)(const void* component, const char* configPath);
    void (*updateFromConfig)(void* component, const char* configPath);
} ComponentDescriptor;

// Component storage (data-oriented)
typedef struct {
    ComponentType type;
    void* data;              // Component data array
    ComponentMask* entities;  // Entity ownership mask
    uint32_t* entityToIndex; // Map from entity ID to component index
    uint32_t count;
    uint32_t capacity;
    size_t componentSize;
    
    // Component descriptor
    const ComponentDescriptor* descriptor;
} ComponentStorage;
```

### 2. Entity Manager

```c
// Entity manager core
typedef struct {
    // Entity storage
    Entity* entities;
    uint32_t count;
    uint32_t capacity;
    
    // Free list for entity reuse
    uint32_t* freeList;
    uint32_t freeCount;
    
    // Component masks for each entity
    ComponentMask* componentMasks;
    ExtendedComponentMask* extendedMasks;  // For >32 components
    
    // Component storage
    ComponentStorage* components;
    uint32_t componentCount;
    ComponentType nextComponentType;
    
    // Component type lookup
    ComponentType typeMap[MAX_COMPONENT_TYPES];
    const char* typeNames[MAX_COMPONENT_TYPES];
    
    // Memory and hot reloading
    MemoryArena* arena;
    bool hotReloadingEnabled;
    
    // Serialization
    char* serializationBuffer;
    size_t serializationSize;
} EntityManager;

// Entity manager API
EntityManager* EntityManager_Create(MemoryArena* arena);
void EntityManager_Destroy(EntityManager* manager);
Entity EntityManager_CreateEntity(EntityManager* manager);
void EntityManager_DestroyEntity(EntityManager* manager, Entity entity);
bool EntityManager_IsAlive(EntityManager* manager, Entity entity);

// Component management
ComponentType EntityManager_RegisterComponent(EntityManager* manager, const ComponentDescriptor* descriptor);
bool EntityManager_AddComponent(EntityManager* manager, Entity entity, ComponentType type, const void* data);
bool EntityManager_RemoveComponent(EntityManager* manager, Entity entity, ComponentType type);
bool EntityManager_HasComponent(EntityManager* manager, Entity entity, ComponentType type);
void* EntityManager_GetComponent(EntityManager* manager, Entity entity, ComponentType type);

// Component queries
void EntityManager_QueryAll(EntityManager* manager, ComponentMask mask, 
                          void (*callback)(Entity entity, void** components, void* userData), void* userData);
void EntityManager_QueryOne(EntityManager* manager, ComponentType type, 
                          void (*callback)(Entity entity, void* component, void* userData), void* userData);
uint32_t EntityManager_GetEntitiesWithComponents(EntityManager* manager, ComponentMask mask, Entity* outEntities, uint32_t maxCount);
```

### 3. Standard Component Definitions

#### Transform Component

```c
typedef struct {
    Vec2 position;
    float rotation;
    Vec2 scale;
    Vec2 velocity;
    float angularVelocity;
    
    // Hierarchy support
    Entity parent;
    Entity* children;
    uint32_t childCount;
} TransformComponent;

// Component descriptor implementation
void Transform_Init(void* component, Entity entity);
void Transform_Serialize(const void* component, char* buffer, size_t* size);
void Transform_Deserialize(void* component, const char* buffer, size_t size);
void Transform_NetSerialize(const void* component, char* buffer, size_t* size);
void Transform_NetDeserialize(void* component, const char* buffer, size_t size);
```

#### Physics Component

```c
typedef struct {
    uint32_t bodyId;        // Physics body ID
    uint32_t shapeId;       // Physics shape ID
    float mass;
    bool isDynamic;
    bool isVehicle;
    uint32_t collisionLayer;
    uint32_t collisionMask;
    
    // Forces and constraints
    Vec2 appliedForce;
    float appliedTorque;
    bool useGravity;
} PhysicsComponent;

void Physics_Init(void* component, Entity entity);
void Physics_Update(void* component, Entity entity, float deltaTime);
void Physics_Serialize(const void* component, char* buffer, size_t* size);
void Physics_DebugDraw(const void* component, Entity entity);
```

#### Rendering Component

```c
typedef struct {
    // Rendering data
    uint32_t meshId;
    uint32_t textureId;
    uint32_t materialId;
    Vec4 color;
    bool visible;
    bool castShadows;
    bool receiveShadows;
    
    // Sorting and culling
    float renderDepth;
    uint8_t renderQueue;
    bool frustumCulled;
    
    // Animation support
    uint32_t animationId;
    float animationTime;
    bool isPlaying;
} RenderComponent;

void Render_Init(void* component, Entity entity);
void Render_Serialize(const void* component, char* buffer, size_t* size);
void Render_DebugDraw(const void* component, Entity entity);
```

#### Vehicle Component

```c
typedef struct {
    // Vehicle configuration
    const char* vehicleConfigPath;
    const char* liveryTexturePath;
    
    // Physics integration
    uint32_t chassisBodyId;
    uint32_t wheelBodyIds[4];
    EngineModule engine;
    VehicleChassis chassis;
    
    // Input handling
    float throttleInput;
    float brakeInput;
    float steeringInput;
    float clutchInput;
    uint32_t gearInput;
    
    // Vehicle state
    float currentSpeed;
    float currentRPM;
    uint32_t currentGear;
    bool engineOn;
    bool headlightsOn;
    
    // Damage and wear
    float health;
    float tireWear[4];
    float fuelLevel;
    
    // Audio
    uint32_t engineSoundId;
    uint32_t tireSoundId;
    float enginePitch;
    float tireVolume;
} VehicleComponent;

void Vehicle_Init(void* component, Entity entity);
void Vehicle_Update(void* component, Entity entity, float deltaTime);
void Vehicle_HandleInput(void* component, const InputState* input);
void Vehicle_Serialize(const void* component, char* buffer, size_t* size);
void Vehicle_NetSerialize(const void* component, char* buffer, size_t* size);
void Vehicle_DebugDraw(const void* component, Entity entity);
```

#### Track Component

```c
typedef struct {
    // Track geometry
    uint32_t meshId;
    uint32_t surfaceTextureId;
    Vec2* checkpoints;
    uint32_t checkpointCount;
    Vec2* spawnPoints;
    uint32_t spawnPointCount;
    
    // Track properties
    float width;
    float friction;
    float grip;
    bool isWet;
    float ambientTemperature;
    
    // Track boundaries
    Vec2* boundaryPoints;
    uint32_t boundaryCount;
    
    // Pit lane
    Vec2 pitEntry;
    Vec2 pitExit;
    Vec2 pitStops[8];
    uint32_t pitStopCount;
} TrackComponent;

void Track_Init(void* component, Entity entity);
void Track_CheckCollisions(void* component, VehicleComponent* vehicle);
void Track_Serialize(const void* component, char* buffer, size_t* size);
```

### 4. Query System

```c
// Query types for different access patterns
typedef enum {
    QUERY_READ_ONLY,
    QUERY_READ_WRITE,
    QUERY_WRITE_ONLY,
    QUERY_PARALLEL
} QueryType;

// Query builder for complex filtering
typedef struct {
    EntityManager* manager;
    ComponentMask includeMask;
    ComponentMask excludeMask;
    ComponentMask optionalMask;
    QueryType type;
    
    // Sorting
    bool sortByDistance;
    Vec2 sortOrigin;
    bool sortByDepth;
    
    // Limits
    uint32_t maxResults;
    uint32_t skipCount;
} QueryBuilder;

// Query builder API
QueryBuilder QueryBuilder_Create(EntityManager* manager);
QueryBuilder* QueryBuilder_WithComponents(QueryBuilder* builder, ComponentMask mask);
QueryBuilder* QueryBuilder_WithoutComponents(QueryBuilder* builder, ComponentMask mask);
QueryBuilder* QueryBuilder_OptionalComponents(QueryBuilder* builder, ComponentMask mask);
QueryBuilder* QueryBuilder_SortByDistance(QueryBuilder* builder, Vec2 origin);
QueryBuilder* QueryBuilder_SortByDepth(QueryBuilder* builder);
QueryBuilder* QueryBuilder_Limit(QueryBuilder* builder, uint32_t maxResults);
QueryBuilder* QueryBuilder_Skip(QueryBuilder* builder, uint32_t skipCount);

// Query execution
void QueryBuilder_Execute(QueryBuilder* builder, 
                        void (*callback)(Entity entity, void** components, void* userData), 
                        void* userData);
void QueryBuilder_ExecuteParallel(QueryBuilder* builder,
                                void (*callback)(Entity entity, void** components, void* userData, uint32_t threadIndex),
                                void* userData);

// Predefined common queries
void EntityManager_QueryVehicles(EntityManager* manager, void (*callback)(Entity entity, VehicleComponent* vehicle, void* userData), void* userData);
void EntityManager_QueryRenderables(EntityManager* manager, void (*callback)(Entity entity, RenderComponent* render, TransformComponent* transform, void* userData), void* userData);
void EntityManager_QueryPhysicsBodies(EntityManager* manager, void (*callback)(Entity entity, PhysicsComponent* physics, TransformComponent* transform, void* userData), void* userData);
```

### 5. Serialization and State Management

```c
// Entity state for debugging and networking
typedef struct {
    Entity entity;
    ComponentMask componentMask;
    uint32_t componentCount;
    
    // Component data (variable length)
    struct {
        ComponentType type;
        uint32_t dataSize;
        char data[256];  // Max component data size
    } components[MAX_COMPONENTS_PER_ENTITY];
} EntityState;

// World state for complete snapshot
typedef struct {
    uint32_t timestamp;
    uint32_t entityCount;
    EntityState* entities;
    uint32_t frameNumber;
    float deltaTime;
} WorldState;

// Serialization API
bool EntityManager_SerializeEntity(const EntityManager* manager, Entity entity, char* buffer, size_t* size);
bool EntityManager_DeserializeEntity(EntityManager* manager, const char* buffer, size_t size, Entity* outEntity);
bool EntityManager_SerializeWorld(const EntityManager* manager, WorldState* state, char* buffer, size_t* size);
bool EntityManager_DeserializeWorld(EntityManager* manager, const char* buffer, size_t size, WorldState* state);

// State management for debugging and networking
WorldState* EntityManager_CreateSnapshot(const EntityManager* manager, MemoryArena* arena);
bool EntityManager_RestoreSnapshot(EntityManager* manager, const WorldState* state);
bool EntityManager_CompareStates(const WorldState* stateA, const WorldState* state);
void EntityManager_DumpState(const EntityManager* manager, const char* filename);
```

### 6. Hot Reloading Support

```c
// Component update configuration
typedef struct {
    ComponentType type;
    const char* configPath;
    uint64_t lastModified;
    bool needsReload;
    void (*updateCallback)(void* component, const char* configPath);
} ComponentUpdateConfig;

// Hot reloading system
typedef struct {
    ComponentUpdateConfig* configs;
    uint32_t configCount;
    bool enabled;
    uint64_t checkInterval;
    uint64_t lastCheckTime;
} HotReloadSystem;

// Hot reloading API
void HotReload_Init(HotReloadSystem* system, MemoryArena* arena);
void HotReload_RegisterComponent(HotReloadSystem* system, ComponentType type, const char* configPath);
void HotReload_Update(HotReloadSystem* system, EntityManager* manager);
void HotReload_ForceReload(HotReloadSystem* system, ComponentType type);
```

## Implementation Roadmap

### Phase 1: Core Entity System (Week 1-2)
- [ ] Entity manager foundation
- [ ] Basic component registration
- [ ] Component storage and retrieval
- [ ] Simple query system
- [ ] Memory arena integration

### Phase 2: Standard Components (Week 3-4)
- [ ] Transform component
- [ ] Basic rendering component
- [ ] Simple physics component
- [ ] Component serialization
- [ ] Debug visualization

### Phase 3: Advanced Components (Week 5-6)
- [ ] Vehicle component integration
- [ ] Track component
- [ ] Audio component
- [ ] Input component
- [ ] Network components

### Phase 4: Query Optimization (Week 7-8)
- [ ] Advanced query system
- [ ] Parallel query execution
- [ ] Component caching
- [ ] Memory pool optimization
- [ ] Performance profiling

### Phase 5: Serialization and Hot Reloading (Week 9-10)
- [ ] Complete state serialization
- [ ] World state snapshots
- [ ] Hot reloading system
- [ ] Component configuration files
- [ ] Editor integration

## Integration Points

### With Physics System
- Physics component integration
- Vehicle component physics data
- Collision event handling
- Spatial queries

### With Rendering System
- Render component management
- Culling and sorting
- Debug drawing
- Material management

### With Input System
- Input component handling
- Vehicle input mapping
- Camera control components
- UI input components

### With Multiplayer System
- Network serialization
- State synchronization
- Client-side prediction
- Entity spawning/destroying

## Performance Considerations

### Data Layout
- Structure of Arrays for component data
- Cache-friendly component iteration
- Minimize pointer chasing
- Aligned memory allocation

### Query Optimization
- Component mask bit operations
- Lazy query evaluation
- Query result caching
- Parallel query execution

### Memory Management
- Arena allocation for components
- Free list for entity reuse
- Component data pooling
- Compact data storage

## Testing Strategy

### Unit Tests
- Component creation and destruction
- Entity lifecycle management
- Query correctness
- Serialization accuracy

### Integration Tests
- Component interactions
- Multi-system integration
- Hot reloading scenarios
- Performance benchmarks

### Stress Tests
- Maximum entity counts
- Component explosion scenarios
- Memory usage validation
- Frame time consistency

## Configuration Files

### Component Configuration
```json
{
    "components": [
        {
            "type": "Transform",
            "size": 48,
            "serialize": true,
            "networkSync": false
        },
        {
            "type": "Vehicle",
            "size": 256,
            "serialize": true,
            "networkSync": true,
            "configPath": "configs/vehicles/default_car.json"
        }
    ]
}
```

### Entity Templates
```json
{
    "templates": {
        "race_car": {
            "components": [
                {"type": "Transform"},
                {"type": "Physics", "mass": 1200},
                {"type": "Vehicle", "config": "sports_car"},
                {"type": "Render", "mesh": "car_body", "texture": "car_livery"}
            ]
        },
        "checkpoint": {
            "components": [
                {"type": "Transform"},
                {"type": "Trigger", "type": "checkpoint", "index": 0}
            ]
        }
    }
}
```