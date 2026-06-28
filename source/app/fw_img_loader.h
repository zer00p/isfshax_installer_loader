#pragma once

#include <stdint.h>
#include <string_view>
#include "common_paths.h"

void loadFwImg(const std::string& fwPath = Paths::SdInstallerImg, uint32_t command = 0, uint32_t parameter = 0);
