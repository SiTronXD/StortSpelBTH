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
#include "src/Scenes/LoadingScene.h"
#include "src/Network/NetworkHandlerGame.h"

#include <fstream>

#include "src/Scenes/RoomTesting.h"

int main(int argc, char* argv[])
{
    // Set flags for tracking CPU memory leaks
#if defined(_WIN32) && defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _WIN32 && _DEBUG

#ifdef _CONSOLE 
    uint32_t seed = (unsigned int)time(0);
    if(argc > 1)
    {
        seed = (uint32_t)std::stoi(argv[1]);
    }
    Log::write("Seed was: " + std::to_string(seed));
    srand(seed);
#else
    srand((unsigned int)time(0));
#endif

    {
        Engine engine;
        engine.setCustomNetworkHandler(new NetworkHandlerGame());
        //engine.run("Presumed Dead", "", new LoadingScene());
        //engine.run("Presumed Dead", "", new LobbyScene());
        //engine.run("Presumed Dead", "scripts/gamescene.lua", new GameScene());
        //engine.run("Presumed Dead", "scripts/GameOverScene.lua", new GameOverScene());
        engine.run("Presumed Dead", "scripts/gamescene.lua", new GameScene());
        //engine.run("Presumed Dead", "", new RoomTesting());
        //engine.run("Presumed Dead", "scripts/MainMenu.lua", new MainMenu());
    }

    return EXIT_SUCCESS;
}
