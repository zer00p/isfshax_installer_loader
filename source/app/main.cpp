#include "menu.h"
#include "isfshax_menu.h"
#include "navigation.h"
#include "filesystem.h"
#include "gui.h"
#include <unistd.h>
#include <chrono>
#include <thread>
#include <whb/sdcard.h>
#include <mocha/mocha.h>

using namespace std::chrono_literals;

// Initialize correct heaps for CustomRPXLoader
extern "C" void __init_wut_malloc();
extern "C" [[maybe_unused]] void __preinit_user(MEMHeapHandle *outMem1, MEMHeapHandle *outFG, MEMHeapHandle *outMem2) {
    __init_wut_malloc();
}

int main() {
    // Initialize libraries
    initializeGUI();
    WHBLogCafeInit();
    FSInit();
    FSAInit();
    nn::act::Initialize();
    ACPInitialize();
    initializeInputs();

    // Start ISFShax Installer Launcher
    showLoadingScreen();
    
    // We rely on Mocha functionality being available via Aroma
    Mocha_InitLibrary();
    WHBMountSdCard();

    WHBLogFreetypePrint(L" ");
    WHBLogPrint("Finished loading!");
    WHBLogFreetypeDraw();
    sleep_for(2s);
    
    showMainMenu();

    WHBLogPrint("");
    WHBLogPrint("Exiting ISFShax Installer Launcher...");
    WHBLogFreetypeDraw();
    sleep_for(2s);

    // Close application properly
    WHBUnmountSdCard();
    Mocha_DeInitLibrary();
    ACPFinalize();
    nn::act::Finalize();
    FSShutdown();
    shutdownInputs();
    shutdownGUI();

    exitApplication(false, false);
}