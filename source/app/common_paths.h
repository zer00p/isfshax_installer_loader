#pragma once
#include <string>

namespace Paths {
    // SD vol Paths
    inline const std::string SdRoot = "/vol/external01";
    inline const std::string SdInstallerImg = SdRoot + "/ios.img";
    inline const std::string SdInstallerImgIos = "/vol/sdcard/ios.img";
    inline const std::string SdSuperblockImg = SdRoot + "/superblock.img";
    inline const std::string SdSuperblockSha = SdRoot + "/superblock.img.sha";
}
