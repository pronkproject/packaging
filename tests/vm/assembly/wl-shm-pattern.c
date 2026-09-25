// SPDX-License-Identifier: MIT
// A direct Wayland client for isolating compositor rendering from GTK.
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <linux/memfd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>

#include "xdg-shell-client-protocol.h"

struct image {
	struct wl_buffer *buffer;
	uint32_t *pixels;
	size_t length;
	int busy;
};

struct app {
	struct wl_display *display;
	struct wl_compositor *compositor;
	struct wl_shm *shm;
	struct xdg_wm_base *wm_base;
	struct wl_surface *surface;
	struct xdg_surface *xdg_surface;
	struct xdg_toplevel *toplevel;
	int configured;
	int width;
	int height;
	struct image images[2];
};

static void fail(const char *message)
{
	fprintf(stderr, "Wayland pattern: %s: %s\n", message, strerror(errno));
	exit(1);
}

static void image_released(void *data, struct wl_buffer *buffer)
{
	struct image *image = data;
	(void)buffer;
	image->busy = 0;
}

static const struct wl_buffer_listener image_listener = {
	.release = image_released,
};

static void wm_ping(void *data, struct xdg_wm_base *wm_base, uint32_t serial)
{
	(void)data;
	xdg_wm_base_pong(wm_base, serial);
}

static const struct xdg_wm_base_listener wm_listener = {
	.ping = wm_ping,
};

static void surface_configure(void *data, struct xdg_surface *surface,
			      uint32_t serial)
{
	struct app *app = data;
	xdg_surface_ack_configure(surface, serial);
	app->configured = 1;
}

static const struct xdg_surface_listener surface_listener = {
	.configure = surface_configure,
};

static void toplevel_configure(void *data, struct xdg_toplevel *toplevel,
			       int32_t width, int32_t height,
			       struct wl_array *states)
{
	(void)data;
	(void)toplevel;
	(void)width;
	(void)height;
	(void)states;
}

static void toplevel_close(void *data, struct xdg_toplevel *toplevel)
{
	(void)data;
	(void)toplevel;
	exit(0);
}

static const struct xdg_toplevel_listener toplevel_listener = {
	.configure = toplevel_configure,
	.close = toplevel_close,
};

static void registry_global(void *data, struct wl_registry *registry,
			    uint32_t name, const char *interface, uint32_t version)
{
	struct app *app = data;
	if (strcmp(interface, wl_compositor_interface.name) == 0)
		app->compositor = wl_registry_bind(registry, name,
			&wl_compositor_interface, version < 4 ? version : 4);
	else if (strcmp(interface, wl_shm_interface.name) == 0)
		app->shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
	else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
		app->wm_base = wl_registry_bind(registry, name,
			&xdg_wm_base_interface, 1);
}

static void registry_removed(void *data, struct wl_registry *registry,
			     uint32_t name)
{
	(void)data;
	(void)registry;
	(void)name;
}

static const struct wl_registry_listener registry_listener = {
	.global = registry_global,
	.global_remove = registry_removed,
};

static void create_image(struct app *app, struct image *image)
{
	int stride = app->width * 4;
	int fd = memfd_create("pronk-wayland-pattern", MFD_CLOEXEC);
	struct wl_shm_pool *pool;
	if (fd < 0)
		fail("create shared image");
	image->length = (size_t)stride * app->height;
	if (ftruncate(fd, (off_t)image->length) < 0)
		fail("size shared image");
	image->pixels = mmap(NULL, image->length, PROT_READ | PROT_WRITE,
		MAP_SHARED, fd, 0);
	if (image->pixels == MAP_FAILED)
		fail("map shared image");
	pool = wl_shm_create_pool(app->shm, fd, (int)image->length);
	image->buffer = wl_shm_pool_create_buffer(pool, 0, app->width,
		app->height, stride, WL_SHM_FORMAT_XRGB8888);
	wl_shm_pool_destroy(pool);
	close(fd);
	if (!image->buffer || wl_buffer_add_listener(image->buffer,
		&image_listener, image) < 0)
		fail("create Wayland buffer");
}

int main(int argc, char **argv)
{
	struct app app = { 0 };
	struct wl_registry *registry;
	int frame = 0;
	if (argc != 3)
		fail("expected width and height");
	app.width = atoi(argv[1]);
	app.height = atoi(argv[2]);
	if (app.width <= 0 || app.height <= 0 || app.width > 4096 ||
	    app.height > 4096)
		fail("invalid dimensions");
	app.display = wl_display_connect(NULL);
	if (!app.display)
		fail("connect display");
	registry = wl_display_get_registry(app.display);
	wl_registry_add_listener(registry, &registry_listener, &app);
	if (wl_display_roundtrip(app.display) < 0 || !app.compositor ||
	    !app.shm || !app.wm_base)
		fail("find Wayland interfaces");
	xdg_wm_base_add_listener(app.wm_base, &wm_listener, &app);
	app.surface = wl_compositor_create_surface(app.compositor);
	app.xdg_surface = xdg_wm_base_get_xdg_surface(app.wm_base, app.surface);
	xdg_surface_add_listener(app.xdg_surface, &surface_listener, &app);
	app.toplevel = xdg_surface_get_toplevel(app.xdg_surface);
	xdg_toplevel_add_listener(app.toplevel, &toplevel_listener, &app);
	xdg_toplevel_set_title(app.toplevel, "Pronk direct Wayland pattern");
	xdg_toplevel_set_fullscreen(app.toplevel, NULL);
	wl_surface_commit(app.surface);
	while (!app.configured)
		if (wl_display_roundtrip(app.display) < 0)
			fail("wait for initial configure");
	create_image(&app, &app.images[0]);
	create_image(&app, &app.images[1]);
	for (;;) {
		struct image *image = &app.images[frame % 2];
		uint32_t shade = frame % 2 ? 0x68 : 0x49;
		uint32_t pixel = 0xff000000u | (shade << 16) |
			(shade << 8) | shade;
		if (wl_display_roundtrip(app.display) < 0)
			fail("dispatch Wayland events");
		if (!image->busy) {
			for (size_t i = 0; i < image->length / 4; i++)
				image->pixels[i] = pixel;
			image->busy = 1;
			wl_surface_attach(app.surface, image->buffer, 0, 0);
			wl_surface_damage_buffer(app.surface, 0, 0,
				app.width, app.height);
			wl_surface_commit(app.surface);
			fprintf(stderr, "Wayland pattern submitted shade %02x\n", shade);
		}
		frame++;
		usleep(700000);
	}
}
