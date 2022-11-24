#include <stdio.h>
#include <stdlib.h>
#if defined(_WIN32) && defined(_DEBUG)
#include <crtdbg.h>
#endif // _WIN32 && _DEBUG

#include <cstdlib>

#include "vengine.h"
#include "src/Scenes/MainMenu.h"
#include "src/Scenes/GameScene.h"
//#include "src/Scenes/CreateAPuzzle.h"
#include "src/Scenes/NetworkAI.h"

#include <fstream>

#include "src/Scenes/RoomTesting.h"

int main(int argc, char* argv[])
{
    // Set flags for tracking CPU memory leaks
#if defined(_WIN32) && defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _WIN32 && _DEBUG
    
    //int seed = (unsigned int)time(0); 
    int seed = 1669306701; 

    // SUSS SEED: 1669306701

    Log::write("SEED: " + std::to_string(seed));
    srand(seed);
    {
        Engine engine;
        //engine.run("Presumed Dead", "", new RoomTesting());
        //engine.run("Presumed Dead", "scripts/MainMenu.lua", new MainMenu());
        engine.run("Presumed Dead", "scripts/gamescene.lua", new GameScene());
        //engine.run("Presumed Dead", "", new RoomTesting());
    }

    return EXIT_SUCCESS;
}
