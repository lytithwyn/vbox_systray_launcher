#!/usr/bin/python
# Virtualbox Systray Launcher
# Copyright 20105 Matthew Morgan <matthew@lifandi.org>
# to show a list of my virtualbox vm's and let me
# run them easily
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.


import gtk
import glib
import subprocess
import re

class VboxSystrayLauncher:
    def __init__(self):
        self.vm_list = {}
        self.menu = None
        self.tray = gtk.StatusIcon()
        self.tray.set_from_icon_name('virtualbox')
        self.tray.connect('button_press_event', self.handle_button_press)
        self.tray.set_tooltip('Virtualbox Systray Launcher')

        self.menu = gtk.Menu()

        # create static menu items and add them to a list
        # we can reference when [re]loading dynamic items
        self.static_menuitems = []

        separator_menuitem = gtk.SeparatorMenuItem()
        self.menu.append(separator_menuitem)
        self.static_menuitems.append(separator_menuitem)

        launch_vbox_menuitem = gtk.MenuItem('Launch Virtualbox')
        launch_vbox_menuitem.connect('activate', self.launch_vbox)
        self.menu.append(launch_vbox_menuitem)
        self.static_menuitems.append(launch_vbox_menuitem)

        about_menuitem = gtk.MenuItem('About')
        about_menuitem.connect('activate', self.show_about_dialog)
        self.menu.append(about_menuitem)
        self.static_menuitems.append(about_menuitem)

        quit_menuitem = gtk.MenuItem('Quit')
        quit_menuitem.connect('activate', gtk.main_quit)
        self.menu.append(quit_menuitem)
        self.static_menuitems.append(quit_menuitem)

        self.menu.show_all()

        # kick off the dynamic part of the menu
        self.reload_dynamic_items()

        # set a timer to periodically reload the list
        glib.timeout_add(10000, self.reload_dynamic_items)

    def reload_dynamic_items(self):
        self.update_vm_list()
        self.rebuild_menu()
        return True

    def rebuild_menu(self):
        current_menuitems = self.menu.get_children()
        for menuitem in current_menuitems:
            if not menuitem in self.static_menuitems:
                self.menu.remove(menuitem)

        item_index = 0
        for vm_guid, vm_name in self.vm_list.iteritems():
            new_menuitem = gtk.MenuItem(vm_name)
            new_menuitem.show()
            new_menuitem.connect('activate', self.run_vm, vm_guid)
            self.menu.insert(new_menuitem, item_index)
            item_index += 1

        self.menu.show_all()

    def update_vm_list(self):
        try :
            self.vm_list = {}
            vm_text_list = subprocess.check_output(["vboxmanage", "list", "vms"])
            vm_lines_list = vm_text_list.splitlines()
            vm_regex = re.compile('"(.*)" ({.*})')
            for line in vm_lines_list:
                vm_re_match = vm_regex.search(line)
                if vm_re_match:
                    self.vm_list[vm_re_match.group(2)] = vm_re_match.group(1)
        except Exception, e:
            print "Got exception: %s" % e

    def handle_button_press(self, icon, event):
        self.show_menu(icon, event.button, event.time)

    def show_menu(self, icon, event_button, event_time):
        self.menu.popup(None, None, gtk.status_icon_position_menu,
            event_button, event_time, self.tray)

    def show_about_dialog(self, widget):
        about_dialog = gtk.AboutDialog()
        about_dialog.set_destroy_with_parent (True)
        about_dialog.set_icon_name ("Virtualbox Systray Launcher")
        about_dialog.set_name('Virtualbox Systray Launcher')
        about_dialog.set_version('1.0')
        about_dialog.set_copyright("(C) 2015 Matthew Morgan")
        about_dialog.set_comments("Give a list of currently registered VM's for easy starting")
        about_dialog.set_authors(['Matthew Morgan <matthew@lifandi.org>'])
        about_dialog.run()
        about_dialog.destroy()

    def launch_vbox(self, widget):
        subprocess.Popen(['virtualbox'])

    def run_vm(self, widget, vm_guid):
        subprocess.Popen(["vboxmanage", "startvm", vm_guid])

if __name__ == "__main__":
    VboxSystrayLauncher()
    gtk.main()
