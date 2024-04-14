/**************************************************************************/
/*  epas_editor_animation.h                                               */
/**************************************************************************/
/*                         This file is part of:                          */
/*                               SWANSONG                                 */
/*                          https://eirteam.moe                           */
/**************************************************************************/
/* Copyright (c) 2023-present Álex Román Núñez (EIRTeam).                 */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

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
