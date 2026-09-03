%global pipewire_version 1.2.0

Name:           pronk
Version:        0.1.0
Release:        7%{?dist}
Summary:        Use Google Cast devices as managed desktop displays

License:        MIT
URL:            https://github.com/pronkproject/pronk
Source0:        %{name}-%{version}.tar.gz
Source1:        %{name}-%{version}-vendor.tar.xz

BuildRequires:  cargo >= 1.83
BuildRequires:  clang-devel
BuildRequires:  dbus-daemon
BuildRequires:  edid-decode
BuildRequires:  gcc
BuildRequires:  jq
BuildRequires:  meson >= 1.4
BuildRequires:  pipewire-gstreamer
BuildRequires:  pipewire-utils
BuildRequires:  pkgconfig(gstreamer-1.0)
BuildRequires:  pkgconfig(gstreamer-app-1.0)
BuildRequires:  pkgconfig(gstreamer-video-1.0)
BuildRequires:  pkgconfig(libdrm)
BuildRequires:  pkgconfig(libpipewire-0.3) >= %{pipewire_version}
BuildRequires:  pkgconfig(libsystemd)
BuildRequires:  rust >= 1.83
BuildRequires:  systemd-rpm-macros
BuildRequires:  wireplumber
BuildRequires:  libxml2
BuildRequires:  xz

Requires:       pipewire >= %{pipewire_version}
Requires:       pipewire-gstreamer
Requires:       polkit
Requires:       systemd
Requires:       wireplumber >= 0.5.15
Requires:       gstreamer1-plugins-base
Requires:       gstreamer1-plugins-bad-free
Requires:       gstreamer1-plugins-good
# x264enc is distributed by RPM Fusion on Fedora. Keeping this as a hard
# requirement makes a missing encoder an installation error instead of a
# casting failure after setup.
Requires:       gstreamer1-plugins-ugly
Recommends:     akmod-castkms >= 0.12.0

%description
Pronk coordinates an experimental CastKMS virtual monitor with an isolated
Google Cast backend. GNOME treats the Device as an ordinary extended display,
while restricted PipeWire connections carry its captured video and optional
audio between the local processes.


%prep
%autosetup
tar -xJf %{SOURCE1}


%build
export CARGO_NET_OFFLINE=true
%meson -Dcargo-profile=release
%meson_build


%install
%meson_install

# Discovery is socket activated and should work immediately after package
# installation without writing enablement state into each existing home.
install -d %{buildroot}%{_userunitdir}/sockets.target.wants
ln -s ../pronk-chromiacast.socket \
  %{buildroot}%{_userunitdir}/sockets.target.wants/pronk-chromiacast.socket


%check
runtime_dir=$(mktemp -d)
trap 'rm -rf "$runtime_dir"' EXIT
chmod 0700 "$runtime_dir"
XDG_RUNTIME_DIR="$runtime_dir" CARGO_NET_OFFLINE=true dbus-run-session -- \
  meson test -C %{_vpath_builddir} --no-suite integration --print-errorlogs


%files
%license LICENSE
%doc README.md
%{_bindir}/pronkctl
%{_bindir}/pronkd
%{_libexecdir}/pronk-grant-helper
%{_libexecdir}/pronk/
%{_prefix}/lib/pronk/
%{_unitdir}/pronk.service
%{_unitdir}/pronk-backend-mock.socket
%{_unitdir}/pronk-backend-mock@.service
%{_unitdir}/pronk-chromiacast.socket
%{_unitdir}/pronk-chromiacast@.service
%{_unitdir}/pronk-pipewire.service
%{_unitdir}/pronk-pipewire.socket
%{_unitdir}/pronk-wireplumber.service
%{_userunitdir}/pronk.service
%{_userunitdir}/pronk-backend-mock.socket
%{_userunitdir}/pronk-backend-mock@.service
%{_userunitdir}/pronk-chromiacast.socket
%{_userunitdir}/pronk-chromiacast@.service
%{_userunitdir}/pronk-pipewire.service
%{_userunitdir}/pronk-pipewire.socket
%{_userunitdir}/pronk-wireplumber.service
%{_userunitdir}/sockets.target.wants/pronk-chromiacast.socket
%{_sysusersdir}/pronk.conf
%{_tmpfilesdir}/pronk.conf
%{_datadir}/dbus-1/interfaces/io.github.pronkproject.Pronk1.xml
%{_datadir}/dbus-1/services/io.github.pronkproject.Pronk1.service
%{_datadir}/dbus-1/system-services/io.github.pronkproject.Pronk1.service
%{_datadir}/dbus-1/system.d/io.github.pronkproject.Pronk1.conf
%{_datadir}/pipewire/pipewire.conf.d/80-pronk-remotes.conf
%{_datadir}/polkit-1/actions/io.github.pronkproject.Pronk.policy
%{_datadir}/polkit-1/rules.d/20-pronk.rules
%{_datadir}/wireplumber/scripts/pronk-cast-audio-default.lua
%{_datadir}/wireplumber/scripts/pronk-policy-marker.lua
%{_datadir}/wireplumber/scripts/pronk-private-policy.lua
%{_datadir}/wireplumber/wireplumber.conf.d/80-pronk-access.conf
%{_datadir}/wireplumber/wireplumber.conf.d/81-pronk-castkms-audio.conf


%changelog
* Thu Sep 03 2026 Ray Strode <rstrode@redhat.com> - 0.1.0-7
- Build against Chromiacast 0.3.1 receiver-driven feedback notifications

* Thu Sep 03 2026 Ray Strode <rstrode@redhat.com> - 0.1.0-6
- Add system-service deployment and VP8 casting with H.264 fallback

* Mon Aug 31 2026 Ray Strode <rstrode@redhat.com> - 0.1.0-4
- Build against the published Chromiacast 0.3.0 crate

* Sun Aug 30 2026 Ray Strode <rstrode@redhat.com> - 0.1.0-3
- Install the H.264 parser required by the media graph

* Sun Aug 30 2026 Ray Strode <rstrode@redhat.com> - 0.1.0-2
- Build against the published Chromiacast 0.2.0 crate

* Sun Aug 30 2026 Ray Strode <rstrode@redhat.com> - 0.1.0-1
- Initial Pronk COPR package
