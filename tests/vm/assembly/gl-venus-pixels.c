// SPDX-License-Identifier: GPL-2.0-or-later
// Check pixels across a guest GLES/GBM producer and a Venus Vulkan consumer.
#define _GNU_SOURCE
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <fcntl.h>
#include <gbm.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vulkan/vulkan.h>

#define WIDTH 64
#define HEIGHT 64
#define CHECK(condition, message) do { \
	if (!(condition)) { fprintf(stderr, "FAIL: %s\n", message); return 1; } \
} while (0)
#define VK_CHECK(call) do { \
	VkResult result_ = (call); \
	if (result_ != VK_SUCCESS) { \
		fprintf(stderr, "FAIL: %s returned %d\n", #call, result_); \
		return 1; \
	} \
} while (0)

static int memory_type(VkPhysicalDeviceMemoryProperties properties,
		       uint32_t bits, VkMemoryPropertyFlags required)
{
	for (uint32_t i = 0; i < properties.memoryTypeCount; i++)
		if ((bits & (1u << i)) &&
		    (properties.memoryTypes[i].propertyFlags & required) == required)
			return i;
	return -1;
}

int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	CHECK(argc == 2, "usage: gl-venus-pixels RENDER_NODE");
	int render_fd = open(argv[1], O_RDWR | O_CLOEXEC);
	CHECK(render_fd >= 0, "open render node");
	struct gbm_device *gbm = gbm_create_device(render_fd);
	CHECK(gbm, "create GBM device");
	EGLDisplay display = eglGetPlatformDisplay(EGL_PLATFORM_GBM_KHR, gbm, NULL);
	CHECK(display != EGL_NO_DISPLAY && eglInitialize(display, NULL, NULL),
	      "initialize EGL on virtio GPU");
	EGLint count;
	EGLConfig config;
	const EGLint config_attrs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_NONE,
	};
	CHECK(eglChooseConfig(display, config_attrs, &config, 1, &count) && count,
	      "choose EGL window config");
	CHECK(eglBindAPI(EGL_OPENGL_ES_API), "bind GLES API");
	struct gbm_surface *surface = gbm_surface_create(gbm, WIDTH, HEIGHT,
		GBM_FORMAT_ARGB8888, GBM_BO_USE_RENDERING | GBM_BO_USE_SCANOUT);
	CHECK(surface, "create GBM surface");
	EGLSurface window = eglCreateWindowSurface(display, config,
		(EGLNativeWindowType)surface, NULL);
	CHECK(window != EGL_NO_SURFACE, "create EGL window");
	const EGLint context_attrs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT,
		context_attrs);
	CHECK(context != EGL_NO_CONTEXT &&
	      eglMakeCurrent(display, window, window, context), "create GLES context");
	const char *gl_renderer = (const char *)glGetString(GL_RENDERER);
	CHECK(gl_renderer && strstr(gl_renderer, "virgl"),
	      "GLES did not select virgl");
	printf("GLES renderer: %s\n", gl_renderer);
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_SCISSOR_TEST);
	glScissor(0, 0, WIDTH / 2, HEIGHT / 2);
	glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
	glFinish();
	CHECK(glGetError() == GL_NO_ERROR, "clear two colors in GLES");
	CHECK(eglSwapBuffers(display, window), "publish GBM front buffer");
	struct gbm_bo *front = gbm_surface_lock_front_buffer(surface);
	CHECK(front, "lock GL-rendered GBM buffer");
	uint64_t modifier = gbm_bo_get_modifier(front);
	uint32_t stride = gbm_bo_get_stride(front);
	int dma_fd = gbm_bo_get_fd(front);
	CHECK(dma_fd >= 0, "export GL-rendered DMA-BUF");
	struct stat stat;
	CHECK(fstat(dma_fd, &stat) == 0, "stat DMA-BUF");
	printf("GBM modifier: 0x%016llx, stride: %u, size: %lld\n",
	       (unsigned long long)modifier, stride, (long long)stat.st_size);
	CHECK(stat.st_size >= (off_t)(stride * HEIGHT), "DMA-BUF holds image rows");

	VkApplicationInfo application = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.apiVersion = VK_API_VERSION_1_2,
	};
	VkInstanceCreateInfo instance_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &application,
	};
	VkInstance instance;
	VK_CHECK(vkCreateInstance(&instance_info, NULL, &instance));
	uint32_t physical_count = 0;
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physical_count, NULL));
	CHECK(physical_count > 0 && physical_count <= 16, "find Vulkan GPU");
	VkPhysicalDevice physicals[16];
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physical_count, physicals));
	VkPhysicalDevice physical = physicals[0];
	VkPhysicalDeviceProperties physical_properties;
	vkGetPhysicalDeviceProperties(physical, &physical_properties);
	CHECK(strstr(physical_properties.deviceName, "Virtio-GPU Venus"),
	      "Vulkan did not select Venus");
	printf("Vulkan device: %s\n", physical_properties.deviceName);
	uint32_t family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical, &family_count, NULL);
	CHECK(family_count > 0 && family_count <= 16, "find Vulkan queue families");
	VkQueueFamilyProperties families[16];
	vkGetPhysicalDeviceQueueFamilyProperties(physical, &family_count, families);
	uint32_t family = 0;
	while (family < family_count &&
	       !(families[family].queueFlags & VK_QUEUE_TRANSFER_BIT))
		family++;
	CHECK(family < family_count, "find transfer queue");
	float priority = 1.0f;
	VkDeviceQueueCreateInfo queue_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = family,
		.queueCount = 1,
		.pQueuePriorities = &priority,
	};
	const char *extensions[] = {
		VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
		VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME,
		VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME,
		VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME,
	};
	VkDeviceCreateInfo device_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queue_info,
		.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]),
		.ppEnabledExtensionNames = extensions,
	};
	VkDevice device;
	VK_CHECK(vkCreateDevice(physical, &device_info, NULL, &device));
	VkQueue queue;
	vkGetDeviceQueue(device, family, 0, &queue);
	VkSubresourceLayout plane = { .offset = 0, .rowPitch = stride };
	VkImageDrmFormatModifierExplicitCreateInfoEXT drm_modifier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT,
		.drmFormatModifier = modifier,
		.drmFormatModifierPlaneCount = 1,
		.pPlaneLayouts = &plane,
	};
	VkExternalMemoryImageCreateInfo external = {
		.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
		.pNext = &drm_modifier,
		.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
	};
	VkImageCreateInfo image_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = &external,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = VK_FORMAT_B8G8R8A8_UNORM,
		.extent = { WIDTH, HEIGHT, 1 },
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT,
		.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};
	VkImage image;
	VK_CHECK(vkCreateImage(device, &image_info, NULL, &image));
	VkMemoryRequirements image_requirements;
	vkGetImageMemoryRequirements(device, image, &image_requirements);
	CHECK(image_requirements.size <= (VkDeviceSize)stat.st_size,
	      "DMA-BUF meets Vulkan image size");
	PFN_vkGetMemoryFdPropertiesKHR fd_properties_fn =
		(PFN_vkGetMemoryFdPropertiesKHR)vkGetDeviceProcAddr(
			device, "vkGetMemoryFdPropertiesKHR");
	CHECK(fd_properties_fn, "load DMA-BUF properties query");
	VkMemoryFdPropertiesKHR fd_properties = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR,
	};
	VK_CHECK(fd_properties_fn(device, VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
				dma_fd, &fd_properties));
	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(physical, &memory_properties);
	int image_type = memory_type(memory_properties,
		image_requirements.memoryTypeBits & fd_properties.memoryTypeBits, 0);
	CHECK(image_type >= 0, "find compatible DMA-BUF memory type");
	int imported_fd = dup(dma_fd);
	CHECK(imported_fd >= 0, "duplicate DMA-BUF for Vulkan");
	VkImportMemoryFdInfoKHR import = {
		.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR,
		.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
		.fd = imported_fd,
	};
	VkMemoryDedicatedAllocateInfo dedicated = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
		.pNext = &import,
		.image = image,
	};
	VkMemoryAllocateInfo image_allocation = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = &dedicated,
		.allocationSize = stat.st_size,
		.memoryTypeIndex = image_type,
	};
	VkDeviceMemory image_memory;
	VkResult import_result = vkAllocateMemory(device, &image_allocation, NULL,
		&image_memory);
	if (import_result != VK_SUCCESS)
		close(imported_fd);
	VK_CHECK(import_result);
	VK_CHECK(vkBindImageMemory(device, image, image_memory, 0));

	VkBufferCreateInfo buffer_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = WIDTH * HEIGHT * 4,
		.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};
	VkBuffer buffer;
	VK_CHECK(vkCreateBuffer(device, &buffer_info, NULL, &buffer));
	VkMemoryRequirements buffer_requirements;
	vkGetBufferMemoryRequirements(device, buffer, &buffer_requirements);
	int buffer_type = memory_type(memory_properties,
		buffer_requirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	CHECK(buffer_type >= 0, "find host-visible staging memory");
	VkMemoryAllocateInfo buffer_allocation = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = buffer_requirements.size,
		.memoryTypeIndex = buffer_type,
	};
	VkDeviceMemory buffer_memory;
	VK_CHECK(vkAllocateMemory(device, &buffer_allocation, NULL, &buffer_memory));
	VK_CHECK(vkBindBufferMemory(device, buffer, buffer_memory, 0));
	VkCommandPoolCreateInfo pool_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.queueFamilyIndex = family,
	};
	VkCommandPool pool;
	VK_CHECK(vkCreateCommandPool(device, &pool_info, NULL, &pool));
	VkCommandBufferAllocateInfo command_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};
	VkCommandBuffer command;
	VK_CHECK(vkAllocateCommandBuffers(device, &command_info, &command));
	VkCommandBufferBeginInfo begin = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};
	VK_CHECK(vkBeginCommandBuffer(command, &begin));
	VkImageMemoryBarrier acquire = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_GENERAL,
		.newLayout = VK_IMAGE_LAYOUT_GENERAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT,
		.dstQueueFamilyIndex = family,
		.image = image,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.levelCount = 1, .layerCount = 1,
		},
	};
	vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &acquire);
	VkBufferImageCopy region = {
		.imageSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.layerCount = 1,
		},
		.imageExtent = { WIDTH, HEIGHT, 1 },
	};
	vkCmdCopyImageToBuffer(command, image, VK_IMAGE_LAYOUT_GENERAL, buffer,
		1, &region);
	VkBufferMemoryBarrier host = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_HOST_READ_BIT,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.buffer = buffer,
		.size = VK_WHOLE_SIZE,
	};
	vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_HOST_BIT, 0, 0, NULL, 1, &host, 0, NULL);
	VK_CHECK(vkEndCommandBuffer(command));
	VkSubmitInfo submit = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &command,
	};
	VK_CHECK(vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE));
	VK_CHECK(vkQueueWaitIdle(queue));
	void *pixels;
	VK_CHECK(vkMapMemory(device, buffer_memory, 0, buffer_info.size, 0,
			    &pixels));
	int red = 0, green = 0;
	for (int y = 8; y < HEIGHT; y += HEIGHT / 2) {
		for (int x = 8; x < WIDTH; x += WIDTH / 2) {
			const uint8_t *sample = (const uint8_t *)pixels +
				(y * WIDTH + x) * 4;
			printf("Vulkan pixel (%d,%d) BGRA: %02x %02x %02x %02x\n",
			       x, y, sample[0], sample[1], sample[2], sample[3]);
			if (sample[0] == 0 && sample[1] == 0 &&
			    sample[2] == 255 && sample[3] == 255)
				red++;
			if (sample[0] == 0 && sample[1] == 255 &&
			    sample[2] == 0 && sample[3] == 255)
				green++;
		}
	}
	CHECK(red == 3 && green == 1,
	      "Vulkan readback matches the latest two-color GLES image");
	puts("PASS: GL-rendered DMA-BUF pixels reached Venus");
	vkUnmapMemory(device, buffer_memory);
	vkDestroyCommandPool(device, pool, NULL);
	vkDestroyBuffer(device, buffer, NULL);
	vkFreeMemory(device, buffer_memory, NULL);
	vkDestroyImage(device, image, NULL);
	vkFreeMemory(device, image_memory, NULL);
	vkDestroyDevice(device, NULL);
	vkDestroyInstance(instance, NULL);
	close(dma_fd);
	gbm_surface_release_buffer(surface, front);
	eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(display, context);
	eglDestroySurface(display, window);
	gbm_surface_destroy(surface);
	eglTerminate(display);
	gbm_device_destroy(gbm);
	close(render_fd);
	return 0;
}
