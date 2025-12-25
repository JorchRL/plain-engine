# Resource System Specification

## Overview

The Resource System provides a format-agnostic, modular asset management system. Designed for maximum flexibility with hot-reloading support, streaming capabilities, and platform independence. Uses plain C style with composability as a key principle.

## Design Philosophy

- **Format-Agnostic**: Support multiple asset formats through plugin system
- **Composable Loaders**: Modular asset loaders that can be combined
- **Hot Reloading**: Real-time asset updates during development
- **Streaming Support**: Background loading for large assets
- **Memory Efficient**: Smart caching and memory management
- **Plain C Style**: Simple interfaces, minimal C++ features

## Core Architecture

### 1. Resource Handle System

```c
// Resource handle (64-bit)
typedef uint64_t ResourceHandle;

// Handle parts for debugging
typedef struct {
    uint32_t index;
    uint32_t generation;
} ResourceHandleParts;

// Resource types
typedef enum {
    RESOURCE_UNKNOWN = 0,
    RESOURCE_TEXTURE,
    RESOURCE_MESH,
    RESOURCE_AUDIO,
    RESOURCE_SHADER,
    RESOURCE_MATERIAL,
    RESOURCE_ANIMATION,
    RESOURCE_VEHICLE_CONFIG,
    RESOURCE_TRACK_CONFIG,
    RESOURCE_TEXTURE_PACK,
    RESOURCE_MODEL,
    RESOURCE_LEVEL,
    RESOURCE_SCRIPT,
    RESOURCE_DATA,
    RESOURCE_COUNT
} ResourceType;

// Resource state
typedef enum {
    RESOURCE_UNLOADED,
    RESOURCE_LOADING,
    RESOURCE_LOADED,
    RESOURCE_READY,
    RESOURCE_ERROR
} ResourceState;

// Resource metadata
typedef struct {
    ResourceType type;
    const char* name;
    const char* path;
    uint64_t size;
    uint64_t lastModified;
    ResourceHandle handle;
    
    // Loading information
    ResourceState state;
    float loadProgress;
    const char* error;
    bool isPersistent;
    bool isStreaming;
    
    // Dependencies
    ResourceHandle* dependencies;
    uint32_t dependencyCount;
    
    // Hot reloading
    bool watchForChanges;
    uint64_t lastCheckTime;
} ResourceInfo;
```

### 2. Resource Manager

```c
// Resource storage
typedef struct {
    void* data;
    size_t size;
    ResourceHandle handle;
    ResourceType type;
    bool isOwner;
    uint32_t refCount;
    uint64_t lastUsed;
} ResourceStorage;

// Loading operation
typedef struct {
    ResourceHandle handle;
    const char* path;
    ResourceType type;
    void* userData;
    bool isAsync;
    
    // Callbacks
    void (*onLoad)(ResourceStorage* resource, void* userData);
    void (*onProgress)(float progress, void* userData);
    void (*onError)(const char* error, void* userData);
} LoadOperation;

// Resource manager
typedef struct {
    // Resource storage
    ResourceStorage* resources;
    uint32_t resourceCount;
    uint32_t resourceCapacity;
    
    // Free list for handle reuse
    uint32_t* freeHandles;
    uint32_t freeCount;
    
    // Resource metadata
    ResourceInfo* resourceInfo;
    
    // Loading system
    LoadOperation* loadingOperations;
    uint32_t loadingCount;
    uint32_t loadingCapacity;
    PlatformThread* loadingThreads;
    uint32_t threadCount;
    
    // Registry
    const char** extensions;
    ResourceType* extensionTypes;
    uint32_t extensionCount;
    
    // Cache management
    uint64_t maxCacheSize;
    uint64_t currentCacheSize;
    float maxMemoryUsage;
    
    // Hot reloading
    FileWatchHandle* fileWatchers;
    uint32_t watcherCount;
    bool hotReloadingEnabled;
    
    // Memory
    MemoryArena* arena;
    MemoryArena* loadArena;
} ResourceManager;

// Resource manager API
ResourceManager* ResourceManager_Create(MemoryArena* arena, uint64_t maxCacheSize);
void ResourceManager_Destroy(ResourceManager* manager);
void ResourceManager_Update(ResourceManager* manager);

// Resource loading
ResourceHandle ResourceManager_Load(ResourceManager* manager, const char* path, ResourceType type);
ResourceHandle ResourceManager_LoadAsync(ResourceManager* manager, const char* path, ResourceType type, 
                                   void (*onComplete)(ResourceStorage* resource, void* userData), void* userData);
ResourceHandle ResourceManager_LoadWithDependencies(ResourceManager* manager, const char* path, ResourceType type, 
                                                ResourceHandle* dependencies, uint32_t dependencyCount);

// Resource access
ResourceStorage* ResourceManager_GetResource(ResourceManager* manager, ResourceHandle handle);
void* ResourceManager_GetData(ResourceManager* manager, ResourceHandle handle);
ResourceInfo* ResourceManager_GetInfo(ResourceManager* manager, ResourceHandle handle);

// Resource management
void ResourceManager_AddRef(ResourceManager* manager, ResourceHandle handle);
void ResourceManager_Release(ResourceManager* manager, ResourceHandle handle);
void ResourceManager_Unload(ResourceManager* manager, ResourceHandle handle);
void ResourceManager_Reload(ResourceManager* manager, ResourceHandle handle);

// Cache management
void ResourceManager_SetMaxCacheSize(ResourceManager* manager, uint64_t maxSize);
void ResourceManager_ClearCache(ResourceManager* manager);
void ResourceManager_Precache(ResourceManager* manager, const char** paths, uint32_t count);

// Utilities
bool ResourceManager_IsLoaded(ResourceManager* manager, ResourceHandle handle);
bool ResourceManager_IsLoading(ResourceManager* manager, ResourceHandle handle);
float ResourceManager_GetLoadProgress(ResourceManager* manager, ResourceHandle handle);
```

### 3. Resource Loader System

```c
// Asset data structure
typedef struct {
    void* data;
    size_t size;
    uint64_t timestamp;
    const char* format;
    const char* metadata;
} AssetData;

// Loader interface (function pointers for polymorphism)
typedef struct ResourceLoader {
    const char* name;
    const char** supportedExtensions;
    uint32_t extensionCount;
    
    // Required functions
    AssetData* (*load)(const char* path, MemoryArena* arena);
    void* (*process)(const AssetData* assetData, MemoryArena* arena);
    void (*unload)(void* resourceData);
    size_t (*getSize)(const void* resourceData);
    bool (*validate)(const void* resourceData);
    
    // Optional functions
    void (*onHotReload)(void* resourceData, const AssetData* newAssetData);
    bool (*canStream)(const char* path);
    void* (*loadStream)(const char* path, size_t offset, size_t size, MemoryArena* arena);
    
    // Metadata
    ResourceType outputType;
    bool supportsHotReloading;
    bool supportsStreaming;
    uint32_t priority;
} ResourceLoader;

// Loader registry
typedef struct {
    ResourceLoader* loaders;
    uint32_t loaderCount;
    uint32_t loaderCapacity;
    ResourceLoader* defaultLoaders[RESOURCE_COUNT];
} LoaderRegistry;

// Loader registry API
LoaderRegistry* LoaderRegistry_Create(MemoryArena* arena);
void LoaderRegistry_Destroy(LoaderRegistry* registry);
bool LoaderRegistry_Register(LoaderRegistry* registry, const ResourceLoader* loader);
bool LoaderRegistry_Unregister(LoaderRegistry* registry, const char* name);
ResourceLoader* LoaderRegistry_GetLoader(LoaderRegistry* registry, const char* extension);
ResourceLoader* LoaderRegistry_GetLoaderByType(LoaderRegistry* registry, ResourceType type);

// Asset data utilities
AssetData* AssetData_Create(MemoryArena* arena, const char* path);
void AssetData_Destroy(AssetData* asset);
bool AssetData_IsValid(const AssetData* asset);
```

### 4. Specific Asset Loaders

#### Texture Loader

```c
// Texture formats
typedef enum {
    TEXTURE_FORMAT_UNKNOWN,
    TEXTURE_FORMAT_PNG,
    TEXTURE_FORMAT_JPG,
    TEXTURE_FORMAT_BMP,
    TEXTURE_FORMAT_TGA,
    TEXTURE_FORMAT_DDS,
    TEXTURE_FORMAT_KTX,
    TEXTURE_FORMAT_WEBP,
    TEXTURE_FORMAT_ASTC
} TextureFormat;

// Texture data
typedef struct {
    uint8_t* pixels;
    uint32_t width;
    uint32_t height;
    uint32_t channels;
    TextureFormat format;
    bool hasMipmaps;
    bool isCompressed;
    uint8_t compressionLevel;
    
    // OpenGL/Vulkan/Metal specific data
    uint32_t glFormat;
    uint32_t glInternalFormat;
    uint32_t glType;
} TextureData;

// Texture loader
ResourceLoader TextureLoader_Create(void);
AssetData* TextureLoader_LoadPNG(const char* path, MemoryArena* arena);
AssetData* TextureLoader_LoadJPG(const char* path, MemoryArena* arena);
AssetData* TextureLoader_LoadDDS(const char* path, MemoryArena* arena);
TextureData* TextureLoader_Process(const AssetData* assetData, MemoryArena* arena);
void TextureLoader_Unload(void* resourceData);
```

#### Mesh Loader

```c
// Mesh data structures
typedef struct {
    Vec3* positions;
    Vec3* normals;
    Vec2* texCoords;
    Vec3* tangents;
    Vec4* colors;
    uint32_t* indices;
    
    uint32_t vertexCount;
    uint32_t indexCount;
    
    // Submesh information
    uint32_t materialIndex;
    uint32_t startIndex;
    uint32_t indexOffset;
} MeshData;

typedef struct {
    MeshData* submeshes;
    uint32_t submeshCount;
    Vec3 boundsMin;
    Vec3 boundsMax;
    Vec3 center;
    float radius;
    
    // Material references
    const char** materialNames;
    uint32_t materialCount;
} ModelData;

// Mesh loader
ResourceLoader MeshLoader_Create(void);
AssetData* MeshLoader_LoadOBJ(const char* path, MemoryArena* arena);
AssetData* MeshLoader_LoadFBX(const char* path, MemoryArena* arena);
AssetData* MeshLoader_LoadGLTF(const char* path, MemoryArena* arena);
ModelData* MeshLoader_Process(const AssetData* assetData, MemoryArena* arena);
void MeshLoader_Unload(void* resourceData);
```

#### Audio Loader

```c
// Audio formats
typedef enum {
    AUDIO_FORMAT_UNKNOWN,
    AUDIO_FORMAT_WAV,
    AUDIO_FORMAT_OGG,
    AUDIO_FORMAT_MP3,
    AUDIO_FORMAT_FLAC
} AudioFormat;

// Audio data
typedef struct {
    float* samples;
    uint32_t sampleCount;
    uint32_t channels;
    uint32_t sampleRate;
    uint16_t bitsPerSample;
    AudioFormat format;
    bool isLooping;
    float duration;
} AudioData;

// Audio loader
ResourceLoader AudioLoader_Create(void);
AssetData* AudioLoader_LoadWAV(const char* path, MemoryArena* arena);
AssetData* AudioLoader_LoadOGG(const char* path, MemoryArena* arena);
AudioData* AudioLoader_Process(const AssetData* assetData, MemoryArena* arena);
void AudioLoader_Unload(void* resourceData);
```

#### Vehicle Config Loader

```c
// Vehicle configuration
typedef struct {
    // Basic info
    const char* name;
    const char* manufacturer;
    const char* class;
    uint32_t year;
    
    // Physics parameters
    float mass;
    Vec3 centerOfMass;
    Vec3 inertiaTensor;
    
    // Engine
    struct {
        float maxPower;
        float maxTorque;
        float redline;
        float idleRPM;
        float* torqueCurve;  // Array of RPM->torque pairs
        uint32_t curvePoints;
    } engine;
    
    // Transmission
    struct {
        float* gearRatios;
        uint32_t gearCount;
        float finalDrive;
        bool automatic;
    } transmission;
    
    // Wheels and tires
    struct {
        float radius;
        float width;
        float mass;
        const char* tireModel;
    } wheels[4];
    
    // Aerodynamics
    struct {
        float dragCoefficient;
        float frontalArea;
        float liftCoefficient;
        Vec3 downforceCenter;
    } aerodynamics;
    
    // Visual
    const char* meshPath;
    const char* liveryPath;
    const char** wheelMeshPaths;
    const char** liveryPaths;
    
    // Audio
    const char* engineSoundPath;
    const char* tireSoundPath;
    const char* crashSoundPath;
} VehicleConfig;

// Vehicle config loader
ResourceLoader VehicleConfigLoader_Create(void);
AssetData* VehicleConfigLoader_LoadJSON(const char* path, MemoryArena* arena);
VehicleConfig* VehicleConfigLoader_Process(const AssetData* assetData, MemoryArena* arena);
void VehicleConfigLoader_Unload(void* resourceData);
```

### 5. Streaming System

```c
// Streaming request
typedef struct {
    ResourceHandle handle;
    const char* path;
    uint64_t offset;
    uint64_t size;
    ResourceType type;
    uint8_t priority;
    
    // Stream chunking
    uint64_t chunkSize;
    uint64_t currentOffset;
    
    // Callbacks
    void (*onChunkLoaded)(void* chunkData, size_t chunkSize, uint64_t offset, void* userData);
    void (*onComplete)(void* fullData, size_t totalSize, void* userData);
    void (*onError)(const char* error, void* userData);
    
    void* userData;
} StreamRequest;

// Streaming manager
typedef struct {
    StreamRequest* activeRequests;
    uint32_t requestCount;
    uint32_t requestCapacity;
    
    // Streaming thread pool
    PlatformThread* streamingThreads;
    uint32_t threadCount;
    
    // Bandwidth management
    float maxBandwidthMBps;
    float currentBandwidth;
    uint64_t bytesTransferred;
    
    // Priority queue
    StreamRequest** priorityQueue;
    uint32_t queueSize;
    
    // Memory
    MemoryArena* streamArena;
} StreamingManager;

// Streaming API
StreamingManager* StreamingManager_Create(MemoryArena* arena, uint32_t threadCount);
void StreamingManager_Destroy(StreamingManager* manager);
void StreamingManager_Update(StreamingManager* manager);

StreamRequest* StreamingManager_StartStream(StreamingManager* manager, const char* path, uint64_t offset, uint64_t size);
void StreamingManager_CancelStream(StreamingManager* manager, StreamRequest* request);
void StreamingManager_SetBandwidthLimit(StreamingManager* manager, float maxBandwidthMBps);
```

### 6. Resource Bundles and Packs

```c
// Bundle entry
typedef struct {
    const char* path;
    uint64_t offset;
    uint64_t size;
    uint64_t compressedSize;
    ResourceHandle handle;
    bool isCompressed;
    uint32_t compressionLevel;
} BundleEntry;

// Resource bundle
typedef struct {
    const char* name;
    BundleEntry* entries;
    uint32_t entryCount;
    
    // Bundle file
    PlatformFile bundleFile;
    uint64_t bundleSize;
    
    // Index for fast lookup
    struct {
        const char* path;
        BundleEntry* entry;
    }* index;
    uint32_t indexSize;
    
    // Compression
    bool useCompression;
    uint32_t compressionType;
} ResourceBundle;

// Bundle manager
typedef struct {
    ResourceBundle* bundles;
    uint32_t bundleCount;
    uint32_t bundleCapacity;
    
    // Mount points
    struct {
        const char* virtualPath;
        const char* bundlePath;
    }* mounts;
    uint32_t mountCount;
    
    // Fast lookup cache
    struct {
        const char* path;
        BundleEntry* entry;
    }* pathCache;
    uint32_t cacheSize;
} BundleManager;

// Bundle API
BundleManager* BundleManager_Create(MemoryArena* arena);
void BundleManager_Destroy(BundleManager* manager);
bool BundleManager_LoadBundle(BundleManager* manager, const char* bundlePath);
bool BundleManager_MountBundle(BundleManager* manager, const char* virtualPath, const char* bundlePath);
BundleEntry* BundleManager_FindEntry(BundleManager* manager, const char* path);
bool BundleManager_CreateBundle(BundleManager* manager, const char** paths, uint32_t pathCount, const char* outputPath);
```

## Implementation Roadmap

### Phase 1: Core Resource System (Week 1-2)
- [ ] Resource handle system
- [ ] Basic resource manager
- [ ] Simple loader registry
- [ ] Synchronous loading
- [ ] Memory management

### Phase 2: Basic Asset Loaders (Week 3-4)
- [ ] Texture loader (PNG, JPG)
- [ ] Mesh loader (OBJ)
- [ ] Audio loader (WAV)
- [ ] JSON config loader
- [ ] Error handling

### Phase 3: Advanced Features (Week 5-6)
- [ ] Asynchronous loading
- [ ] Hot reloading support
- [ ] Dependency management
- [ ] Cache management
- [ ] Performance optimization

### Phase 4: Streaming (Week 7-8)
- [ ] Streaming system foundation
- [ ] Chunked loading
- [ ] Bandwidth management
- [ ] Priority queues
- [ ] Large asset streaming

### Phase 5: Bundles and Packaging (Week 9-10)
- [ ] Resource bundle system
- [ ] Compression support
- [ ] Bundle creation tools
- [ ] Virtual mounting
- [ ] Platform-specific packaging

## Integration Points

### With Rendering System
- Texture and mesh loading
- Material resource management
- Shader loading and compilation
- GPU resource management

### With Audio System
- Audio sample loading
- Streaming audio support
- Sound bank management
- Audio configuration

### With Entity System
- Entity configuration loading
- Component data resources
- Level data loading
- Prefab system

### With Physics System
- Collision mesh loading
- Physics configuration
- Vehicle physics data
- Track geometry loading

## Performance Considerations

### Memory Management
- Arena allocation for temporary data
- Smart caching with LRU eviction
- Memory pool for small allocations
- Reference counting for shared resources

### Loading Performance
- Parallel loading with thread pool
- Asynchronous I/O operations
- Priority-based loading
- Progressive loading for large assets

### Storage Efficiency
- Asset compression
- Delta updates for hot reloading
- Bundle packing for distribution
- Memory-mapped file access

## Testing Strategy

### Unit Tests
- Loader functionality
- Handle generation and reuse
- Cache behavior
- Hot reloading scenarios

### Integration Tests
- Multi-format asset loading
- Dependency resolution
- Streaming performance
- Bundle creation and mounting

### Performance Tests
- Loading time benchmarks
- Memory usage validation
- Streaming bandwidth tests
- Concurrent access patterns

## Configuration Files

### Resource System Config
```json
{
    "cache": {
        "maxSize": "2GB",
        "maxMemoryUsage": 0.8,
        "persistentFiles": ["core_textures", "engine_sounds"]
    },
    "streaming": {
        "enabled": true,
        "threadCount": 4,
        "maxBandwidthMBps": 50,
        "chunkSize": "1MB"
    },
    "hotReloading": {
        "enabled": true,
        "checkInterval": 1000,
        "watchExtensions": [".png", ".jpg", ".obj", ".json", ".wav"]
    },
    "bundles": {
        "autoMount": true,
        "compressionLevel": 6,
        "createIndex": true
    }
}
```

### Asset Registry
```json
{
    "loaders": [
        {
            "name": "PNG Texture Loader",
            "extensions": [".png"],
            "type": "TEXTURE",
            "priority": 10,
            "supportsHotReloading": true
        },
        {
            "name": "OBJ Mesh Loader", 
            "extensions": [".obj"],
            "type": "MESH",
            "priority": 10,
            "supportsHotReloading": true
        }
    ],
    "aliases": {
        "car_body": "assets/models/vehicles/sports_car_body.obj",
        "default_texture": "assets/textures/missing.png",
        "engine_sound": "assets/audio/engine/sports_car_idle.ogg"
    }
}
```