%global debug_package %{nil}

Name:           gsettings-desktop-schemas
Version:        51.0
Release:        0.1.pronk%{?dist}
Summary:        A collection of GSettings schemas
License:        LGPL-2.1-or-later
URL:            https://gitlab.gnome.org/GNOME/gsettings-desktop-schemas
%global pronk_base 1db238b6a349ea7fae6f1c0713afe04d1bb7ea5c
Source0:        https://gitlab.gnome.org/GNOME/gsettings-desktop-schemas/-/archive/%{pronk_base}/gsettings-desktop-schemas-%{pronk_base}.tar.bz2
BuildRequires:  gettext
BuildRequires:  glib2-devel >= 2.31.0
BuildRequires:  gobject-introspection-devel
BuildRequires:  meson
Requires:       glib2 >= 2.31.0
Recommends:     font(adwaitasans)
Recommends:     font(adwaitamono)

%description
gsettings-desktop-schemas contains schemas shared by desktop components.

%package devel
Summary: Development files for %{name}
Requires: %{name}%{?_isa} = %{version}-%{release}

%description devel
Headers and metadata for developing applications using desktop schemas.

%prep
%autosetup -n %{name}-%{pronk_base}

%build
%meson
%meson_build

%install
%meson_install

%find_lang %{name} --with-gnome

%check
glib-compile-schemas --dry-run --strict %{buildroot}%{_datadir}/glib-2.0/schemas

%files -f %{name}.lang
%doc AUTHORS MAINTAINERS NEWS README
%license COPYING
%{_datadir}/glib-2.0/schemas/*
%{_libdir}/girepository-1.0/GDesktopEnums-3.0.typelib

%files devel
%doc HACKING
%{_includedir}/*
%{_datadir}/pkgconfig/*
%{_datadir}/gir-1.0/GDesktopEnums-3.0.gir

%changelog
* Tue Sep 22 2026 Ray Strode <rstrode@redhat.com> - 51.0-0.1.pronk
- Package GNOME 51 schemas for the Pronk desktop stack
