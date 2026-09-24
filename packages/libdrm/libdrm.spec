%global pronk_commit 050ba20132b0424d7b0238a8e164f6036663bd38

Name:           libdrm
Version:        2.4.134
Release:        100.pronk1%{?dist}
Summary:        Userspace interface to kernel DRM services
License:        MIT
URL:            https://github.com/pronkproject/libdrm
Source0:        %{url}/archive/%{pronk_commit}/%{name}-%{pronk_commit}.tar.gz

BuildRequires:  gcc
BuildRequires:  meson
BuildRequires:  ninja-build
BuildRequires:  python3
BuildRequires:  pkgconfig(pciaccess)

%description
The core DRM userspace library, including typed constraints-list events.

%package devel
Summary:        Development files for libdrm
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description devel
Headers and pkg-config metadata for the core DRM userspace library.

%prep
%autosetup -n %{name}-%{pronk_commit}

%build
%meson \
  -Dtests=false \
  -Dintel=enabled \
  -Dradeon=enabled \
  -Damdgpu=enabled \
  -Dnouveau=enabled \
  -Dvmwgfx=disabled \
  -Domap=disabled \
  -Dexynos=disabled \
  -Dtegra=disabled \
  -Dvc4=disabled \
  -Detnaviv=disabled \
  -Dcairo-tests=disabled \
  -Dvalgrind=disabled \
  -Dman-pages=disabled
%meson_build

%install
%meson_install

%files
%license LICENSES/MIT.txt
%doc README.rst
%{_libdir}/libdrm.so.2*
%{_libdir}/libdrm_amdgpu.so.1*
%{_libdir}/libdrm_intel.so.1*
%{_libdir}/libdrm_nouveau.so.2*
%{_libdir}/libdrm_radeon.so.1*
%{_datadir}/libdrm/amdgpu.ids

%files devel
%{_includedir}/libdrm/
%{_includedir}/libsync.h
%{_includedir}/xf86drm.h
%{_includedir}/xf86drmMode.h
%{_libdir}/libdrm.so
%{_libdir}/libdrm_amdgpu.so
%{_libdir}/libdrm_intel.so
%{_libdir}/libdrm_nouveau.so
%{_libdir}/libdrm_radeon.so
%{_libdir}/pkgconfig/libdrm.pc
%{_libdir}/pkgconfig/libdrm_amdgpu.pc
%{_libdir}/pkgconfig/libdrm_intel.pc
%{_libdir}/pkgconfig/libdrm_nouveau.pc
%{_libdir}/pkgconfig/libdrm_radeon.pc
