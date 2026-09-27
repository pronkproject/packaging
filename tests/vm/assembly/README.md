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
Gallium, GBM, and EGL libraries together. The build includes virgl, Zink,
and softpipe so it can also exercise the full GPU capture assembly. The
stage stays separate from the host graphics stack:

```sh
tests/vm/assembly/build-mesa
```

For virtio-gpu VA video, use the Mesa stage and build the
pinned QEMU and virglrenderer sources separately. The video build retains
its own QEMU executable and shared library; it does not replace the host
graphics stack. Keep the default build job count at two on memory-limited
hosts.

```sh
tests/vm/assembly/build-virtio-video
```

Set `PRONK_VM_MESA_STAGE` and `PRONK_VM_VIDEO_BUILD` to the paths printed by
those commands. `run-virtio-video` boots a two-CPU VM, encodes a moving test
pattern through the guest virtio-gpu VA driver, and requires every output
frame to decode on the host. It also sends red and blue RGB and NV12 images
through VA postprocessing before encoding, then checks every decoded frame's
center pixel. Those cases exercise conversion separately from direct encoder
upload. It checks H.264 Constrained Baseline, Main and High by default;
`PRONK_VM_VIDEO_PROFILE` can select one profile while diagnosing a failure.
The script confines the pinned QEMU and virglrenderer to that VM. When the
host has a GBM render node, QEMU offers its video capability automatically.

```sh
PRONK_VM_KERNEL_IMAGE=/path/to/bzImage \
PRONK_VM_MESA_STAGE=/path/to/video/mesa/stage \
PRONK_VM_VIDEO_BUILD=/path/to/video/build \
tests/vm/assembly/run-virtio-video
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
`pronk-capture-mutter-media-live-test`,
`pronk-capture-mutter-gpu-live-test`, and `pronk-capture-pattern-client`
binaries in `sources/pronk/target/debug`, and
provide a Mutter build directory for its session test runner. The launcher
checks the staged CastKMS, Pronk, and Mutter revisions against the source
submodules. It uses the installed `virtme-run`, QEMU, and host EGL stack.
Set `PRONK_VM_MESA_STAGE` to the `build-mesa` stage to overlay the pinned
runtime in the full assembly test. The stage's source revision and Mesa
version are checked before booting.

For a GPU-only capture check, set `PRONK_VM_TEST=gpu-capture` with
`PRONK_VM_COPY_MODE=zero-copy`. This test allocates one Vulkan-owned
destination, registers it with capture, and checks that three requests
complete with advancing timestamps. A test-only Vulkan readback then checks
the center pixel for nonblack content and a changing shade; the production
capture path does not map pixels on the CPU. The probe does not encode video
or contact a receiver. Run `run-gl-venus` separately to check a
GLES-produced buffer's pixels after Venus import.
Both virtio-gpu and CastKMS call their first connector `Virtual-1`.
Do not pass `video=Virtual-1:d`: it disables the CastKMS test output too.
The guest retains both displays and the test checks the CastKMS target.

For a full GPU media test, set `PRONK_VM_TEST=media-gpu` alongside the
video-enabled Mesa stage and QEMU/virglrenderer build. The launcher selects
Mutter's zero-copy path and starts the separate renderer daemon. The guest
test waits for Mutter to select its renderer, allocates GPU capture
destinations, transports
their DMA-BUFs through private PipeWire, and uses the selected virtio-gpu VA
H.264 encoder. It verifies decoded changing pixels and the media graph's
encoder, memory path, and render device. This probe uses test-only CPU
decoding to check the encoded result; it does not copy raw capture frames to
the CPU for encoding. Add `PRONK_VM_RECEIVER=HOST:PORT` only when a receiver
may be interrupted. The test receives only final-image capture authority;
the daemon owns its renderer endpoint independently.

`PRONK_VM_TEST=service-gpu` exercises the installed Pronk service path rather
than the live capture probe. It boots systemd in the guest, creates an active
local Wayland login session, starts the real user units for `pronkd`, private
PipeWire, WirePlumber, and the socket-activated Chromiacast backend, then uses
`pronkctl add-display` and `gdctl` to route the CastKMS monitor. It runs a
changing Wayland pattern for twelve seconds and requires a persistent Running
media state, GPU-owned final-image capture, VA H.264 with DMA-BUF input, and
advancing receiver acknowledgements. The backend authenticates the receiver
at `PRONK_VM_RECEIVER=IP:PORT` over a direct connection; this avoids relying
on multicast discovery through QEMU's user-mode NAT. The service test runs
the daemon and backend binaries from the rebuilt system extension. The
guest presents the staged backend registry as root-owned because virtme's 9p
mount exposes the host's rootless-build UID, which production Pronk correctly
rejects. A pass establishes transport and service integration, not visual
playback on the television.

`PRONK_VM_VIDEO_RATE` selects 30 (default) or 60 frames per second for capture,
encoding, and the receiver offer. With a receiver, the probe requires both
encoded and acknowledged throughput to reach at least 95% of that rate during
its 15-second observation window. The window starts after a two-second warmup
and the first receiver acknowledgement; the probe reports the excluded warmup
separately and ends the window before stopping capture or media. Receiver
acknowledgements do not establish how many distinct frames the television
displays.

For an interactive guest desktop, use `PRONK_VM_TEST=service-gpu` with
`PRONK_VM_INTERACTIVE=1` and a receiver. The guest starts GNOME Shell rather
than the finite test pattern inside a guest-only PAM/logind session. It stops
the guest's virtual console before Shell takes the virtio DRM device; casting
continues until the launcher is stopped. QEMU serves the guest's local display
over VNC on `127.0.0.1:5901`. Open
`vncviewer localhost:1` on the host to control the guest with the mouse and
keyboard; closing the viewer does not stop the cast. The VNC listener is not
exposed on the network. From another machine, forward port 5901 over SSH
before connecting a viewer. The CastKMS display remains a separate guest
monitor, so the VNC window and receiver show different parts of the desktop
unless the guest's display arrangement is changed.

`PRONK_VM_COPY_MODE=primary-gpu-gpu` is also accepted by the GPU capture
probe to isolate Mutter's GPU copy from its direct import path.

`PRONK_VM_PRONK_BIN_DIR` can select separately built test executables without
changing the installed service or staged source revisions.

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
Mutter's texture upload and rendering from GTK. In the media test it alternates
the two gray values checked by the decoded-pixel oracle.
Set `PRONK_VM_DRM_DEBUG=0x1ff` when diagnosing a kernel DRM rejection; the
default mask is `0`, and the guest's `dmesg` is saved as `kernel.log`.
The guest requires the kernel release in the stage manifest to match its
running kernel. Set `PRONK_VM_ALLOW_KERNEL_RELEASE_MISMATCH=1` only for a
deliberate local build from the same kernel sources with a different release
suffix; the guest reports that mismatch. Do not use that override to qualify a
publishable assembly.

`PRONK_VM_USE_RENDERER=0` leaves the delegated renderer stopped so the VM can
exercise the built-in HOST executor separately. The default is `1`. A HOST
media pass does not qualify GPU composition or DMA-BUF delivery.

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
