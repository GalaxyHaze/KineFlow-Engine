// scene.cpp
#include "scene.hpp"

namespace kine {

// ═══════════════════════════════════════════════════════════════════════════════
// ARENA
// ═══════════════════════════════════════════════════════════════════════════════

    Arena::Arena(std::size_t size) : capacity_(size)
    {
        buffer_ = static_cast<std::byte*>(std::malloc(size));
        if (!buffer_) {
            std::cerr << "[Kine] Fatal: arena allocation failed (" << size << " bytes).\n";
            std::abort();
        }
    }

    Arena::~Arena() { std::free(buffer_); }

    void* Arena::allocate(std::size_t size, std::size_t alignment)
    {
        const std::size_t current = reinterpret_cast<std::size_t>(buffer_) + offset_;
        const std::size_t aligned = (current + alignment - 1) & ~(alignment - 1);
        const std::size_t padding = aligned - current;

        if (offset_ + padding + size > capacity_) {
            std::cerr << "[Kine] Arena OOM — requested " << size
                      << " bytes, " << (capacity_ - offset_) << " available.\n";
            return nullptr;
        }

        offset_ += padding + size;
        return reinterpret_cast<void*>(aligned);
    }

    void        Arena::reset()    noexcept { offset_ = 0; }
    std::size_t Arena::used()     const noexcept { return offset_; }
    std::size_t Arena::free()     const noexcept { return capacity_ - offset_; }
    std::size_t Arena::capacity() const noexcept { return capacity_; }

// ═══════════════════════════════════════════════════════════════════════════════
// SCENE
// ═══════════════════════════════════════════════════════════════════════════════

    // Ponto central de destruição — todos os caminhos passam aqui
    void Scene::teardown(bool callDrop)
    {
        if (!data_) return;

        // 1. Lógica do utilizador (save, log) — apenas se pedido e se ainda vivo
        if (callDrop && data_->slice.self && data_->vtable.drop)
            data_->vtable.drop(data_->slice.self);

        // 2. Destruir o pack de argumentos
        if (data_->pack.ptr && data_->pack.drop) {
            data_->pack.drop(data_->pack.ptr);
            data_->pack.ptr  = nullptr;
            data_->pack.drop = nullptr;
        }

        // 3. Chamar ~Self() — FIX: garantido mesmo que execute() nunca tenha sido chamado
        if (data_->slice.self && data_->vtable.destroy) {
            data_->vtable.destroy(data_->slice.self);
            data_->slice.self = nullptr;  // guarda contra double-destroy
        }

        // 4. Resetar arenas (memória reclamada, mas buffers mantêm-se para reutilização)
        if (data_->taskArena)  data_->taskArena->reset();
        if (data_->sceneArena) data_->sceneArena->reset();
    }

    Scene::~Scene()
    {
        // FIX: teardown garante ~Self() mesmo que execute() nunca tenha sido chamado
        teardown(true);

        if (data_) {
            delete data_->taskArena;
            delete data_->sceneArena;
            delete data_;
            data_ = nullptr;
        }
    }

    Scene* Scene::execute()
    {
        if (!data_ || !data_->vtable.work)    return nullptr;
        if (!data_->slice.self || !data_->pack.ptr) return nullptr;

        // 1. Executar work — obter próxima scene
        Scene* next = data_->vtable.work(data_->slice.self, data_->pack.ptr);

        // 2. Teardown: drop → destroy pack → ~Self() → reset arenas
        teardown(true);

        return next;
    }

    void Scene::shutdown()
    {
        // Teardown forçado — engine a terminar ou quit abrupto
        teardown(true);
    }

} // namespace kine