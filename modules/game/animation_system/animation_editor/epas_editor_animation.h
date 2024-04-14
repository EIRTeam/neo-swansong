#ifndef EPAS_EDITOR_ANIMATION_H
#define EPAS_EDITOR_ANIMATION_H

#include "modules/game/animation_system/epas_animation.h"
#include "modules/game/animation_system/epas_animation_node.h"

class EPASEditorAnimation : public EPASAnimation {
	GDCLASS(EPASEditorAnimation, EPASAnimation);

	Ref<EPASAnimation> editor_animation;
	EPASAnimation::InterpolationMethod editor_interpolation_method = EPASAnimation::InterpolationMethod::LINEAR;

protected:
	static void _bind_methods();

public:
	Ref<EPASAnimation> get_editor_animation() const { return editor_animation; }
	void set_editor_animation(const Ref<EPASAnimation> &p_editor_animation) { editor_animation = p_editor_animation; }

	EPASAnimation::InterpolationMethod get_editor_interpolation_method() const { return editor_interpolation_method; }
	void set_editor_interpolation_method(EPASAnimation::InterpolationMethod p_editor_interpolation_method) { editor_interpolation_method = p_editor_interpolation_method; }
};

class EPASEditorAnimationNode : public EPASAnimationNode {
	GDCLASS(EPASEditorAnimationNode, EPASAnimationNode);
	EPASAnimationPlaybackInfo playback_info;
	virtual void interpolate(const Ref<EPASPose> &p_base_pose, Ref<EPASPose> p_target_pose, float p_time) override;
};

#endif // EPAS_EDITOR_ANIMATION_H
