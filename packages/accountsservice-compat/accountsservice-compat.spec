Name:           accountsservice-compat
Version:        23.13.9
Release:        0.1.pronk%{?dist}
Summary:        Legacy AccountsService client ABI for Fedora 44 upgrades
License:        GPL-3.0-or-later
URL:            https://www.freedesktop.org/wiki/Software/AccountsService/
%global pronk_base 57e491f5e6f3da2d5a949f4f8747c8f4e8ed799d
Source0:        https://gitlab.freedesktop.org/accountsservice/accountsservice/-/archive/%{pronk_base}/accountsservice-%{pronk_base}.tar.bz2
Patch1:         0001-mocklibc-Fix-compiler-warning.patch
Patch2:         0002-user-manager-Fix-another-compiler-warning.patch
Patch3:         0003-act-user-Use-the-reentrant-interfaces-of-crypt-_gens.patch

BuildRequires:  gettext-devel
BuildRequires:  pkgconfig(dbus-1)
BuildRequires:  glib2-devel
BuildRequires:  polkit-devel
BuildRequires:  systemd
BuildRequires:  systemd-devel
BuildRequires:  gobject-introspection-devel
BuildRequires:  meson
BuildRequires:  libxcrypt-devel

%description
AccountsService 26 replaces the client library's libaccountsservice.so.0 ABI
with libaccountsservice.so.1. Fedora 44 desktop packages still linked against
soname 0 can use this compatibility library while the current AccountsService
daemon supplies the D-Bus service.

%prep
%autosetup -p1 -n accountsservice-%{pronk_base}

%build
%meson \
    -Dgtk_doc=false \
    -Dintrospection=false \
    -Dvapi=false \
    -Dadmin_group=wheel
%meson_build

%install
%meson_install

# The current AccountsService package owns the daemon, data files, headers,
# and introspection metadata. Keep only the legacy soname in this package.
find %{buildroot} -type f ! -path '%{buildroot}%{_libdir}/libaccountsservice.so.0*' -delete
find %{buildroot} -type l ! -path '%{buildroot}%{_libdir}/libaccountsservice.so.0*' -delete
find %{buildroot} -depth -type d -empty -delete

%ldconfig_scriptlets

%files
%license COPYING
%{_libdir}/libaccountsservice.so.0{,.*}

%changelog
* Tue Sep 22 2026 Ray Strode <rstrode@redhat.com> - 23.13.9-0.1.pronk
- Preserve the AccountsService soname 0 ABI during the Fedora 44 upgrade
