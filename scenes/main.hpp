#include <kineflow/scene.hpp>
#include <iostream>

struct Main: public kine::Scene{

    Scene* work(){
        std::cout << "-Hello world!\n";
        return enter<Main>(5);
    }

    Scene* work(int a){
        std::cout << "-Another Scene!\n";
        return nullptr;
        //return nullptr;
    }

};