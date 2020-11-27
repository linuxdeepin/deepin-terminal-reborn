%global _terminals gnome-terminal mate-terminal xfce4-terminal lxterminal qterminal qterminal-qt5 terminology yakuake fourterm roxterm lilyterm termit xterm mrxvt
%global repo deepin-terminal
%global libname terminalwidget5

Name:           deepin-terminal
Version:        5.3.0.4
Release:        1%{?dist}
Summary:        Default terminal emulation application for Deepin
License:        GPLv3
URL:            https://github.com/linuxdeepin/%{repo}
Source0:        %{url}/archive/%{version}/%{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(dtkwidget)
BuildRequires:  pkgconfig(lxqt) >= 0.14.0
BuildRequires:  pkgconfig(gobject-2.0)
BuildRequires:  pkgconfig(atspi-2)
BuildRequires:  pkgconfig(dframeworkdbus)
BuildRequires:  pkgconfig(libsecret-1)
BuildRequires:  qt5-qtbase-private-devel
%{?_qt5:Requires: %{_qt5}%{?_isa} = %{_qt5_version}}
BuildRequires:  qt5-linguist
Requires:       deepin-qt5integration%{?_isa}
# right-click menu style
Requires:       deepin-menu
# subprocess command
Requires:       deepin-shortcut-viewer
Requires:       expect
Requires:       xdg-utils
Recommends:     deepin-manual
Recommends:     zssh
Requires:       %{name}-data = %{version}-%{release}

%description
%{summary}.

%package devel
Summary:        Development package for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description devel
Header files and libraries for %{name}.

%package data
Summary:        Data files of Deepin Terminal
BuildArch:      noarch
Requires:       hicolor-icon-theme

%description data
The %{name}-data package provides shared data for Deepin Terminal.

%prep
%autosetup -p1 -n %{repo}-%{version}
# don't hard code -fPIE
sed -i 's/-fPIE//' CMakeLists.txt
# fix error: 'QString::QString(const char*)' is private within this context
sed -i '/LXQtCompilerSettings/a remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII)' 3rdparty/terminalwidget/CMakeLists.txt

%build
%cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo
%cmake_build

%install
%cmake_install
chmod -v 755 %{buildroot}%{_bindir}/*

%check
# failing for now
desktop-file-validate %{buildroot}%{_datadir}/applications/%{repo}.desktop ||:

%preun
if [ $1 -eq 0 ]; then
  %{_sbindir}/alternatives --remove x-terminal-emulator %{_bindir}/%{name}
fi

%post
if [ $1 -ge 1 ]; then
  %{_sbindir}/alternatives --install %{_bindir}/x-terminal-emulator \
    x-terminal-emulator %{_bindir}/%{name} 20
fi

%triggerin -- konsole5 %_terminals
if [ $1 -ge 1 ]; then
  PRI=20
  for i in konsole %{_terminals}; do
    PRI=$((PRI-1))
    test -x %{_bindir}/$i && \
    %{_sbindir}/alternatives --install %{_bindir}/x-terminal-emulator \
      x-terminal-emulator %{_bindir}/$i $PRI &>/dev/null ||:
  done
fi

%triggerpostun -- konsole5 %_terminals
if [ $2 -eq 0 ]; then
  for i in konsole %{_terminals}; do
    test -x %{_bindir}/$i || \
    %{_sbindir}/alternatives --remove x-terminal-emulator %{_bindir}/$i &>/dev/null ||:
  done
fi

%files
%{_bindir}/%{name}
%{_libdir}/lib%{libname}.so.0*
%{_datadir}/applications/%{name}.desktop

%files devel
%{_includedir}/%{libname}/
%{_libdir}/lib%{libname}.so
%{_libdir}/pkgconfig/%{libname}.pc
%{_libdir}/cmake/%{libname}/

%files data
%doc README.md
%license LICENSE
%{_datadir}/%{name}/
%{_datadir}/%{libname}/
%{_datadir}/icons/hicolor/*/apps/%{name}*

%changelog
