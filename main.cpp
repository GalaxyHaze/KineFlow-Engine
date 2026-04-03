//#include <iostream>
#include <kineflow/scene.hpp>
#include <unistd.h>
//#include "scenes/main.hpp"
#include "scenes/sample.hpp"

using namespace kine;

int main() {
    
    Scene entry{};
    auto i = entry.enter<Main>();
    while(i){
        i = i->execute();
        sleep(1);
        //i = entry.enter<Main>();
        if (!i)
            std::cout << "-Null\n";
            //break;
    }
}
