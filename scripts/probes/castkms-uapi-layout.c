/* SPDX-License-Identifier: MIT */

#include <stddef.h>
#include <stdio.h>

#include <drm/castkms_drm.h>

#define LAYOUT(type) \
	printf("layout " #type " %zu %zu\n", sizeof(struct type), \
	       _Alignof(struct type))
#define FIELD(type, member) \
	printf("field " #type "." #member " %zu\n", \
	       offsetof(struct type, member))
#define VALUE(name) printf("value " #name " %llu\n", (unsigned long long)(name))
#define IOCTL(name) printf("ioctl " #name " %lu\n", (unsigned long)(name))

int main(void)
{
	LAYOUT(drm_castkms_renderer_query);
	LAYOUT(drm_castkms_renderer_configure);
	LAYOUT(drm_castkms_renderer_publish);
	LAYOUT(drm_castkms_renderer_publish_result);
	LAYOUT(drm_castkms_renderer_withdraw);
	LAYOUT(drm_castkms_renderer_memory_plane);
	LAYOUT(drm_castkms_renderer_release_job);
	LAYOUT(drm_castkms_renderer_acquire_output);
	LAYOUT(drm_castkms_renderer_output);
	LAYOUT(drm_castkms_renderer_release_output);
	LAYOUT(drm_castkms_renderer_constraints);
	LAYOUT(drm_castkms_renderer_constraints_format);
	LAYOUT(drm_castkms_renderer_acquire_job);
	LAYOUT(drm_castkms_renderer_register_image);
	LAYOUT(drm_castkms_renderer_unregister_image);

	FIELD(drm_castkms_renderer_query, version);
	FIELD(drm_castkms_renderer_query, state);
	FIELD(drm_castkms_renderer_query, constraints_id);
	FIELD(drm_castkms_renderer_query, reserved);
	FIELD(drm_castkms_renderer_configure, constraints);
	FIELD(drm_castkms_renderer_configure, constraints_size);
	FIELD(drm_castkms_renderer_configure, flags);
	FIELD(drm_castkms_renderer_configure, width);
	FIELD(drm_castkms_renderer_configure, height);
	FIELD(drm_castkms_renderer_configure, reserved);
	FIELD(drm_castkms_renderer_publish, result);
	FIELD(drm_castkms_renderer_publish, ready_fence_fd);
	FIELD(drm_castkms_renderer_publish, flags);
	FIELD(drm_castkms_renderer_publish, reserved);
	FIELD(drm_castkms_renderer_publish_result, constraints_id);
	FIELD(drm_castkms_renderer_publish_result, reserved);
	FIELD(drm_castkms_renderer_withdraw, flags);
	FIELD(drm_castkms_renderer_withdraw, reserved);
	FIELD(drm_castkms_renderer_memory_plane, dma_buf_fd);
	FIELD(drm_castkms_renderer_memory_plane, pitch);
	FIELD(drm_castkms_renderer_memory_plane, offset);
	FIELD(drm_castkms_renderer_memory_plane, reserved);
	FIELD(drm_castkms_renderer_release_job, job_id);
	FIELD(drm_castkms_renderer_release_job, release_fence_fd);
	FIELD(drm_castkms_renderer_release_job, kind);
	FIELD(drm_castkms_renderer_release_job, flags);
	FIELD(drm_castkms_renderer_release_job, reserved);
	FIELD(drm_castkms_renderer_acquire_output, result);
	FIELD(drm_castkms_renderer_acquire_output, image_id);
	FIELD(drm_castkms_renderer_acquire_output, flags);
	FIELD(drm_castkms_renderer_acquire_output, reserved);
	FIELD(drm_castkms_renderer_acquire_output, padding);
	FIELD(drm_castkms_renderer_output, job_id);
	FIELD(drm_castkms_renderer_output, image_id);
	FIELD(drm_castkms_renderer_output, width);
	FIELD(drm_castkms_renderer_output, height);
	FIELD(drm_castkms_renderer_output, format);
	FIELD(drm_castkms_renderer_output, memory_plane_count);
	FIELD(drm_castkms_renderer_output, modifier);
	FIELD(drm_castkms_renderer_output, dma_buf_fd);
	FIELD(drm_castkms_renderer_output, pitch);
	FIELD(drm_castkms_renderer_output, offset);
	FIELD(drm_castkms_renderer_output, reserved);
	FIELD(drm_castkms_renderer_release_output, job_id);
	FIELD(drm_castkms_renderer_release_output, release_fence_fd);
	FIELD(drm_castkms_renderer_release_output, kind);
	FIELD(drm_castkms_renderer_release_output, flags);
	FIELD(drm_castkms_renderer_release_output, reserved);
	FIELD(drm_castkms_renderer_constraints, version);
	FIELD(drm_castkms_renderer_constraints, kind);
	FIELD(drm_castkms_renderer_constraints, flags);
	FIELD(drm_castkms_renderer_constraints, format_count);
	FIELD(drm_castkms_renderer_constraints, max_output);
	FIELD(drm_castkms_renderer_constraints, max_source);
	FIELD(drm_castkms_renderer_constraints, min_scale);
	FIELD(drm_castkms_renderer_constraints, max_scale);
	FIELD(drm_castkms_renderer_constraints, max_planes);
	FIELD(drm_castkms_renderer_constraints, max_roles);
	FIELD(drm_castkms_renderer_constraints, max_color_operations);
	FIELD(drm_castkms_renderer_constraints, max_lut_entries);
	FIELD(drm_castkms_renderer_constraints, yuv_encodings);
	FIELD(drm_castkms_renderer_constraints, yuv_ranges);
	FIELD(drm_castkms_renderer_constraints, min_output);
	FIELD(drm_castkms_renderer_constraints, min_source);
	FIELD(drm_castkms_renderer_constraints, reserved);
	FIELD(drm_castkms_renderer_constraints_format, fourcc);
	FIELD(drm_castkms_renderer_constraints_format, memory_plane_count);
	FIELD(drm_castkms_renderer_constraints_format, modifier);
	FIELD(drm_castkms_renderer_constraints_format, flags);
	FIELD(drm_castkms_renderer_constraints_format, roles);
	FIELD(drm_castkms_renderer_constraints_format, width_alignment);
	FIELD(drm_castkms_renderer_constraints_format, height_alignment);
	FIELD(drm_castkms_renderer_constraints_format, pitch_alignment);
	FIELD(drm_castkms_renderer_constraints_format, offset_alignment);
	FIELD(drm_castkms_renderer_constraints_format, min_pitch);
	FIELD(drm_castkms_renderer_constraints_format, max_pitch);
	FIELD(drm_castkms_renderer_constraints_format, reserved);
	FIELD(drm_castkms_renderer_acquire_job, result);
	FIELD(drm_castkms_renderer_acquire_job, target_image_id);
	FIELD(drm_castkms_renderer_acquire_job, capacity);
	FIELD(drm_castkms_renderer_acquire_job, flags);
	FIELD(drm_castkms_renderer_acquire_job, reserved);
	FIELD(drm_castkms_renderer_register_image, image_id);
	FIELD(drm_castkms_renderer_register_image, buffers);
	FIELD(drm_castkms_renderer_register_image, width);
	FIELD(drm_castkms_renderer_register_image, height);
	FIELD(drm_castkms_renderer_register_image, num_buffers);
	FIELD(drm_castkms_renderer_register_image, flags);
	FIELD(drm_castkms_renderer_register_image, reserved);
	FIELD(drm_castkms_renderer_unregister_image, image_id);
	FIELD(drm_castkms_renderer_unregister_image, flags);
	FIELD(drm_castkms_renderer_unregister_image, reserved);
	LAYOUT(drm_castkms_renderer_job);
	FIELD(drm_castkms_renderer_job, version);
	FIELD(drm_castkms_renderer_job, bytes);
	FIELD(drm_castkms_renderer_job, job_id);
	FIELD(drm_castkms_renderer_job, constraints_id);
	FIELD(drm_castkms_renderer_job, content_serial);
	FIELD(drm_castkms_renderer_job, width);
	FIELD(drm_castkms_renderer_job, height);
	FIELD(drm_castkms_renderer_job, plane_count);
	FIELD(drm_castkms_renderer_job, acquire_fence_fd);
	FIELD(drm_castkms_renderer_job, output_color_op_count);
	FIELD(drm_castkms_renderer_job, reserved);
	LAYOUT(drm_castkms_renderer_plane);
	FIELD(drm_castkms_renderer_plane, bytes);
	FIELD(drm_castkms_renderer_plane, role);
	FIELD(drm_castkms_renderer_plane, zpos);
	FIELD(drm_castkms_renderer_plane, format);
	FIELD(drm_castkms_renderer_plane, modifier);
	FIELD(drm_castkms_renderer_plane, width);
	FIELD(drm_castkms_renderer_plane, height);
	FIELD(drm_castkms_renderer_plane, src_x);
	FIELD(drm_castkms_renderer_plane, src_y);
	FIELD(drm_castkms_renderer_plane, src_w);
	FIELD(drm_castkms_renderer_plane, src_h);
	FIELD(drm_castkms_renderer_plane, crtc_x);
	FIELD(drm_castkms_renderer_plane, crtc_y);
	FIELD(drm_castkms_renderer_plane, crtc_w);
	FIELD(drm_castkms_renderer_plane, crtc_h);
	FIELD(drm_castkms_renderer_plane, color_encoding);
	FIELD(drm_castkms_renderer_plane, color_range);
	FIELD(drm_castkms_renderer_plane, memory_plane_count);
	FIELD(drm_castkms_renderer_plane, color_op_count);
	FIELD(drm_castkms_renderer_plane, memory_planes);
	LAYOUT(drm_castkms_renderer_color_op);
	FIELD(drm_castkms_renderer_color_op, kind);
	FIELD(drm_castkms_renderer_color_op, payload_bytes);

	VALUE(DRM_CASTKMS_RENDERER_VERSION);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_VERSION);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_KIND);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_MAX_FORMATS);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_HEADER_BYTES);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_FORMAT_BYTES);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_MAX_BYTES);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_CROP);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_FRACTIONAL);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_POSITION);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_SCALE);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_SRGB);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_PLANE_MATRIX);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_OUTPUT_MATRIX);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_FORMAT_NATIVE);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_FORMAT_IMPORTED);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_FORMAT_EXPLICIT_MODIFIER);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_ROLE_PRIMARY);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_ROLE_OVERLAY);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_ROLE_CURSOR);
	VALUE(DRM_CASTKMS_YUV_ENCODING_BT601);
	VALUE(DRM_CASTKMS_YUV_ENCODING_BT709);
	VALUE(DRM_CASTKMS_YUV_ENCODING_BT2020);
	VALUE(DRM_CASTKMS_YUV_RANGE_LIMITED);
	VALUE(DRM_CASTKMS_YUV_RANGE_FULL);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_YUV_ENCODING_BT601);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_YUV_ENCODING_BT709);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_YUV_ENCODING_BT2020);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_YUV_RANGE_LIMITED);
	VALUE(DRM_CASTKMS_RENDERER_CONSTRAINTS_YUV_RANGE_FULL);
	VALUE(DRM_CASTKMS_RENDERER_RELEASE_NO_ACCESS);
	VALUE(DRM_CASTKMS_RENDERER_RELEASE_CPU_DONE);
	VALUE(DRM_CASTKMS_RENDERER_RELEASE_SUBMITTED);
	VALUE(DRM_CASTKMS_RENDERER_MAX_MEMORY_PLANES);
	VALUE(DRM_CASTKMS_RENDERER_STATE_EMPTY);
	VALUE(DRM_CASTKMS_RENDERER_STATE_CONFIGURED);
	VALUE(DRM_CASTKMS_RENDERER_STATE_PUBLISHING);
	VALUE(DRM_CASTKMS_RENDERER_STATE_PUBLISHED);
	VALUE(DRM_CASTKMS_RENDERER_STATE_WITHDRAWN);
	VALUE(DRM_CASTKMS_RENDERER_JOB_VERSION);
	VALUE(DRM_CASTKMS_RENDERER_JOB_MAX_BYTES);
	VALUE(DRM_CASTKMS_RENDERER_JOB_MAX_PLANES);
	VALUE(DRM_CASTKMS_RENDERER_JOB_MAX_COLOR_OPS);
	VALUE(DRM_CASTKMS_RENDERER_PLANE_PRIMARY);
	VALUE(DRM_CASTKMS_RENDERER_PLANE_OVERLAY);
	VALUE(DRM_CASTKMS_RENDERER_PLANE_CURSOR);
	VALUE(DRM_CASTKMS_RENDERER_COLOR_OP_BYPASS);
	VALUE(DRM_CASTKMS_RENDERER_COLOR_OP_SRGB_EOTF);
	VALUE(DRM_CASTKMS_RENDERER_COLOR_OP_SRGB_INVERSE_EOTF);
	VALUE(DRM_CASTKMS_RENDERER_COLOR_OP_MATRIX);
	VALUE(DRM_CASTKMS_RENDERER_COLOR_OP_LUT);

	IOCTL(DRM_IOCTL_CASTKMS_RENDERER_QUERY);
	IOCTL(DRM_IOCTL_CASTKMS_RENDERER_CONFIGURE);
	IOCTL(DRM_IOCTL_CASTKMS_RENDERER_PUBLISH);
	IOCTL(DRM_IOCTL_CASTKMS_RENDERER_WITHDRAW);
	IOCTL(DRM_IOCTL_CASTKMS_RENDERER_REGISTER_IMAGE);
	IOCTL(DRM_IOCTL_CASTKMS_RENDERER_UNREGISTER_IMAGE);
	IOCTL(DRM_IOCTL_CASTKMS_RENDERER_ACQUIRE_JOB);
	IOCTL(DRM_IOCTL_CASTKMS_RENDERER_RELEASE_JOB);
	IOCTL(DRM_IOCTL_CASTKMS_RENDERER_ACQUIRE_OUTPUT);
	IOCTL(DRM_IOCTL_CASTKMS_RENDERER_RELEASE_OUTPUT);

	return 0;
}
