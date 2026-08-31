Name:           pronk
Version:        0.1.0
Release:        3%{?dist}
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
BuildRequires:  pkgconfig(libpipewire-0.3)
BuildRequires:  pkgconfig(libsystemd)
BuildRequires:  rust >= 1.83
BuildRequires:  systemd-rpm-macros
BuildRequires:  wireplumber
BuildRequires:  libxml2
BuildRequires:  xz

Requires:       pipewire
Requires:       pipewire-gstreamer
Requires:       systemd
Requires:       wireplumber >= 0.5.15
Requires:       gstreamer1-plugins-base
Requires:       gstreamer1-plugins-bad-free
# x264enc is distributed by RPM Fusion on Fedora. Keeping this as a hard
# requirement makes a missing encoder an installation error instead of a
# casting failure after setup.
Requires:       gstreamer1-plugins-ugly
Recommends:     akmod-castkms >= 0.11.0

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
%{_libexecdir}/pronk/
%{_prefix}/lib/pronk/
%{_userunitdir}/pronk.service
%{_userunitdir}/pronk-backend-mock.socket
%{_userunitdir}/pronk-backend-mock@.service
%{_userunitdir}/pronk-chromiacast.socket
%{_userunitdir}/pronk-chromiacast@.service
%{_userunitdir}/sockets.target.wants/pronk-chromiacast.socket
%{_datadir}/dbus-1/interfaces/io.github.pronkproject.Pronk1.xml
%{_datadir}/dbus-1/services/io.github.pronkproject.Pronk1.service
%{_datadir}/pipewire/pipewire.conf.d/80-pronk-remotes.conf
%{_datadir}/wireplumber/scripts/pronk-cast-audio-default.lua
%{_datadir}/wireplumber/scripts/pronk-policy-marker.lua
%{_datadir}/wireplumber/scripts/pronk-private-policy.lua
%{_datadir}/wireplumber/wireplumber.conf.d/80-pronk-access.conf
%{_datadir}/wireplumber/wireplumber.conf.d/81-pronk-castkms-audio.conf


%changelog
* Sun Aug 30 2026 Ray Strode <rstrode@redhat.com> - 0.1.0-3
- Install the H.264 parser required by the media graph

* Sun Aug 30 2026 Ray Strode <rstrode@redhat.com> - 0.1.0-2
- Build against the published Chromiacast 0.2.0 crate

* Sun Aug 30 2026 Ray Strode <rstrode@redhat.com> - 0.1.0-1
- Initial Pronk COPR package
