# Multiplayer System Specification

## Overview

The Multiplayer System provides a comprehensive networking solution for both authoritative server with client prediction and peer-to-peer multiplayer. Designed for iRacing-style ranked races with custom lobbies, supporting both networked and local split-screen multiplayer. Uses plain C style with composability and performance as key principles.

## Design Philosophy

- **Authoritative Server**: Server-authoritative simulation with client prediction
- **Client Prediction**: Smooth gameplay with lag compensation
- **Peer-to-Peer**: Optional P2P for casual multiplayer
- **Local Multiplayer**: Split-screen support for local play
- **Plain C Style**: Simple structures, function pointers for polymorphism
- **Performance First**: Optimized for real-time multiplayer games

## Core Architecture

### 1. Network Foundation

```c
// Network protocols
typedef enum {
    NETWORK_PROTOCOL_TCP,
    NETWORK_PROTOCOL_UDP,
    NETWORK_PROTOCOL_WEBRTC,
    NETWORK_PROTOCOL_STEAM_P2P
} NetworkProtocol;

// Connection states
typedef enum {
    CONNECTION_DISCONNECTED,
    CONNECTION_CONNECTING,
    CONNECTION_CONNECTED,
    CONNECTION_DISCONNECTING,
    CONNECTION_ERROR
} ConnectionState;

// Network address
typedef struct {
    uint8_t ip[16];  // IPv4 or IPv6
    uint16_t port;
    bool isIPv6;
} NetworkAddress;

// Network packet
typedef struct {
    uint32_t id;
    uint32_t timestamp;
    uint8_t* data;
    uint32_t size;
    bool reliable;
    bool ordered;
    NetworkAddress sender;
} NetworkPacket;

// Network socket
typedef struct {
    NetworkProtocol protocol;
    NetworkAddress localAddress;
    bool isServer;
    bool isNonBlocking;
    
    // Platform-specific socket
    void* platformSocket;
    
    // Statistics
    uint64_t bytesSent;
    uint64_t bytesReceived;
    uint32_t packetsSent;
    uint32_t packetsReceived;
    float latency;
    float packetLoss;
} NetworkSocket;

// Network API
NetworkSocket* NetworkSocket_Create(NetworkProtocol protocol, bool isServer);
void NetworkSocket_Destroy(NetworkSocket* socket);
bool NetworkSocket_Bind(NetworkSocket* socket, const NetworkAddress* address);
bool NetworkSocket_Connect(NetworkSocket* socket, const NetworkAddress* address);
bool NetworkSocket_Listen(NetworkSocket* socket, uint32_t maxConnections);
NetworkSocket* NetworkSocket_Accept(NetworkSocket* socket);
bool NetworkSocket_Send(NetworkSocket* socket, const NetworkPacket* packet);
bool NetworkSocket_Receive(NetworkSocket* socket, NetworkPacket* packet);
void NetworkSocket_Update(NetworkSocket* socket);

// Address utilities
NetworkAddress NetworkAddress_CreateIPv4(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port);
NetworkAddress NetworkAddress_CreateIPv6(const uint8_t ip[16], uint16_t port);
NetworkAddress NetworkAddress_CreateFromString(const char* address, uint16_t port);
bool NetworkAddress_Equals(const NetworkAddress* a, const NetworkAddress* b);
char* NetworkAddress_ToString(const NetworkAddress* address, char* buffer, size_t bufferSize);
```

### 2. Client-Server Architecture

```c
// Client connection
typedef struct {
    uint32_t clientId;
    NetworkAddress address;
    NetworkSocket* socket;
    ConnectionState state;
    
    // Connection quality
    float latency;
    float packetLoss;
    float bandwidth;
    uint32_t lastPacketTime;
    
    // Reliability
    uint32_t nextSequenceId;
    uint32_t nextAckId;
    uint32_t* ackedPackets;
    uint32_t ackedCount;
    
    // Authentication
    bool isAuthenticated;
    const char* username;
    const char* authToken;
    
    // Game state
    uint32_t lastStateUpdate;
    WorldState* predictedState;
    WorldState* confirmedState;
} ClientConnection;

// Server state
typedef struct {
    NetworkSocket* socket;
    ClientConnection* clients;
    uint32_t clientCount;
    uint32_t maxClients;
    
    // Game state
    WorldState* worldState;
    uint32_t currentFrame;
    float deltaTime;
    
    // Tick rate
    uint32_t tickRate;
    uint32_t currentTick;
    uint64_t lastTickTime;
    
    // Authentication
    bool requireAuthentication;
    const char* authServerUrl;
    
    // Memory
    MemoryArena* arena;
    MemoryArena* frameArena;
} GameServer;

// Client state
typedef struct {
    NetworkSocket* socket;
    NetworkAddress serverAddress;
    ConnectionState state;
    
    // Connection quality
    float latency;
    float packetLoss;
    float bandwidth;
    uint32_t lastPacketTime;
    
    // Reliability
    uint32_t nextSequenceId;
    uint32_t nextAckId;
    uint32_t* ackedPackets;
    uint32_t ackedCount;
    
    // Prediction
    WorldState* localState;
    WorldState* serverState;
    WorldState* predictedState;
    uint32_t lastConfirmedFrame;
    
    // Input
    InputState* localInput;
    InputState* confirmedInput;
    uint32_t inputSequence;
    
    // Authentication
    const char* username;
    const char* authToken;
    
    // Memory
    MemoryArena* arena;
    MemoryArena* frameArena;
} GameClient;

// Server API
GameServer* GameServer_Create(uint32_t maxClients, uint32_t port, MemoryArena* arena);
void GameServer_Destroy(GameServer* server);
void GameServer_Update(GameServer* server, float deltaTime);
void GameServer_Tick(GameServer* server);

// Client management
uint32_t GameServer_AddClient(GameServer* server, const NetworkAddress* address);
void GameServer_RemoveClient(GameServer* server, uint32_t clientId);
ClientConnection* GameServer_GetClient(GameServer* server, uint32_t clientId);

// State synchronization
void GameServer_BroadcastState(GameServer* server);
void GameServer_SendStateToClient(GameServer* server, uint32_t clientId);
void GameServer_ProcessInput(GameServer* server, uint32_t clientId, const InputState* input);

// Client API
GameClient* GameClient_Create(const char* serverAddress, uint16_t port, MemoryArena* arena);
void GameClient_Destroy(GameClient* client);
void GameClient_Update(GameClient* client, float deltaTime);
void GameClient_SendInput(GameClient* client, const InputState* input);

// Connection management
bool GameClient_Connect(GameClient* client, const char* username, const char* authToken);
void GameClient_Disconnect(GameClient* client);
bool GameClient_IsConnected(const GameClient* client);
```

### 3. Message Protocol

```c
// Message types
typedef enum {
    MSG_CONNECT,
    MSG_DISCONNECT,
    MSG_AUTHENTICATE,
    MSG_AUTH_RESPONSE,
    MSG_HEARTBEAT,
    MSG_STATE_UPDATE,
    MSG_INPUT_UPDATE,
    MSG_SPAWN_ENTITY,
    MSG_DESTROY_ENTITY,
    MSG_CHAT_MESSAGE,
    MSG_RACE_START,
    MSG_RACE_FINISH,
    MSG_RACE_POSITION,
    MSG_CUSTOM
} MessageType;

// Message header
typedef struct {
    MessageType type;
    uint32_t sequenceId;
    uint32_t timestamp;
    uint32_t size;
    bool reliable;
    bool ordered;
} MessageHeader;

// Connect message
typedef struct {
    const char* username;
    const char* version;
    uint32_t protocolVersion;
} ConnectMessage;

// State update message
typedef struct {
    uint32_t frameNumber;
    float deltaTime;
    uint32_t entityCount;
    EntityState* entities;
} StateUpdateMessage;

// Input update message
typedef struct {
    uint32_t sequenceId;
    uint32_t frameNumber;
    InputState input;
} InputUpdateMessage;

// Spawn entity message
typedef struct {
    uint32_t entityId;
    EntityType type;
    Vec3 position;
    Vec3 rotation;
    const char* configPath;
} SpawnEntityMessage;

// Race position message
typedef struct {
    uint32_t clientId;
    uint32_t position;
    uint32_t lap;
    float lapTime;
    float bestLapTime;
    float raceTime;
} RacePositionMessage;

// Message serialization
uint32_t Message_Serialize(const void* message, MessageType type, char* buffer, uint32_t bufferSize);
void* Message_Deserialize(const char* buffer, uint32_t size, MessageType* outType);

// Message handlers
typedef void (*MessageHandler)(void* message, void* userData);
void GameServer_RegisterMessageHandler(GameServer* server, MessageType type, MessageHandler handler, void* userData);
void GameClient_RegisterMessageHandler(GameClient* client, MessageType type, MessageHandler handler, void* userData);
```

### 4. Client Prediction System

```c
// Prediction state
typedef struct {
    WorldState* state;
    InputState* input;
    uint32_t frameNumber;
    float timestamp;
    bool isConfirmed;
} PredictionFrame;

// Client prediction
typedef struct {
    PredictionFrame* frames;
    uint32_t frameCount;
    uint32_t frameCapacity;
    uint32_t currentFrame;
    uint32_t confirmedFrame;
    
    // Reconciliation
    bool needsReconciliation;
    uint32_t reconciliationFrame;
    
    // Smoothing
    float smoothingFactor;
    float maxPredictionTime;
    
    // Memory
    MemoryArena* arena;
} ClientPrediction;

// Prediction API
ClientPrediction* ClientPrediction_Create(uint32_t maxFrames, MemoryArena* arena);
void ClientPrediction_Destroy(ClientPrediction* prediction);
void ClientPrediction_Update(ClientPrediction* prediction, const WorldState* serverState, const InputState* localInput, float deltaTime);

// Frame management
void ClientPrediction_AddFrame(ClientPrediction* prediction, const WorldState* state, const InputState* input, uint32_t frameNumber);
void ClientPrediction_ConfirmFrame(ClientPrediction* prediction, uint32_t frameNumber);
void ClientPrediction_Reconcile(ClientPrediction* prediction, const WorldState* serverState, uint32_t frameNumber);

// State queries
const WorldState* ClientPrediction_GetPredictedState(const ClientPrediction* prediction);
const WorldState* ClientPrediction_GetStateAtFrame(const ClientPrediction* prediction, uint32_t frameNumber);
bool ClientPrediction_IsFrameConfirmed(const ClientPrediction* prediction, uint32_t frameNumber);
```

### 5. Lag Compensation

```c
// Lag compensation types
typedef enum {
    LAG_COMPENSATION_NONE,
    LAG_COMPENSATION_CLIENT_SIDE,
    LAG_COMPENSATION_SERVER_SIDE,
    LAG_COMPENSATION_HYBRID
} LagCompensationType;

// Server-side lag compensation
typedef struct {
    LagCompensationType type;
    float maxRewindTime;
    bool enableEntityRewind;
    bool enableHitDetection;
    
    // History for rewinding
    struct {
        WorldState* state;
        uint32_t frameNumber;
        float timestamp;
    }* history;
    uint32_t historyCount;
    uint32_t historyCapacity;
    uint32_t historyHead;
    
    // Hit detection
    struct {
        Vec3 position;
        Vec3 direction;
        float range;
        uint32_t clientId;
        uint32_t timestamp;
    }* shots;
    uint32_t shotCount;
    uint32_t shotCapacity;
} ServerLagCompensation;

// Client-side lag compensation
typedef struct {
    LagCompensationType type;
    float interpolationDelay;
    float extrapolationTime;
    bool enableInterpolation;
    bool enableExtrapolation;
    
    // Interpolation
    WorldState* interpolatedState;
    float interpolationFactor;
    
    // Extrapolation
    WorldState* extrapolatedState;
    float extrapolationTime;
    
    // Network smoothing
    float positionSmoothFactor;
    float rotationSmoothFactor;
    float velocitySmoothFactor;
} ClientLagCompensation;

// Lag compensation API
ServerLagCompensation* ServerLagCompensation_Create(uint32_t historySize, MemoryArena* arena);
void ServerLagCompensation_Destroy(ServerLagCompensation* compensation);
void ServerLagCompensation_Update(ServerLagCompensation* compensation, const WorldState* state, uint32_t frameNumber, float timestamp);

// Server-side functions
WorldState* ServerLagCompensation_RewindToTime(ServerLagCompensation* compensation, float timestamp);
void ServerLagCompensation_Restore(ServerLagCompensation* compensation);
bool ServerLagCompensation_CheckHit(ServerLagCompensation* compensation, Vec3 position, Vec3 direction, float range, uint32_t clientId, float timestamp);

// Client-side functions
ClientLagCompensation* ClientLagCompensation_Create(MemoryArena* arena);
void ClientLagCompensation_Destroy(ClientLagCompensation* compensation);
void ClientLagCompensation_Update(ClientLagCompensation* compensation, const WorldState* serverState, float networkLatency);
const WorldState* ClientLagCompensation_GetSmoothedState(const ClientLagCompensation* compensation);
```

### 6. Local Multiplayer (Split-Screen)

```c
// Split-screen configuration
typedef struct {
    uint32_t playerCount;
    uint32_t rows;
    uint32_t columns;
    float aspectRatio;
    float bezelCompensation;
    bool enableCrosshair;
} SplitScreenConfig;

// Local player
typedef struct {
    uint32_t playerId;
    uint32_t gamepadIndex;
    InputState input;
    Camera camera;
    Viewport viewport;
    bool isActive;
    
    // Player-specific state
    VehicleComponent* vehicle;
    uint32_t position;
    uint32_t lap;
    float lapTime;
} LocalPlayer;

// Local multiplayer manager
typedef struct {
    SplitScreenConfig config;
    LocalPlayer* players;
    uint32_t playerCount;
    uint32_t maxPlayers;
    
    // Shared game state
    WorldState* worldState;
    uint32_t currentFrame;
    float deltaTime;
    
    // Input handling
    InputManager* inputManager;
    ActionSystem* actionSystem;
    
    // Rendering
    Renderer* renderer;
    RenderTarget* playerRenderTargets[4];
    
    // Memory
    MemoryArena* arena;
    MemoryArena* frameArena;
} LocalMultiplayer;

// Local multiplayer API
LocalMultiplayer* LocalMultiplayer_Create(const SplitScreenConfig* config, MemoryArena* arena);
void LocalMultiplayer_Destroy(LocalMultiplayer* multiplayer);
void LocalMultiplayer_Update(LocalMultiplayer* multiplayer, float deltaTime);
void LocalMultiplayer_Render(LocalMultiplayer* multiplayer);

// Player management
uint32_t LocalMultiplayer_AddPlayer(LocalMultiplayer* multiplayer, uint32_t gamepadIndex);
void LocalMultiplayer_RemovePlayer(LocalMultiplayer* multiplayer, uint32_t playerId);
LocalPlayer* LocalMultiplayer_GetPlayer(LocalMultiplayer* multiplayer, uint32_t playerId);

// Configuration
void LocalMultiplayer_SetSplitScreenConfig(LocalMultiplayer* multiplayer, const SplitScreenConfig* config);
void LocalMultiplayer_UpdateViewports(LocalMultiplayer* multiplayer);
```

### 7. Racing-Specific Multiplayer

```c
// Race session types
typedef enum {
    RACE_TYPE_PRACTICE,
    RACE_TYPE_QUALIFYING,
    RACE_TYPE_RACE,
    RACE_TYPE_TIME_TRIAL,
    RACE_TYPE_ENDURANCE
} RaceType;

// Race session configuration
typedef struct {
    RaceType type;
    uint32_t maxPlayers;
    uint32_t maxLaps;
    float maxTime;
    float practiceTime;
    float qualifyingTime;
    
    // Track
    const char* trackName;
    const char* trackConfig;
    
    // Vehicle restrictions
    const char** allowedVehicles;
    uint32_t vehicleCount;
    bool allowCustomVehicles;
    
    // Weather
    const char* weatherCondition;
    float temperature;
    float humidity;
    
    // Rules
    bool allowDamage;
    bool allowCollisions;
    bool allowPitStops;
    float pitSpeedLimit;
} RaceSessionConfig;

// Race session state
typedef struct {
    RaceSessionConfig config;
    uint32_t sessionId;
    RaceType type;
    RaceState state;
    
    // Timing
    uint64_t startTime;
    uint64_t currentTime;
    float timeRemaining;
    
    // Players
    struct {
        uint32_t clientId;
        const char* username;
        uint32_t position;
        uint32_t lap;
        float lapTime;
        float bestLapTime;
        float totalTime;
        bool isFinished;
        bool isDNF;
    }* players;
    uint32_t playerCount;
    
    // Track state
    TrackComponent* track;
    uint32_t currentCheckpoint;
    
    // Events
    struct {
        uint32_t clientId;
        RaceEventType type;
        uint64_t timestamp;
        void* eventData;
    }* events;
    uint32_t eventCount;
} RaceSession;

// Race manager
typedef struct {
    GameServer* server;
    RaceSession* currentSession;
    RaceSession* nextSession;
    
    // Lobby system
    struct {
        bool isOpen;
        uint32_t maxPlayers;
        uint32_t currentPlayers;
        const char* lobbyName;
        const char* password;
        bool isPrivate;
        RaceSessionConfig sessionConfig;
    } lobby;
    
    // Matchmaking
    struct {
        bool enabled;
        const char* matchmakingServer;
        uint32_t skillRating;
        uint32_t region;
        bool searching;
    } matchmaking;
    
    // Ranking system
    struct {
        bool enabled;
        uint32_t playerId;
        uint32_t rating;
        uint32_t wins;
        uint32_t losses;
        uint32_t DNFs;
        float bestLapTime;
    } ranking;
    
    // Memory
    MemoryArena* arena;
} RaceManager;

// Race manager API
RaceManager* RaceManager_Create(GameServer* server, MemoryArena* arena);
void RaceManager_Destroy(RaceManager* manager);
void RaceManager_Update(RaceManager* manager, float deltaTime);

// Session management
uint32_t RaceManager_CreateSession(RaceManager* manager, const RaceSessionConfig* config);
void RaceManager_StartSession(RaceManager* manager, uint32_t sessionId);
void RaceManager_EndSession(RaceManager* manager, uint32_t sessionId);
RaceSession* RaceManager_GetSession(RaceManager* manager, uint32_t sessionId);

// Lobby management
void RaceManager_CreateLobby(RaceManager* manager, const char* name, const char* password, uint32_t maxPlayers);
void RaceManager_CloseLobby(RaceManager* manager);
void RaceManager_JoinLobby(RaceManager* manager, uint32_t clientId);
void RaceManager_LeaveLobby(RaceManager* manager, uint32_t clientId);

// Matchmaking
void RaceManager_StartMatchmaking(RaceManager* manager, const RaceSessionConfig* config);
void RaceManager_StopMatchmaking(RaceManager* manager);
void RaceManager_OnMatchFound(RaceManager* manager, const char* serverAddress);

// Ranking
void RaceManager_UpdateRanking(RaceManager* manager, uint32_t clientId, int32_t ratingChange);
void RaceManager_SaveRanking(RaceManager* manager, uint32_t clientId);
void RaceManager_LoadRanking(RaceManager* manager, uint32_t clientId);
```

## Implementation Roadmap

### Phase 1: Network Foundation (Week 1-2)
- [ ] Network socket abstraction
- [ ] UDP/TCP protocol support
- [ ] Basic client-server connection
- [ ] Message serialization
- [ ] Error handling

### Phase 2: Client Prediction (Week 3-4)
- [ ] Input prediction system
- [ ] State reconciliation
- [ ] Frame history management
- [ ] Smooth interpolation
- [ ] Lag compensation

### Phase 3: Authoritative Server (Week 5-6)
- [ ] Server game state management
- [ ] Client connection handling
- [ ] State synchronization
- [ ] Input validation
- [ ] Authentication system

### Phase 4: Local Multiplayer (Week 7-8)
- [ ] Split-screen rendering
- [ ] Local input handling
- [ ] Multiple gamepad support
- [ ] Viewport management
- [ ] Local race sessions

### Phase 5: Racing Features (Week 9-10)
- [ ] Lobby system
- [ ] Matchmaking integration
- [ ] Ranking system
- [ ] Race session management
- [ ] Spectator mode

## Integration Points

### With Entity System
- Network entity synchronization
- Component state replication
- Entity spawning/destruction
- State snapshotting

### With Physics System
- Deterministic physics for networking
- Physics state synchronization
- Collision event networking
- Vehicle physics replication

### With Input System
- Input prediction and validation
- Local input handling
- Network input synchronization
- Input lag compensation

### With Rendering System
- Split-screen rendering
- Network player visualization
- Spectator camera system
- Debug network visualization

## Performance Considerations

### Network Optimization
- Message compression
- Delta compression for state updates
- Priority-based message sending
- Bandwidth usage monitoring

### Server Performance
- Multi-threaded server processing
- Efficient client management
- Load balancing for large sessions
- Database optimization

### Client Performance
- Efficient prediction algorithms
- Minimal memory allocations
- Smooth interpolation
- Frame rate consistency

## Security Considerations

- Input validation and sanitization
- Anti-cheat measures
- Rate limiting for network requests
- Secure authentication
- Protection against DDoS attacks
- Server-side validation of all game state

## Configuration Files

### Network Config
```json
{
    "server": {
        "port": 27015,
        "maxClients": 32,
        "tickRate": 60,
        "requireAuthentication": true
    },
    "client": {
        "serverAddress": "127.0.0.1",
        "serverPort": 27015,
        "predictionFrames": 10,
        "interpolationDelay": 0.1
    },
    "network": {
        "protocol": "UDP",
        "maxPacketSize": 1200,
        "reliableTimeout": 5.0,
        "heartbeatInterval": 1.0
    }
}
```

### Race Config
```json
{
    "race": {
        "type": "RACE",
        "maxPlayers": 16,
        "maxLaps": 10,
        "maxTime": 600,
        "trackName": "silverstone",
        "allowDamage": true,
        "allowCollisions": true
    },
    "lobby": {
        "isOpen": true,
        "maxPlayers": 16,
        "isPrivate": false
    },
    "ranking": {
        "enabled": true,
        "skillRating": 1500,
        "region": "europe"
    }
}
```