#include <iostream>
#include <wx/wx.h>
#include <wx/taskbar.h>
#include <wx/artprov.h>
#include <map>
#include <string>

wxDECLARE_EVENT(EVT_UPDATE_READY, wxCommandEvent);

enum VBTBI_MENU_ID {
    MID_QUIT = 1001,
    MID_LAUNCH_VBOX,
    MID_LAUNCH_VM = 1200
};

enum APP_EVENT_ID {
    AID_TIMER
};

class VBoxManagerThread : public wxThread {
    public:
        VBoxManagerThread(wxEvtHandler* owner, int eventID) { this->owner = owner; this->eventID = eventID; };
        virtual wxThread::ExitCode Entry();

    private:
        wxEvtHandler* owner;
        int eventID;
};

class VBoxTaskBarIcon : public wxTaskBarIcon {
    public:
        VBoxTaskBarIcon();
        virtual ~VBoxTaskBarIcon();
        virtual wxMenu *CreatePopupMenu();
        void SetVMList(std::map<std::string, std::string> vmList);

    protected:
        void OnMenuLaunchVM(wxCommandEvent& event);
        void OnMenuLaunchVBoxApp(wxCommandEvent& event);
        void OnMenuQuit(wxCommandEvent& event);

        wxIcon* iconImage;
        std::map<std::string, std::string>* vmList;

    wxDECLARE_EVENT_TABLE();
};

class VBoxSTL : public wxApp {
    public:
        VBoxSTL();
        bool OnInit() override;
        int OnExit() override;

    private:
        VBoxTaskBarIcon* vbtbIcon;
        wxTimer timerMenuUpdate;

        void PerformVMListUpdate();
        void OnUpdateTimer(wxTimerEvent& event);
        void OnNewVMList(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};
