#pragma once
#include <cstddef>

// ============================================================
//  BUILD MODE
//  -DKINE_DEBUG | -DKINE_RELEASE | -DKINE_PROFILE
// ============================================================

#if !defined(KINE_DEBUG) && !defined(KINE_RELEASE) && !defined(KINE_PROFILE)
#   define KINE_DEBUG
#endif

#ifdef KINE_RELEASE
#   define KINE_INLINE    __forceinline
#   define KINE_ASSERT(x) ((void)0)
#else
#   include <cassert>
#   define KINE_INLINE    inline
#   define KINE_ASSERT(x) assert(x)
#endif

// ============================================================
//  RUNTIME
// ============================================================

// Fixed timestep — work() recebe dt constante se activado
#ifndef KINE_FIXED_TIMESTEP
#   define KINE_FIXED_TIMESTEP 0
#endif

constexpr bool kFixedTimestep = KINE_FIXED_TIMESTEP;
#undef KINE_FIXED_TIMESTEP

// ============================================================
//  BUILD-SPECIFIC — injectado pelo CMake via [debug] / [release]
// ============================================================

#ifndef KINE_FPS_CAP
#   ifdef KINE_RELEASE
#       define KINE_FPS_CAP 120
#   else
#       define KINE_FPS_CAP 60
#   endif
#endif

#ifndef KINE_POOL_MB
#   ifdef KINE_RELEASE
#       define KINE_POOL_MB 64
#   else
#       define KINE_POOL_MB 128
#   endif
#endif

constexpr int    kFpsCap       = KINE_FPS_CAP;
constexpr size_t kPoolCapacity = (size_t)KINE_POOL_MB * 1024 * 1024;

#undef KINE_FPS_CAP
#undef KINE_POOL_MB

// ============================================================
//  WINDOW
// ============================================================

#ifndef KINE_WINDOW_TITLE
#   define KINE_WINDOW_TITLE "KineFlow"
#endif
#ifndef KINE_WINDOW_WIDTH
#   define KINE_WINDOW_WIDTH 1280
#endif
#ifndef KINE_WINDOW_HEIGHT
#   define KINE_WINDOW_HEIGHT 720
#endif

// "windowed" = 0 | "borderless" = 1 | "fullscreen" = 2
#if defined(KINE_FULLSCREEN)
#   define KINE_WINDOW_MODE 2
#elif defined(KINE_BORDERLESS)
#   define KINE_WINDOW_MODE 1
#else
#   define KINE_WINDOW_MODE 0
#endif

#ifndef KINE_VSYNC
#   define KINE_VSYNC 1
#endif

constexpr const char* kWindowTitle  = KINE_WINDOW_TITLE;
constexpr int         kWindowWidth  = KINE_WINDOW_WIDTH;
constexpr int         kWindowHeight = KINE_WINDOW_HEIGHT;
constexpr int         kWindowMode   = KINE_WINDOW_MODE;
constexpr bool        kVsync        = KINE_VSYNC;

#undef KINE_WINDOW_TITLE
#undef KINE_WINDOW_WIDTH
#undef KINE_WINDOW_HEIGHT
#undef KINE_WINDOW_MODE
#undef KINE_VSYNC

// ============================================================
//  RENDERER
//  -DKINE_RENDERER_OPENGL | _VULKAN | _METAL | _DX11
// ============================================================

#if   defined(KINE_RENDERER_OPENGL)
#   define KINE_RENDERER 0
#elif defined(KINE_RENDERER_VULKAN)
#   define KINE_RENDERER 1
#elif defined(KINE_RENDERER_METAL)
#   define KINE_RENDERER 2
#elif defined(KINE_RENDERER_DX11)
#   define KINE_RENDERER 3
#else
#   define KINE_RENDERER 0    // fallback: OpenGL
#endif

constexpr int kRenderer = KINE_RENDERER;
#undef KINE_RENDERER

// ============================================================
//  THREAD POOL
// ============================================================

#ifndef KINE_THREAD_COUNT
#   define KINE_THREAD_COUNT 0      // 0 = auto (hardware_concurrency)
#endif
#ifndef KINE_MAX_ASYNC_TASKS
#   define KINE_MAX_ASYNC_TASKS 256
#endif
#ifndef KINE_MAX_DAEMON_TASKS
#   define KINE_MAX_DAEMON_TASKS 32
#endif

// Pool de threads pode crescer dinamicamente
#ifdef KINE_THREAD_POOL_GROW
#   define KINE_THREAD_GROW 1
#else
#   define KINE_THREAD_GROW 0
#endif

constexpr size_t kThreadCount    = KINE_THREAD_COUNT;
constexpr size_t kMaxAsyncTasks  = KINE_MAX_ASYNC_TASKS;
constexpr size_t kMaxDaemonTasks = KINE_MAX_DAEMON_TASKS;
constexpr bool   kThreadGrow     = KINE_THREAD_GROW;

#undef KINE_THREAD_COUNT
#undef KINE_MAX_ASYNC_TASKS
#undef KINE_MAX_DAEMON_TASKS
#undef KINE_THREAD_GROW

// ============================================================
//  FEATURES
// ============================================================

#ifdef KINE_AUDIO
#   define KINE_AUDIO_ENABLED 1
#else
#   define KINE_AUDIO_ENABLED 0
#endif

#ifdef KINE_PHYSICS
#   define KINE_PHYSICS_ENABLED 1
#else
#   define KINE_PHYSICS_ENABLED 0
#endif

#ifdef KINE_ECS
#   define KINE_ECS_ENABLED 1
#else
#   define KINE_ECS_ENABLED 0
#endif

#ifdef KINE_NETWORK
#   define KINE_NETWORK_ENABLED 1
#else
#   define KINE_NETWORK_ENABLED 0
#endif

#ifdef KINE_JSON
#   define KINE_JSON_ENABLED 1
#else
#   define KINE_JSON_ENABLED 0
#endif

#ifdef KINE_XML
#   define KINE_XML_ENABLED 1
#else
#   define KINE_XML_ENABLED 0
#endif

#ifdef KINE_SERIALIZE
#   define KINE_SERIALIZE_ENABLED 1
#else
#   define KINE_SERIALIZE_ENABLED 0
#endif

// Inspector (ImGui) — bloqueado silenciosamente em KINE_RELEASE
#if defined(KINE_INSPECTOR) && !defined(KINE_RELEASE)
#   define KINE_INSPECTOR_ENABLED 1
#else
#   define KINE_INSPECTOR_ENABLED 0
#endif

constexpr bool kAudio     = KINE_AUDIO_ENABLED;
constexpr bool kPhysics   = KINE_PHYSICS_ENABLED;
constexpr bool kECS       = KINE_ECS_ENABLED;
constexpr bool kNetwork   = KINE_NETWORK_ENABLED;
constexpr bool kJson      = KINE_JSON_ENABLED;
constexpr bool kXml       = KINE_XML_ENABLED;
constexpr bool kSerialize = KINE_SERIALIZE_ENABLED;
constexpr bool kInspector = KINE_INSPECTOR_ENABLED;

#undef KINE_AUDIO_ENABLED
#undef KINE_PHYSICS_ENABLED
#undef KINE_ECS_ENABLED
#undef KINE_NETWORK_ENABLED
#undef KINE_JSON_ENABLED
#undef KINE_XML_ENABLED
#undef KINE_SERIALIZE_ENABLED
#undef KINE_INSPECTOR_ENABLED

// ============================================================
//  PLUGINS
// ============================================================

#ifdef KINE_PLUGINS
#   define KINE_PLUGINS_ENABLED 1
#else
#   define KINE_PLUGINS_ENABLED 0
#endif

// Permite .dll/.so em envelope assinado
#if KINE_PLUGINS_ENABLED && defined(KINE_PLUGINS_NATIVE)
#   define KINE_PLUGINS_NATIVE_ENABLED 1
#else
#   define KINE_PLUGINS_NATIVE_ENABLED 0
#endif

// Permite bytecode Kalidous via VM
#if KINE_PLUGINS_ENABLED && defined(KINE_PLUGINS_VM)
#   define KINE_PLUGINS_VM_ENABLED 1
#else
#   define KINE_PLUGINS_VM_ENABLED 0
#endif

// Rejeita plugins sem assinatura válida — sempre true em release
#if defined(KINE_PLUGINS_STRICT) || defined(KINE_RELEASE)
#   define KINE_PLUGINS_STRICT_ENABLED 1
#else
#   define KINE_PLUGINS_STRICT_ENABLED 0
#endif

// Sandbox — PluginCtx expõe apenas dt + input
#ifdef KINE_PLUGINS_SANDBOX
#   define KINE_PLUGINS_SANDBOX_ENABLED 1
#else
#   define KINE_PLUGINS_SANDBOX_ENABLED 0
#endif

constexpr bool kPlugins       = KINE_PLUGINS_ENABLED;
constexpr bool kPluginsNative = KINE_PLUGINS_NATIVE_ENABLED;
constexpr bool kPluginsVM     = KINE_PLUGINS_VM_ENABLED;
constexpr bool kPluginsStrict = KINE_PLUGINS_STRICT_ENABLED;
constexpr bool kPluginsSandbox = KINE_PLUGINS_SANDBOX_ENABLED;

#undef KINE_PLUGINS_ENABLED
#undef KINE_PLUGINS_NATIVE_ENABLED
#undef KINE_PLUGINS_VM_ENABLED
#undef KINE_PLUGINS_STRICT_ENABLED
#undef KINE_PLUGINS_SANDBOX_ENABLED