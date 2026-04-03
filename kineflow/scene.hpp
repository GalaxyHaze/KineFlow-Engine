#pragma once

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include "arena.hpp"
#include <tuple>
#include <type_traits>
#include <utility>

namespace kine {

// ═══════════════════════════════════════════════════════════════════════════════
// § 1. CONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════════

    constexpr std::size_t kSceneArenaSize = 4 * 1024 * 1024; // 4 MB
    constexpr std::size_t kTaskArenaSize  = 1 * 1024 * 1024; // 1 MB

    template<class Sig> using fn = Sig*;
    class Scene;
    class Thread;

// ═══════════════════════════════════════════════════════════════════════════════
// § 2. VTABLE
// ═══════════════════════════════════════════════════════════════════════════════

    struct VTable {
        fn<Scene*(void*, void*)> work    = nullptr;  // despacho de work(args...)
        fn<void(void*)>          drop    = nullptr;  // logica do utilizador
        fn<void(void*)>          destroy = nullptr;  // ~Self()

        [[nodiscard]] bool valid() const noexcept { return work && drop && destroy; }
    };

// ═══════════════════════════════════════════════════════════════════════════════
// § 3. CONCEPTS
// ═══════════════════════════════════════════════════════════════════════════════

    template<class T, class... Args>
    concept HasWork = requires(T& t, Args... args) {
        { t.work(args...) } -> std::convertible_to<Scene*>;
    };

    template<class T>
    concept HasDrop = requires(T& t) {
        { t.drop() } -> std::same_as<void>;
    };

    template<class T>
    concept SceneSubclass = std::is_base_of_v<Scene, T>;

// ═══════════════════════════════════════════════════════════════════════════════
// § 4. SCENE
// ═══════════════════════════════════════════════════════════════════════════════

    class Scene {
        Thread* pool = nullptr;

    public:
        Scene()  = default;
        ~Scene();

        Scene(const Scene&)            = delete;
        Scene& operator=(const Scene&) = delete;
        Scene(Scene&&)                 = delete;
        Scene& operator=(Scene&&)      = delete;

        // ── API PUBLICA ──────────────────────────────────────────────────────

        // Prepara a scene com args — retorna this para chaining
        template<class Self, class... Args>
        [[nodiscard]] Scene* enter(Args&&... args)
        requires SceneSubclass<Self>
              && std::is_default_constructible_v<Self>
              && HasWork<Self, Args...>;

        // Executa um tick: work -> drop -> destroy -> reset arenas -> retorna proxima
        [[nodiscard]] Scene* execute();

        // Teardown forcado pelo engine (ex: quit abrupto)
        void shutdown();

        // Allocator exposto ao utilizador — unico modo de criar objectos
        template<class T, class... Args>
        [[nodiscard]] Handle<T> create(Args&&... args)
        {
            assert(data_ && data_->sceneArena && "[Kine] alloc called before enter()");
            return data_->sceneArena->create<T>(std::forward<Args>(args)...);
        }

        // ── PRIMITIVAS DE CONCORRENCIA ───────────────────────────────────────

        template<class... Args> void  append(Args&&...)   { /* TODO */ }
        template<class... Args> void  async(Args&&...)    { /* TODO */ }
        template<class... Args> void* parallel(Args&&...) { return nullptr; }
        template<class... Args> void  daemon(Args&&...)   { /* TODO */ }

    private:

        // ── ESTRUTURA INTERNA ────────────────────────────────────────────────

        struct Pack {
            void*           ptr  = nullptr;  // std::tuple<Args...> na task arena
            fn<void(void*)> drop = nullptr;  // destrutor da tuple
        };

        struct Slice {
            void*       self = nullptr;  // objecto derivado na task arena
            std::size_t size = 0;
        };

        struct Data {
            VTable  vtable;
            Slice   slice;
            Pack    pack;
            Arena* sceneArena = new Arena(kSceneArenaSize);
            Arena* taskArena  = new Arena(kTaskArenaSize);
        };

        Data* data_ = new Data{};

        // ── HELPERS INTERNOS (template) ──────────────────────────────────────

        template<class Self, class... Args>
        void init();

        template<class Self, class... Args>
        static Scene* dispatchWork(void* self, void* pack);

        template<class... Args>
        void packArgs(Args&&... args);

        // Centraliza destroy + reset — chamado por execute(), shutdown(), ~Scene()
        // callDrop: false quando ja foi chamado (evita double-drop)
        void teardown(bool callDrop);
    };

// ═══════════════════════════════════════════════════════════════════════════════
// § 5. TEMPLATE IMPLEMENTATIONS
// ═══════════════════════════════════════════════════════════════════════════════

    template<class Self, class... Args>
    void Scene::init()
    {
        static_assert(std::is_base_of_v<Scene, Self>,
                      "Scene subclass must derive from Scene.");
        static_assert(std::is_default_constructible_v<Self>,
                      "Scene subclass must be default-constructible.");

        if (!data_) {
            std::cerr << "[Kine] Fatal: could not initialize internal 'data'.\n";
            std::exit(EXIT_FAILURE);
        }

        if (!data_->sceneArena || !data_->taskArena) {
            std::cerr << "[Kine] Fatal: could not initialize internal Arena.\n";
            std::exit(EXIT_FAILURE);
        }

        // Objecto derivado vive dentro da task arena
        Self* instance = data_->taskArena->create<Self>();
        assert(instance && "[Kine] Task arena failed to allocate derived object.");

        // Derivado partilha o mesmo Data — assim create() funciona dentro de work()
        instance->data_   = data_;
        data_->slice.self = instance;
        data_->slice.size = sizeof(Self);

        // Preenche a vtable
        data_->vtable.work = &dispatchWork<Self, Args...>;

        data_->vtable.destroy = [](void* self) {
            static_cast<Self*>(self)->~Self();
        };

        if constexpr (HasDrop<Self>) {
            data_->vtable.drop = [](void* self) {
                static_cast<Self*>(self)->drop();
            };
        } else {
            data_->vtable.drop = [](void*) {};
        }

        assert(data_->vtable.valid());
    }

    template<class Self, class... Args>
    Scene* Scene::dispatchWork(void* self, void* pack)
    {
        assert(self && "[Kine] dispatchWork: self is null.");
        assert(pack && "[Kine] dispatchWork: pack is null.");

        auto* instance = static_cast<Self*>(self);
        using Tuple    = std::tuple<std::decay_t<Args>...>;
        auto* args     = static_cast<Tuple*>(pack);

        return std::apply(
            [instance]<typename... T0>(T0&&... a) -> Scene* {
                return instance->work(std::forward<T0>(a)...);
            },
            *args
        );
    }

    template<class... Args>
    void Scene::packArgs(Args&&... args)
    {
        assert(data_ && data_->taskArena);

        // Destruir pack anterior se packArgs for chamado mais de uma vez
        if (data_->pack.ptr && data_->pack.drop) {
            data_->pack.drop(data_->pack.ptr);
            data_->pack.ptr  = nullptr;
            data_->pack.drop = nullptr;
        }

        using Tuple = std::tuple<std::decay_t<Args>...>;

        void* mem = data_->taskArena->allocate(sizeof(Tuple), alignof(Tuple));
        if (!mem) {
            std::cerr << "[Kine] packArgs: task arena OOM.\n";
            return;
        }

        auto* tuple      = new (mem) Tuple(std::forward<Args>(args)...);
        data_->pack.ptr  = tuple;
        data_->pack.drop = [](void* p) {
            static_cast<Tuple*>(p)->~Tuple();
        };
    }

    template<class Self, class... Args>
    Scene* Scene::enter(Args&&... args)
    requires SceneSubclass<Self>
          && std::is_default_constructible_v<Self>
          && HasWork<Self, Args...>
    {
        init<Self, Args...>();
        packArgs(std::forward<Args>(args)...);
        return static_cast<Scene*>(data_->slice.self);
    }

} // namespace kine
