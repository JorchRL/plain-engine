# Rendering System Specification

## Overview

The Rendering System provides a platform-agnostic, multi-API rendering abstraction. Designed for maximum performance and flexibility, supporting OpenGL, Metal, DirectX, and Vulkan through a common interface. Uses plain C style with composability as a key principle.

## Design Philosophy

- **Platform-Agnostic**: Abstract multiple graphics APIs behind common interface
- **Data-Oriented**: GPU-friendly data layouts and batch processing
- **Composable**: Modular rendering stages that can be combined
- **Performance First**: Optimized for real-time rendering
- **Plain C Style**: Simple structures, function pointers for polymorphism
- **Extensible**: Easy to add new rendering techniques and effects

## Core Architecture

### 1. Graphics Abstraction Layer

```c
// Graphics API types
typedef enum {
    GRAPHICS_API_NONE,
    GRAPHICS_API_OPENGL,
    GRAPHICS_API_METAL,
    GRAPHICS_API_DIRECTX11,
    GRAPHICS_API_DIRECTX12,
    GRAPHICS_API_VULKAN
} GraphicsAPI;

// GPU resource handles (opaque pointers)
typedef struct GPUBuffer* GPUBuffer;
typedef struct GPUTexture* GPUTexture;
typedef struct GPUShader* GPUShader;
typedef struct GPUPipeline* GPUPipeline;
typedef struct GPURenderTarget* GPURenderTarget;
typedef struct GPUSampler* GPUSampler;

// GPU buffer types
typedef enum {
    BUFFER_VERTEX,
    BUFFER_INDEX,
    BUFFER_UNIFORM,
    BUFFER_STORAGE,
    BUFFER_STAGING,
    BUFFER_INDIRECT
} BufferType;

// GPU texture formats
typedef enum {
    FORMAT_R8_UNORM,
    FORMAT_RG8_UNORM,
    FORMAT_RGB8_UNORM,
    FORMAT_RGBA8_UNORM,
    FORMAT_RGB16_FLOAT,
    FORMAT_RGBA16_FLOAT,
    FORMAT_R32_FLOAT,
    FORMAT_RG32_FLOAT,
    FORMAT_RGB32_FLOAT,
    FORMAT_RGBA32_FLOAT,
    FORMAT_D24_UNORM_S8_UINT,
    FORMAT_D32_FLOAT,
    FORMAT_BC1_UNORM,    // DXT1
    FORMAT_BC3_UNORM,    // DXT5
    FORMAT_BC5_UNORM,    // Normal maps
    FORMAT_BC7_UNORM     // High quality compression
} TextureFormat;

// GPU shader types
typedef enum {
    SHADER_VERTEX,
    SHADER_FRAGMENT,
    SHADER_GEOMETRY,
    SHADER_COMPUTE,
    SHADER_HULL,
    SHADER_DOMAIN
} ShaderType;

// Blend modes
typedef enum {
    BLEND_NONE,
    BLEND_ALPHA,
    BLEND_ADDITIVE,
    BLEND_MULTIPLY,
    BLEND_SCREEN,
    BLEND_OVERLAY
} BlendMode;

// Cull modes
typedef enum {
    CULL_NONE,
    CULL_FRONT,
    CULL_BACK
} CullMode;

// Compare functions
typedef enum {
    COMPARE_NEVER,
    COMPARE_LESS,
    COMPARE_EQUAL,
    COMPARE_LESS_EQUAL,
    COMPARE_GREATER,
    COMPARE_NOT_EQUAL,
    COMPARE_GREATER_EQUAL,
    COMPARE_ALWAYS
} CompareFunction;

// Graphics capabilities
typedef struct {
    GraphicsAPI api;
    const char* rendererName;
    const char* vendorName;
    const char* version;
    
    // Limits
    uint32_t maxTextureSize;
    uint32_t maxRenderTargets;
    uint32_t maxUniformBufferSize;
    uint32_t maxStorageBufferSize;
    uint32_t maxVertexAttributes;
    float maxAnisotropy;
    
    // Features
    bool supportsCompute;
    bool supportsGeometryShaders;
    bool supportsTessellation;
    bool supportsMultiSample;
    bool supportsS3TC;
    bool supportsBC7;
    bool supportsASTC;
    
    // Performance
    bool supportsAsyncTransfer;
    bool supportsPersistentMapping;
    bool supportsBindlessTextures;
} GraphicsCapabilities;
```

### 2. Graphics Context

```c
// Graphics context
typedef struct {
    GraphicsAPI api;
    GraphicsCapabilities capabilities;
    
    // Platform-specific context
    void* platformContext;
    
    // Default resources
    GPUBuffer* defaultVertexBuffer;
    GPUBuffer* defaultIndexBuffer;
    GPUTexture* defaultTexture;
    GPUSampler* defaultSampler;
    
    // Frame management
    uint32_t currentFrame;
    uint32_t frameIndex;
    uint64_t frameCounter;
    
    // Command buffers
    struct {
        void* commandBuffer;
        bool recording;
    } commandBuffers[3];  // Triple buffering
    
    // Synchronization
    struct {
        PlatformMutex* mutex;
        PlatformCondition* condition;
        bool ready;
    } frameSync;
    
    // Memory
    MemoryArena* arena;
    MemoryArena* frameArena;
    
    // Statistics
    struct {
        uint32_t drawCalls;
        uint32_t triangles;
        uint32_t vertices;
        uint32_t stateChanges;
        float frameTime;
        float gpuTime;
    } stats;
} GraphicsContext;

// Context API
GraphicsContext* GraphicsContext_Create(PlatformWindow window, GraphicsAPI preferredAPI);
void GraphicsContext_Destroy(GraphicsContext* context);
void GraphicsContext_BeginFrame(GraphicsContext* context);
void GraphicsContext_EndFrame(GraphicsContext* context);
void GraphicsContext_Present(GraphicsContext* context);
void GraphicsContext_WaitForFrame(GraphicsContext* context);

// Capabilities
const GraphicsCapabilities* GraphicsContext_GetCapabilities(GraphicsContext* context);
bool GraphicsContext_SupportsFeature(GraphicsContext* context, const char* feature);

// Statistics
void GraphicsContext_ResetStats(GraphicsContext* context);
const GraphicsStats* GraphicsContext_GetStats(GraphicsContext* context);
```

### 3. Resource Management

```c
// Buffer description
typedef struct {
    BufferType type;
    size_t size;
    bool isDynamic;
    bool isCPUAccessible;
    void* initialData;
} BufferDesc;

// Texture description
typedef struct {
    TextureFormat format;
    uint32_t width, height, depth;
    uint32_t mipLevels;
    uint32_t arraySize;
    bool isRenderTarget;
    bool isDepthStencil;
    bool generateMipmaps;
    void* initialData;
    size_t dataSize;
} TextureDesc;

// Shader description
typedef struct {
    ShaderType type;
    const char* source;
    const char* entryPoint;
    const char** defines;
    uint32_t defineCount;
} ShaderDesc;

// Pipeline description
typedef struct {
    GPUShader* vertexShader;
    GPUShader* fragmentShader;
    GPUShader* geometryShader;
    GPUShader* hullShader;
    GPUShader* domainShader;
    
    // Rasterizer state
    CullMode cullMode;
    bool wireframe;
    float lineWidth;
    
    // Depth stencil state
    bool depthTest;
    bool depthWrite;
    CompareFunction depthCompare;
    bool stencilTest;
    uint8_t stencilReadMask;
    uint8_t stencilWriteMask;
    
    // Blend state
    bool blendEnabled;
    BlendMode sourceBlend;
    BlendMode destBlend;
    
    // Render targets
    GPURenderTarget* renderTargets[8];
    uint32_t renderTargetCount;
    GPURenderTarget* depthStencilTarget;
    
    // Vertex layout
    struct {
        uint32_t location;
        TextureFormat format;
        uint32_t offset;
        uint32_t stride;
    } vertexLayout[16];
    uint32_t vertexLayoutCount;
} PipelineDesc;

// Sampler description
typedef struct {
    TextureFilter minFilter;
    TextureFilter magFilter;
    TextureFilter mipmapFilter;
    TextureWrapMode wrapU;
    TextureWrapMode wrapV;
    TextureWrapMode wrapW;
    float maxAnisotropy;
    bool compareEnabled;
    CompareFunction compareFunc;
} SamplerDesc;

// Resource creation API
GPUBuffer* GraphicsContext_CreateBuffer(GraphicsContext* context, const BufferDesc* desc);
void GraphicsContext_DestroyBuffer(GraphicsContext* context, GPUBuffer* buffer);
void* GraphicsContext_MapBuffer(GraphicsContext* context, GPUBuffer* buffer, size_t offset, size_t size);
void GraphicsContext_UnmapBuffer(GraphicsContext* context, GPUBuffer* buffer);
void GraphicsContext_UpdateBuffer(GraphicsContext* context, GPUBuffer* buffer, size_t offset, size_t size, const void* data);

GPUTexture* GraphicsContext_CreateTexture(GraphicsContext* context, const TextureDesc* desc);
void GraphicsContext_DestroyTexture(GraphicsContext* context, GPUTexture* texture);
void GraphicsContext_UpdateTexture(GraphicsContext* context, GPUTexture* texture, uint32_t level, const void* data, size_t size);
void GraphicsContext_GenerateMipmaps(GraphicsContext* context, GPUTexture* texture);

GPUShader* GraphicsContext_CreateShader(GraphicsContext* context, const ShaderDesc* desc);
void GraphicsContext_DestroyShader(GraphicsContext* context, GPUShader* shader);

GPUPipeline* GraphicsContext_CreatePipeline(GraphicsContext* context, const PipelineDesc* desc);
void GraphicsContext_DestroyPipeline(GraphicsContext* context, GPUPipeline* pipeline);

GPUSampler* GraphicsContext_CreateSampler(GraphicsContext* context, const SamplerDesc* desc);
void GraphicsContext_DestroySampler(GraphicsContext* context, GPUSampler* sampler);
```

### 4. Rendering Pipeline

```c
// Render target
typedef struct {
    GPUTexture* colorTextures[8];
    GPUTexture* depthTexture;
    uint32_t width, height;
    uint32_t colorAttachmentCount;
    bool hasDepth;
    uint32_t sampleCount;
} RenderTarget;

// View and projection
typedef struct {
    Vec3 position;
    Vec3 target;
    Vec3 up;
    float fov;
    float aspectRatio;
    float nearPlane, farPlane;
    
    // Precomputed matrices
    Mat4 viewMatrix;
    Mat4 projectionMatrix;
    Mat4 viewProjectionMatrix;
    Mat4 inverseViewMatrix;
    Mat4 inverseProjectionMatrix;
} Camera;

// Render object
typedef struct {
    uint32_t meshId;
    uint32_t materialId;
    Mat4 transform;
    uint32_t sortKey;
    bool visible;
    bool castShadows;
    bool receiveShadows;
    uint8_t renderQueue;
} RenderObject;

// Render queue
typedef struct {
    RenderObject* objects;
    uint32_t count;
    uint32_t capacity;
    bool sorted;
    uint8_t queueType;
} RenderQueue;

// Rendering stage
typedef struct {
    const char* name;
    bool enabled;
    void (*render)(GraphicsContext* context, const RenderData* data);
    void (*setup)(GraphicsContext* context, const RenderData* data);
    void (*cleanup)(GraphicsContext* context);
    struct RenderStage* next;
} RenderStage;

// Main renderer
typedef struct {
    GraphicsContext* context;
    
    // Render queues
    RenderQueue* queues;
    uint32_t queueCount;
    
    // Active camera
    Camera* activeCamera;
    uint32_t cameraCount;
    
    // Render pipeline
    RenderStage* pipeline;
    
    // Global resources
    struct {
        GPUBuffer* globalUniforms;
        GPUBuffer* cameraUniforms;
        GPUBuffer* objectUniforms;
        GPUSampler* defaultSampler;
        GPUSampler* shadowSampler;
        GPUTexture* defaultWhite;
        GPUTexture* defaultBlack;
        GPUTexture* defaultNormal;
    } globalResources;
    
    // Shadows
    struct {
        GPURenderTarget* shadowMaps[4];
        Mat4 shadowMatrices[4];
        uint32_t shadowMapSize;
        bool enabled;
        float cascadeSplits[4];
    } shadowSystem;
    
    // Post-processing
    struct {
        GPURenderTarget* hdrTarget;
        GPURenderTarget* bloomTargets[3];
        GPUTexture* luminanceTexture;
        GPUPipeline* tonemapPipeline;
        GPUPipeline* bloomPipeline;
        bool enabled;
    } postProcess;
    
    // Memory
    MemoryArena* arena;
    MemoryArena* frameArena;
} Renderer;

// Renderer API
Renderer* Renderer_Create(PlatformWindow window, GraphicsAPI api);
void Renderer_Destroy(Renderer* renderer);
void Renderer_BeginFrame(Renderer* renderer);
void Renderer_EndFrame(Renderer* renderer);
void Renderer_Render(Renderer* renderer);

// Camera management
uint32_t Renderer_AddCamera(Renderer* renderer, const Camera* camera);
void Renderer_RemoveCamera(Renderer* renderer, uint32_t cameraId);
void Renderer_SetActiveCamera(Renderer* renderer, uint32_t cameraId);
Camera* Renderer_GetCamera(Renderer* renderer, uint32_t cameraId);

// Render object management
uint32_t Renderer_AddRenderObject(Renderer* renderer, const RenderObject* object);
void Renderer_UpdateRenderObject(Renderer* renderer, uint32_t objectId, const RenderObject* object);
void Renderer_RemoveRenderObject(Renderer* renderer, uint32_t objectId);
void Renderer_SetRenderObjectVisibility(Renderer* renderer, uint32_t objectId, bool visible);

// Pipeline management
void Renderer_AddRenderStage(Renderer* renderer, RenderStage* stage);
void Renderer_RemoveRenderStage(Renderer* renderer, const char* name);
void Renderer_EnableRenderStage(Renderer* renderer, const char* name, bool enabled);
```

### 5. 2D Rendering System

```c
// 2D vertex format
typedef struct {
    Vec2 position;
    Vec2 texCoord;
    uint32_t color;
} SpriteVertex;

// 2D sprite
typedef struct {
    Vec2 position;
    Vec2 size;
    Vec4 color;
    float rotation;
    Vec2 origin;
    Vec2 texRect;  // x, y, width, height in texture coordinates
    uint32_t textureId;
    uint8_t blendMode;
    float depth;
} Sprite2D;

// Text rendering
typedef struct {
    const char* text;
    Vec2 position;
    Vec2 scale;
    uint32_t color;
    uint32_t fontId;
    float maxWidth;
    bool wordWrap;
    uint8_t alignment;  // 0=left, 1=center, 2=right
} Text2D;

// 2D renderer
typedef struct {
    // Buffers
    GPUBuffer* vertexBuffer;
    GPUBuffer* indexBuffer;
    SpriteVertex* vertices;
    uint32_t vertexCount;
    uint32_t vertexCapacity;
    
    // Shaders and pipelines
    GPUPipeline* spritePipeline;
    GPUPipeline* textPipeline;
    GPUPipeline* uiPipeline;
    
    // 2D camera (orthographic)
    Mat4 projectionMatrix;
    Vec2 viewportSize;
    
    // Batching
    struct {
        GPUTexture* currentTexture;
        uint8_t currentBlendMode;
        uint32_t drawCalls;
    } batch;
    
    // Fonts
    struct {
        GPUTexture* texture;
        uint32_t firstChar;
        uint32_t charCount;
        struct {
            uint32_t x, y;
            uint32_t width, height;
            float advance;
            float offsetX, offsetY;
        }* glyphs;
    }* fonts;
    uint32_t fontCount;
    
    // UI system
    struct {
        Vec2 mousePosition;
        bool mouseButtons[3];
        struct {
            bool visible;
            Vec2 position;
            Vec2 size;
            const char* text;
            void (*onClick)();
            void (*onHover)();
        }* elements;
        uint32_t elementCount;
    } ui;
    
    MemoryArena* arena;
} Renderer2D;

// 2D rendering API
Renderer2D* Renderer2D_Create(GraphicsContext* context);
void Renderer2D_Destroy(Renderer2D* renderer);
void Renderer2D_Begin(Renderer2D* renderer);
void Renderer2D_End(Renderer2D* renderer);
void Renderer2D_Render(Renderer2D* renderer, const Mat4* viewProjection);

// Drawing functions
void Renderer2D_DrawSprite(Renderer2D* renderer, const Sprite2D* sprite);
void Renderer2D_DrawText(Renderer2D* renderer, const Text2D* text);
void Renderer2D_DrawRect(Renderer2D* renderer, Vec2 position, Vec2 size, uint32_t color);
void Renderer2D_DrawCircle(Renderer2D* renderer, Vec2 center, float radius, uint32_t color);
void Renderer2D_DrawLine(Renderer2D* renderer, Vec2 start, Vec2 end, uint32_t color, float thickness);

// Font management
uint32_t Renderer2D_LoadFont(Renderer2D* renderer, const char* fontPath, uint32_t size);
void Renderer2D_SetFont(Renderer2D* renderer, uint32_t fontId);
Vec2 Renderer2D_MeasureText(Renderer2D* renderer, const char* text, uint32_t fontId, Vec2 scale);

// UI functions
uint32_t Renderer2D_AddButton(Renderer2D* renderer, Vec2 position, Vec2 size, const char* text, void (*onClick)());
void Renderer2D_AddLabel(Renderer2D* renderer, Vec2 position, const char* text, uint32_t color);
bool Renderer2D_IsMouseOver(Renderer2D* renderer, Vec2 position, Vec2 size);
```

### 6. Debug Rendering

```c
// Debug vertex format
typedef struct {
    Vec3 position;
    uint32_t color;
    float thickness;
} DebugVertex;

// Debug rendering types
typedef enum {
    DEBUG_LINES,
    DEBUG_TRIANGLES,
    DEBUG_POINTS
} DebugPrimitiveType;

// Debug renderer
typedef struct {
    // Buffers
    GPUBuffer* vertexBuffer;
    GPUBuffer* indexBuffer;
    DebugVertex* vertices;
    uint32_t* indices;
    uint32_t vertexCount;
    uint32_t indexCount;
    uint32_t vertexCapacity;
    uint32_t indexCapacity;
    
    // Shaders
    GPUPipeline* linePipeline;
    GPUPipeline* trianglePipeline;
    GPUPipeline* pointPipeline;
    
    // Camera for debug view
    Mat4 viewProjectionMatrix;
    
    // Primitives
    struct {
        DebugVertex* vertices;
        uint32_t vertexCount;
        DebugPrimitiveType type;
        float lifetime;
        float age;
    }* primitives;
    uint32_t primitiveCount;
    uint32_t primitiveCapacity;
    
    MemoryArena* arena;
} DebugRenderer;

// Debug rendering API
DebugRenderer* DebugRenderer_Create(GraphicsContext* context);
void DebugRenderer_Destroy(DebugRenderer* renderer);
void DebugRenderer_Begin(DebugRenderer* renderer);
void DebugRenderer_End(DebugRenderer* renderer);
void DebugRenderer_Render(DebugRenderer* renderer, const Mat4* viewProjection);

// Drawing functions
void DebugRenderer_DrawLine(DebugRenderer* renderer, Vec3 start, Vec3 end, uint32_t color, float thickness, float lifetime);
void DebugRenderer_DrawTriangle(DebugRenderer* renderer, Vec3 a, Vec3 b, Vec3 c, uint32_t color, float lifetime);
void DebugRenderer_DrawBox(DebugRenderer* renderer, Vec3 center, Vec3 extents, Mat4 rotation, uint32_t color, float lifetime);
void DebugRenderer_DrawSphere(DebugRenderer* renderer, Vec3 center, float radius, uint32_t color, float lifetime);
void DebugRenderer_DrawCapsule(DebugRenderer* renderer, Vec3 start, Vec3 end, float radius, uint32_t color, float lifetime);
void DebugRenderer_DrawFrustum(DebugRenderer* renderer, const Mat4* viewProjection, uint32_t color, float lifetime);
void DebugRenderer_DrawGrid(DebugRenderer* renderer, Vec3 origin, Vec2 size, float cellSize, uint32_t color, float lifetime);
void DebugRenderer_DrawAxis(DebugRenderer* renderer, Vec3 position, float size, float lifetime);

// Physics debug helpers
void DebugRenderer_DrawPhysicsBody(DebugRenderer* renderer, const PhysicsBody* body, uint32_t color, float lifetime);
void DebugRenderer_DrawCollisionShape(DebugRenderer* renderer, const PhysicsShape* shape, Vec3 position, Mat4 rotation, uint32_t color, float lifetime);
void DebugRenderer_DrawContactPoint(DebugRenderer* renderer, Vec3 point, Vec3 normal, float penetration, uint32_t color, float lifetime);
void DebugRenderer_DrawVehicle(DebugRenderer* renderer, const VehicleComponent* vehicle, uint32_t color, float lifetime);

// Update and management
void DebugRenderer_Update(DebugRenderer* renderer, float deltaTime);
void DebugRenderer_Clear(DebugRenderer* renderer);
```

## Implementation Roadmap

### Phase 1: Core Graphics Abstraction (Week 1-2)
- [ ] Graphics context creation
- [ ] Basic resource management
- [ ] Simple triangle rendering
- [ ] Platform-specific backends
- [ ] Error handling

### Phase 2: 3D Rendering Foundation (Week 3-4)
- [ ] Mesh loading and rendering
- [ ] Material system
- [ ] Camera system
- [ ] Basic lighting
- [ ] Shadow mapping

### Phase 3: Advanced Rendering (Week 5-6)
- [ ] Deferred rendering
- [ ] Post-processing effects
- [ ] Multiple render targets
- [ ] Instanced rendering
- [ ] Performance optimization

### Phase 4: 2D Rendering (Week 7-8)
- [ ] Sprite batching system
- [ ] Text rendering
- [ ] UI system
- [ ] Font loading
- [ ] 2D debug rendering

### Phase 5: Debug and Tools (Week 9-10)
- [ ] Debug rendering system
- [ ] Physics visualization
- [ ] Performance overlays
- [ ] Rendering statistics
- [ ] Editor integration

## Integration Points

### With Resource System
- Texture and mesh loading
- Shader compilation and caching
- Material resource management
- Asset hot reloading

### With Entity System
- Entity rendering components
- Transform component integration
- Visibility culling
- LOD system integration

### With Physics System
- Physics debug visualization
- Collision rendering
- Vehicle debug rendering
- Track boundary rendering

### With Input System
- UI interaction handling
- Camera controls
- Debug tool interaction
- Touch UI elements

## Performance Considerations

### GPU Optimization
- Minimize state changes
- Efficient batch rendering
- Optimize vertex layouts
- Use instanced rendering

### Memory Management
- Arena allocation for frame data
- GPU buffer pooling
- Texture streaming
- Resource caching

### CPU Optimization
- Multi-threaded command building
- SIMD for vertex processing
- Efficient culling algorithms
- Parallel resource loading

## Platform-Specific Considerations

### OpenGL
- Legacy feature support
- Compatibility with different versions
- Extension detection and usage
- Desktop optimization

### Metal (macOS/iOS)
- Metal Shading Language
- Command buffer optimization
- Metal-specific features
- iOS integration

### DirectX 11/12
- Windows optimization
- DirectX 12 low-level features
- Windows Store compatibility
- Xbox integration potential

### Vulkan
- Cross-platform compatibility
- Low-level optimization
- Mobile device support
- Future-proofing

## Configuration Files

### Rendering Config
```json
{
    "graphics": {
        "preferredAPI": "OpenGL",
        "vsync": true,
        "tripleBuffering": true,
        "maxAnisotropy": 16,
        "shadowMapSize": 2048,
        "maxShadowCascades": 4
    },
    "quality": {
        "textureQuality": "high",
        "shadowQuality": "medium",
        "postProcessing": true,
        "antiAliasing": "FXAA",
        "LOD": true
    },
    "debug": {
        "showStats": false,
        "showWireframe": false,
        "showBoundingVolumes": false,
        "profilingEnabled": false
    }
}
```

### Material Config
```json
{
    "material": {
        "name": "car_body",
        "albedoTexture": "car_body_diffuse.png",
        "normalTexture": "car_body_normal.png",
        "roughnessTexture": "car_body_roughness.png",
        "metallicTexture": "car_body_metallic.png",
        "shader": "pbr_standard",
        "parameters": {
            "roughness": 0.7,
            "metallic": 0.1,
            "emissive": 0.0
        }
    }
}
```