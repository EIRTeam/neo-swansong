/**************************************************************************/
/*  epas_editor_animation.cpp                                             */
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