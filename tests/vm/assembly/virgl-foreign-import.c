// SPDX-License-Identifier: MIT
#define _GNU_SOURCE
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <drm_fourcc.h>
#include <errno.h>
#include <fcntl.h>
#include <gbm.h>
#include <linux/dma-heap.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <virtgpu_drm.h>
#include <xf86drm.h>

static PFNEGLCREATEIMAGEKHRPROC create_image;
static PFNEGLDESTROYIMAGEKHRPROC destroy_image;
static PFNGLEGLIMAGETARGETTEXTURE2DOESPROC image_target;

static void require(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "%s errno=%d egl=%x gl=%x\n", message,
                errno, eglGetError(), glGetError());
        exit(1);
    }
}

static unsigned char clear_and_read(GLuint texture, const char *label,
                                    unsigned char shade)
{
    GLuint framebuffer;
    unsigned char pixel[4] = {0xde, 0xad, 0xbe, 0xef};
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                          GL_TEXTURE_2D, texture, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glViewport(0, 0, 64, 64);
    glClearColor(shade / 255.0f, shade / 255.0f, shade / 255.0f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glFinish();
    glReadPixels(32, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    fprintf(stderr, "%s status=%x error=%x expected=%u pixel=%u,%u,%u,%u\n",
            label, status, glGetError(), shade, pixel[0], pixel[1], pixel[2], pixel[3]);
    glDeleteFramebuffers(1, &framebuffer);
    return pixel[0];
}

static GLuint import_image(EGLDisplay display, int fd, uint32_t stride,
                           const char *label)
{
    EGLint attrs[] = {
        EGL_WIDTH, 64, EGL_HEIGHT, 64,
        EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_XRGB8888,
        EGL_DMA_BUF_PLANE0_FD_EXT, fd,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, stride,
        EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT, 0,
        EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT, 0,
        EGL_NONE,
    };
    EGLImageKHR image = create_image(display, EGL_NO_CONTEXT,
                                    EGL_LINUX_DMA_BUF_EXT, NULL, attrs);
    fprintf(stderr, "%s image=%p egl_error=%x\n", label, image, eglGetError());
    if (image == EGL_NO_IMAGE_KHR)
        return 0;
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    image_target(GL_TEXTURE_2D, image);
    fprintf(stderr, "%s bind_error=%x\n", label, glGetError());
    destroy_image(display, image);
    return texture;
}

int main(void)
{
    setbuf(stderr, NULL);
    int fd = open("/dev/dri/renderD128", O_RDWR | O_CLOEXEC);
    require(fd >= 0, "render node");
    int info_fd = open("/dev/dri/renderD128", O_RDWR | O_CLOEXEC);
    require(info_fd >= 0, "resource information node");
    struct gbm_device *gbm = gbm_create_device(fd);
    require(gbm != NULL, "gbm device");
    EGLDisplay display = eglGetPlatformDisplay(EGL_PLATFORM_GBM_KHR, gbm, NULL);
    require(eglInitialize(display, NULL, NULL), "egl init");
    require(eglBindAPI(EGL_OPENGL_ES_API), "bind GLES");
    EGLint ctx_attrs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    EGLContext context = eglCreateContext(display, EGL_NO_CONFIG_KHR,
                                         EGL_NO_CONTEXT, ctx_attrs);
    require(context != EGL_NO_CONTEXT, "create context");
    require(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context), "bind context");
    fprintf(stderr, "Renderer: %s\n", glGetString(GL_RENDERER));
    create_image = (void *)eglGetProcAddress("eglCreateImageKHR");
    destroy_image = (void *)eglGetProcAddress("eglDestroyImageKHR");
    image_target = (void *)eglGetProcAddress("glEGLImageTargetTexture2DOES");
    require(create_image && destroy_image && image_target, "image API");
    GLuint own;
    glGenTextures(1, &own);
    glBindTexture(GL_TEXTURE_2D, own);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    require(clear_and_read(own, "before imports", 73) == 73,
            "initial GL rendering");

    struct gbm_bo *bo = gbm_bo_create(gbm, 64, 64, GBM_FORMAT_XRGB8888,
                                    GBM_BO_USE_RENDERING | GBM_BO_USE_LINEAR);
    require(bo != NULL, "create host buffer");
    int host_fd = gbm_bo_get_fd(bo);
    require(host_fd >= 0, "export host buffer");
    GLuint imported_host = import_image(display, host_fd, gbm_bo_get_stride(bo), "host buffer");
    require(imported_host != 0, "host image import");
    require(clear_and_read(imported_host, "host buffer", 104) == 104,
            "host image rendering");
    require(clear_and_read(own, "after host import", 135) == 135,
            "GL rendering after host import");

    int heap = open("/dev/dma_heap/system", O_RDWR | O_CLOEXEC);
    require(heap >= 0, "system heap");
    struct dma_heap_allocation_data alloc = {
        .len = 64 * 64 * 4, .fd_flags = O_RDWR | O_CLOEXEC,
    };
    require(ioctl(heap, DMA_HEAP_IOCTL_ALLOC, &alloc) == 0, "allocate guest buffer");
    uint32_t handle = 0;
    require(drmPrimeFDToHandle(info_fd, alloc.fd, &handle) == 0, "import guest GEM");
    struct drm_virtgpu_resource_info info = {.bo_handle = handle};
    require(ioctl(info_fd, DRM_IOCTL_VIRTGPU_RESOURCE_INFO, &info) == 0, "guest resource info");
    fprintf(stderr, "Guest resource: id=%u handle=%u blob_mem=%u size=%u\n",
            info.res_handle, handle, info.blob_mem, info.size);
    GLuint imported_guest = import_image(display, alloc.fd, 64 * 4, "guest buffer");
    if (imported_guest)
        clear_and_read(imported_guest, "guest buffer", 166);
    unsigned char after_guest = clear_and_read(own, "after guest import", 197);

    require(info.blob_mem == VIRTGPU_BLOB_MEM_GUEST,
            "guest blob memory type");
    require(imported_guest == 0, "guest image rejected");
    require(after_guest == 197, "unrelated GL rendering survives rejection");

    glDeleteTextures(1, &own);
    glDeleteTextures(1, &imported_host);
    glDeleteTextures(1, &imported_guest);
    drmCloseBufferHandle(info_fd, handle);
    close(info_fd);
    close(alloc.fd);
    close(heap);
    close(host_fd);
    gbm_bo_destroy(bo);
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(display, context);
    eglTerminate(display);
    gbm_device_destroy(gbm);
    close(fd);
    return 0;
}
