CXXFLAGS+=`wx-config --cflags` -g -std=c++20
LDFLAGS=`wx-config --libs`

all: vbox_systray_launcher

vbox_systray_launcher: vbox_systray_launcher.cpp
	g++ -o vbox_systray_launcher ${CXXFLAGS} vbox_systray_launcher.cpp ${LDFLAGS}

clean:
	rm vbox_systray_launcher
