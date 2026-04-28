#include <iostream>
#include <wx/wx.h>
#include <wx/taskbar.h>
#include <wx/artprov.h>

enum VBTBI_MENU_ID {
    MID_QUIT = 100
};

/*=======================================================*/
// VBoxTaskBarIcon
/*=======================================================*/
class VBoxTaskBarIcon : public wxTaskBarIcon {
    public:
        VBoxTaskBarIcon();
        ~VBoxTaskBarIcon();
        virtual wxMenu* GetPopupMenu();
        void OnQuit(wxCommandEvent& event);

    private:
        wxMenu* vmMenu;
        wxTaskBarIcon* tbIcon;
        wxIcon* iconImage;

        wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(VBoxTaskBarIcon, wxTaskBarIcon)
    EVT_MENU(MID_QUIT, VBoxTaskBarIcon::OnQuit)
wxEND_EVENT_TABLE()

VBoxTaskBarIcon::VBoxTaskBarIcon() {
    this->tbIcon = nullptr;
    this->iconImage = nullptr;
    this->vmMenu = nullptr;

    this->tbIcon = new wxTaskBarIcon();
    this->iconImage = new wxIcon();
    this->iconImage->CopyFromBitmap(wxArtProvider::GetBitmap("virtualbox", wxART_OTHER, wxSize(32,32)));
    this->tbIcon->SetIcon(*(this->iconImage));
}

void VBoxTaskBarIcon::OnQuit(wxCommandEvent& WXUNUSED(event)) {
    wxTheApp->ExitMainLoop();
}

VBoxTaskBarIcon::~VBoxTaskBarIcon() {
    if(this->tbIcon != nullptr) {
        delete tbIcon;
    }

    if(this->iconImage != nullptr) {
        delete iconImage;
    }

    if(this->vmMenu != nullptr) {
        delete this->vmMenu;
    }
}

wxMenu* VBoxTaskBarIcon::GetPopupMenu() {
    std::cout << "Requesting menu" << std::endl;
    if(this->vmMenu == nullptr) {
        std::cout << "Building menu" << std::endl;
        this->vmMenu = new wxMenu();
        this->vmMenu->Append(MID_QUIT, "Quit");
    }

    return this->vmMenu;
}

/*=======================================================*/
/*=======================================================*/


/*=======================================================*/
// VBoxSTL
/*=======================================================*/
class VBoxSTL : public wxApp {
    public:
        VBoxSTL();
        ~VBoxSTL();
        bool OnInit() override;

    private:
        VBoxTaskBarIcon* vbtbIcon;
};

wxIMPLEMENT_APP(VBoxSTL);

VBoxSTL::VBoxSTL() {
    this->vbtbIcon = nullptr;
}

bool VBoxSTL::OnInit() {
    if(!wxApp::OnInit()) {
        return false;
    }

    if(!wxTaskBarIcon::IsAvailable()) {
        // there is no taskbar (system tray) support avaiable right now
        // might as well just quit
        return false;
    }

    this->vbtbIcon = new VBoxTaskBarIcon();

    std::cout << "Testing..." << std::endl;

    return true;
}

VBoxSTL::~VBoxSTL() {
    if(this->vbtbIcon != nullptr) {
        delete this->vbtbIcon;
    }
}

/*=======================================================*/
/*=======================================================*/
