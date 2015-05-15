# VBox Systray Launcher
This is a very small python script that acts as a front-end to vboxmanage.  It creates a systray
icon that, when right-clicked, will present a list of virtual machines to run along with an item 
that will let you run the main VirtualBox interface.

Features:
 * Automatically polls vboxmanage for the list of virtual machines ever 10 seconds
 * Uses VirtualBox icon from the current icon theme, so is themeable

Dependencies:
 * python 2.7 (might work with 3, haven't tested)
 * pygtk

This program was inspired by cinnamon-vbox-launcher ([github][launcher_github] / [cinnamon spices][spice]) by [mockturtl][mockturtl_github].  I mainly wanted a version that would
work in Openbox, which is the WM I work with when I'm cheating on Cinnamon.  ;)

[launcher_github]: https://github.com/mockturtl/cinnamon-vbox-launcher
[spice]: http://cinnamon-spices.linuxmint.com/applets/view/138
[mockturtl_github]: https://github.com/mockturtl
