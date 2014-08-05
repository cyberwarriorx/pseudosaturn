Introduction
------------
This code was designed as a proof of concept for running Saturn code from cd's without requiring special mastering hardware. It was the product of several months of studying the process by which the cd block authenticates a disc and unlocks functionality.

The code for booting unmodified Saturn disc images came from jhl's research and algorithm. I can only claim the work of the actual implementation of it in Pseudo Saturn.

Dependencies
------------
Everything was compiled using the kpit elf compiler and the Iapetus library. Iapetus itself is downloaded from github if it isn't already present. You will need to compile some kind of SH2 cross compiler beforehand. Otherwise everything else should be detected using cmake.

Build Instructions
------------------
1. Start up cmake-gui.
2. Set "Where is the source code" to where you downloaded source code to.
3. Set "Where to build the binaries" to another directory, ideally not the same directory as source code. Make a note of directory.
4. Click on "Configure" and answer yes to create directory if it doesn't exist. 
5. For generator you should be using "Unix Makefiles" or something equivalent. Make sure "Specify toolchain file for cross-compiling". Click "Next".
6. For toolchain file go to "Platform" subdirectory in source code directory and select "SegaSaturn".
7. If there's any errors, correct them. Otherwise click "Generate" button.
8. Close cmake.
9. On the command-line, go into binaries directory you made note of above and type: "make".
10. If everything is successful you should get a message "[100%] Built target PseudoSaturn". The final binary is called "PseudoSaturn.BIN" is is located in the binaries directory.

Install Instructions
--------------------
Requirements: Installation requires you to own an Action Replay(ideally EMS) and a method for flashing data to the Action Replay. The default
installer is via cd image, which requires you to burn and boot the disc on the Saturn, which requires a blank cd, and modchip or disc swapping which won't
be covered here. 
WARNING: Flashing data to your Action Replay may possibly fail. This is especially true on Saturn's with old and/or faulty cartridge connectors. If your Saturn has a history of corrupted saves or cheats it's quite possible flashing will fail and your Action Replay will be bricked.

1. Burn installer.iso to a cd
2. Boot Saturn and let CD load
3. Select "Backup AR to Commlink" (Optional)
4. Use a utility designed for your commlink and download memory addresses 0x00200000-0x00240000. When finished go back to menu. (Optional)
5. Select "Inst Firmware from Disc".
6. When ready hit buttons A+B+C.
7. Wait for firmware install to finish. DO NOT TURN OFF SYSTEM. It may take a few minutes.
8. You should see a "SUCCESS" message if all goes well. Press reset to enjoy your new firmware!

Usage Instruction
-----------------
1. Make sure your Action Replay is inserted while the Saturn is off.
2. Turn on Saturn and go through boot process. License screen should pop up, and then a new menu should show.
3. Use d-pad to move selection. By default it should be over "Start Game". Make sure it's selected and press 'A' button.
4. If all goes well, game should be detected as a "Pseudo Saturn" or "Saturn" disc and boot up. Retail discs should also boot fine.

Todo List
---------
There's a few things I really want to add at some point given the time:
-Clean up code and improve build setup.
-Cheats support like stock Action Replay software
-Saves support of some sort like stock action replay. Ideally I'd like to hook into the bios's save functions so it can save directly to the cart.
-More debug functionality for developers.
-A better looking menu

Special Thanks
--------------
Thanks to my buds on #yabause, rhdn, and assemblergames
-Amon
-Bacon
-BlueCrab
-Charles MacDonald
-esperknight
-Guill
-jhl
-pinchy
-SamIAm
-SaturnAR
-tehcloud
-WhiteSnake
-zorlon

The "Screw you" list(by request)
--------------------------------
-KrossX
-vbt