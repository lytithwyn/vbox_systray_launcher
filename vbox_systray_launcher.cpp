#include "vbox_systray_launcher.h"

wxIMPLEMENT_APP(VBoxSTL);

wxBEGIN_EVENT_TABLE(VBoxTaskBarIcon, wxTaskBarIcon)
    EVT_MENU(MID_QUIT, VBoxTaskBarIcon::OnMenuQuit)
    EVT_MENU(MID_LAUNCH_VBOX, VBoxTaskBarIcon::OnMenuLaunchVBoxApp)
    EVT_MENU(MID_LAUNCH_VM, VBoxTaskBarIcon::OnMenuLaunchVM)
    EVT_MENU(MID_QUIT, VBoxTaskBarIcon::OnMenuQuit)
wxEND_EVENT_TABLE()

VBoxTaskBarIcon::VBoxTaskBarIcon() : wxTaskBarIcon(wxTBI_DEFAULT_TYPE) {
    this->iconImage = new wxIcon();
    this->iconImage->CopyFromBitmap(wxArtProvider::GetBitmap("virtualbox", wxART_OTHER, wxSize(32,32)));
    this->SetIcon(*(this->iconImage));
}

void VBoxTaskBarIcon::OnMenuLaunchVBoxApp(wxCommandEvent& WXUNUSED(event)) {
    return;
}

void VBoxTaskBarIcon::OnMenuLaunchVM(wxCommandEvent& event) {
    return;
}

void VBoxTaskBarIcon::OnMenuQuit(wxCommandEvent& WXUNUSED(event)) {
    wxGetApp().ExitMainLoop();
}

VBoxTaskBarIcon::~VBoxTaskBarIcon() {
    if(this->iconImage != nullptr) {
        delete iconImage;
    }
}

wxMenu* VBoxTaskBarIcon::CreatePopupMenu() {
    std::cout << "Building menu" << std::endl;
    wxMenu* vmMenu = new wxMenu();
    vmMenu->Append(MID_QUIT, "Quit");

    return vmMenu;
}

bool VBoxSTL::OnInit() {
    if(!wxApp::OnInit()) {
        return false;
    }

    if(!wxTaskBarIcon::IsAvailable()) {
        // there is no taskbar (system tray) support avaiable right now
        // might as well just quit
        wxMessageBox
        (
            "There appears to be no system tray support in your current environment. Exiting!",
            "Error",
            wxOK | wxICON_EXCLAMATION
        );
        return false;
    }

    this->vbtbIcon = new VBoxTaskBarIcon();

    return true;
}

int VBoxSTL::OnExit() {
    delete this->vbtbIcon;
    return 0;
}
