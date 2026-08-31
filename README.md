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
