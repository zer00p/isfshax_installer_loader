#include "isfshax_menu.h"
#include "menu.h"
#include "gui.h"
#include "fw_img_loader.h"
#include "filesystem.h"
#include "common_paths.h"
#include "common.h"
#include "navigation.h"
#include <mbedtls/sha1.h>
#include <isfshax_cmd.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>

using namespace std::chrono_literals;

#define OPTION(n) (selectedOption == (n) ? L'>' : L' ')

bool confirmIsfshaxAction(const wchar_t* action, bool isUninstall = false) {
    std::wstring message = L"";
    if (isUninstall) {
        message += L"WARNING: Before Uninstalling ISFShax make sure the console\n"
                   L"boots correctly using the 'Patch ISFShax and boot IOS (slc)'\n"
                   L"option in minute. If your console can't boot correctly,\n"
                   L"uninstalling ISFShax will BRICK the console!!!\n \n";
    }

    message += L"WARNING: You are about to make modifications to the console.\n"
               L"This software comes with ABSOLUTELY NO WARRANTY!\n"
               L"You are choosing to use this at your own risk.\n"
               L"The author(s) will not be held liable for any damage.\n \n";

    message += L"Do you want to proceed with ";
    message += action;
    message += L"?";

    return showDialogPrompt(message.c_str(), L"Yes", L"No", nullptr, nullptr, 1) == 0;
}

std::vector<unsigned char> calculateSHA1(const std::string& path) {
    FILE* file = fileFopen(path.c_str(), "rb");
    if (!file) {
        return {};
    }

    mbedtls_sha1_context ctx;
    mbedtls_sha1_init(&ctx);
    mbedtls_sha1_starts(&ctx);

    unsigned char buffer[4096];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        mbedtls_sha1_update(&ctx, buffer, bytesRead);
    }

    unsigned char hash[20];
    mbedtls_sha1_finish(&ctx, hash);
    mbedtls_sha1_free(&ctx);
    fclose(file);

    return std::vector<unsigned char>(hash, hash + 20);
}

bool verifySuperblock() {
    std::wstring sblockFilename = toWstring(std::filesystem::path(Paths::SdSuperblockImg).filename().string());
    std::wstring shaFilename = toWstring(std::filesystem::path(Paths::SdSuperblockSha).filename().string());

    if (!fileExist(Paths::SdSuperblockImg)) {
        setErrorPrompt((L"Superblock image (" + sblockFilename + L") is missing from SD!").c_str());
        showErrorPrompt(L"OK");
        return false;
    }
    if (!fileExist(Paths::SdSuperblockSha)) {
        setErrorPrompt((L"Superblock SHA file (" + shaFilename + L") is missing from SD!").c_str());
        showErrorPrompt(L"OK");
        return false;
    }

    std::vector<unsigned char> expectedSha;
    FILE* shaFile = fileFopen(Paths::SdSuperblockSha.c_str(), "rb");
    if (shaFile) {
        unsigned char hashBuf[20];
        if (fread(hashBuf, 1, 20, shaFile) == 20) {
            expectedSha.assign(hashBuf, hashBuf + 20);
        }
        fclose(shaFile);
    }

    if (expectedSha.empty()) {
        setErrorPrompt((L"Failed to read SHA-1 hash from " + shaFilename + L"!").c_str());
        showErrorPrompt(L"OK");
        return false;
    }

    std::vector<unsigned char> actualSha = calculateSHA1(Paths::SdSuperblockImg);
    if (actualSha.empty()) {
        setErrorPrompt((L"Failed to calculate SHA-1 of " + sblockFilename + L"!").c_str());
        showErrorPrompt(L"OK");
        return false;
    }

    if (actualSha != expectedSha) {
        auto toHex = [](const std::vector<unsigned char>& v) {
            std::wstringstream wss;
            for (size_t i = 0; i < std::min(v.size(), (size_t)8); i++) {
                wss << std::hex << std::setw(2) << std::setfill(L'0') << (int)v[i];
            }
            return wss.str();
        };
        setErrorPrompt(L"Superblock hash mismatch!\nExpected: " + toHex(expectedSha) + L"...\nActual: " + toHex(actualSha) + L"...");
        showErrorPrompt(L"OK");
        return false;
    }

    return true;
}

void installIsfshax(bool uninstall, bool manual) {
    if (!fileExist(Paths::SdInstallerImg)) {
        setErrorPrompt(L"The ISFShax installer (ios.img) is missing from the SD card root!");
        showErrorPrompt(L"OK");
        return;
    }

    if (manual) {
        while (true) {
            sleep_for(1s);
            uint8_t choice = showDialogPrompt(L"The ISFShax installer is controlled with the buttons on the main console.\nPOWER: moves the curser\nEJECT: confirm\nPress A to launch into the ISFShax Installer", L"Continue", L"Cancel");
            if (choice == 0) {
                if (verifySuperblock()) {
                    loadFwImg(Paths::SdInstallerImgIos, 0, 0);
                    break;
                }
            } else {
                return;
            }
        }
        return;
    }

    if(uninstall){
        if (confirmIsfshaxAction(L"Uninstall", true)) {
            loadFwImg(Paths::SdInstallerImgIos, ISFSHAX_CMD_UNINSTALL, (uint32_t)(ISFSHAX_CMD_POST_REBOOT) << 30 | ISFSHAX_CMD_SOURCE_SD);
        }
        return;
    }

    if (confirmIsfshaxAction(L"Install")) {
        if (verifySuperblock()) {
            loadFwImg(Paths::SdInstallerImgIos, ISFSHAX_CMD_INSTALL, (uint32_t)(ISFSHAX_CMD_POST_REBOOT) << 30 | ISFSHAX_CMD_SOURCE_SD);
        }
    }
}

void showMainMenu() {
    uint8_t selectedOption = 0;
    while(true) {
        bool startSelectedOption = false;
        while(!startSelectedOption) {
            // Print menu text
            WHBLogFreetypeStartScreen();
            WHBLogFreetypePrint(L"ISFShax Installer Launcher");
            WHBLogFreetypePrint(L"===============================");
            WHBLogFreetypePrintf(L"%C Install ISFShax", OPTION(0));
            WHBLogFreetypePrintf(L"%C Uninstall ISFShax", OPTION(1));
            WHBLogFreetypePrintf(L"%C Boot ISFShax Installer (Manual)", OPTION(2));
            WHBLogFreetypePrint(L" ");
            WHBLogFreetypeScreenPrintBottom(L"===============================");
            WHBLogFreetypeScreenPrintBottom(L"\uE000 Button = Select Option \uE001 Button = Exit");
            WHBLogFreetypeScreenPrintBottom(L"");
            WHBLogFreetypeDrawScreen();

            // Loop until there's new input
            updateInputs();
            while(!startSelectedOption) {
                updateInputs();
                // Check each button state
                if (navigatedUp()) {
                    if (selectedOption > 0) {
                        selectedOption--;
                        break;
                    }
                }
                if (navigatedDown()) {
                    if (selectedOption < 2) {
                        selectedOption++;
                        break;
                    }
                }
                if (pressedOk()) {
                    startSelectedOption = true;
                    break;
                }
                if (pressedBack()) {
                    WHBLogFreetypeClear();
                    return;
                }
            }
        }

        // Go to the selected menu
        switch(selectedOption) {
            case 0:
                installIsfshax(false, false);
                break;
            case 1:
                installIsfshax(true, false);
                break;
            case 2:
                installIsfshax(false, true);
                break;
            default:
                break;
        }
    }
}
