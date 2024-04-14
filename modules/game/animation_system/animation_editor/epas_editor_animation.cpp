#include "epas_editor_animation.h"

void EPASEditorAnimationNode::interpolate(const Ref<EPASPose> &p_base_pose, Ref<EPASPose> p_target_pose, float p_time) {
	Ref<EPASEditorAnimation> anim = get_animation();
	ERR_FAIL_COND(!anim.is_valid());

	// UGLY HACK: So events work... maybe we have to rethink the whole editor animation system or something.
	if (anim->get_event_count() != anim->get_editor_animation()->get_event_count()) {
		anim->get_editor_animation()->clear_events();
		for (int i = 0; i < anim->get_event_count(); i++) {
			anim->get_editor_animation()->add_event(anim->get_event(i));
		}
	}

	anim->get_editor_animation()->interpolate(p_time, p_base_pose, p_target_pose, get_interpolation_method(), &playback_info);
	playback_info.last_time = p_time;

	for (size_t i = 0; i < playback_info.emitted_events.size(); i++) {
		_on_animation_event_fired(playback_info.emitted_events[i]);
	}
}