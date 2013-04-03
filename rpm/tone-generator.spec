Name:       tone-generator
Summary:    Tone generator daemon for call progress indication and DTMF
Version:    1.5
Release:    1
Group:      System/Daemons
License:    LGPLv2.1
URL:        https://github.com/nemomobile/tone-generator
Source0:    %{name}-%{version}.tar.gz
Source1:    %{name}.service
Source2:    tonegend.desktop
Requires:   pulseaudio
BuildRequires:  pkgconfig(dbus-1)
BuildRequires:  pkgconfig(dbus-glib-1)
BuildRequires:  pkgconfig(gobject-2.0)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(libpulse)
BuildRequires:  pkgconfig(libpulse-mainloop-glib)

%description
Tone generator daemon for call progress indication and DTMF

%prep
%setup -q -n %{name}-%{version}

%build

%reconfigure --disable-static
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}/usr/lib/systemd/user/
cp %{_sourcedir}/%{name}.service %{buildroot}/usr/lib/systemd/user/

mkdir -p %{buildroot}/etc/xdg/autostart/
cp %SOURCE2 %{buildroot}/etc/xdg/autostart/

%files
%defattr(-,root,root,-)
/usr/bin/tonegend
%config /etc/dbus-1/system.d/tone-generator.conf
/usr/lib/systemd/user/%{name}.service
%exclude /etc/xdg/autostart/tonegend.desktop

