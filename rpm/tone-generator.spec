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
Requires:   systemd
Requires:   systemd-user-session-targets
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

mkdir -p %{buildroot}%{_libdir}/systemd/user/user-session.target.wants
install -m 0644  %SOURCE1 %{buildroot}%{_libdir}/systemd/user/
ln -s ../tone-generator.service %{buildroot}%{_libdir}/systemd/user/user-session.target.wants/


mkdir -p %{buildroot}/etc/xdg/autostart/
install -m 0644 %SOURCE2 %{buildroot}/etc/xdg/autostart/

%post
if [ "$1" -ge 1 ]; then
systemctl-user daemon-reload || :
systemctl-user restart tone-generator.service || :
fi

%postun
if [ "$1" -eq 0 ]; then
systemctl-user stop tone-generator.service || :
systemctl-user daemon-reload || :
fi

%files
%defattr(-,root,root,-)
/usr/bin/tonegend
%config /etc/dbus-1/system.d/tone-generator.conf
/usr/lib/systemd/user/%{name}.service
/usr/lib/systemd/user/user-session.target.wants/%{name}.service
%exclude /etc/xdg/autostart/tonegend.desktop

