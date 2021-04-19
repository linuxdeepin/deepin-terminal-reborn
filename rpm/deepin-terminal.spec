%define specrelease 1%{?dist}
%if 0%{?openeuler}
%define specrelease 1
%endif

%define libname libqtermwidget5

Name:           deepin-terminal
Version:        5.4.1.1
Release:        %{specrelease}
Summary:        Default terminal emulation application for Deepin
License:        GPLv3+
URL:            https://github.com/linuxdeepin/%{name}
Source0:        %{url}/archive/%{version}/%{name}-%{version}.tar.gz

Provides:      deepin-terminal-data
Obsoletes:     deepin-terminal-data

BuildRequires: gcc-c++
BuildRequires: cmake3
BuildRequires: qt5-linguist

BuildRequires: pkgconfig(dtkcore)
BuildRequires: pkgconfig(dtkwidget)
BuildRequires: pkgconfig(dtkgui)
BuildRequires: pkgconfig(dframeworkdbus)
BuildRequires: pkgconfig(gobject-2.0)
BuildRequires: pkgconfig(atspi-2)
BuildRequires: pkgconfig(libsecret-1)

BuildRequires: pkgconfig(Qt5Widgets)
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5Gui)
BuildRequires: pkgconfig(Qt5DBus)
BuildRequires: pkgconfig(Qt5Network)
BuildRequires: lxqt-build-tools >= 0.6.0~
BuildRequires: utf8proc-devel
BuildRequires: gtest-devel
BuildRequires: gmock-devel

Requires: libqtermwidget5
Requires: libqtermwidget5-data
Requires: expect
Requires: zssh
Requires: qt-at-spi
Requires: at-spi2-core

%description
%{summary}.

%package -n %{libname}
Summary:        Terminal emulator widget for Qt 5
%description  -n %{libname}
Terminal emulator widget for Qt 5 (shared libraries)
QTermWidget is a Unicode-enabled, embeddable Qt widget that can be used as
built-in console or terminal emulation widget.

%package -n %{libname}-devel
Summary:        Terminal emulator widget for Qt 5
%description -n %{libname}-devel
Terminal emulator widget for Qt 5 (shared libraries)
QTermWidget is a Unicode-enabled, embeddable Qt widget that can be used as
built-in console or terminal emulation widget.


%package -n %{libname}-data
Summary:        Terminal emulator widget for Qt 5
%description -n %{libname}-data
Terminal emulator widget for Qt 5 (shared libraries)
QTermWidget is a Unicode-enabled, embeddable Qt widget that can be used as
built-in console or terminal emulation widget.

%prep
%autosetup

%build
# help find (and prefer) qt5 utilities, e.g. qmake, lrelease
export PATH=%{_qt5_bindir}:$PATH
# cmake_minimum_required version is too high
sed -i "s|^cmake_minimum_required.*|cmake_minimum_required(VERSION 3.0)|" $(find . -name "CMakeLists.txt")
mkdir build && pushd build
%cmake -DCMAKE_BUILD_TYPE=Release -DAPP_VERSION=%{version} -DVERSION=%{version}  ../
%make_build
popd

%install
%make_install -C build INSTALL_ROOT="%buildroot"

%post -n %{libname}
ldconfig

%postun -n %{libname}
ldconfig

%files
%doc README.md
%license LICENSE
%{_bindir}/%{name}
%{_datadir}/%{name}/translations/*.qm
%{_datadir}/icons/hicolor/scalable/apps/%{name}.svg
%{_datadir}/applications/%{name}.desktop
%{_datadir}/deepin-manual/manual-assets/application/deepin-terminal/terminal/

%files -n %{libname}
%{_libdir}/libterminalwidget5.so.0.14.1

%files -n %{libname}-devel
%{_includedir}/terminalwidget5/*.h
%{_libdir}/cmake/terminalwidget5/*.cmake
%{_libdir}/libterminalwidget5.so
%{_libdir}/libterminalwidget5.so.0
%{_libdir}/pkgconfig/terminalwidget5.pc

%files -n %{libname}-data
%{_datadir}/terminalwidget5/color-schemes/*.schema
%{_datadir}/terminalwidget5/color-schemes/*.colorscheme
%{_datadir}/terminalwidget5/color-schemes/historic/*.schema
%{_datadir}/terminalwidget5/kb-layouts/*.keytab
%{_datadir}/terminalwidget5/kb-layouts/historic/*.keytab
%{_datadir}/terminalwidget5/translations/*.qm

%changelog
* Mon Apr 19 2021 zhangdingwen <zhangdingwen@uniontech.com> - 5.4.1.1-1
- init spec for euler

* Tue Oct 20 2020 guoqinglan <guoqinglan@uniontech.com> - 5.2.36-1
- update to 5.2.36

* Tue Sep 29 2020 guoqinglan <guoqinglan@uniontech.com> - 5.2.35-1
- update to 5.2.35
