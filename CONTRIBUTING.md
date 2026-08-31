# Contributing to Pronk packaging

This repository is the compatibility authority for a supported Pronk stack.
Changes should update the source submodule, package metadata, and
`compatibility.toml` together whenever their recorded contract changes.

Do not commit generated patches from Mutter, GNOME Settings, or another source
repository. `scripts/make-srpm` generates downstream patch series directly from
the pinned branches. Keep those branches as reviewable, bisectable histories
and update the corresponding `pronk_base` and `Patch` entries when their export
range changes.

Before submitting a change, initialize every submodule and run:

```sh
scripts/check-compatibility
scripts/check-castkms-uapi-layout
output_dir=$(mktemp -d)
scripts/generate-patches sources/mutter packages/mutter/mutter.spec \
  "$output_dir/mutter"
scripts/generate-patches sources/gnome-control-center \
  packages/gnome-control-center/gnome-control-center.spec \
  "$output_dir/gnome-control-center"
shellcheck scripts/check-castkms-uapi-layout scripts/generate-patches \
  scripts/make-pronk-sources scripts/make-srpm
```

Build affected SRPMs with `scripts/make-srpm` before publishing a package-set
update. Whole-stack COPR and virtual-machine validation remains the release
gate for changes that alter installed binaries or integration contracts.

Report packaging and compatibility problems in this repository. Product
behavior belongs in the Pronk repository; Cast protocol issues belong in
Chromiacast; kernel capture issues belong in CastKMS.
