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

int main(void)
{
	LAYOUT(drm_castkms_capture_format);
	LAYOUT(drm_castkms_capture_query_caps);
	LAYOUT(drm_castkms_capture_start);
	LAYOUT(drm_castkms_capture_stop);
	LAYOUT(drm_castkms_capture_register_buffer);
	LAYOUT(drm_castkms_capture_unregister_buffer);
	LAYOUT(drm_castkms_capture_queue_buffer);
	LAYOUT(drm_castkms_capture_set_output_edid);
	LAYOUT(drm_castkms_capture_attach_monitor);
	LAYOUT(drm_castkms_capture_detach_monitor);
	LAYOUT(drm_castkms_get_grant);
	LAYOUT(drm_castkms_get_output);
	LAYOUT(drm_event_castkms_grant_revoked);
	LAYOUT(drm_event_castkms_grant_state);
	LAYOUT(drm_event_castkms_capture_frame);
	LAYOUT(drm_castkms_cec_query_caps);
	LAYOUT(drm_castkms_cec_bind_transport);
	LAYOUT(drm_castkms_cec_unbind_transport);
	LAYOUT(drm_castkms_cec_set_transport_state);
	LAYOUT(drm_castkms_cec_tx_complete);
	LAYOUT(drm_castkms_cec_receive);
	LAYOUT(drm_castkms_cec_get_state);
	LAYOUT(drm_castkms_cec_event_tx);

	FIELD(drm_castkms_capture_query_caps, formats_ptr);
	FIELD(drm_castkms_capture_start, mode_generation);
	FIELD(drm_castkms_capture_register_buffer, mode_generation);
	FIELD(drm_castkms_capture_queue_buffer, user_data);
	FIELD(drm_castkms_capture_attach_monitor, display_name_ptr);
	FIELD(drm_castkms_get_grant, output_index);
	FIELD(drm_event_castkms_capture_frame, cursor_serial);
	FIELD(drm_castkms_cec_get_state, pending_cookie);
	FIELD(drm_castkms_cec_event_tx, msg);
	FIELD(drm_castkms_cec_event_tx, signal_free_time);

	return 0;
}
