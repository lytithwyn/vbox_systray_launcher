#include "vbox_systray_launcher.h"

wxIMPLEMENT_APP(VBoxSTL);

wxDEFINE_EVENT(EVT_UPDATE_READY, wxCommandEvent);

VBoxTaskBarIcon::VBoxTaskBarIcon(VBoxSTL* owner) : wxTaskBarIcon(wxTBI_DEFAULT_TYPE) {
    this->owner = owner;
    this->iconImage = new wxIcon();
    this->iconImage->CopyFromBitmap(wxArtProvider::GetBitmap("virtualbox", wxART_OTHER, wxSize(32,32)));
    this->SetIcon(*(this->iconImage));
    this->vmList = nullptr;

    Bind(wxEVT_MENU, &VBoxSTL::OnQuit, owner, MID_QUIT);
    Bind(wxEVT_MENU, &VBoxSTL::OnLaunchVBoxApp, owner, MID_LAUNCH_VBOX);
    Bind(wxEVT_MENU, &VBoxSTL::OnLaunchVM, owner, MID_LAUNCH_VM);
}

void VBoxSTL::OnLaunchVBoxApp(wxCommandEvent& WXUNUSED(event)) {
    this->LaunchExe("virtualbox");
    return;
}

void VBoxSTL::OnLaunchVM(wxCommandEvent& event) {
    return;
}

void VBoxSTL::OnQuit(wxCommandEvent& WXUNUSED(event)) {
    this->ExitMainLoop();
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
    vmMenu->Append(MID_LAUNCH_VBOX, "Launch Virtualbox");
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

    this->vbtbIcon = new VBoxTaskBarIcon(this);
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

void VBoxSTL::LaunchExe(const char* imageName) {
    int fds[2];
    if(pipe(fds) != 0) {
        wxMessageBox("Failed to create pipes for the subprocess", "System Error", wxOK | wxICON_EXCLAMATION);
        return;
    }

    pid_t forkResult = fork();
    if(forkResult < 0) {
        wxMessageBox("Failed to fork a child PID", "System Error", wxOK | wxICON_EXCLAMATION);
        return;
    } else if(forkResult > 0) {
        // we're in the parent
        return;
    } else if(forkResult == 0) {
        // we're in the child
        close(fds[0]);
        close(fds[1]);
        if(execlp(imageName, imageName, (char *)0) < 0) {
            // TODO figure out how to handle an error here since we're in the child
            exit(-1);
        }
    }
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
