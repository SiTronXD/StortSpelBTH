#include <stdio.h>
#include <stdlib.h>
#if defined(_WIN32) && defined(_DEBUG)
#include <crtdbg.h>
#endif // _WIN32 && _DEBUG

#include <cstdlib>

#include "vengine.h"
#include "src/Scenes/MainMenu.h"
#include "src/Scenes/GameScene.h"
#include "src/Scenes/GameOverScene.h"

#ifdef WIN32
#include "src/Scenes/LevelEditor.h"
#endif

#include "src/Scenes/LobbyScene.h"
#include "src/Network/NetworkHandlerGame.h"

#include <fstream>

#include "src/Scenes/RoomTesting.h"

int main(int argc, char* argv[])
{
    // Set flags for tracking CPU memory leaks
#if defined(_WIN32) && defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _WIN32 && _DEBUG

    uint32_t seed = (unsigned int)time(0);
#ifdef _CONSOLE 
    if(argc > 1)
    {
        seed = (uint32_t)std::stoi(argv[1]);
    }
    Log::write("Seed was: " + std::to_string(seed));
#endif

    srand(seed);
    {
        
        Engine engine;
        engine.setCustomNetworkHandler(new(__FILE__, __LINE__) NetworkHandlerGame());
        engine.run("Presumed Dead", "", new(__FILE__, __LINE__) LobbyScene());
        //engine.run("Presumed Dead", "", new RoomTesting());
        //engine.run("Presumed Dead", "scripts/MainMenu.lua", new MainMenu());
        //engine.run("Presumed Dead", "scripts/gamescene.lua", new GameScene());
    }
    reportMemoryLeaks();
    return EXIT_SUCCESS;
}
