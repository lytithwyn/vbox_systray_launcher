#include "vbox_systray_launcher.h"

wxIMPLEMENT_APP(VBoxSTL);

wxDEFINE_EVENT(EVT_UPDATE_READY, wxCommandEvent);

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
    this->vmList = nullptr;
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

    if(this->vmList != nullptr) {
        delete this->vmList;
    }
}

wxMenu* VBoxTaskBarIcon::CreatePopupMenu() {
    std::cout << "Building menu" << std::endl;
    wxMenu* vmMenu = new wxMenu();
    vmMenu->Append(MID_QUIT, "Quit");

    return vmMenu;
}

wxBEGIN_EVENT_TABLE(VBoxSTL, wxApp)
    EVT_TIMER(AID_TIMER, VBoxSTL::OnUpdateTimer)
    EVT_COMMAND(wxID_ANY, EVT_UPDATE_READY, VBoxSTL::OnNewVMList)
wxEND_EVENT_TABLE()

VBoxSTL::VBoxSTL() : timerMenuUpdate(this, AID_TIMER) { };

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
    this->PerformVMListUpdate();

    return true;
}

void VBoxSTL::OnUpdateTimer(wxTimerEvent& WXUNUSED(event)) {
    this->PerformVMListUpdate();
}

void VBoxSTL::PerformVMListUpdate() {
    std::cout << "Kicking off update thread" << std::endl;
    VBoxManagerThread* updateThread = new VBoxManagerThread(this, EVT_UPDATE_READY);
    if(updateThread->Create() != wxTHREAD_NO_ERROR) {
        std::cout << "Failed to create thread!" << std::endl;
    }
    updateThread->Run();
}

void VBoxSTL::OnNewVMList(wxCommandEvent& event) {
    std::map<std::string, std::string>* vmList = (std::map<std::string, std::string>*)event.GetClientData();

    std::cout << "Got new vm list: " << std::endl << "\t num vms's: " << vmList->size() << std::endl;
    auto it = vmList->begin();
    std::cout << "\tfirst entry: " << it->first << " : " << it->second << std::endl << std::endl;
    delete vmList;
    this->timerMenuUpdate.StartOnce(5000);
}

int VBoxSTL::OnExit() {
    delete this->vbtbIcon;
    return 0;
}

wxThread::ExitCode VBoxManagerThread::Entry() {
    std::cout << "In manager thread" << std::endl;
    this->Sleep(1500);
    std::cout << "Thread finished sleep" << std::endl;

    std::map<std::string, std::string>* vmList = new std::map<std::string, std::string>();
    (*vmList)["{3443-1234asdf-2ff542r-fasdfqwer}"] = "Windows 7 Pro";
    (*vmList)["{4443-1234asdf-2ff542r-fasdfqwer}"] = "FreeBSD";
    (*vmList)["{73443-1234asdf-2ff542r-fasdfqwer}"] = "OpenSuse Linux";

    wxCommandEvent* retEvent = new wxCommandEvent(this->eventID);
    retEvent->SetClientData((void*) vmList);
    wxQueueEvent(owner, retEvent);

    return 0;
}
