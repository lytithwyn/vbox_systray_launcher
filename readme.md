# VBox Systray Launcher
This is an application that will act as a front-end to vboxmanage.  It creates a systray icon that, when right-clicked, will present a list of virtual machines to run, along with an item that will let you run the main VirtualBox interface.

I had originally written this in Python in a single thread (so the list hung while rebuilding... a lot).  I decided to transition to c++/wxwidgets since it had a threading model I already knew for separating GUI & background tasks; also I hope for better performance and more interesting bugs.

Features:
 * Automatically polls vboxmanage for the list of virtual machines every 5 seconds
 * Uses VirtualBox icon from the current icon theme, so is themeable

Dependencies:
 * linux (BSDs will probably work if you tweak the Makefile)
 * a WM/DE that supports the FreeDesktop System Tray Protocol
 * wxwidgets 3 development files
 * gcc? other compilers might work

This program was inspired by cinnamon-vbox-launcher ([github][launcher_github] / [cinnamon spices][spice]) by [mockturtl][mockturtl_github].  I mainly wanted a version that would work in (Flux/Open)box, which are the WM's I work with when I'm cheating on Cinnamon.  ;)

[launcher_github]: https://github.com/mockturtl/cinnamon-vbox-launcher
[spice]: http://cinnamon-spices.linuxmint.com/applets/view/138
[mockturtl_github]: https://github.com/mockturtl
