# Star Fox Adventures HD Texture Pack for Foxhollow

HD world, character, boss, object, sky and effect textures for Star Fox Adventures.
In-world item textures remain in this pack. HD text, HUD and inventory/menu item icons
are available separately in **Star Fox Adventures HD Text Pack**.

## Installation

Install the `.fox` bundle through Foxhollow, or extract the release ZIP into its mods folder.
Enable **Star Fox Adventures HD Texture Pack**. This pack is texture-only and requires no native library.
Use it alone or alongside the HD Text Pack to restore the complete combined pack's coverage.
Replace the previous combined pack when upgrading; do not retain another copy of it.
All original regional/revision texture mappings are preserved.

## Packaging

Run `cmake -S . -B build`, then `cmake --build build --target package`.
The `build/dist` directory contains a ZIP and `.fox` bundle usable on all supported platforms.
`pack-split.json` records each original texture's destination and SHA-256 checksum.

## Compatibility

Supports USA 1.0/1.1 and Europe (PAL) 1.0/1.1, preserving the original combined pack's mappings.
Japan is not supported. All 1,785 original texture files are preserved byte-for-byte across the two packs,
and no replacement filename is shared between the packs.
The HD Text Pack retains the original dynamic Latin GameText replacement module.
Unavailable HD glyphs retain their original artwork.

The native Text Pack requires Foxhollow mod ABI 2 and the `gameTextFinalizeLoad` and `selectTexture` exports.
The texture-only pack has no native ABI dependency. Neither manifest imposes a Foxhollow release-version limit.
The workflows package Windows x64, Linux x64 and macOS arm64; the world texture bundle is platform-independent.

## Credits

### Original HD Texture Pack

The majority of the HD texture artwork in this project originates from the original Star Fox Adventures HD Texture Pack for Dolphin by **CYB3RTR0N**.

[Star Fox Adventures HD Texture Pack ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Å“ By CYB3RTR0N](https://forums.dolphin-emu.org/Thread-star-fox-adventures-hd-texture-pack)

### HD Fonts

The HD font textures originate from the Star Fox Adventures HD Font pack by **Calinou**.

[Star Fox Adventures HD Font ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Å“ By Calinou](https://github.com/Calinou/media/releases/download/download/Star.Fox.Adventures.HD.Font.zip)

Please retain the original artists' credits when redistributing or modifying this project.



## Disclaimer

This is an unofficial fan project and is not affiliated with or endorsed by Nintendo, Rare, Microsoft, or the developers of Foxhollow.

Star Fox and Star Fox Adventures are properties of their respective rights holders.
