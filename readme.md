# GW2Mount Overlay

A radial GW2-style overlay to select your mounts. Currently, it is compatible with mounts included until *Living World Season 4: Long Live the Lich*.

## Installation
- Download the file ``GW2MountOverlay.zip`` from [the latest release](https://github.com/rsaezc/GW2MountOverlay/releases/latest).
- Copy ``d3d9.dll`` into the bin64 directory of your GW2 installation folder (default path: ``C:\Program Files\Guild Wars 2\bin64``).
- And Done!, run the game and enjoy.

## Usage
- Open the game option window, move to the control options tab and set the in-game keybind for each mount.
- Press ``Shift+Alt+M`` for displaying the overlay configuration window.
- Set a keybind for the configuration window. You can skip this step.
- Set a keybind for the GW2Mount Overlay.
- Set the keybind for each mount. Make sure the overlay mount keybind matches the in-game keybind. ![Keybinds](https://raw.githubusercontent.com/rsaezc/GW2MountOverlay/assets/keybinds.png)
- Set your favorite mount. You can skip this step.
-- The favorite mount is selected when you have not selected any other mount, so you can mount in your favorite mount quickly opening the overlay and clicking anywhere on the screen.
- Close the configuration window and open the GW2Mount overlay pressing the overlay keybind. Then, you should see the radial GW2-style overlay with the default size. You can adjust the overlay size in the configuration window.
- The GW2Mount overlay is ready but What happens if you use the overlay while you character is mounting? Oops! After selecting a mount, your character dismounts and does not mount again. Do you like this behaviour? OK, all done!, enjoy the overlay. No? Don't worry, use the dismount feature.

## Dismount feature
GW2MountOverlay can dismount for you when you open the overlay. In this way, you can change your mount quickly even when your character is mounting.
This feature is based on a very basic image recognition. GW2MountOverlay gets an in-game capture and recognizes the dismount icon in your skill bar. Unfortunately, the size and position of dismount icon changes depending on screen size. Hence, GW2MountOverlay needs your help to work properly.
- Open the overlay configuration window.
- Check the dismount calibration checkbox.
- Close the overlay configuration window and mount (you can use the overlay if you wish).
- Move the mouse cursor to the left-top corner of the dismount icon. Ensure that the dismount icon does not light up. ![Dismount calibration](https://raw.githubusercontent.com/rsaezc/GW2MountOverlay/assets/dismount_calibration.png)
- Open the GW2Mount overlay and close it (you can select a mount or use the ESC key).
- Open the overlay configuration window again and uncheck the dismount calibration checkbox.
- That's all. Open the overlay, select a mount, open the overlay again and your character should dismount.

## Credits
This project is a fork from [GW2Addons](https://github.com/Friendly0Fire/GW2Addons). GW2MountOverlay would not have been possible without the hard work of [Friendly0Fire](https://github.com/Friendly0Fire) and his contributors.
So, I would like to cite the credits from [GW2Addons](https://github.com/Friendly0Fire/GW2Addons) here:
>- Cerulean Dream for providing me with the initial motivator and inspiration in making his AutoHotkey-based radial menu
>- Ghost for the new mount art found in v0.3+
>- Lavender for showing me where to get the mount concept art (especially the separate Skimmer image)
>- deltaconnected and Bhagawan for their amazing addons which helped frame this and direct the approach to take, including the ReShade compatibility fixes
>- Shaun Lebron for his nice, straightforward [D3D9 wrapper](https://gist.github.com/shaunlebron/3854bf4eec5bec297907)
>- Dulfy and /u/Levi4than for the Griffon ingame art screenshot
>- /u/that_shaman for another really nice radial menu concept which requires far better Photoshop skills than I have to reproduce

## FAQ

### Q: Can I use this code for my project?

A: This repository is MIT licensed, so you can use it as you wish. However, I would appreciate a small credit :).

### Q: I want to load up ArcDPS/GW2Hook/something else which also needs to be called ``d3d9.dll``, how do I load both?

A: You have two options: either the other thing you want to run supports *chainloading* this, in which case you should look up the documentation for that plugin (e.g. ArcDPS supports chainloading by renaming this plugin to ``d3d9_chainload.dll``), or you can make this plugin chainload something else by renaming that other plugin to ``d3d9_mchain.dll``.

The most common use case would be combining ArcDPS, GW2Hook and this. For this instance, I heavily recommend setting things up as follows:
- ArcDPS is named ``d3d9.dll``.
- GW2Mounts is named ``d3d9_chainload.dll``.
- GW2Hook is named ``ReShade64.dll``.

*N.B. If your Windows options hide file extensions (which is the default, you can confirm by looking at whether the game's file name is "Gw2-64.exe" or just "Gw2-64") leave out the ".dll" part of the file names (i.e. use "d3d9", "d3d9_chainload" and "ReShade64" respectively).

This should allow all addons to load properly. Note that there is special code present within GW2MountOverlay to load GW2Hook properly, but this could break unexpectedly if GW2Hook changes.

Finally, note that combining addons is largely unsupported.

### Q: I'm having a crash on launch mentioning "Coherent DLL", what do?

A: There seems to be a lot of potential reasons for this particular crash. [BGDM's website](http://gw2bgdm.blogspot.com/p/faq.html#2.5) lists quite a few. I'd especially recommend making sure you have the very latest [VC++ Redist](https://go.microsoft.com/fwlink/?LinkId=746572).

### Q: I'm having problems with the overlay, Can I contact you?

A: Please make an issue in the Github page, I'll try to help you as soon as possible.