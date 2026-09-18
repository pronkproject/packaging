# Pronk Fedora packaging

This repository builds the Fedora package set for the Pronk casting stack.
Each source tree is pinned as a Git submodule. No patches from one project are
stored in another project: the SRPM helper generates the Mutter and GNOME
Control Center patch series directly from their pinned branches with
`git format-patch`.

The package set contains Pronk, replacement Mutter and GNOME Control Center
packages, and WirePlumber 0.5.15. The additional GVDB,
libgnome-volume-control, and libgxdp submodules supply content omitted from
GitLab-generated source archives. The CastKMS submodule pins the complete
kernel source that supplies both the driver and its DRM infrastructure; it is
not an out-of-tree module source.

## Development system extension

On Fedora Silverblue, the checked-out userspace stack can be built without
installing development packages into a transient `/usr` overlay. The helper
builds Pronk, Mutter, GNOME Settings, and WirePlumber in a dedicated rootless
Podman image, then publishes only their runtime files as a
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
worktrees are supported for userspace builds. Mutter and GNOME Settings
consume the separately pinned GVDB,
libgnome-volume-control, and libgxdp submodules as their Meson subprojects.
WirePlumber is built directly from its submodule rather than copied from an
outside RPM build. Run `git submodule update --init` if any source is missing.

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

The extension must be built while running a kernel made from the pinned
CastKMS source. CastKMS depends on DRM-core interfaces from that tree and
cannot be supplied as an akmod for a stock Fedora kernel. The kernel build and
installation remain a separate, explicitly bootable artifact. Build the
binary RPMs with:

```sh
scripts/build-castkms-kernel ./rpms
```

The builder starts with the running kernel's configuration, enables built-in
CastKMS, and gives the result a revision-derived release name. CastKMS audio is
disabled while its kernel interface is under development, and KUnit test
modules remain part of separate validation builds. Set `PRONK_KERNEL_CONFIG`
to start from another configuration and
`PRONK_KERNEL_BUILD_DIR` to relocate the persistent build directory.
`PRONK_KERNEL_RUSTC` selects a specific compiler; otherwise the builder prefers
the installed stable rustup toolchain. The builder keeps compiler temporary
files with its persistent products and rejects concurrent use of the same build
directory. It archives a committed CastKMS tree and rejects tracked changes;
commit kernel work before producing a bootable package. The compatibility
manifest records the kernel tree's distinct Rust
compiler floor. Install and boot that kernel before building the extension; do
not use an extension built for another kernel release. The extension builder
checks both the revision embedded in the running kernel's release and the
presence of built-in CastKMS before it starts a userspace build.

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
scripts/make-srpm wireplumber ./srpms
scripts/make-srpm mutter ./srpms
scripts/make-srpm gnome-control-center ./srpms
```

The helper archives only the commits recorded by the submodules. For Mutter
and GNOME Control Center it archives the upstream base named in the spec and
generates the remaining commits as patches. It verifies that the resulting
filenames exactly match the spec before invoking `rpmbuild`.

The helper needs Git, Python 3.11 or newer, `rpmbuild`, gzip, bzip2, and diff.
The Pronk SRPM also needs Cargo and network access to create its offline
dependency archive. Cargo 1.85 or newer is required to inspect every vendored
manifest, including target-specific dependencies that use the 2024 edition.
Select a suitable installed rustup toolchain with `CARGO_TOOLCHAIN=stable` when
the system Cargo is older. Binary package builds consume only the resulting
SRPMs and do not contact source hosting or crates.io.

Build WirePlumber before Pronk and build the replacement Mutter and GNOME
Control Center packages before updating a test machine. These packages target
Fedora 45 and replace core desktop components, so keep a working recovery path
while testing. RPM Fusion Free is required for the H.264 encoder supplied by
`gstreamer1-plugins-ugly`.

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
