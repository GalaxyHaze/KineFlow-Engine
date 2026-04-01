#include <print>
#include <kineflow/scene.hpp>
#include "scenes/main.hpp"

using namespace kine;

int main() {
    
    Scene entry{};
    auto i = entry.enter<Main>(0);
    while(i){
        i = entry.execute();
        std::cout << "-Entry add: " << &entry;
        std::cout << "-\nScene add: " << i << '\n';
    }
}
