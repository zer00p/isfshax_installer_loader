<div style="text-align: center;">
    <img src="dist/isfshax_installer_loader-banner.png" alt="ISFShax Installer Launcher Banner" style="width: 250px;" />
    <h3>ISFShax Installer Launcher</h3>
    <p>A streamlined utility for the Wii U designed to be launched directly from the Aroma environment. Its sole purpose is to boot the ISFShax installer (<code>ios.img</code>) directly from your SD card</p>
</div>

## Features
- **Automated Installer Control**: Automatically passes commands to the ISFShax installer, saving you from navigating menus with the console buttons.
- **Direct Installer Boot**: Boot the Installer directly from the Aroma environment.

## How to use
1. Download the [latest release from GitHub](https://github.com/zer00p/isfshax_installer_loader/releases).
2. Copy the `isfshax_installer_loader.wuhb` file to `sd:/wiiu/apps/` on your SD card.
3. Ensure the ISFShax installer (`ios.img`) and `superblock.img` / `superblock.img.sha` are present on the root of your SD card.
4. Launch the application from the Wii U Menu via Aroma.

## How to compile
 - Install [DevkitPro](https://devkitpro.org/wiki/Getting_Started) for your platform.
 - Install [wut](https://github.com/devkitpro/wut) through DevkitPro's pacman or compile (and install) the latest source yourself.
 - Compile [libmocha](https://github.com/wiiu-env/libmocha).
 - Ensure [libstroopwafel](https://github.com/StroopwafelCFW/libstroopwafel) is installed.
 - Run `make` to generate the `.wuhb` file.

## Credits
 - [Crementif](https://github.com/Crementif) for the original [dumpling](https://github.com/dumpling-app/dumpling)
 - [emiyl](https://github.com/emiyl) for [dumpling-classic](https://github.com/emiyl/dumpling-classic)
 - [wut](https://github.com/devkitpro/wut) for providing the Wii U toolchain
 - [Dimok](https://github.com/dimok789) for the original fw_img_payload
 - The [wiiu-env](https://github.com/wiiu-env) developers (especially [Maschell](https://github.com/Maschell)) for Aroma
 - [shinyquagsire23](https://github.com/shinyquagsire23) for stroopwafel and fw_img_loader additions
 - [rw_r_r_0644](https://github.com/rw_r_r_0644) for ISFShax
 - [Google Jules](https://jules.google.com)

## License

This project is licensed under the **GNU General Public License v2.0**. See the `LICENSE` file for details.

Some parts of this project were originally licensed under the MIT license. The original MIT license is preserved in the `LICENSE-MIT.md` file.

This project also includes [libschrift](https://github.com/tomolt/libschrift), see its ISC-styled license [here](https://github.com/tomolt/libschrift/blob/master/LICENSE).
