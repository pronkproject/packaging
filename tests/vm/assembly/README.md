# CastKMS VM assembly test

`run` boots a disposable guest with a CastKMS kernel, the staged Pronk system
extension, virgl/Venus graphics, and an isolated Mutter/PipeWire/WirePlumber
session. Pronk's live capture probe renders a changing fullscreen pattern and
checks decoded H.264 pixels. The default test stays local; it does not contact
a receiver.

Before testing the full assembly, `run-gl-venus` can isolate one graphics
boundary. It clears a two-color pattern through guest GLES into a virtio-GBM
buffer, exports that buffer, imports it as a Venus Vulkan image, copies its
pixels to a Vulkan staging buffer, and checks four samples. It uses the booted
kernel and host graphics libraries but does not require a staged extension,
Mutter build, CastKMS monitor IDs, or a receiver:

```sh
PRONK_VM_KERNEL_IMAGE=/path/to/kernel-build/arch/x86/boot/bzImage \
tests/vm/assembly/run-gl-venus
```

The host launcher enables `VIRGL_GBM_LAYOUT_ENABLE=1` by default. Set it to
`0` to reproduce the missing host-GBM-storage path; a failure is expected for
that negative control. The log is `gl-venus.log` below `PRONK_VM_RESULTS`.
Passing this check proves pixel transfer for a virtio-owned allocation. It
does not prove that GLES can render to a foreign CastKMS or system-heap
allocation, nor that Mutter selects the working path for its output.

Build the kernel and userspace stage first. The kernel image must correspond
to the release recorded in the stage manifest. Build the Pronk
`pronk-capture-mutter-media-live-test` and
`pronk-capture-pattern-client` binaries in `sources/pronk/target/debug`, and
provide a Mutter build directory for its session test runner. The launcher
checks the staged CastKMS, Pronk, and Mutter revisions against the source
submodules. It uses the installed `virtme-run`, QEMU, and host EGL stack.

The CRTC and connector IDs are explicit because they belong to the booted
CastKMS instance, not to the packaging source tree. For a previously inspected
guest, run:

```sh
PRONK_VM_KERNEL_IMAGE=/path/to/kernel-build/arch/x86/boot/bzImage \
PRONK_VM_MUTTER_BUILD=/path/to/mutter-build \
PRONK_VM_CRTC_ID=CRTC_ID \
PRONK_VM_CONNECTOR_ID=CONNECTOR_ID \
tests/vm/assembly/run
```

Set `PRONK_VM_KERNEL_BUILD` if the image is not below its build directory.
`PRONK_VM_STAGE` selects a different staged system extension; by default the
launcher uses `pronk-dev-sysext`'s state directory. `PRONK_VM_RESULTS` selects
where the logs remain. `PRONK_VM_WIDTH` and `PRONK_VM_HEIGHT` default to
1920×1080. `PRONK_VM_COPY_MODE` accepts `primary-gpu-cpu` (default),
`primary-gpu-gpu`, or `zero-copy`. The copy-mode setting is diagnostic: a
successful local test does not by itself prove that GPU composition was used.
The guest requires the kernel release in the stage manifest to match its
running kernel. Set `PRONK_VM_ALLOW_KERNEL_RELEASE_MISMATCH=1` only for a
deliberate local build from the same kernel sources with a different release
suffix; the guest reports that mismatch. Do not use that override to qualify a
publishable assembly.

For Venus, the host launcher defaults `VIRGL_GBM_LAYOUT_ENABLE=1` before
starting QEMU. The variable makes virgl use host GBM storage for eligible
shared resources so that its Venus proxy can export them. Set it explicitly
to `0` for a negative-control run. It does not make arbitrary foreign
DMA-BUFs importable, and the guest must not set it in place of the host.

To test a real receiver, explicitly set `PRONK_VM_RECEIVER=HOST:PORT`. That
can interrupt whatever the receiver is displaying. The test checks receiver
acknowledgement, not visible playback; ask someone watching the display to
confirm the image separately. Without that variable, no receiver is contacted.

The guest logs remain under `PRONK_VM_RESULTS` (by default
`$XDG_STATE_HOME/pronk-vm-results` or `~/.local/state/pronk-vm-results`).
`assembly.log` is the boot and test transcript; each session has its own
`assembly.*` directory with compositor, media, and client logs. The launcher
rejects an already-running QEMU VM and runs the guest at lower process
priority with a 2 GiB memory limit to avoid crowding the host.
