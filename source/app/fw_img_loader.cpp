#include "fw_img_loader.h"
#include "gui.h"
#include "menu.h"
#include "navigation.h"
#include <mocha/mocha.h>
#include <sysapp/launch.h>
#include <isfshax_cmd.h>
#include <whb/sdcard.h>
#include <nn/act.h>
#include <coreinit/thread.h>

#include <cstring>
#include <sstream>
#include <iomanip>

using namespace std::chrono_literals;

/* from smealum's iosuhax: must be placed at 0x05059938
	0x0000000005059938:  47 78    bx   pc
	...
*/
static const char os_launch_hook[] = {
	0x47, 0x78, 0x00, 0x00, 0xe9, 0x2d, 0x40, 0x0f, 0xe2, 0x4d, 0xd0, 0x08, 0xeb,
	0xff, 0xfd, 0xfd, 0xe3, 0xa0, 0x00, 0x00, 0xeb, 0xff, 0xfe, 0x03, 0xe5, 0x9f,
	0x10, 0x4c, 0xe5, 0x9f, 0x20, 0x4c, 0xe3, 0xa0, 0x30, 0x00, 0xe5, 0x8d, 0x30,
	0x00, 0xe5, 0x8d, 0x30, 0x04, 0xeb, 0xff, 0xfe, 0xf1, 0xe2, 0x8d, 0xd0, 0x08,
	0xe8, 0xbd, 0x80, 0x0f, 0x2f, 0x64, 0x65, 0x76, 0x2f, 0x73, 0x64, 0x63, 0x61,
	0x72, 0x64, 0x30, 0x31, 0x00, 0x2f, 0x76, 0x6f, 0x6c, 0x2f, 0x73, 0x64, 0x63,
	0x61, 0x72, 0x64, 0x00, 0x00, 0x00, 0x2f, 0x76, 0x6f, 0x6c, 0x2f, 0x73, 0x64,
	0x63, 0x61, 0x72, 0x64, 0x00, 0x05, 0x11, 0x60, 0x00, 0x05, 0x0b, 0xe0, 0x00,
	0x05, 0x0b, 0xcf, 0xfc, 0x05, 0x05, 0x99, 0x70, 0x05, 0x05, 0x99, 0x7e,
};

static const char ancast_decrypt_hook[] = {
	0x47, 0x78, 0xbf, 0x00,
	0xe5, 0x9f, 0x70, 0x24, 0xe1, 0xd7, 0x70, 0xb0,
    0xe3, 0x17, 0x00, 0x01, 0x1a, 0x00, 0x00, 0x02,
    0xe2, 0x8d, 0x70, 0x24, 0xe5, 0x8d, 0x70, 0x18,
    0xe1, 0x2f, 0xff, 0x1e, 0xe8, 0xbd, 0x40, 0xf0,
    0xe2, 0x8d, 0xd0, 0x10, 0xe3, 0xa0, 0x00, 0x00,
    0xe1, 0x2f, 0xff, 0x1e, 0x01, 0x00, 0x01, 0xa0,
};

static uint32_t generate_bl_t(uint32_t from, uint32_t to)
{
	int32_t bl_offs = (((int32_t)to - (int32_t)(from)) - 4) / 2;
	uint32_t bl_insn = 0xF000F800 | ((uint32_t)bl_offs & 0x7FF) | ((((uint32_t)bl_offs >> 11) & 0x3FF) << 16);
	return bl_insn;
}

static bool applyPatch(uint32_t addr, const void* data, size_t size, const wchar_t* description) {
    WHBLogFreetypePrint(description);
    WHBLogFreetypeDrawScreen();

    for (size_t i = 0; i < size; i += 4) {
        uint32_t word = 0;
        std::memcpy(&word, (const uint8_t*)data + i, size - i < 4 ? size - i : 4);
        MochaUtilsStatus status = Mocha_IOSUKernelWrite32(addr + i, word);
        if (status != MOCHA_RESULT_SUCCESS) {
            std::wstringstream ss;
            ss << L"Failed to write patch at 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill(L'0') << (addr + i) << L"!";
            setErrorPrompt(ss.str());
            showErrorPrompt(L"OK");
            return false;
        }
    }
    return true;
}


void loadFwImg(const std::string& fwPath, uint32_t command, uint32_t parameter) {
    WHBLogFreetypeStartScreen();

    if (command != 0) {
        WHBLogFreetypePrint(L"Writing automated ISFShax commands...");
        WHBLogFreetypeDrawScreen();

        uint32_t magic[2];
        memcpy(magic, ISFSHAX_CMD_MAGIC, 8);

        Mocha_IOSUKernelWrite32(ISFSHAX_CMD_ADDR + 0, magic[0]);
        Mocha_IOSUKernelWrite32(ISFSHAX_CMD_ADDR + 4, magic[1]);
        Mocha_IOSUKernelWrite32(ISFSHAX_CMD_ADDR + 8, command);
        Mocha_IOSUKernelWrite32(ISFSHAX_CMD_ADDR + 12, parameter);
    }

    WHBLogFreetypePrint(L"Applying patches...");
    WHBLogFreetypeDrawScreen();

    // The IOSU hook mounts /dev/sdcard01 as /vol/sdcard
    // So the installer file path directory needs to be /vol/sdcard
    char fwDir[64];
    std::memset(fwDir, 0, sizeof(fwDir));
    std::strncpy(fwDir, "/vol/sdcard", sizeof(fwDir) - 1);
    if (!applyPatch(0x050663B4, fwDir, sizeof(fwDir), L"Applying fw_path...")) return;

    // Update "fw.img" to "ios.img". Overwrites the start of "htk.bin" but that's fine.
    char iosImgStr[] = "ios.img";
    if (!applyPatch(0x0506231C, iosImgStr, sizeof(iosImgStr), L"Applying ios.img rename...")) return;

    uint64_t p2 = 0x00a4F031FB43193b; // need to align patch for old mocha
    if (!applyPatch(0x050282AC, &p2, 8, L"Applying patch 2 (launch_os_hook bl)...")) return;

    uint32_t p3 = 0xE3A00000;
    if (!applyPatch(0x05052C44, &p3, 4, L"Applying patch 3 (mov r0, #0)...")) return;

    uint32_t p4 = 0xE12FFF1E;
    if (!applyPatch(0x05052C48, &p4, 4, L"Applying patch 4 (bx lr)...")) return;

    uint32_t p5 = 0x20002000;
    if (!applyPatch(0x0500A818, &p5, 4, L"Applying patch 5 (mov r0, #0; mov r0, #0)...")) return;

    if (!applyPatch(0x05059938, os_launch_hook, sizeof(os_launch_hook), L"Applying os_launch_hook...")) return;

    uint32_t ancast_hook_start = (0x05059938 + sizeof(os_launch_hook) + 3) & ~3;
    if (!applyPatch(ancast_hook_start, ancast_decrypt_hook, sizeof(ancast_decrypt_hook), L"Applying ancast_decrypt_hook...")) return;

    uint32_t p8 = generate_bl_t(0x0500A678, ancast_hook_start);
    if (!applyPatch(0x0500A678, &p8, 4, L"Applying patch 8 (generate_bl_t)...")) return;

    uint32_t p9 = 0xe00fbf00;
    if (!applyPatch(0x0500A7C8, &p9, 4, L"Applying patch 9 (Ancast header nop nop)...")) return;

    uint32_t p10 = 0x2302e003;
    if (!applyPatch(0x0500a7f4, &p10, 4, L"Applying patch 10 (movs r3, #2; b #0x500a800)...")) return;

    // Undo ISFShax fallback reload patch
    uint32_t val = 0; 
    MochaUtilsStatus status = Mocha_IOSUKernelRead32(0x0501f578, &val);
    if (status != MOCHA_RESULT_SUCCESS) {
        WHBLogFreetypePrint(L"Warning: Failed to read ISFShax fallback patch!");
        WHBLogFreetypeDrawScreen();
    } else if (val == 0x32044bcc) {
        uint32_t undo_val = 0xe0914bcc;
        if (!applyPatch(0x0501f578, &undo_val, 4, L"Undoing ISFShax fallback reload patch...")) return;
    }

    uint32_t bl_LaunchBootrom = 0xebffffca;
    applyPatch(0x0812a120, &bl_LaunchBootrom, 4, L"Undoing PayloadLoader reboot Patch...");

    WHBLogFreetypeClear();
    WHBLogFreetypePrintf(L"Patches applied. Launching ISFShax Installer (%S)...", toWstring(fwPath).c_str());
    WHBLogFreetypeDraw();
    sleep_for(3s);

    WHBUnmountSdCard();
    Mocha_DeInitLibrary();
    ACPFinalize();
    nn::act::Finalize();
    FSShutdown();
    shutdownInputs();
    shutdownGUI();

    SYSLaunchMenu();
    _Exit(0);
}
