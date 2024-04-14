/**************************************************************************/
/*  epas_animation_editor_selectable.cpp                                  */
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

#include "epas_animation_editor_selectable.h"
#include "../../transform_conversions.h"
#include "epas_animation_editor_document.h"

Ref<EPASAnimationEditorDocument> EPASAnimationEditorSelection::get_document() const {
	return document;
}

EPASAnimationEditorSelection::EPASAnimationEditorSelection(Ref<EPASAnimationEditorDocument> p_document) {
	document = p_document;
}

void EPASAnimationEditorBoneSelectable::_world_to_bone_trf(int p_bone_idx, const Transform3D &p_world_trf, Transform3D &p_out) {
	p_out = p_world_trf;
	Ref<EPASAnimationEditorDocument> doc = get_document();
	ERR_FAIL_COND(!doc.is_valid());

	Skeleton3D *editing_skeleton = doc->get_skeleton();

	p_out = editing_skeleton->get_global_transform().affine_inverse() * p_out;
	int parent = editing_skeleton->get_bone_parent(p_bone_idx);
	if (parent != -1) {
		p_out = editing_skeleton->get_bone_global_pose(parent).affine_inverse() * p_out;
	}
}

Vector3 EPASAnimationEditorBoneSelectable::get_handle_position() const {
	Ref<EPASAnimationEditorDocument> doc = get_document();
	Skeleton3D *skel = doc->get_skeleton();
	if (!skel) {
		return Vector3();
	}
	return skel->to_global(skel->get_bone_global_pose(bone_idx).origin);
}

String EPASAnimationEditorBoneSelectable::get_handle_name() const {
	Ref<EPASAnimationEditorDocument> doc = get_document();
	if (ik_joint.is_valid()) {
		if (is_ik_magnet) {
			return ik_joint->get_ik_magnet_bone_name();
		} else {
			return ik_joint->get_ik_target_bone_name();
		}
	}

	Skeleton3D *skel = doc->get_skeleton();
	if (!skel) {
		return "<unk>";
	}

	return skel->get_bone_name(bone_idx);
}

Color EPASAnimationEditorBoneSelectable::get_handle_color() const {
	Color color_right = Color::named("Red");
	Color color_left = Color::named("Lime Green");
	Color color_ik = Color::named("Blue");
	Color color_unlit = Color::named("Dark Magenta");

	if (ik_joint.is_valid()) {
		return color_ik;
	}
	const String handle_name = get_handle_name();
	if (handle_name.ends_with(".L")) {
		return color_left;
	} else if (handle_name.ends_with(".R")) {
		return color_right;
	}

	return color_unlit;
}

Transform3D EPASAnimationEditorBoneSelectable::get_handle_transform() const {
	Ref<EPASAnimationEditorDocument> doc = get_document();
	Skeleton3D *skel = doc->get_skeleton();
	if (!skel) {
		return Transform3D();
	}
	Ref<EPASPose> pose = doc->get_current_pose();
	if (!pose.is_valid()) {
		return Transform3D();
	}

	String bone_name = doc->get_skeleton()->get_bone_name(bone_idx);
	Transform3D handle_trf;
	if (skel) {
		handle_trf = skel->get_bone_global_pose(bone_idx);
		handle_trf = skel->get_global_transform() * handle_trf;
		return handle_trf;
	}
	return Transform3D();
}

void EPASAnimationEditorBoneSelectable::set_handle_transform(const Transform3D &p_trf, BitField<TransformChangeMask> p_change_mask) {
	Ref<EPASAnimationEditorDocument> doc = get_document();
	UndoRedo *undo_redo = doc->get_undo_redo();

	Ref<EPASPose> current_pose = doc->get_current_pose();

	Skeleton3D *skel = doc->get_skeleton();
	String bone_name = skel->get_bone_name(bone_idx);

	if (!current_pose->has_bone(bone_name)) {
		current_pose->create_bone(bone_name);
	}

	int editing_bone = bone_idx;
	Transform3D new_bone_trf;
	_world_to_bone_trf(editing_bone, p_trf, new_bone_trf);

	Transform3D old_bone_trf = skel->get_bone_pose(editing_bone);

	if (p_change_mask & TransformChangeMask::TRANSLATION) {
		undo_redo->add_do_method(callable_mp(current_pose.ptr(), &EPASPose::set_bone_position).bind(bone_name, new_bone_trf.origin));
		undo_redo->add_undo_method(callable_mp(current_pose.ptr(), &EPASPose::set_bone_position).bind(bone_name, old_bone_trf.origin));
	}
	if (p_change_mask & TransformChangeMask::ROTATION) {
		undo_redo->add_do_method(callable_mp(current_pose.ptr(), &EPASPose::set_bone_rotation).bind(bone_name, new_bone_trf.basis.get_rotation_quaternion()));
		undo_redo->add_undo_method(callable_mp(current_pose.ptr(), &EPASPose::set_bone_rotation).bind(bone_name, old_bone_trf.basis.get_rotation_quaternion()));
	}
}

int EPASAnimationEditorBoneSelectable::get_bone_index() const { return bone_idx; }

void EPASAnimationEditorBoneSelectable::set_bone_index(int p_bone_idx) { bone_idx = p_bone_idx; }

EPASAnimationEditorBoneSelectable::EPASAnimationEditorBoneSelectable(Ref<EPASAnimationEditorDocument> p_document, Ref<EPASAnimationEditorIKJoint> p_ik_joint, int p_bone_idx) :
		EPASAnimationEditorSelection(p_document) {
	ik_joint = p_ik_joint;
	bone_idx = p_bone_idx;
}

Vector3 EPASAnimationEditorWarpPointSelectable::get_handle_position() const {
	return get_handle_transform().origin;
}

Transform3D EPASAnimationEditorWarpPointSelectable::get_handle_transform() const {
	return warp_point->get_transform();
}

String EPASAnimationEditorWarpPointSelectable::get_handle_name() const {
	return warp_point->get_name();
}

void EPASAnimationEditorWarpPointSelectable::set_handle_transform(const Transform3D &p_trf, BitField<TransformChangeMask> p_change_mask) {
	Transform3D prev_trf = get_handle_transform();
	Ref<EPASAnimationEditorDocument> doc = get_document();
	UndoRedo *undo_redo = doc->get_undo_redo();
	undo_redo->add_do_method(callable_mp(warp_point.ptr(), &EPASWarpPoint::set_transform).bind(p_trf));
	undo_redo->add_undo_method(callable_mp(warp_point.ptr(), &EPASWarpPoint::set_transform).bind(prev_trf));
}

EPASAnimationEditorWarpPointSelectable::EPASAnimationEditorWarpPointSelectable(Ref<EPASWarpPoint> p_warp_point, Ref<EPASAnimationEditorDocument> p_document) :
		EPASAnimationEditorSelection(p_document) {
	warp_point = p_warp_point;
}

void EPASAnimationEditorDirectionalEventSelection::draw() {
	debug_geo->clear();
	Transform3D trf = event->get_transform();
	debug_geo->debug_arrow(trf.origin, trf.origin + trf.basis.get_column(2));
}

EPASAnimationEditorDirectionalEventSelection::EPASAnimationEditorDirectionalEventSelection(Ref<EPASAnimationEditorDocument> p_document, Ref<EPASDirectionalAnimationEvent> p_event) :
		EPASAnimationEditorSelection(p_document) {
	debug_geo = memnew(HBDebugGeometry);
	event = p_event;
	p_document->get_3d_root()->add_child(debug_geo);
}

Transform3D EPASAnimationEditorDirectionalEventSelection::get_handle_transform() const {
	return event->get_transform();
}

void EPASAnimationEditorDirectionalEventSelection::set_handle_transform(const Transform3D &p_trf, BitField<TransformChangeMask> p_change_mask) {
	Ref<EPASAnimationEditorDocument> doc = get_document();
	UndoRedo *undo_redo = doc->get_undo_redo();
	Transform3D prev_trf = get_handle_transform();
	undo_redo->add_do_method(callable_mp(event.ptr(), &EPASDirectionalAnimationEvent::set_transform).bind(p_trf));
	undo_redo->add_undo_method(callable_mp(event.ptr(), &EPASDirectionalAnimationEvent::set_transform).bind(prev_trf));
}

Vector3 EPASAnimationEditorDirectionalEventSelection::get_handle_position() const {
	return event->get_transform().origin;
};
