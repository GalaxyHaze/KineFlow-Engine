#pragma once

#include <atomic>
#include <cstddef>
#include <cstdlib>
namespace kine{

    


    template<class Sig>
    using fn = Sig*;
    using u16 = unsigned short;

    struct Slot{
        
        private:
        size_t offset = 0;
        u16 gen = 0, threadId = 0;
    };

    const size_t getThreadId(){
        //0 reserved to an error
        static std::atomic<size_t> counter = 1;
        thread_local static size_t id = counter.fetch_add(1);
        return id;

    }

    class Allocator{
        alignas(void*) std::byte* buffer = nullptr;
        size_t capacity = 0;
        size_t offset = 0;
        //size_t threadId = 0;
        u16 generation = 0;
        u16 scope = 0;

        auto init(Allocator& a){
            a.buffer = (std::byte*)malloc(threadCapacity);
            a.capacity = threadCapacity;
            
        }
        

        static Allocator& getAllocator(){
            thread_local static Allocator a{};
            a.buffer = 
            a.capacity = 
            a.threadId = (size_t)&HACK;
            return a;
        }

        public:


        
    };

    class Scene{

        protected:
        Allocator* allocator = nullptr;

        public:
        Scene* work(){
            return nullptr;
        }

        //Will do an async scene
        template<class... Args>
        void async(Args&&... args);

        //Will execute one scene only one time
        template<class... Args>
        void once(Args&&... args);

        //After scene end, will all append task will be execute
        template<class... Args>
        void append(Args&&... args);

        //Returns a ThreadHandle, if scene ends, thread will be killed
        template<class... Args>
        void* parallel(Args&&... args);

        //fire & forget
        template<class... Args>
        void daemon(Args&&... args);\


    };

}