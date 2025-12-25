# Physics System Specification

## Overview

The Physics System provides a modular, composable pipeline for realistic vehicle simulation. Built incrementally from simple to sophisticated models, inspired by Wassimulator's approach. Uses plain C style with minimal C++ features, emphasizing composability and performance.

## Design Philosophy

- **Composable Pipeline**: Modular stages that can be combined as needed
- **Incremental Complexity**: Start simple, add sophistication incrementally
- **Plain C Style**: Simple structures and functions, minimal C++ features
- **Performance First**: Optimized for real-time simulation
- **Data-Oriented**: Cache-friendly data layouts
- **Deterministic**: Consistent results across platforms for networking

## Core Architecture

### 1. Physics Pipeline Structure

```c
// Forward declarations
typedef struct PhysicsWorld PhysicsWorld;
typedef struct PhysicsBody PhysicsBody;
typedef struct PhysicsShape PhysicsShape;
typedef struct PhysicsConstraint PhysicsConstraint;

// Pipeline stages - composable functions
typedef struct PhysicsStage {
    const char* name;
    bool enabled;
    void (*update)(PhysicsWorld* world, float deltaTime);
    void (*debugDraw)(PhysicsWorld* world);
    struct PhysicsStage* next;
} PhysicsStage;

// Physics pipeline
typedef struct {
    PhysicsStage* stages;
    uint32_t stageCount;
    bool parallelUpdate;
    float timeScale;
    uint32_t substeps;
} PhysicsPipeline;

// Pipeline configuration
typedef struct {
    bool enableGravity;
    bool enableCollisions;
    bool enableConstraints;
    bool enableVehicles;
    bool enableParticles;
    uint32_t substeps;
    float timeScale;
} PhysicsConfig;
```

### 2. Core Data Structures

#### World and Bodies

```c
// Physics world - container for all physics objects
typedef struct PhysicsWorld {
    // Core data
    PhysicsBody* bodies;
    uint32_t bodyCount;
    uint32_t bodyCapacity;
    
    // Spatial partitioning
    uint32_t* grid;
    uint32_t gridWidth, gridHeight;
    float cellSize;
    
    // Collision detection
    CollisionPair* potentialCollisions;
    uint32_t collisionCount;
    
    // Pipeline
    PhysicsPipeline pipeline;
    
    // Configuration
    Vec2 gravity;
    float deltaTime;
    bool paused;
    
    // Memory
    MemoryArena* arena;
} PhysicsWorld;

// Rigid body properties
typedef struct PhysicsBody {
    // Transform
    Vec2 position;
    float rotation;
    Vec2 velocity;
    float angularVelocity;
    
    // Mass properties
    float mass;
    float inverseMass;
    float inertia;
    float inverseInertia;
    
    // Material properties
    float restitution;    // Bounciness (0-1)
    float friction;       // Friction coefficient
    float damping;        // Linear damping
    float angularDamping;  // Angular damping
    
    // Shape and collision
    PhysicsShape* shape;
    uint32_t categoryBits;
    uint32_t maskBits;
    bool isActive;
    bool isStatic;
    
    // Forces and impulses
    Vec2 force;
    float torque;
    Vec2 impulse;
    float angularImpulse;
    
    // Vehicle-specific extension
    void* vehicleData;
    
    // User data
    void* userData;
} PhysicsBody;

// Shape base class (using function pointers for polymorphism)
typedef enum {
    SHAPE_CIRCLE,
    SHAPE_BOX,
    SHAPE_POLYGON,
    SHAPE_EDGE,
    SHAPE_CHAIN
} ShapeType;

typedef struct PhysicsShape {
    ShapeType type;
    
    // Shape-specific data (union)
    union {
        struct { float radius; } circle;
        struct { Vec2 extents; } box;
        struct { Vec2* vertices; uint32_t count; } polygon;
        struct { Vec2 v1, v2; } edge;
    };
    
    // Shape functions
    bool (*testOverlap)(const PhysicsShape* a, const PhysicsShape* b, const Vec2& posA, const Vec2& posB);
    void (*computeAABB)(const PhysicsShape* shape, const Vec2& pos, float rot, Rect* aabb);
    float (*computeMass)(const PhysicsShape* shape, float density);
    
    // Debug drawing
    void (*debugDraw)(const PhysicsShape* shape, const Vec2& pos, float rot);
} PhysicsShape;
```

### 3. Vehicle Physics Modules

#### Module 1: Basic Rigid Body (Foundation)

```c
// Basic rigid body physics
typedef struct RigidBodyModule {
    bool enabled;
    float maxVelocity;
    float maxAngularVelocity;
    bool allowSleep;
    float sleepThreshold;
} RigidBodyModule;

// Module functions
void RigidBodyModule_Init(RigidBodyModule* module, const PhysicsConfig* config);
void RigidBodyModule_Update(PhysicsWorld* world, float deltaTime);
void RigidBodyModule_IntegrateForces(PhysicsBody* body, float deltaTime);
void RigidBodyModule_IntegrateVelocity(PhysicsBody* body, float deltaTime);
void RigidBodyModule_ApplyImpulse(PhysicsBody* body, Vec2 impulse, Vec2 contact);
```

#### Module 2: Collision Detection

```c
// Collision pair for broadphase
typedef struct {
    uint32_t bodyA, bodyB;
    Rect aabbA, aabbB;
} CollisionPair;

// Contact manifold for detailed collision
typedef struct {
    Vec2 points[4];        // Contact points (max 4)
    Vec2 normal;          // Collision normal
    float penetration;     // Penetration depth
    uint32_t pointCount;  // Number of contact points
    float restitution;    // Combined restitution
    float friction;       // Combined friction
    uint32_t bodyA, bodyB;
} ContactManifold;

typedef struct {
    bool enabled;
    bool enableContinuous;  // Continuous collision detection
    uint32_t maxCollisions;
    float baumgarte;       // Position correction bias
    float slop;          // Position correction allowance
} CollisionModule;

// Module functions
void CollisionModule_Init(CollisionModule* module, const PhysicsConfig* config);
void CollisionModule_Broadphase(PhysicsWorld* world);
void CollisionModule_Narrowphase(PhysicsWorld* world);
void CollisionModule_ResolveCollisions(PhysicsWorld* world);
void CollisionModule_PositionalCorrection(ContactManifold* contact);
```

#### Module 3: Vehicle Dynamics (Wassimulator-inspired)

```c
// Engine torque curve (black box function)
typedef struct {
    float minRPM, maxRPM;
    float peakTorqueRPM;
    float maxTorque;
    float maxPower;
    
    // Curve parameters (can be tuned)
    float baselineTorque;
    float peakMagnitude;
    float endSlope;
    float peakPosition;
    float width;
} TorqueCurve;

typedef struct {
    float ratios[8];     // Gear ratios (including reverse)
    float finalDrive;    // Differential ratio
    uint32_t currentGear;
    float shiftTime;     // Time for gear shift animation
    bool automatic;
} Gearbox;

typedef struct {
    TorqueCurve torqueCurve;
    Gearbox gearbox;
    
    // State variables
    float rpm;
    float throttle;      // 0-1
    float clutch;        // 0-1 (0 = fully engaged)
    float engineBraking;
    bool isShifting;
    
    // Outputs
    float engineTorque;
    float wheelTorque;
} EngineModule;

// Module functions
float TorqueCurve_GetTorque(const TorqueCurve* curve, float rpm);
void EngineModule_Init(EngineModule* engine, const char* configPath);
void EngineModule_Update(EngineModule* engine, float wheelSpeed, float deltaTime);
void EngineModule_SetThrottle(EngineModule* engine, float throttle);
void EngineModule_ShiftGear(EngineModule* engine, uint32_t gear);
```

#### Module 4: Tire Model (Pacejka-inspired)

```c
// Tire slip parameters
typedef struct {
    float longitudinalStiffness;
    float lateralStiffness;
    float peakSlipRatio;
    float peakSlipAngle;
    
    // Pacejka-like parameters
    float B, C, D, E;   // Magic formula parameters
    
    // Combined slip
    float combinedFactor;
    float frictionCircle;
} TireModel;

// Wheel state
typedef struct {
    Vec2 position;        // Wheel position relative to body
    float radius;         // Wheel radius
    float rotation;       // Current rotation angle
    float angularVelocity; // Angular velocity
    
    // Slip quantities
    float slipRatio;      // Longitudinal slip
    float slipAngle;      // Lateral slip
    float loadTransfer;   // Weight on this wheel
    
    // Forces
    Vec2 longitudinalForce;  // Forward/backward force
    Vec2 lateralForce;        // Side force
    Vec2 totalForce;          // Combined force
    
    // Contact
    bool isInContact;
    float normalForce;
    
    // Tire model
    TireModel tireModel;
} Wheel;

typedef struct {
    Wheel wheels[4];     // FL, FR, RL, RR
    Vec2 wheelbase;     // Distance between front/rear axles
    Vec2 trackWidth;     // Distance between left/right wheels
    float mass;         // Vehicle mass
    float centerOfGravityHeight;
    float antiRollBar;
    
    // Steering
    float maxSteeringAngle;
    float steeringRatio;
    float ackermann;
    
    // Brakes
    float brakeBias;
    float maxBrakeTorque;
    
    // Suspension (simplified)
    float springRate;
    float dampingRate;
    float travel;
} VehicleChassis;

// Module functions
void TireModel_Init(TireModel* tire, const char* configPath);
Vec2 TireModel_GetForces(const TireModel* tire, float slipRatio, float slipAngle, float load);
void VehicleChassis_Update(VehicleChassis* chassis, const EngineModule* engine, float deltaTime);
void VehicleChassis_ApplySteering(VehicleChassis* chassis, float steeringInput);
void VehicleChassis_ApplyBrakes(VehicleChassis* chassis, float brakeInput);
void VehicleChassis_UpdateWeightTransfer(VehicleChassis* chassis, Vec2 acceleration);
```

### 4. Physics Pipeline Implementation

```c
// Pipeline creation and management
PhysicsWorld* PhysicsWorld_Create(const PhysicsConfig* config, MemoryArena* arena);
void PhysicsWorld_Destroy(PhysicsWorld* world);
void PhysicsWorld_Update(PhysicsWorld* world, float deltaTime);

// Pipeline stage management
void PhysicsWorld_AddStage(PhysicsWorld* world, PhysicsStage* stage);
void PhysicsWorld_RemoveStage(PhysicsWorld* world, const char* name);
void PhysicsWorld_EnableStage(PhysicsWorld* world, const char* name, bool enabled);

// Standard pipeline stages
PhysicsStage* PhysicsStage_CreateRigidBody(const RigidBodyModule* config);
PhysicsStage* PhysicsStage_CreateCollision(const CollisionModule* config);
PhysicsStage* PhysicsStage_CreateVehicles(const EngineModule* engineConfig, const VehicleChassis* chassisConfig);
PhysicsStage* PhysicsStage_CreateParticles(void);

// Pipeline builder for easy setup
void PhysicsWorld_CreateStandardPipeline(PhysicsWorld* world);     // Basic pipeline
void PhysicsWorld_CreateVehiclePipeline(PhysicsWorld* world);       // Vehicle simulation
void PhysicsWorld_CreateRacingPipeline(PhysicsWorld* world);        // Full racing simulation
```

### 5. Integration with Entity System

```c
// Physics component for entity system
typedef struct {
    uint32_t bodyId;
    uint32_t shapeId;
    bool isDynamic;
    bool isVehicle;
} PhysicsComponent;

// Vehicle component (extends physics)
typedef struct {
    PhysicsComponent base;
    EngineModule engine;
    VehicleChassis chassis;
    uint32_t wheels[4];  // Wheel body IDs
    uint32_t chassisId;  // Main chassis body ID
} VehicleComponent;

// Component system integration
void PhysicsComponent_Create(PhysicsWorld* world, PhysicsComponent* comp, const Vec2& pos, float mass);
void VehicleComponent_Create(PhysicsWorld* world, VehicleComponent* vehicle, const char* configPath);
void PhysicsComponent_SetPosition(PhysicsWorld* world, PhysicsComponent* comp, const Vec2& pos);
Vec2 PhysicsComponent_GetPosition(const PhysicsWorld* world, const PhysicsComponent* comp);
```

## Implementation Roadmap

### Phase 1: Basic Physics (Week 1-2)
- [ ] Core data structures
- [ ] Basic rigid body integration
- [ ] Simple shape collision (circles and boxes)
- [ ] Basic pipeline system
- [ ] Memory arena integration

### Phase 2: Advanced Collision (Week 3-4)
- [ ] Broadphase collision detection
- [ ] Contact manifold generation
- [ ] Collision resolution
- [ ] Spatial partitioning grid
- [ ] Performance optimization

### Phase 3: Vehicle Foundation (Week 5-6)
- [ ] Engine torque curves
- [ ] Simple tire model
- [ ] Basic vehicle chassis
- [ ] Weight transfer simulation
- [ ] Steering mechanics

### Phase 4: Sophisticated Vehicle Physics (Week 7-8)
- [ ] Pacejka tire model
- [ ] Advanced engine simulation
- [ ] Suspension system
- [ ] Aerodynamics
- [ ] Vehicle tuning parameters

### Phase 5: Performance and Polish (Week 9-10)
- [ ] Parallel pipeline execution
- [ ] SIMD optimization
- [ ] Debug visualization
- [ ] Performance profiling
- [ ] Network synchronization

## Integration Points

### With Rendering System
- Debug drawing of physics objects
- Visual feedback for vehicle state
- Collision visualization
- Tire contact patches

### With Input System
- Vehicle control inputs (throttle, brake, steering)
- Camera following vehicle
- Debug controls

### With Multiplayer System
- Deterministic physics for networking
- State snapshot/restore
- Client prediction support
- Interpolation/extrapolation

### With Entity System
- Component-based integration
- Spatial queries
- Event system for collisions
- Vehicle state management

## Testing Strategy

### Unit Tests
- Integration accuracy (position, velocity)
- Collision detection correctness
- Torque curve evaluation
- Tire force calculations

### Integration Tests
- Vehicle behavior consistency
- Multi-body interactions
- Pipeline stage interactions
- Memory leak detection

### Performance Tests
- Frame time benchmarks
- Memory usage profiling
- Scalability with object count
- Platform performance parity

### Network Tests
- Deterministic behavior validation
- State consistency tests
- Latency handling
- Packet loss scenarios

## Performance Considerations

### Data Layout
- Structure of Arrays for better cache usage
- Align structures for SIMD
- Minimize pointer chasing
- Memory arena allocation patterns

### Algorithmic
- Spatial partitioning for O(n) collision detection
- Broadphase culling
- Object pooling for temporary data
- Lazy evaluation where possible

### Platform-Specific
- SIMD for vector operations
- Multithreading for parallel pipeline
- GPU acceleration where beneficial
- Platform-specific optimizations

## Future Extensions

### Advanced Physics
- Deformable bodies
- Fluid simulation
- Soft body physics
- Destruction modeling

### Racing-Specific
- Track surface properties
- Weather effects on physics
- Tire wear and temperature
- Fuel consumption modeling

### Networking
- Client-side prediction
- Server reconciliation
- State compression
- Lag compensation

## Configuration and Tuning

### Vehicle Configuration Files
```json
{
    "engine": {
        "torqueCurve": {
            "minRPM": 1000,
            "maxRPM": 8000,
            "peakTorqueRPM": 4500,
            "maxTorque": 400,
            "maxPower": 300
        },
        "gearbox": {
            "ratios": [-2.92, 0, 2.50, 1.61, 1.10, 0.81, 0.68],
            "finalDrive": 4.1,
            "automatic": false
        }
    },
    "chassis": {
        "mass": 1200,
        "wheelbase": [2.5, 1.4],
        "centerOfGravityHeight": 0.5,
        "maxSteeringAngle": 0.6
    },
    "tires": {
        "longitudinalStiffness": 1000,
        "lateralStiffness": 800,
        "frictionCircle": 1.2
    }
}
```

### Physics Tuning Parameters
- Time step and substeps
- Iteration counts for solver
- Damping factors
- Position correction parameters
- Friction and restitution values