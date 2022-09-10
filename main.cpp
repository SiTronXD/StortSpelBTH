#include <cstdlib>



#include "tracy/Tracy.hpp"
#include "Engine.h"
#include "TestScene.h"

int main(int argc, char* argv[])
{
    {
        Engine engine;                
        engine.run(new TestScene(engine.sceneHandler)); 
    }

    return EXIT_SUCCESS;
}
