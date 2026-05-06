#include "vbox_systray_launcher.h"

wxIMPLEMENT_APP(VBoxSTL);

wxDEFINE_EVENT(EVT_UPDATE_READY, wxCommandEvent);
wxDEFINE_EVENT(EVT_SHUTDOWN, wxCommandEvent);

VBoxTaskBarIcon::VBoxTaskBarIcon(VBoxSTL* owner) : wxTaskBarIcon(wxTBI_DEFAULT_TYPE) {
    this->owner = owner;
    this->iconImage = new wxIcon();
    this->iconImage->CopyFromBitmap(wxArtProvider::GetBitmap("virtualbox", wxART_OTHER, wxSize(32,32)));
    this->SetIcon(*(this->iconImage));

    Bind(wxEVT_MENU, &VBoxSTL::OnQuit, owner, MID_QUIT);
    Bind(wxEVT_MENU, &VBoxSTL::OnLaunchVBoxApp, owner, MID_LAUNCH_VBOX);
    Bind(wxEVT_MENU, &VBoxSTL::OnLaunchVM, owner, MID_LAUNCH_VM, MID_END_LAUNCH_VM);
    Bind(wxEVT_TASKBAR_LEFT_UP, &VBoxTaskBarIcon::OnLeftClick, this);
}

void VBoxTaskBarIcon::OnLeftClick(wxTaskBarIconEvent& WXUNUSED(event)) {
    // for some reason this doesn't work
    // if we attach to LEFT_DOWN it works, but the menu immediately appears and disappears
    wxTaskBarIconEvent* clickEvent = new wxTaskBarIconEvent(wxEVT_TASKBAR_CLICK, this);
    wxQueueEvent(this, clickEvent);
}

void VBoxSTL::OnLaunchVBoxApp(wxCommandEvent& WXUNUSED(event)) {
    this->LaunchExe("virtualbox");
    return;
}

void VBoxSTL::OnLaunchVM(wxCommandEvent& event) {
    int vmIndex = event.GetId() - MID_LAUNCH_VM;

    std::string guid;
    try {
        std::pair<std::string, std::string> vmPair = this->GetVMAtIndex(vmIndex);
        guid = vmPair.first;
    } catch(const std::runtime_error& e) {
        wxMessageBox(e.what(), "Error", wxOK | wxICON_EXCLAMATION);
    }

    this->LaunchExe("vboxmanage", nullptr, "startvm", guid.c_str(), (void*)0);
}

void VBoxSTL::OnQuit(wxCommandEvent& WXUNUSED(event)) {
    if(this->updateRunning) {
        this->doShutdown = true;
    } else {
        this->ExitMainLoop();
    }
}

wxMenu* VBoxTaskBarIcon::CreatePopupMenu() {
    wxMenu* vmMenu = new wxMenu();
    vmMenu->Append(MID_LAUNCH_VBOX, "Launch Virtualbox");
    vmMenu->AppendSeparator();

    // TODO figure out a better way to identify the VM to be launched
    // We don't have the opportunity to add extra info, but if the list is changed
    // while we are clicking in the menu there might be a difference between when
    // this list gets built and when the event handler for launching the vm gets run
    int vmIndex = 0;
    const std::map<std::string, std::string>* vmList = this->owner->GetVMList();
    if(vmList->size() > 0) {
        for(auto const& [guid, name] : *vmList) {
            vmMenu->Append(MID_LAUNCH_VM + vmIndex, name);
            ++vmIndex;
        }
    }

    vmMenu->AppendSeparator();
    vmMenu->Append(MID_QUIT, "Quit");

    return vmMenu;
}

VBoxTaskBarIcon::~VBoxTaskBarIcon() {
    if(this->iconImage != nullptr) {
        delete iconImage;
    }

}

wxBEGIN_EVENT_TABLE(VBoxSTL, wxApp)
    EVT_TIMER(AID_UPDATE_TIMER, VBoxSTL::OnUpdateTimer)
    EVT_COMMAND(wxID_ANY, EVT_UPDATE_READY, VBoxSTL::OnNewVMList)
    EVT_COMMAND(wxID_ANY, EVT_SHUTDOWN, VBoxSTL::OnQuit)
wxEND_EVENT_TABLE()

VBoxSTL::VBoxSTL() : timerMenuUpdate(this, AID_UPDATE_TIMER) { }

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

    this->updateRunning = false;
    this->doShutdown = false;
    this->vmList = nullptr;
    this->vbtbIcon = new VBoxTaskBarIcon(this);
    this->PerformVMListUpdate();

    return true;
}

void VBoxSTL::OnUpdateTimer(wxTimerEvent& WXUNUSED(event)) {
    this->PerformVMListUpdate();
}

void VBoxSTL::PerformVMListUpdate() {
    int readFD = -1;
    this->LaunchExe("vboxmanage", &readFD, "list", "vms", (void*)0);
    VBoxManagerThread* updateThread = new VBoxManagerThread(this, EVT_UPDATE_READY, readFD);
    if(updateThread->Create() != wxTHREAD_NO_ERROR) {
        std::cerr << "Failed to create thread!" << std::endl;
    }
    updateThread->Run();
    this->updateRunning = true;
}

void VBoxSTL::OnNewVMList(wxCommandEvent& event) {
    this->updateRunning = false;
    std::map<std::string, std::string>* vmList = (std::map<std::string, std::string>*)event.GetClientData();

    std::cout << "Got new vm list: " << std::endl << "\t num vms's: " << vmList->size() << std::endl;
    this->SetVMList(vmList);
    if(!this->doShutdown) {
        this->timerMenuUpdate.StartOnce(5000);
    } else {
        wxCommandEvent* shutdownEvent = new wxCommandEvent(EVT_SHUTDOWN);
        wxQueueEvent(this, shutdownEvent);
        std::cout << "Sent shutdown event" << std::endl;
    }
}

void VBoxSTL::SetVMList(std::map<std::string, std::string>* vmList) {
    if(this->vmList != nullptr) {
        delete this->vmList;
    }

    this->vmList = vmList;
}

std::pair<std::string, std::string> VBoxSTL::GetVMAtIndex(unsigned int index) {
    if(index >= this->vmList->size()) {
        throw(std::runtime_error("The requested index is out of range in the list of VM's"));
    }

    std::map<std::string, std::string>::iterator it = this->vmList->begin();
    for(unsigned int i = 0; i < index; ++i) {
        ++it;
    }

    return *it;
}

int VBoxSTL::OnExit() {
    if(this->vmList != nullptr) {
        delete this->vmList;
    }

    delete this->vbtbIcon;
    return 0;
}

void VBoxSTL::LaunchExe(const char* imageName) {
    return this->LaunchExe(imageName, nullptr, (void*) 0);
}

void VBoxSTL::LaunchExe(const char* imageName, int* readFD, ...) {
    // first, get our variable arguments
    // we'll iterate through one time to get the count, then again to build an array of the arguments
    // notice, this will NOT count the final null byte - we will account for this later
    va_list argList;
    va_start(argList, readFD);
    int numInputArgs = 0;
    while(va_arg(argList, const char*) != ((void*)0)) {
        ++numInputArgs;
    }
    va_end(argList);

    // construct a sufficiently sized array and load the image name as the first element of this array
    // account for 1 extra element at beginning for image name, and 1 extra at end for null byte
    char** progArgs = (char**)calloc(numInputArgs + 2, sizeof(char*));
    progArgs[0] = (char*)calloc(strlen(imageName) + 1, sizeof(char));
    strcpy(progArgs[0], imageName); // first element is image name
    progArgs[numInputArgs+1] = ((char*)0); // last element is null/zero pointer

    // now copy in all the others
    va_start(argList, readFD);
    for(int i = 0; i < numInputArgs; ++i) {
        const char* thisArg = va_arg(argList, const char*);
        progArgs[i+1] = (char*)calloc(strlen(thisArg) + 1, sizeof(char));
        strcpy(progArgs[i+1], thisArg);
    }
    va_end(argList);

    int fds[2]{-1};
    bool wantsFD = (nullptr != readFD);

    if(wantsFD) {
        if(pipe(fds) != 0) {
            wxMessageBox("Failed to create pipes for the subprocess", "System Error", wxOK | wxICON_EXCLAMATION);
            return;
        }
    }

    pid_t forkResult = fork();
    if(forkResult < 0) {
        wxMessageBox("Failed to fork a child PID", "System Error", wxOK | wxICON_EXCLAMATION);
        if(wantsFD) {
            close(fds[0]);
            close(fds[1]);
            *readFD = -1;
        }
        return;
    } else if(forkResult > 0) {
        // we're in the parent
        close(fds[1]); // close the writing end
        if(wantsFD) {
            // we have gotten the pipe set up, we've successfully forked, and the caller wants the read FD
            *readFD = fds[0];
        }
        return;
    } else if(forkResult == 0) {
        // we're in the child
        if(wantsFD) {
            close(fds[0]); // close the reading end
            dup2(fds[1], STDOUT_FILENO);
            dup2(fds[1], STDERR_FILENO);
            close(fds[1]);
        }

        if(execvp(imageName, progArgs) < 0) {
            // TODO figure out how to handle an error here since we're in the child
            // maybe have a special value to send over the pipe?
            std::cerr << "Execvp failed: " << strerror(errno) << std::endl;
            exit(-1);
        }
    }
}

VBoxManagerThread::VBoxManagerThread(wxEvtHandler* owner, int eventID, int readFD) {
    this->owner = owner;
    this->eventID = eventID;
    this->readFD = readFD;
    this->regexVMLine = std::regex("\"(.*)\" (\\{.*\\})");
};

bool VBoxManagerThread::MatchVMLine(std::string line, std::pair<std::string, std::string>& outPair) {
    std::smatch vmLineMatches;

    if(std::regex_search(line, vmLineMatches, this->regexVMLine)) {
        outPair.first = vmLineMatches[1];
        outPair.second = vmLineMatches[2];
        return true;
    } else {
        return false;
    }
}

wxThread::ExitCode VBoxManagerThread::Entry() {
    std::cout << "Attempting to read data from pipe" << std::endl;
    ssize_t bytesRead = 0;
    size_t readBufSize = 128;
    char readBuf[readBufSize];
    std::stringstream vbmOutput;
    while(true) {
        bytesRead = read(readFD, readBuf, readBufSize);
        if(bytesRead < 0) {
            // TODO error handling - probably fire an event to main thread
            std::cerr << "Failed to read from pipe: " << strerror(errno) << std::endl;
            break;
        } else if(bytesRead == 0) {
            // EOF
            break;
        } else {
            vbmOutput.write(readBuf, bytesRead);
        }
    }
    close(readFD);

    std::map<std::string, std::string>* vmList = new std::map<std::string, std::string>();
    for(std::string line; std::getline(vbmOutput, line); ) {
        std::pair<std::string, std::string> outPair;
        if(this->MatchVMLine(line, outPair)) {
            (*vmList)[outPair.second] = outPair.first;
        }
    }

    wxCommandEvent* retEvent = new wxCommandEvent(this->eventID);
    retEvent->SetClientData((void*) vmList);
    wxQueueEvent(owner, retEvent);

    return 0;
}
