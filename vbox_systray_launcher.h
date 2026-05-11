#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <map>
#include <string>
#include <cstring>
#include <sstream>
#include <regex>
#include <exception>
#include <wx/wx.h>
#include <wx/taskbar.h>
#include <wx/artprov.h>

wxDECLARE_EVENT(EVT_UPDATE_READY, wxCommandEvent);

enum VBTBI_MENU_ID {
    MID_QUIT = 1001,
    MID_LAUNCH_VBOX,
    MID_LAUNCH_VM = 1200,
    MID_END_LAUNCH_VM = 1300
};

enum APP_EVENT_ID {
    AID_UPDATE_TIMER,
};

struct UpdateThreadControl {
    wxCriticalSection controlCS;
    bool requestStop;
};

class VBoxSTL;

class VBoxManagerThread : public wxThread {
    public:
        VBoxManagerThread(wxEvtHandler* owner, int eventID, int readFD, UpdateThreadControl* utControl);
        virtual wxThread::ExitCode Entry();

    private:
        void LaunchExeGetOutput(const char* imageName, ...);
        bool MatchVMLine(std::string line, std::pair<std::string, std::string>& outPair);

        wxEvtHandler* owner;
        int eventID;
        int readFD;
        UpdateThreadControl* utControl;
        std::regex regexVMLine;
};

class VBoxTaskBarIcon : public wxTaskBarIcon {
    public:
        VBoxTaskBarIcon(VBoxSTL* owner);
        virtual ~VBoxTaskBarIcon();
        virtual wxMenu *CreatePopupMenu();

    private:
        VBoxSTL* owner;
        wxIcon* iconImage;

        void OnLeftClick(wxTaskBarIconEvent& event);
};

class VBoxSTL : public wxApp {
    public:
        VBoxSTL();
        const std::map<std::string, std::string>* GetVMList() { return this->vmList; };
        void SetVMList(std::map<std::string, std::string>* vmList);
        bool OnInit() override;
        int OnExit() override;
        void OnLaunchVM(wxCommandEvent& event);
        void OnLaunchVBoxApp(wxCommandEvent& event);
        void OnQuit(wxCommandEvent& event);

    private:
        VBoxTaskBarIcon* vbtbIcon;
        wxTimer timerMenuUpdate;
        std::map<std::string, std::string>* vmList;
        bool updateRunning;
        bool doShutdown;
        UpdateThreadControl utControl;

        void PerformVMListUpdate();
        std::pair<std::string, std::string> GetVMAtIndex(unsigned int index);
        void OnUpdateTimer(wxTimerEvent& event);
        void OnNewVMList(wxCommandEvent& event);
        void LaunchExe(const char* imageName);
        void LaunchExe(const char* imageName, int* readFD, ...);

    wxDECLARE_EVENT_TABLE();
};
