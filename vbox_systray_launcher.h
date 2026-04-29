#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <map>
#include <string>
#include <cstring>
#include <sstream>
#include <wx/wx.h>
#include <wx/taskbar.h>
#include <wx/artprov.h>

wxDECLARE_EVENT(EVT_UPDATE_READY, wxCommandEvent);

enum VBTBI_MENU_ID {
    MID_QUIT = 1001,
    MID_LAUNCH_VBOX,
    MID_LAUNCH_VM = 1200
};

enum APP_EVENT_ID {
    AID_TIMER
};

class VBoxSTL;

class VBoxManagerThread : public wxThread {
    public:
        VBoxManagerThread(wxEvtHandler* owner, int eventID, int readFD) { this->owner = owner; this->eventID = eventID; };
        virtual wxThread::ExitCode Entry();

    private:
        void LaunchExeGetOutput(const char* imageName, ...);

        wxEvtHandler* owner;
        int eventID;
};

class VBoxTaskBarIcon : public wxTaskBarIcon {
    public:
        VBoxTaskBarIcon(VBoxSTL* owner);
        virtual ~VBoxTaskBarIcon();
        virtual wxMenu *CreatePopupMenu();
        void SetVMList(std::map<std::string, std::string> vmList);

    protected:
        VBoxSTL* owner;
        wxIcon* iconImage;
        std::map<std::string, std::string>* vmList;
};

class VBoxSTL : public wxApp {
    public:
        VBoxSTL();
        bool OnInit() override;
        int OnExit() override;
        void OnLaunchVM(wxCommandEvent& event);
        void OnLaunchVBoxApp(wxCommandEvent& event);
        void OnQuit(wxCommandEvent& event);

    private:
        VBoxTaskBarIcon* vbtbIcon;
        wxTimer timerMenuUpdate;

        void PerformVMListUpdate();
        void OnUpdateTimer(wxTimerEvent& event);
        void OnNewVMList(wxCommandEvent& event);
        void LaunchExe(const char* imageName);
        void LaunchExe(const char* imageName, int* readFD, ...);

    wxDECLARE_EVENT_TABLE();
};
