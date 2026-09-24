%global lcms2_version 2.6
%global colord_version 1.4.5
%global glib_version 2.81.1
%global gobject_introspection_version 1.41.4
%global gtk3_version 3.19.8
%global gtk4_version 4.14.0
%global gsettings_desktop_schemas_version 51~alpha
%global pixman_version 0.42
%global libei_version 1.3.901
%global wayland_server_version 1.24
%global wayland_protocols_version 1.48
%global libinput_version 1.30.0
%global libdrm_version 2.4.118
%global pipewire_version 1.6.0

%global mutter_api_version 51

Name:          mutter
Version:       51~rc
Release:       100.pronk5%{?dist}
Summary:       Window and compositing manager based on Clutter

# Automatically converted from old format: GPLv2+ - review is highly recommended.
License:       GPL-2.0-or-later
URL:           http://www.gnome.org
%global pronk_base ef900bd0dc2bc2df4d45596024d18d63d79759b2
%global gvdb_commit b54bc5da25127ef416858a3ad92e57159ff565b3
Source0:        https://gitlab.gnome.org/GNOME/%{name}/-/archive/%{pronk_base}/%{name}-%{pronk_base}.tar.bz2
Source1:        https://gitlab.gnome.org/GNOME/gvdb/-/archive/%{gvdb_commit}/gvdb-%{gvdb_commit}.tar.gz

Patch0100:      0001-backend-native-Add-secondary-GPU-copy-state.patch
Patch0101:      0002-tests-native-Exercise-secondary-GPU-copy-state.patch
Patch0102:      0003-onscreen-native-Track-damage-for-each-copy-buffer.patch
Patch0103:      0004-edid-Prefer-DisplayID-product-names-for-monitors.patch
Patch0104:      0005-backend-native-Add-CastKMS-grant-UAPI-definitions.patch
Patch0105:      0006-backend-native-Expose-KMS-device-reopening.patch
Patch0106:      0007-backend-native-Add-CastKMS-grant-objects.patch
Patch0107:      0008-backend-native-Define-CastKMS-grant-profiles.patch
Patch0108:      0009-backend-native-Broker-CastKMS-capture-grants.patch
Patch0109:      0010-backend-native-Follow-Pronk-bus-name-ownership.patch
Patch0110:      0011-tests-native-Exercise-CastKMS-grant-contracts.patch
Patch0111:      0012-docs-Identify-the-Pronk-integration-branch.patch
Patch0112:      0013-backends-native-Predict-display-state-only-after-acc.patch
Patch0113:      0014-tests-native-Check-display-state-after-rejected-atom.patch
Patch0114:      0015-backends-native-Only-unexport-a-connected-capture-br.patch
Patch0115:      0016-tests-native-Check-rejected-update-listener-lifetime.patch
Patch0116:      0017-backends-native-Separate-update-submission-from-fina.patch
Patch0117:      0018-backends-native-Retain-buffers-referenced-by-plane-a.patch
Patch0118:      0019-tests-native-Reassign-a-buffer-owned-by-an-update.patch
Patch0119:      0020-backends-native-Dispatch-ready-frames-as-KMS-impleme.patch
Patch0120:      0021-backends-native-Submit-queued-frames-without-the-dea.patch
Patch0121:      0022-backends-native-Represent-retryable-submission-failu.patch
Patch0122:      0023-backends-native-Identify-retryable-atomic-busy-rejec.patch
Patch0123:      0024-backends-native-Extract-terminal-update-finalization.patch
Patch0124:      0025-backends-native-Retry-unaccepted-busy-updates-asynch.patch
Patch0125:      0026-tests-drm-mock-Expose-the-queued-error-count.patch
Patch0126:      0027-tests-drm-mock-Allow-clearing-queued-errors.patch
Patch0127:      0028-tests-native-kms-Cover-asynchronous-busy-update-reco.patch
Patch0128:      0029-backends-native-Preserve-updates-that-remain-inhibit.patch
Patch0129:      0030-tests-native-Resume-an-update-after-partial-inhibiti.patch
Patch0130:      0031-backends-native-Add-experimental-display-preparation.patch
Patch0131:      0032-backends-native-Retain-preparation-with-pending-KMS-.patch
Patch0132:      0033-backends-native-Submit-preparation-tickets-with-atom.patch
Patch0133:      0034-backends-native-Schedule-KMS-updates-around-preparat.patch
Patch0134:      0035-backends-native-Remove-readiness-watches-before-free.patch
Patch0135:      0036-backends-native-Centralize-submitted-update-disposal.patch
Patch0136:      0037-backends-native-Discard-updates-waiting-for-initial-.patch
Patch0137:      0038-tests-drm-mock-Delay-preparation-descriptor-readines.patch
Patch0138:      0039-tests-native-kms-Exercise-pending-preparation-lifecy.patch
Patch0139:      0040-backends-native-Decode-the-CastKMS-execution-descrip.patch
Patch0140:      0041-backends-native-Track-the-renderer-profile-per-conne.patch
Patch0141:      0042-backends-native-Choose-native-buffers-for-host-execu.patch
Patch0142:      0043-kms-Own-anonymous-capture-grant-descriptors.patch
Patch0143:      0044-tests-Exercise-anonymous-capture-grant-ownership.patch
Patch0144:      0045-kms-Retain-the-issuing-file-for-capture-grants.patch
Patch0145:      0046-tests-Exercise-capture-grant-file-retention.patch
Patch0146:      0047-backends-native-Broker-pixel-only-capture-sessions.patch
Patch0147:      0048-backends-native-Release-caller-owned-capture-session.patch
Patch0148:      0049-backends-native-Acquire-CastKMS-monitor-control.patch
Patch0149:      0050-tests-Exercise-CastKMS-monitor-control-acquisition.patch
Patch0150:      0051-backends-native-Broker-CastKMS-display-sessions.patch
Patch0151:      0052-tests-Check-the-CastKMS-display-session-contract.patch
Patch0152:      0053-kms-Acquire-CastKMS-renderer-control.patch
Patch0153:      0054-backends-native-Broker-CastKMS-renderer-control.patch
Patch0154:      0055-backends-native-Accept-CastKMS-renderer-contract-3.patch
Patch0155:      0056-backends-native-Accept-CastKMS-renderer-contract-4.patch
Patch0156:      0057-backends-native-Accept-CastKMS-renderer-contract-5.patch
Patch0157:      0058-backends-native-Retain-renderer-transitions-in-KMS-u.patch
Patch0158:      0059-backends-native-Emit-CastKMS-renderer-transitions.patch
Patch0159:      0060-backends-native-Keep-one-CastKMS-authorization-model.patch
Patch0160:      0061-backends-native-Publish-monitor-control-files-atomic.patch
Patch0161:      0062-backends-native-Require-the-complete-scene-renderer-.patch
Patch0162:      0063-backends-native-Recognize-delegated-GPU-execution.patch
Patch0163:      0064-backends-native-Identify-the-CastKMS-renderer-GPU.patch
Patch0164:      0065-backends-native-Install-CastKMS-renderer-transitions.patch
Patch0165:      0066-backends-native-Issue-replaceable-CastKMS-renderers.patch
Patch0166:      0067-kms-Accept-controls-for-delegated-CastKMS-execution.patch
Patch0167:      0068-backends-native-Queue-additions-for-one-CRTC-update.patch
Patch0168:      0069-backends-native-Submit-CastKMS-transitions-with-fram.patch
Patch0169:      0070-kms-Carry-constraints-selection-in-atomic-updates.patch
Patch0170:      0071-kms-Retain-selected-constraints-in-CRTC-state.patch
Patch0171:      0072-kms-Model-immutable-display-constraints.patch
Patch0172:      0073-kms-Model-immutable-constraints-list-snapshots.patch
Patch0173:      0074-kms-Shape-constraints-for-buffer-allocation.patch
Patch0174:      0075-kms-Decode-display-constraints-snapshots.patch
Patch0175:      0076-tests-Exercise-display-constraints-decoding.patch
Patch0176:      0077-kms-Retain-immutable-display-constraints-snapshots.patch
Patch0177:      0078-kms-Query-coherent-display-constraints-snapshots.patch
Patch0178:      0079-tests-Exercise-display-constraints-queries.patch
Patch0179:      0080-kms-Query-display-constraints-through-DRM.patch
Patch0180:      0081-tests-Exercise-display-constraints-ioctl-queries.patch
Patch0181:      0082-kms-Retain-discovered-constraints-on-CRTCs.patch
Patch0182:      0083-kms-Bind-allocation-targets-to-constraints-snapshots.patch
Patch0183:      0084-native-Isolate-optional-EGL-config-errors.patch
Patch0184:      0085-native-Allocate-scanout-buffers-for-KMS-constraints.patch
Patch0185:      0086-fixup-tests-Exercise-display-constraints-decoding.patch
Patch0186:      0087-fixup-native-Allocate-scanout-buffers-for-KMS-constr.patch
Patch0187:      0088-kms-Match-transforms-against-selected-constraints.patch
Patch0188:      0089-native-Honor-selected-constraints-during-transform-p.patch
Patch0189:      0090-native-Release-imported-cursor-buffers-after-handoff.patch
Patch0190:      0091-native-Constrain-new-hardware-cursor-buffers.patch
Patch0191:      0092-kms-Compare-color-range-with-the-correct-property.patch
Patch0192:      0093-kms-Decode-constraints-list-change-events.patch
Patch0193:      0094-fixup-kms-Decode-constraints-list-change-events.patch
Patch0194:      0095-fixup-kms-Retain-discovered-constraints-on-CRTCs.patch
Patch0195:      0096-fixup-kms-Query-coherent-display-constraints-snapsho.patch
Patch0196:      0097-fixup-tests-Exercise-display-constraints-queries.patch
Patch0197:      0098-fixup-native-Allocate-scanout-buffers-for-KMS-constr.patch
Patch0198:      0099-native-Constrain-primary-plane-color-properties.patch
Patch0199:      0100-fixup-native-Allocate-scanout-buffers-for-KMS-constr.patch
Patch0200:      0101-fixup-native-Constrain-new-hardware-cursor-buffers.patch
Patch0201:      0102-kms-Refresh-constraints-from-DRM-events.patch
Patch0202:      0103-fixup-kms-Refresh-constraints-from-DRM-events.patch
Patch0203:      0104-kms-Subscribe-atomic-clients-to-display-constraints.patch
Patch0204:      0105-fixup-kms-Subscribe-atomic-clients-to-display-constr.patch
Patch0205:      0106-fixup-kms-Carry-constraints-selection-in-atomic-upda.patch
Patch0206:      0107-native-Generalize-constraints-target-binding-name.patch
Patch0207:      0108-native-Prefer-suggested-display-constraints.patch
Patch0208:      0109-fixup-native-Prefer-suggested-display-constraints.patch
Patch0209:      0110-fixup-native-Prefer-suggested-display-constraints.patch
Patch0210:      0111-fixup-kms-Carry-constraints-selection-in-atomic-upda.patch
Patch0211:      0112-kms-Preserve-latch-scope-across-update-merges.patch
Patch0212:      0113-fixup-kms-Carry-constraints-selection-in-atomic-upda.patch
Patch0213:      0114-fixup-native-Constrain-new-hardware-cursor-buffers.patch
Patch0214:      0115-fixup-kms-Refresh-constraints-from-DRM-events.patch
Patch0215:      0116-fixup-kms-Subscribe-atomic-clients-to-display-constr.patch
Patch0216:      0117-kms-Predict-CRTC-degamma-updates-from-their-own-stat.patch
Patch0217:      0118-native-Select-CastKMS-renderers-with-generic-constra.patch
Patch0218:      0119-kms-Identify-constraints-selection-updates.patch
Patch0219:      0120-kms-Permit-direct-constraints-list-reloads.patch
Patch0220:      0121-kms-Rebuild-updates-after-stale-constraints.patch
Patch0221:      0122-kms-Prepare-only-the-CRTC-used-by-one-output-updates.patch
Patch0222:      0123-kms-Decode-framebuffer-storage-constraints.patch
Patch0223:      0124-native-Preserve-layouts-across-DMA-buffer-imports.patch
Patch0224:      0125-kms-Match-exact-framebuffer-constraints.patch
Patch0225:      0126-native-Allocate-scanout-buffers-for-KMS-constraints.patch
Patch0226:      0127-native-Validate-cursors-against-KMS-constraints.patch
Patch0227:      0128-kms-Decode-active-plane-constraints.patch
Patch0228:      0129-native-Honor-active-plane-constraints-for-cursors.patch
Patch0229:      0130-kms-Accept-the-kernel-constraints-format-bound.patch
Patch0230:      0131-kms-Honor-plane-geometry-constraints.patch
Patch0231:      0132-kms-Honor-conditional-YUV-property-constraints.patch
Patch0232:      0133-kms-Honor-framebuffer-allocation-alignments.patch
Patch0233:      0134-backends-native-Accept-CastKMS-CEC-monitor-support.patch
Patch0234:      0135-kms-Bound-preparation-waits-for-blocking-display-cha.patch
Patch0235:      0136-native-Keep-CastKMS-renderer-authority-out-of-Mutter.patch
Patch0236:      0137-kms-Rebuild-onscreens-when-constraints-change.patch
Patch0237:      0138-kms-Consume-typed-constraints-list-notifications.patch
Patch0238:      0139-kms-Gate-constraints-events-on-libdrm-support.patch

%gnome_check_version

BuildRequires: cvt
BuildRequires: desktop-file-utils
BuildRequires: mesa-libEGL-devel
BuildRequires: mesa-libGLES-devel
BuildRequires: mesa-libGL-devel
BuildRequires: mesa-libgbm-devel
BuildRequires: pam-devel
BuildRequires: pkgconfig(bash-completion)
BuildRequires: pkgconfig(colord) >= %{colord_version}
BuildRequires: pkgconfig(glib-2.0) >= %{glib_version}
BuildRequires: pkgconfig(gobject-introspection-1.0) >= %{gobject_introspection_version}
BuildRequires: pkgconfig(sm)
BuildRequires: pkgconfig(lcms2) >= %{lcms2_version}
BuildRequires: pkgconfig(libadwaita-1)
BuildRequires: pkgconfig(libwacom)
BuildRequires: pkgconfig(xkbcommon)
BuildRequires: pkgconfig(glesv2)
BuildRequires: pkgconfig(graphene-gobject-1.0)
BuildRequires: pkgconfig(libdisplay-info)
BuildRequires: pkgconfig(libpipewire-0.3) >= %{pipewire_version}
BuildRequires: pkgconfig(sysprof-capture-4)
BuildRequires: pkgconfig(libsystemd)
BuildRequires: pkgconfig(umockdev-1.0)
BuildRequires: python3-argcomplete
BuildRequires: python3-docutils
# Bootstrap requirements
BuildRequires: gettext-devel git-core
BuildRequires: pkgconfig(libcanberra)
BuildRequires: pkgconfig(gsettings-desktop-schemas) >= %{gsettings_desktop_schemas_version}
BuildRequires: pkgconfig(gtk4) >= %{gtk4_version}
BuildRequires: pkgconfig(gnome-settings-daemon)
BuildRequires: meson
BuildRequires: pkgconfig(gbm)
BuildRequires: pkgconfig(glycin-2)
BuildRequires: pkgconfig(gnome-desktop-4)
BuildRequires: pkgconfig(gudev-1.0)
BuildRequires: pkgconfig(libdrm) >= %{libdrm_version}
BuildRequires: libdrm-devel >= 2.4.134-100.pronk1
BuildRequires: pkgconfig(libei-1.0) >= %{libei_version}
BuildRequires: pkgconfig(libeis-1.0) >= %{libei_version}
BuildRequires: pkgconfig(libstartup-notification-1.0)
BuildRequires: pkgconfig(wayland-protocols) >= %{wayland_protocols_version}
BuildRequires: pkgconfig(wayland-server) >= %{wayland_server_version}
BuildRequires: sysprof-devel

BuildRequires: pkgconfig(libinput) >= %{libinput_version}
BuildRequires: pkgconfig(pixman-1) >= %{pixman_version}
BuildRequires: pkgconfig(xwayland)

BuildRequires: python3-dbusmock

Requires: gnome-control-center-filesystem
Requires: libdrm >= 2.4.134-100.pronk1
Requires: glib2%{?_isa} >= %{glib_version}
Requires: gsettings-desktop-schemas%{?_isa} >= %{gsettings_desktop_schemas_version}
Requires: gnome-settings-daemon
Requires: gtk4%{?_isa} >= %{gtk4_version}
Requires: libeis%{?_isa} >= %{libei_version}
Requires: libinput%{?_isa} >= %{libinput_version}
Requires: pipewire%{_isa} >= %{pipewire_version}
Requires: startup-notification
Requires: dbus
Requires: python3-argcomplete

# Need common
Requires: %{name}-common = %{version}-%{release}

Recommends: mesa-dri-drivers%{?_isa}

Provides: firstboot(windowmanager) = mutter

# Cogl and Clutter were forked at these versions, but have diverged
# significantly since then.
Provides: bundled(cogl) = 1.22.0
Provides: bundled(clutter) = 1.26.0

Conflicts: mutter < 45~beta.1-2

# Make sure dnf updates gnome-shell together with this package; otherwise we
# might end up with broken gnome-shell installations due to mutter ABI changes.
Conflicts: gnome-shell < 45~rc

%description
Mutter is a window and compositing manager that displays and manages
your desktop via OpenGL. Mutter combines a sophisticated display engine
using the Clutter toolkit with solid window-management logic inherited
from the Metacity window manager.

While Mutter can be used stand-alone, it is primarily intended to be
used as the display core of a larger system such as GNOME Shell. For
this reason, Mutter is very extensible via plugins, which are used both
to add fancy visual effects and to rework the window management
behaviors to meet the needs of the environment.

%package common
Summary: Common files used by %{name} and forks of %{name}
BuildArch: noarch
Conflicts: mutter < 45~beta.1-2

%description common
Common files used by Mutter and soft forks of Mutter

%package devel
Summary: Development package for %{name}
Requires: %{name}%{?_isa} = %{version}-%{release}
Requires: libei%{?_isa} >= %{libei_version}
# for EGL/eglmesaext.h that's included from public cogl-egl-defines.h header
Requires: mesa-libEGL-devel

%description devel
Header files and libraries for developing Mutter plugins. Also includes
utilities for testing Metacity/Mutter themes.

%package  tests
Summary:  Tests for the %{name} package
Requires: %{name}-devel%{?_isa} = %{version}-%{release}
Requires: %{name}%{?_isa} = %{version}-%{release}
Requires: gtk3%{?_isa} >= %{gtk3_version}
Requires: libei%{?_isa} >= %{libei_version}

%description tests
The %{name}-tests package contains tests that can be used to verify
the functionality of the installed %{name} package.

%package devkit
Summary: Mutter Development Kit
Requires: %{name}%{?_isa} = %{version}-%{release}

%description devkit
Viewer for nested mutter instances.

%prep
%autosetup -S git -n %{name}-%{pronk_base}
mkdir -p subprojects/gvdb
tar -xzf %{SOURCE1} --strip-components=1 -C subprojects/gvdb

%build
%meson
%meson_build

%install
%meson_install

%find_lang %{name}

%files -f %{name}.lang
%license COPYING
%doc NEWS
%{_bindir}/gdctl
%{_bindir}/gnome-service-client
%{_bindir}/mutter
%{_datadir}/polkit-1/actions/org.gnome.mutter.*.policy
%{_libdir}/lib*.so.*
%{_libdir}/mutter-%{mutter_api_version}/
%exclude %{_libdir}/mutter-%{mutter_api_version}/*.gir
%{_libexecdir}/mutter-backlight-helper
%{_libexecdir}/mutter-x11-frames
%{_mandir}/man1/mutter.1*
%{_mandir}/man1/gdctl.1*
%{_mandir}/man1/gnome-service-client.1*
%{bash_completions_dir}/gdctl

%files common
%{_datadir}/GConf/gsettings/mutter-schemas.convert
%{_datadir}/glib-2.0/schemas/org.gnome.mutter.gschema.xml
%{_datadir}/glib-2.0/schemas/org.gnome.mutter.experimental.gschema.xml
%{_datadir}/glib-2.0/schemas/org.gnome.mutter.wayland.gschema.xml
%{_datadir}/gnome-control-center/keybindings/50-mutter-*.xml
%{_udevrulesdir}/61-mutter.rules

%files devel
%{_includedir}/mutter-%{mutter_api_version}/
%{_libdir}/lib*.so
%{_libdir}/mutter-%{mutter_api_version}/*.gir
%{_libdir}/pkgconfig/*

%files tests
%{_datadir}/installed-tests/mutter-%{mutter_api_version}
%{_datadir}/mutter-%{mutter_api_version}/tests
%{_libexecdir}/installed-tests/mutter-%{mutter_api_version}

%files devkit
%{_datadir}/applications/org.gnome.Mutter.Mdk.desktop
%{_datadir}/glib-2.0/schemas/org.gnome.mutter.devkit.gschema.xml
%{_datadir}/icons/hicolor/*/apps/org.gnome.Mutter.Mdk*
%{_libexecdir}/mutter-devkit

%changelog
%autochangelog
