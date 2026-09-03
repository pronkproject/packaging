# Pronk Fedora packaging

This repository builds the Fedora package set for the Pronk casting stack.
Each source tree is pinned as a Git submodule. No patches from one project are
stored in another project: the SRPM helper generates the Mutter and GNOME
Control Center patch series directly from their pinned branches with
`git format-patch`.

The package set contains Pronk, the CastKMS akmod, replacement Mutter and GNOME
Control Center packages, and WirePlumber 0.5.15. The additional GVDB,
libgnome-volume-control, and libgxdp submodules supply content omitted from
GitLab-generated source archives.

## Development system extension

On Fedora Silverblue, the complete checked-out stack can be built without
installing development packages into a transient `/usr` overlay. The helper
builds Pronk, CastKMS, Mutter, GNOME Settings, and WirePlumber in a dedicated
rootless Podman image, then publishes only their runtime files as a
`systemd-sysext` system extension. It also stages the RPM Fusion GStreamer x264
encoder and its runtime library, so those packages must be available from the
host's enabled repositories:

```sh
scripts/pronk-dev-sysext deploy
```

The helper always builds this repository's `sources/*` submodules. It does not
search sibling repositories or accept source-path overrides. To test a local
change, check out the desired commit or branch inside the corresponding
submodule and edit it there; dirty submodule worktrees are supported. Mutter
and GNOME Settings consume the separately pinned GVDB,
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

Reboot normally after `deploy`. At early boot, a generator exposes the image
only when both the exact Silverblue deployment and the running kernel match its
manifest. A system or kernel update therefore falls back to the stock desktop
until the extension is rebuilt.

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
scripts/make-srpm castkms-kmod ./srpms
scripts/make-srpm wireplumber ./srpms
scripts/make-srpm mutter ./srpms
scripts/make-srpm gnome-control-center ./srpms
```

The helper archives only the commits recorded by the submodules. For Mutter
and GNOME Control Center it archives the upstream base named in the spec and
generates the remaining commits as patches. It verifies that the resulting
filenames exactly match the spec before invoking `rpmbuild`.

The helper needs Git, Python 3.11 or newer, `rpmbuild`, gzip, bzip2, and diff. The Pronk SRPM also
needs Cargo and network access to create its offline dependency archive. If the
system default Cargo cannot read the lockfile, select an installed rustup
toolchain with `CARGO_TOOLCHAIN=stable`. Binary package builds consume only the
resulting SRPMs and do not contact source hosting or crates.io.

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
