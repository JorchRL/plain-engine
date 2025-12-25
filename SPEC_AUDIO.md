# Audio System Specification

## Overview

The Audio System provides a platform-agnostic, high-performance audio engine. Designed for real-time audio processing with support for 3D spatial audio, dynamic music, and vehicle-specific audio simulation. Uses plain C style with composability and minimal dependencies.

## Design Philosophy

- **Platform-Agnostic**: Abstract multiple audio APIs behind common interface
- **Real-Time Processing**: Low-latency audio suitable for games
- **Composable Effects**: Modular audio processing chain
- **3D Spatial Audio**: Positional audio with environmental effects
- **Plain C Style**: Simple structures, function pointers for polymorphism
- **Performance First**: Optimized for real-time audio processing

## Core Architecture

### 1. Audio Abstraction Layer

```c
// Audio API types
typedef enum {
    AUDIO_API_NONE,
    AUDIO_API_OPENAL,
    AUDIO_API_XAUDIO2,
    AUDIO_API_COREAUDIO,
    AUDIO_API_OPENSL_ES,
    AUDIO_API_PULSEAUDIO,
    AUDIO_API_ALSA
} AudioAPI;

// Audio formats
typedef enum {
    AUDIO_FORMAT_UNKNOWN,
    AUDIO_FORMAT_MONO8,
    AUDIO_FORMAT_MONO16,
    AUDIO_FORMAT_STEREO8,
    AUDIO_FORMAT_STEREO16,
    AUDIO_FORMAT_MONO32F,
    AUDIO_FORMAT_STEREO32F
} AudioFormat;

// Audio device capabilities
typedef struct {
    AudioAPI api;
    const char* name;
    const char* driverVersion;
    
    // Device info
    uint32_t maxOutputChannels;
    uint32_t maxInputChannels;
    uint32_t sampleRate;
    uint32_t bufferSize;
    
    // Features
    bool supports3D;
    bool supportsReverb;
    bool supportsChorus;
    bool supportsDistortion;
    bool supportsPitchShift;
    bool supportsDoppler;
    
    // Performance
    uint32_t minLatency;
    uint32_t maxLatency;
    bool supportsLowLatency;
} AudioCapabilities;

// Audio listener for 3D audio
typedef struct {
    Vec3 position;
    Vec3 velocity;
    Vec3 forward;
    Vec3 up;
    float gain;
} AudioListener;
```

### 2. Audio Context

```c
// Audio context (main audio system)
typedef struct {
    AudioAPI api;
    AudioCapabilities capabilities;
    
    // Platform-specific context
    void* platformContext;
    
    // Listener
    AudioListener listener;
    
    // Mixing
    float* mixBuffer;
    uint32_t mixBufferSize;
    uint32_t outputChannels;
    uint32_t sampleRate;
    
    // Performance
    uint32_t frameCount;
    float cpuUsage;
    uint32_t droppedFrames;
    
    // Effects
    struct {
        bool enabled;
        float reverbWet;
        float reverbRoom;
        float reverbDamping;
        float reverbWidth;
    } globalEffects;
    
    // Memory
    MemoryArena* arena;
    MemoryArena* frameArena;
} AudioContext;

// Audio context API
AudioContext* AudioContext_Create(const AudioConfig* config);
void AudioContext_Destroy(AudioContext* context);
void AudioContext_Update(AudioContext* context);
void AudioContext_SetListener(AudioContext* context, const AudioListener* listener);

// Capabilities
const AudioCapabilities* AudioContext_GetCapabilities(AudioContext* context);
bool AudioContext_SupportsFeature(AudioContext* context, const char* feature);

// Performance monitoring
void AudioContext_ResetStats(AudioContext* context);
const AudioStats* AudioContext_GetStats(AudioContext* context);
```

### 3. Audio Samples and Streams

```c
// Audio sample data
typedef struct {
    float* samples;
    uint32_t sampleCount;
    uint32_t channels;
    uint32_t sampleRate;
    AudioFormat format;
    float duration;
    
    // Metadata
    const char* name;
    const char* path;
    uint64_t lastModified;
    
    // Loop information
    bool isLooping;
    uint32_t loopStart;
    uint32_t loopEnd;
    
    // Streaming support
    bool isStreamed;
    const char* streamPath;
} AudioSample;

// Audio stream for large files
typedef struct {
    PlatformFile* file;
    AudioFormat format;
    uint32_t sampleRate;
    uint32_t channels;
    
    // Streaming state
    uint32_t currentPos;
    uint32_t bufferSize;
    float* buffer;
    bool isEndOfFile;
    
    // Compression
    bool isCompressed;
    void* decoder;
    
    // Background loading
    PlatformThread* streamingThread;
    PlatformMutex* bufferMutex;
    PlatformCondition* bufferCondition;
    bool isActive;
} AudioStream;

// Sample and stream management API
AudioSample* AudioSample_Load(const char* path, MemoryArena* arena);
void AudioSample_Destroy(AudioSample* sample);
AudioSample* AudioSample_LoadStreamed(const char* path, uint32_t bufferSize, MemoryArena* arena);

AudioStream* AudioStream_Create(const char* path, MemoryArena* arena);
void AudioStream_Destroy(AudioStream* stream);
uint32_t AudioStream_Read(AudioStream* stream, float* buffer, uint32_t frames);
bool AudioStream_Seek(AudioStream* stream, uint32_t frame);
```

### 4. Audio Sources

```c
// 3D audio source
typedef struct {
    uint32_t sourceId;  // Platform-specific ID
    Vec3 position;
    Vec3 velocity;
    float gain;
    float pitch;
    float innerCone;
    float outerCone;
    Vec3 direction;
    bool relative;
    bool looping;
    
    // Distance attenuation
    float refDistance;
    float maxDistance;
    float rolloffFactor;
    
    // Sample playback
    AudioSample* sample;
    AudioStream* stream;
    uint32_t currentFrame;
    bool isPlaying;
    bool isPaused;
    bool isMuted;
    
    // Effects
    struct {
        float reverbSend;
        float chorusSend;
        float distortion;
        float lowPass;
        float highPass;
    } effects;
    
    // Vehicle-specific audio
    struct {
        float rpm;
        float throttle;
        float speed;
        float load;
        uint32_t gear;
    } vehicleState;
} AudioSource;

// Audio source management
AudioSource* AudioSource_Create(AudioContext* context);
void AudioSource_Destroy(AudioContext* context, AudioSource* source);
void AudioSource_Play(AudioSource* source);
void AudioSource_Pause(AudioSource* source);
void AudioSource_Stop(AudioSource* source);

void AudioSource_SetSample(AudioSource* source, AudioSample* sample);
void AudioSource_SetStream(AudioSource* source, AudioStream* stream);
void AudioSource_SetPosition(AudioSource* source, Vec3 position);
void AudioSource_SetVelocity(AudioSource* source, Vec3 velocity);
void AudioSource_SetGain(AudioSource* source, float gain);
void AudioSource_SetPitch(AudioSource* source, float pitch);
void AudioSource_SetLooping(AudioSource* source, bool looping);

bool AudioSource_IsPlaying(const AudioSource* source);
bool AudioSource_IsPaused(const AudioSource* source);
```

### 5. Vehicle Audio System

```c
// Engine sound parameters
typedef struct {
    // Base engine sound
    AudioSample* idleSound;
    AudioSample* lowSound;
    AudioSample* midSound;
    AudioSample* highSound;
    AudioSample* redlineSound;
    
    // RPM thresholds for interpolation
    float idleRPM;
    float lowRPM;
    float midRPM;
    float highRPM;
    float redlineRPM;
    
    // Sound parameters
    float basePitch;
    float pitchRange;
    float loadFactor;
    float turboWhistleGain;
    
    // Exhaust sounds
    AudioSample* backfireSound;
    AudioSample* turboSpoolSound;
    AudioSample* blowoffValveSound;
    
    // Performance
    bool enableTurbo;
    float turboThreshold;
    float backfireThreshold;
} EngineAudio;

// Tire sounds
typedef struct {
    AudioSample* rollingSound;
    AudioSample* skidSound;
    AudioSample* grassSound;
    AudioSample* gravelSound;
    AudioSample* waterSound;
    
    // Parameters
    float rollingGainAt10MPH;
    float skidThreshold;
    float skidGain;
    float skidPitchRange;
} TireAudio;

// Impact and damage sounds
typedef struct {
    AudioSample* impactSounds[8];
    AudioSample* crashSounds[4];
    AudioSample* scrapeSound;
    AudioSample* sparkSound;
    
    // Parameters
    uint32_t impactCount;
    uint32_t crashCount;
    float impactGain;
    float crashGain;
    float scrapeGain;
} DamageAudio;

// Vehicle audio system
typedef struct {
    AudioSource* engineSource;
    AudioSource* exhaustSource;
    AudioSource* turboSource;
    AudioSource* tireSources[4];  // One for each wheel
    AudioSource* windSource;
    
    // Audio configuration
    EngineAudio engine;
    TireAudio tires;
    DamageAudio damage;
    
    // Current state
    float currentRPM;
    float currentThrottle;
    float currentSpeed;
    float currentLoad;
    uint32_t currentGear;
    Vec3 wheelVelocities[4];
    Vec3 wheelPositions[4];
    
    // Previous state for detection
    float previousRPM;
    float previousThrottle;
    float previousSpeed;
    bool wasSkidding[4];
    
    // Audio effects
    struct {
        float interiorFilter;
        float muffleFactor;
        bool enableExterior;
    } environment;
} VehicleAudio;

// Vehicle audio API
VehicleAudio* VehicleAudio_Create(AudioContext* context, MemoryArena* arena);
void VehicleAudio_Destroy(VehicleAudio* vehicle);
void VehicleAudio_Update(VehicleAudio* vehicle, float deltaTime);

// State updates
void VehicleAudio_SetEngineState(VehicleAudio* vehicle, float rpm, float throttle, float load, uint32_t gear);
void VehicleAudio_SetSpeed(VehicleAudio* vehicle, float speed);
void VehicleAudio_SetWheelState(VehicleAudio* vehicle, uint32_t wheelIndex, Vec3 position, Vec3 velocity, bool isSkidding);
void VehicleAudio_SetPosition(VehicleAudio* vehicle, Vec3 position);
void VehicleAudio_SetEnvironment(VehicleAudio* vehicle, bool isInterior);

// Configuration
void VehicleAudio_SetEngineConfig(VehicleAudio* vehicle, const EngineAudio* config);
void VehicleAudio_SetTireConfig(VehicleAudio* vehicle, const TireAudio* config);
void VehicleAudio_SetDamageConfig(VehicleAudio* vehicle, const DamageAudio* config);

// Events
void VehicleAudio_PlayBackfire(VehicleAudio* vehicle, float intensity);
void VehicleAudio_PlayImpact(VehicleAudio* vehicle, Vec3 position, float intensity);
void VehicleAudio_PlayCrash(VehicleAudio* vehicle, Vec3 position, float intensity);
```

### 6. Environmental Audio

```c
// Reverb presets
typedef enum {
    REVERB_OFF,
    REVERB_SMALL_ROOM,
    REVERB_MEDIUM_ROOM,
    REVERB_LARGE_ROOM,
    REVERB_SMALL_HALL,
    REVERB_MEDIUM_HALL,
    REVERB_LARGE_HALL,
    REVERB_PLATE,
    REVERB_CAVE,
    REVERB_OUTDOORS,
    REVERB_UNDERWATER,
    REVERB_COUNT
} ReverbPreset;

// Environmental audio system
typedef struct {
    // Current environment
    ReverbPreset currentPreset;
    float currentWetness;
    float currentEcho;
    float currentDoppler;
    
    // Zone-based audio
    struct {
        Vec3 position;
        float radius;
        ReverbPreset preset;
        float wetness;
        float echo;
    }* zones;
    uint32_t zoneCount;
    
    // Weather effects
    struct {
        bool enabled;
        float rainIntensity;
        float windIntensity;
        float thunderIntensity;
        AudioSource* rainSource;
        AudioSource* windSource;
        AudioSource* thunderSource;
    } weather;
    
    // Obstruction and occlusion
    struct {
        bool enabled;
        float obstructionFactor;
        float occlusionFactor;
        AudioSource* testSource;
    } occlusion;
    
    // Distance modeling
    struct {
        bool enabled;
        float airAbsorption;
        float temperature;
        float humidity;
    } distanceModel;
} EnvironmentAudio;

// Environment audio API
EnvironmentAudio* EnvironmentAudio_Create(AudioContext* context, MemoryArena* arena);
void EnvironmentAudio_Destroy(EnvironmentAudio* env);
void EnvironmentAudio_Update(EnvironmentAudio* env, Vec3 listenerPosition);

// Environment management
void EnvironmentAudio_SetReverbPreset(EnvironmentAudio* env, ReverbPreset preset, float wetness);
void EnvironmentAudio_AddZone(EnvironmentAudio* env, Vec3 position, float radius, ReverbPreset preset, float wetness);
void EnvironmentAudio_RemoveZone(EnvironmentAudio* env, uint32_t zoneIndex);

// Weather effects
void EnvironmentAudio_SetWeather(EnvironmentAudio* env, float rainIntensity, float windIntensity, float thunderIntensity);
void EnvironmentAudio_PlayThunder(EnvironmentAudio* env, Vec3 position, float intensity);

// Occlusion and obstruction
void EnvironmentAudio_SetOcclusion(EnvironmentAudio* env, Vec3 sourcePos, Vec3 listenerPos, float factor);
```

### 7. Audio Effects System

```c
// Effect types
typedef enum {
    EFFECT_REVERB,
    EFFECT_CHORUS,
    EFFECT_DISTORTION,
    EFFECT_LOWPASS,
    EFFECT_HIGHPASS,
    EFFECT_PITCHSHIFT,
    EFFECT_COMPRESSOR,
    EFFECT_DELAY,
    EFFECT_COUNT
} EffectType;

// Effect parameters (unified interface)
typedef struct {
    EffectType type;
    bool enabled;
    float wetDry;
    uint32_t priority;
    
    // Effect-specific parameters
    union {
        struct {
            float roomSize;
            float damping;
            float width;
            float predelay;
        } reverb;
        
        struct {
            float rate;
            float depth;
            float feedback;
            float delay;
        } chorus;
        
        struct {
            float gain;
            float threshold;
            float ratio;
            float attack;
            float release;
        } compressor;
        
        struct {
            float cutoff;
            float resonance;
        } filter;
        
        struct {
            float pitch;
            float fine;
        } pitchShift;
        
        struct {
            float delay;
            float feedback;
            float wetLevel;
        } delay;
    } params;
} AudioEffect;

// Effects chain
typedef struct {
    AudioEffect* effects;
    uint32_t effectCount;
    uint32_t effectCapacity;
    
    // Processing
    float* processingBuffer;
    uint32_t bufferSize;
    
    // Performance
    uint32_t processingTime;
    float cpuUsage;
} EffectsChain;

// Effects API
EffectsChain* EffectsChain_Create(uint32_t maxEffects, uint32_t bufferSize, MemoryArena* arena);
void EffectsChain_Destroy(EffectsChain* chain);
void EffectsChain_Process(EffectsChain* chain, float* input, float* output, uint32_t frameCount);

uint32_t EffectsChain_AddEffect(EffectsChain* chain, const AudioEffect* effect);
void EffectsChain_RemoveEffect(EffectsChain* chain, uint32_t effectIndex);
void EffectsChain_EnableEffect(EffectsChain* chain, uint32_t effectIndex, bool enabled);

// Preset management
void EffectsChain_LoadPreset(EffectsChain* chain, const char* presetName);
void EffectsChain_SavePreset(EffectsChain* chain, const char* presetName);
void EffectsChain_LoadVehiclePreset(EffectsChain* chain, const char* vehicleType);
```

### 8. Audio Configuration

```c
// Audio system configuration
typedef struct {
    AudioAPI preferredAPI;
    uint32_t sampleRate;
    uint32_t bufferSize;
    uint32_t outputChannels;
    
    // Quality settings
    uint32_t qualityLevel;  // 0-4
    bool enableEffects;
    bool enable3DAudio;
    bool enableDoppler;
    float masterVolume;
    float musicVolume;
    float sfxVolume;
    float voiceVolume;
    
    // Performance
    uint32_t maxSources;
    uint32_t maxStreams;
    bool enableLowLatency;
    uint32_t targetLatency;
    
    // Features
    bool enableVehicleAudio;
    bool enableEnvironmentAudio;
    bool enableMusicSystem;
    bool enableVoiceChat;
} AudioConfig;

// Vehicle audio configuration
typedef struct {
    const char* engineSoundPath;
    const char* exhaustSoundPath;
    const char* turboSoundPath;
    const char* tireSoundPath;
    const char* impactSoundPath;
    
    float engineVolume;
    float exhaustVolume;
    float turboVolume;
    float tireVolume;
    float impactVolume;
    
    bool enableInteriorFiltering;
    float interiorCutoff;
    float interiorResonance;
} VehicleAudioConfig;

// Audio system initialization
AudioContext* AudioContext_Initialize(const AudioConfig* config);
void AudioContext_Shutdown(AudioContext* context);

// Configuration loading
AudioConfig* AudioConfig_Load(const char* path, MemoryArena* arena);
void AudioConfig_Save(const AudioConfig* config, const char* path);
VehicleAudioConfig* VehicleAudioConfig_Load(const char* path, MemoryArena* arena);
```

## Implementation Roadmap

### Phase 1: Core Audio System (Week 1-2)
- [ ] Audio context creation
- [ ] Basic sample loading
- [ ] Simple source management
- [ ] Platform-specific backends
- [ ] Error handling

### Phase 2: 3D Audio (Week 3-4)
- [ ] 3D source positioning
- [ ] Listener system
- [ ] Distance attenuation
- [ ] Doppler effects
- [ ] Cone attenuation

### Phase 3: Vehicle Audio (Week 5-6)
- [ ] Engine sound synthesis
- [ ] RPM-based interpolation
- [ ] Tire sounds
- [ ] Impact and damage sounds
- [ ] Vehicle-specific parameters

### Phase 4: Environmental Audio (Week 7-8)
- [ ] Reverb system
- [ ] Zone-based audio
- [ ] Weather effects
- [ ] Occlusion and obstruction
- [ ] Environmental parameters

### Phase 5: Effects and Streaming (Week 9-10)
- [ ] Audio effects chain
- [ ] Streaming audio
- [ ] Music system
- [ ] Voice chat support
- [ ] Performance optimization

## Integration Points

### With Resource System
- Audio sample loading
- Configuration file loading
- Streaming support
- Hot reloading of audio files

### With Entity System
- Entity audio components
- Vehicle entity audio
- Audio trigger components
- 3D positioning integration

### With Physics System
- Physics event sounds
- Collision audio
- Vehicle physics-based audio
- Speed-based audio effects

### With Rendering System
- Audio-visual sync
- Music sync with graphics
- UI audio feedback
- Debug audio visualization

## Performance Considerations

### Real-Time Processing
- Low-latency buffer management
- Efficient audio mixing
- SIMD audio processing
- Frame-based audio updates

### Memory Management
- Arena allocation for frame data
- Audio buffer pooling
- Stream buffer management
- Sample caching strategy

### CPU Optimization
- Multi-threaded audio processing
- Efficient effect chains
- Minimal memory allocations
- Hardware acceleration where available

## Testing Strategy

### Unit Tests
- Audio sample loading
- Effect processing correctness
- Vehicle audio state transitions
- Configuration parsing

### Integration Tests
- Multi-platform audio behavior
- 3D positioning accuracy
- Real-time performance
- Audio-visual synchronization

### Performance Tests
- Latency measurements
- CPU usage validation
- Memory usage benchmarks
- Streaming performance

## Platform-Specific Considerations

### Desktop (Windows/macOS/Linux)
- ASIO/CoreAudio/PulseAudio
- Low-latency audio support
- Hardware audio acceleration
- Multi-channel output

### Mobile (iOS/Android)
- OpenSL ES/CoreAudio
- Battery optimization
- Interruption handling
- Mobile-specific audio features

### Console (Switch/PS5/Xbox)
- Platform audio SDKs
- Hardware acceleration
- Voice chat integration
- Surround sound support

## Configuration Files

### Audio System Config
```json
{
    "audio": {
        "preferredAPI": "OpenAL",
        "sampleRate": 44100,
        "bufferSize": 512,
        "outputChannels": 2,
        "qualityLevel": 3
    },
    "volumes": {
        "master": 1.0,
        "music": 0.8,
        "sfx": 1.0,
        "voice": 0.9
    },
    "features": {
        "enable3D": true,
        "enableDoppler": true,
        "enableEffects": true,
        "enableLowLatency": true
    },
    "performance": {
        "maxSources": 64,
        "maxStreams": 8,
        "targetLatency": 50
    }
}
```

### Vehicle Audio Config
```json
{
    "engine": {
        "idleSound": "engine_idle.wav",
        "lowSound": "engine_low.wav",
        "midSound": "engine_mid.wav",
        "highSound": "engine_high.wav",
        "redlineSound": "engine_redline.wav",
        "rpmThresholds": [800, 2500, 4000, 6000, 7000],
        "basePitch": 1.0,
        "pitchRange": 0.5
    },
    "tires": {
        "rollingSound": "tire_rolling.wav",
        "skidSound": "tire_skid.wav",
        "grassSound": "tire_grass.wav",
        "skidThreshold": 0.7,
        "skidGain": 1.2
    },
    "effects": {
        "turboSound": "turbo_spool.wav",
        "blowoffValveSound": "blowoff_valve.wav",
        "backfireSound": "backfire.wav"
    }
}
```