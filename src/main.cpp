#include <exlaunch.hpp>
#include "NonStop/NonStop-patches.h"
#include "Speedboot/BootHooks.h"


extern "C" void exl_main(void *x0, void *x1) {
    /* Setup hooking enviroment. */
    exl::hook::Initialize();
    setupBootHooks();
    NonStopPatches::installNonStopPatches();
}
