//#include <iostream>
#include <kineflow/scene.hpp>
#include "scenes/main.hpp"

#include <bits/atomic_base.h>

using namespace kine;

int main() {
    
    Scene entry{};
    auto i = entry.enter<Main>(5);
    while(i){
        i = i->execute();
        //sleep(1);
        //i = entry.enter<Main>();
        if (!i)
            std::cout << "-Null\n";
            //break;
    }
}
