#include <stdio.h>
#include <stdlib.h>
#if defined(_WIN32) && defined(_DEBUG)
#include <crtdbg.h>
#endif // _WIN32 && _DEBUG

#include <cstdlib>

#include "vengine.h"
#include "src/Scenes/MainMenu.h"
#include "src/Scenes/GameScene.h"
#include "src/Scenes/CreateAPuzzle.h"
#include "src/Scenes/NetworkAI.h"

#include <fstream>

int main(int argc, char* argv[])
{
    // Set flags for tracking CPU memory leaks
#if defined(_WIN32) && defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _WIN32 && _DEBUG

    srand((unsigned int)time(0));
    {
        Engine engine;
        //engine.run("Presumed Dead", "scripts/MainMenu.lua", new MainMenu());
        engine.run("Presumed Dead", "scripts/gamescene.lua", new NetworkAI());
    }

    return EXIT_SUCCESS;
}
