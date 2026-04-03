#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string_view>
#include <cassert>
#include <sys/types.h>

namespace kine {

// ─── Chunk ────────────────────────────────────────────────────────────────────

struct Chunk {
    std::size_t capacity = 0;
    std::size_t offset   = 0;
    std::byte   data[];  // FAM

    [[nodiscard]] static Chunk* create(std::size_t size) noexcept {
        void* mem = std::malloc(sizeof(Chunk) + size);
        if (!mem) return nullptr;
        auto* c = static_cast<Chunk*>(mem);
        c->capacity = size;
        c->offset = 0;
        return c;
    }

    void destroy() noexcept { std::free(this); }
};

// ─── ArenaMarker ──────────────────────────────────────────────────────────────

struct ArenaMarker {       // Index in access[]
    std::size_t   offset;      // Offset inside that block
    std::uint32_t generation;  // Generation at time of save
    std::uint16_t scope;       // Scope depth
    std::int8_t   block;
};

template<class T>
class Handle{
    friend class Arena;
    std::uint32_t offset = 0;
    std::uint32_t scope = 0;
    std::uint32_t generation   = 0;
    std::uint8_t block = 0;

    public:
    Handle(std::nullptr_t null) noexcept: block(-1) {}

    Handle(const uint32_t off, const uint32_t scp, const uint32_t gen, const int8_t blc = 0) noexcept:
    offset(off),
    scope(scp),
    generation(gen),
    block(blc){}

    operator T*() const noexcept{
        return new T();
    }
};

// ─── Arena ────────────────────────────────────────────────────────────────────

class Arena {
    static constexpr std::size_t MAX_BLOCKS    = 32;
    static constexpr std::size_t DEFAULT_BLOCK = 4 * 1024 * 1024; // 4 MB

    Chunk*        access[MAX_BLOCKS] = {}; 
    std::uint32_t generation  = 0;     // Version of the arena (changes on full reset)
    std::uint16_t scope       = 0;     // Current scope depth
    std::int8_t   blocks      = 0;     // Count of active (allocated) chunks

public:

    explicit Arena(const std::size_t size = DEFAULT_BLOCK) {
        Chunk* c = Chunk::create(size);
        if (!c) {
            std::cerr << "[Kine] Fatal: Initial arena allocation failed (" << size << " bytes).\n";
            std::abort();
        }
        access[0] = c;
        blocks    = 1;
    }

    ~Arena() {
        // Free ALL physically allocated chunks
        for (std::size_t i = 0; i < MAX_BLOCKS; ++i) {
            if (access[i]) {
                access[i]->destroy();
                access[i] = nullptr;
            }
        }
    }

    Arena(const Arena&)            = delete;
    Arena& operator=(const Arena&) = delete;
    Arena(Arena&&)                 = delete;
    Arena& operator=(Arena&&)      = delete;

    // ── Markers ───────────────────────────────────────────────────────────────

    [[nodiscard]] ArenaMarker save() noexcept {
        ++scope;
        // Capture the tip of the current allocation
        return { currentOffset(), generation, scope, blocks };
    }

    void rewind(const ArenaMarker& marker) noexcept {
        // Safety: If generation changed, this marker is from a past "life" of the arena.
        // However, if we support "permanent" assets across scopes, we usually don't 
        // increment gen on rewind. 
        
        // Logic: Recycle blocks.
        // 1. Reset blocks that were allocated AFTER the marker.
        for (std::size_t i = marker.block + 1; i < blocks; ++i) {
            access[i]->offset = 0; // Recycle: don't free, just reset cursor
        }

        // 2. Restore the offset of the block where the marker was set.
        access[marker.block]->offset = marker.offset;

        // 3. Update block count.
        // Note: We keep 'blocks' at the current high water mark (or max allocated)
        // because we want to reuse those chunks for future allocations.
        // We do NOT decrement blocks to marker.block + 1.
        // But wait... if we keep blocks high, how does allocate know where to write?
        
        // Correction based on your previous snippet logic:
        // The "active" head is determined by the offset restoration.
        // But if we don't change 'blocks', the allocator needs to know it can 
        // overwrite the "recycled" blocks.
        
        blocks = marker.block + 1; // Set the cursor back to this point.
                                   // But the chunks at i > marker.block are still in access[] memory.
        
        scope = marker.scope - 1;
        
        // DO NOT increment generation.
        // We want pointers to data *before* this marker to remain valid.
    }

    [[nodiscard]] bool isValid(const ArenaMarker& marker) const noexcept {
        return marker.generation == generation;
    }

    // ── Alocação ──────────────────────────────────────────────────────────────

    [[nodiscard]] void* allocate(std::size_t size, 
                                 std::size_t alignment = alignof(std::max_align_t)) noexcept {
        Chunk* current = access[blocks - 1];

        auto try_fit = [&](Chunk* c) -> void* {
            std::uintptr_t base      = reinterpret_cast<std::uintptr_t>(c->data + c->offset);
            std::uintptr_t aligned   = (base + alignment - 1) & ~(alignment - 1);
            std::size_t   padding    = static_cast<std::size_t>(aligned - base);

            if (c->offset + padding + size > c->capacity) return nullptr;
            c->offset += padding + size;
            return reinterpret_cast<void*>(aligned);
        };

        // 1. Try to fit in current block
        if (void* ptr = try_fit(current)) return ptr;

        // 2. Try to reuse a previously allocated block that is "inactive" (recycled)
        //    These are blocks sitting in access[] at index >= blocks
        for (std::size_t i = blocks; i < MAX_BLOCKS; ++i) {
            if (access[i]) {
                // Ensure it's clean
                access[i]->offset = 0; 
                if (void* ptr = try_fit(access[i])) {
                    blocks = i + 1; // Move active head forward
                    return ptr;
                }
            }
        }

        // 3. Need a brand new block
        if (blocks >= MAX_BLOCKS) {
            std::cerr << "[Kine] Fatal: Arena hit 32-block limit.\n";
            std::abort();
        }

        // Growth strategy: Max of current size or request
        const std::size_t new_cap = std::max(current->capacity, size + alignment);
        Chunk* next = Chunk::create(new_cap);
        if (!next) {
            std::cerr << "[Kine] Fatal: Chunk allocation failed.\n";
            std::abort();
        }

        access[blocks++] = next;
        return try_fit(next); 
    }

    template<class T, class... Args>
    [[nodiscard]] Handle<T> create(Args&&... args) noexcept {
        void* mem = allocate(sizeof(T), alignof(T));
        if (!mem) return nullptr;
        new (mem) T(std::forward<Args>(args)...);
        return Handle<T>{currentOffset(), scope, generation, blocks};
    }

    template<class T>
    [[nodiscard]] Handle<T> load(std::string_view path) noexcept {
        T* obj = create<T>();
        if (obj) obj->load_from_file(path);
        return obj;
    }

    // ── Utilitários ───────────────────────────────────────────────────────────

    void reset() noexcept {
        // Hard reset. Wipe everything, but keep allocated chunks (recycle them all).
        for (std::size_t i = 0; i < MAX_BLOCKS && access[i]; ++i) {
            access[i]->offset = 0;
        }
        blocks = 1; // Reset to the first block
        ++generation; // ONLY invalidate pointers here
        scope = 0;
    }

    [[nodiscard]] std::size_t used() const noexcept {
        std::size_t total = 0;
        for (std::size_t i = 0; i < blocks; ++i)
            total += access[i]->offset;
        return total;
    }

    [[nodiscard]] std::size_t capacity() const noexcept {
        std::size_t total = 0;
        for (std::size_t i = 0; i < MAX_BLOCKS && access[i]; ++i)
            total += access[i]->capacity;
        return total;
    }

    [[nodiscard]] std::uint32_t currentGeneration() const noexcept { return generation; }
    [[nodiscard]] std::uint32_t currentOffset()     const noexcept { return access[blocks-1]->offset; }
    [[nodiscard]] std::uint32_t currentScope()      const noexcept { return scope; }
    [[nodiscard]] std::size_t   activeBlocks()      const noexcept { return blocks; }
};

// ─── ArenaScope ───────────────────────────────────────────────────────────────

struct ArenaScope {
    Arena&      arena;
    ArenaMarker marker;

    explicit ArenaScope(Arena& a) : arena(a), marker(a.save()) {}
    ~ArenaScope() { arena.rewind(marker); }

    ArenaScope(const ArenaScope&)            = delete;
    ArenaScope& operator=(const ArenaScope&) = delete;
};



} // namespace Kine