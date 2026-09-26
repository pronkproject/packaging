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
Set `PRONK_VM_TEST_WIDTH` and `PRONK_VM_TEST_HEIGHT` to exercise the same
pixel path at a larger image size; both default to 64.
Set `PRONK_VM_TEST_AR30=1` to check the stride and modifier of a separate
ARGB2101010 render allocation, including an explicit-linear allocation, at
that size before the pixel-transfer test.
Set `PRONK_VM_TEST_XR24=1` to exercise an opaque XRGB8888 surface, and
`PRONK_VM_TEST_GLES3=1` to use a GLES3 context. An XRGB buffer's unused alpha
byte is not part of the pixel assertion.
`PRONK_VM_TEST_SECOND_SURFACE=1` first renders to another 1280×800 GBM/EGL
window surface using the same context, then returns to the tested surface.
`PRONK_VM_TEST_NO_CONFIG_CONTEXT=1` uses `EGL_KHR_no_config_context`, and
`PRONK_VM_TEST_EXPLICIT_LINEAR=1` creates both surfaces with an explicit
linear modifier; these isolate two more parts of Mutter's EGL/GBM setup.
`PRONK_VM_TEST_SWAPS` repeats render-and-swap before importing the last GBM
front buffer (1 by default, at most 8).

`build-mesa` builds the pinned Mesa source into a VM-only runtime stage. It
checks that its version matches the host Mesa package and installs matching
Gallium, GBM, and EGL libraries together. This avoids substituting a partial
graphics stack on the host:

```sh
tests/vm/assembly/build-mesa
```

`run-virgl-import` checks a different graphics boundary without Mutter,
Pronk, or CastKMS initialization. It first imports a virtio-owned GBM image,
then asks virgl to import a system-heap DMA-BUF. The guest-only allocation
must be rejected before a host command is submitted, and unrelated GL
rendering must still produce the expected pixels. Set `PRONK_VM_MESA_STAGE`
to the stage printed by `build-mesa`. The kernel image must contain the
matching virtio-gpu blob-memory reporting fix:

```sh
PRONK_VM_KERNEL_IMAGE=/path/to/kernel-build/arch/x86/boot/bzImage \
PRONK_VM_MESA_STAGE=/path/printed/by/build-mesa \
tests/vm/assembly/run-virgl-import
```

Build the kernel and userspace stage first. The kernel image must correspond
to the release recorded in the stage manifest. Build the Pronk
`pronk-capture-mutter-media-live-test` and
`pronk-capture-pattern-client` binaries in `sources/pronk/target/debug`, and
provide a Mutter build directory for its session test runner. The launcher
checks the staged CastKMS, Pronk, and Mutter revisions against the source
submodules. It uses the installed `virtme-run`, QEMU, and host EGL stack.
Set `PRONK_VM_MESA_STAGE` to the `build-mesa` stage to overlay the pinned
runtime in the full assembly test. The stage's source revision and Mesa
version are checked before booting.

For a GPU-only capture check, also build
`pronk-capture-mutter-gpu-live-test` and set `PRONK_VM_TEST=gpu-capture` with
`PRONK_VM_COPY_MODE=zero-copy`. This test allocates one Vulkan-owned
destination, registers it with capture, and checks that three requests
complete with advancing timestamps. A test-only Vulkan readback then checks
the center pixel for nonblack content and a changing shade; the production
capture path does not map pixels on the CPU. The probe does not encode video
or contact a receiver. Run `run-gl-venus` separately to check a
GLES-produced buffer's pixels after Venus import.

`PRONK_VM_COPY_MODE=primary-gpu-gpu` is also accepted by the GPU capture
probe to isolate Mutter's GPU copy from its direct import path.

If the staged Pronk source does not contain the probe, set
`PRONK_VM_PRONK_BIN_DIR` to the directory containing a separately built probe
and matching `pronk-capture-pattern-client`. The installed service and staged
source revisions remain unchanged; the override affects test executables only.

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
Set `PRONK_VM_USE_MUTTER_BUILD=1` to load `libmutter-51.so` from the specified
`PRONK_VM_MUTTER_BUILD` inside the guest while leaving the staged extension
unchanged. This tests local Mutter changes against the packaged assembly; it
does not qualify the pinned staged revision.
`PRONK_VM_USE_FBOS=0` or `1` overrides Mutter's FBO choice for diagnosis;
the default `auto` tests Mutter's own policy.
`PRONK_VM_PATTERN=shm` uses a direct Wayland shared-memory client to isolate
Mutter's texture upload and rendering from GTK; it requires `gpu-capture`.
`PRONK_VM_DISABLE_VIRTIO_OUTPUT=1` disconnects the guest's virtio display
while retaining virtio GPU rendering.
Set `PRONK_VM_DRM_DEBUG=0x1ff` when diagnosing a kernel DRM rejection; the
default mask is `0`, and the guest's `dmesg` is saved as `kernel.log`.
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
