#include "scene.hpp"

namespace kine {

// ═══════════════════════════════════════════════════════════════════════════════
// SCENE
// ═══════════════════════════════════════════════════════════════════════════════

    void Scene::teardown(const bool callDrop)
    {
        if (!data_) return;

        // 1. Logica do utilizador (save, log)
        if (callDrop && data_->slice.self && data_->vtable.drop)
            data_->vtable.drop(data_->slice.self);

        // NOTA: pack, destroy e slice.self NAO sao tocados aqui.
        // Todas as cenas na cadeia partilham o mesmo Data*.
        // execute() verifica slice.self e pack.ptr como guard —
        // anular qualquer um interrompe a cadeia.
        // A arena e bump-allocator: reset() reclama sem invalidar
        // a memoria, logo dangling pointers nao sao um problema.

        // 2. Resetar arenas (memoria reclamada, buffers mantem-se para reutilizacao)
        if (data_->taskArena)  data_->taskArena->reset();
        if (data_->sceneArena) data_->sceneArena->reset();
    }

    Scene::~Scene()
    {
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
        if (!data_ || !data_->vtable.work)                return nullptr;
        if (!data_->slice.self || !data_->pack.ptr)       return nullptr;

        // 1. Executar work — obter proxima scene
        Scene* next = data_->vtable.work(data_->slice.self, data_->pack.ptr);

        teardown(true);

        return next;
    }

    void Scene::shutdown()
    {
        teardown(true);
    }

} // namespace kine
