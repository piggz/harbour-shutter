# Define Sailfish as it is absent
%if !0%{?fedora}
%define sailfishos 1
%endif

# Prevent brp-python-bytecompile from running.
%define __os_install_post %{___build_post}

%if 0%{?sailfishos}
# "Harbour RPM packages should not provide anything."
%define __provides_exclude_from ^%{_datadir}/.*$
%endif

Name:       harbour-shutter

# >> macros
# << macros

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}
Summary:    Shutter Camera
Version:    0.0.1
Release:    1
Group:      Qt/Qt
License:    GPLv2
URL:        http://github.com/piggz/harbour-shutter
Source0:    %{name}-%{version}.tar.bz2
Requires:   sailfishsilica-qt5 >= 0.10.9
Requires:   libcamera
Requires:   opencv
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Multimedia)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  qt5-qttools-linguist
BuildRequires:  ssu-sysinfo-devel
BuildRequires:  libexif-devel
BuildRequires:  desktop-file-utils
BuildRequires:  pkgconfig(libcamera)
BuildRequires:  opencv-devel

%description
Shutter is a camera application built ontop of libcamera.

%if "%{?vendor}" == "chum"
PackageName: Shutter
Type: desktop-application
DeveloperName: Adam Pigg
Categories:
 - Media
 - Video
Custom:
  Repo: https://github.com/piggz/harbour-shutter
Icon: https://raw.githubusercontent.com/piggz/harbour-advanced-camera/master/harbour-advanced-camera.svg
Screenshots:
 - https://github.com/piggz/harbour-advanced-camera/raw/master/screenshots/screenshot4.png
 - https://github.com/piggz/harbour-advanced-camera/raw/master/screenshots/screenshot5.png
 - https://github.com/piggz/harbour-advanced-camera/raw/master/screenshots/screenshot6.png
Url:
  Homepage: https://github.com/piggz/harbour-shutter
  Help: https://github.com/piggz/harbour-shutter/discussions
  Bugtracker: https://github.com/piggz/harbour-shutter/issues
  Donation: https://www.paypal.me/piggz
%endif

%prep
%setup -q -n %{name}-%{version}

# >> setup
# << setup

%build
# >> build pre
# << build pre
%if 0%{?sailfishos}
%qtc_qmake5 VERSION='%{version}-%{release}' FLAVOR=silica
%else
%qtc_qmake5 VERSION='%{version}-%{release}' FLAVOR=kirigami
%endif

%qtc_make %{?_smp_mflags}

# >> build post
# << build post

%install
rm -rf %{buildroot}
# >> install pre
# << install pre
%qmake5_install

# >> install post
# << install post

desktop-file-install --delete-original       \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
# >> files
# << files
