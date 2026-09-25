# Pronk Fedora packaging

This repository builds the Fedora package set for the Pronk casting stack.
Each source tree is pinned as a Git submodule. No patches from one project are
stored in another project: the SRPM helper generates the Mutter and GNOME
Control Center patch series directly from their pinned branches with
`git format-patch`.

The package set contains Pronk, replacement libdrm, Mutter, and GNOME Control
Center packages, WirePlumber 0.5.15, GNOME 51 desktop schemas, GTK 4.23,
AccountsService 26.27.3, its soname 0 compatibility library, GNOME desktop 51,
and the privileged CastKMS renderer. A pinned libdrm fork supplies the
constraints-list event callback used by Mutter. The additional GVDB,
libgnome-volume-control, and libgxdp submodules supply content omitted from
GitLab-generated source archives. The CastKMS submodule pins the complete
kernel source that supplies both the driver and its DRM infrastructure; it is
not an out-of-tree module source.

## Development system extension

On Fedora Silverblue, the checked-out userspace stack can be built without
installing development packages into a transient `/usr` overlay. The helper
builds Pronk, GNOME desktop schemas, GTK, AccountsService, GNOME desktop,
libdrm, Mutter, GNOME Settings, WirePlumber, and the CastKMS renderer in a
dedicated rootless Podman image, then publishes only their runtime files as a
`systemd-sysext` system extension. It also stages the RPM Fusion GStreamer x264
encoder and its runtime library, so those packages must be available from the
host's enabled repositories:

```sh
scripts/pronk-dev-sysext deploy
```

The helper always reads this repository's `sources/*` submodules. It builds the
userspace sources and records the CastKMS kernel revision in the extension
manifest. It does not search sibling repositories or accept source-path
overrides. To test a local change, check out the desired commit or branch
inside the corresponding submodule and edit it there; dirty submodule
worktrees are supported for userspace builds. GNOME desktop schemas, GTK,
AccountsService, and GNOME desktop are built first so Mutter and GNOME
Settings see their GNOME 51 APIs. libdrm is built before Mutter to supply
the typed constraints-list event callback. The extension carries the compiled
schemas cache. Mutter and GNOME Settings consume the separately pinned GVDB,
libgnome-volume-control, and libgxdp submodules as their Meson subprojects.
WirePlumber is built directly from its submodule rather than copied from an
outside RPM build. The extension includes the libdrm runtime library while
Mutter builds against its staged headers and pkg-config metadata. Run
`git submodule update --init` if any source is missing.

The renderer is installed as a system service with the initial-namespace
`CAP_SYS_ADMIN` required to issue renderer endpoints. Mutter retains only
monitor-control and final-image capture issuance; Pronk receives final images,
never compositor source buffers. The renderer service has no network access and
publishes GPU constraints independently. If it cannot run, CastKMS continues
with its built-in HOST renderer.

The compatibility manifest remains the contract for a publishable package
set, so `scripts/check-compatibility` can fail while deliberately testing a
different local submodule revision. Update the recorded gitlink, package
metadata, and `compatibility.toml` together before publishing that revision.
The sysext helper reuses incremental build directories and limits parallel jobs
according to available memory; set `PRONK_BUILD_JOBS` to override that limit.

A system extension layers on top of Silverblue's immutable `/usr`. The helper
uses systemd-sysext's ephemeral mutable mode, so writes to the merged tree last
only until the extension is refreshed, unmerged, or the machine is rebooted.
Do not add an `rpm-ostree usroverlay` after booting with the extension active;
its ordering would hide the earlier merge. Building and publishing from an
already unlocked boot is safe because `deploy` does not alter the running
desktop.

For a host deployment, build the extension while running a kernel made from
the pinned CastKMS source. CastKMS depends on DRM-core interfaces from that
tree and cannot be supplied as an akmod for a stock Fedora kernel. The kernel
build and installation remain a separate, explicitly bootable artifact. Build the
binary RPMs with:

```sh
scripts/build-castkms-kernel ./rpms
```

For a guest with a separately built kernel, `prepare` and `build` can stage
the userspace extension on a Fedora host without changing that host's kernel:

```sh
scripts/pronk-dev-sysext --target-kernel-release RELEASE prepare
scripts/pronk-dev-sysext --target-kernel-release RELEASE build
```

`RELEASE` must end in the pinned CastKMS revision, and the resulting staged
extension records that exact release. Install and activate the stage in the
guest only after booting the corresponding kernel; `install` and `deploy`
reject the target-kernel option so they cannot publish a guest extension on
the build host.

The builder starts with the running kernel's configuration, enables built-in
CastKMS, strips module debug information before signing, and gives the result
a revision-derived release name. CastKMS audio is disabled while its kernel
interface is under development, and KUnit test modules remain part of separate
validation builds. Set `PRONK_KERNEL_CONFIG` to start from another configuration
and `PRONK_KERNEL_BUILD_DIR` to relocate the persistent build directory.
`PRONK_KERNEL_RUSTC` selects a specific compiler; otherwise the builder prefers
the installed stable rustup toolchain. The builder keeps compiler temporary
files with its persistent products and rejects concurrent use of the same build
directory. It archives a committed CastKMS tree and rejects tracked changes;
commit kernel work before producing a bootable package. The compatibility
manifest records the kernel tree's distinct Rust
compiler floor. For a host deployment, install and boot that kernel before
building the extension; do not use an extension built for another kernel
release. The host build checks both the revision embedded in the running
kernel's release and the presence of built-in CastKMS before it starts.

On a Secure Boot system, provide a private key and its enrolled certificate:

```sh
PRONK_KERNEL_SIGNING_KEY=/path/to/private-key.pem \
PRONK_KERNEL_SIGNING_CERT=/path/to/certificate.der \
scripts/build-castkms-kernel ./rpms
```

Both variables are required together. The certificate may use PEM or DER
encoding. The builder records its digest in the build identity, embeds and
verifies the image signature before packaging, and never copies the private key
into the build or package output. Enrolling a new certificate remains an
explicit machine-administration step outside this helper.

Reboot normally after `deploy`. At early boot, a generator exposes the image
only when both the exact Silverblue deployment and the running kernel match its
manifest. A system or kernel update therefore falls back to the stock desktop
until the extension is rebuilt. Built-in CastKMS is initialized by the kernel;
the extension does not load or replace kernel code.

Useful diagnostics do not change the system:

```sh
scripts/pronk-dev-sysext check
scripts/pronk-dev-sysext status
```

Published images live below `/var/lib/pronk-sysext`; the active image is an
atomic symlink and older images remain available for diagnosis. The exact-host
manifest and boot generator live below `/etc/pronk-sysext` and
`/etc/systemd/system-generators`. Create `/etc/pronk-sysext/disabled` to keep
the extension inactive at the next boot.

## Building source RPMs

Clone and initialize every source tree:

```sh
git clone --recurse-submodules https://github.com/pronkproject/packaging.git
cd packaging
```

Then build any package into an output directory:

```sh
scripts/make-srpm pronk ./srpms
scripts/make-srpm castkms-renderer ./srpms
scripts/make-srpm libdrm ./srpms
scripts/make-srpm wireplumber ./srpms
scripts/make-srpm gsettings-desktop-schemas ./srpms
scripts/make-srpm gtk4 ./srpms
scripts/make-srpm accountsservice ./srpms
scripts/make-srpm accountsservice-compat ./srpms
scripts/make-srpm gnome-desktop3 ./srpms
scripts/make-srpm mutter ./srpms
scripts/make-srpm gnome-control-center ./srpms
```

The helper archives only the commits recorded by the submodules. Build and
install the libdrm and libdrm-devel RPMs before building Mutter, whose
constraints notifications require the fork's typed event callback. For Mutter
and GNOME Control Center it archives the upstream base named in the spec and
generates the remaining commits as patches. It verifies that the resulting
filenames exactly match the spec before invoking `rpmbuild`.

The helper needs Git, Python 3.11 or newer, `rpmbuild`, gzip, bzip2, and diff.
The Pronk and CastKMS renderer SRPMs also need Cargo and network access to
create their offline dependency archives. Cargo 1.85 or newer is required to
inspect every vendored manifest, including target-specific dependencies that
use the 2024 edition.
Select a suitable installed rustup toolchain with `CARGO_TOOLCHAIN=stable` when
the system Cargo is older. Binary package builds consume only the resulting
SRPMs and do not contact source hosting or crates.io.

Build libdrm, GNOME desktop schemas, GTK, AccountsService, its compatibility
library, GNOME desktop, and WirePlumber before Mutter and Pronk. Build the
CastKMS renderer against the same kernel as the display stack. Build the
replacement Mutter and GNOME Control Center packages before updating a test
machine. These packages replace core desktop components, so keep a working
recovery path while testing. RPM Fusion Free is required for the H.264
encoder supplied by `gstreamer1-plugins-ugly`.

To update a package, first advance the appropriate source submodule. If the
upstream base changes, update the spec's `pronk_base` at the same time. A patch
name or count mismatch intentionally stops the SRPM build so branch history and
packaging cannot silently diverge.

`compatibility.toml` is the machine-readable assembly contract. It records the
exact source commits, public protocol versions, component API floors, and the
tested whole-stack baseline. In particular, Pronk's PipeWire API floor is
separate from the newer PipeWire version required by the supported Mutter and
Fedora stack. Every SRPM build validates the checked-out submodules, duplicated
D-Bus contract, package version floors, and checked-in interface constants
before producing an archive.

The hosted compatibility gate performs these inexpensive contract checks and
regenerates the downstream patch series on every change. Full SRPM, COPR, and
virtual-machine builds remain release gates because they are substantially
more resource intensive.

## VM assembly test

The [VM assembly harness](tests/vm/assembly/README.md) boots a disposable
CastKMS guest with the staged userspace stack and checks changing decoded
frames through Mutter, Pronk, PipeWire, and H.264. It runs without a receiver
unless a receiver is explicitly selected. The harness also configures the
host virgl GBM layout needed for Venus to import eligible shared GL buffers;
that setting does not qualify foreign-buffer GPU composition by itself.
