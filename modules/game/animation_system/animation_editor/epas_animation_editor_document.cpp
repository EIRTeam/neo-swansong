/**************************************************************************/
/*  epas_animation_editor_document.cpp                                    */
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

#include "epas_animation_editor_document.h"
#include "../../fabrik/fabrik.h"
#include "scene/resources/packed_scene.h"

void EPASAnimationEditorDocument::_bind_methods() {
	ADD_SIGNAL(MethodInfo("show_error", PropertyInfo(Variant::STRING, "message")));
}

bool EPASAnimationEditorDocument::is_unsaved() const {
	return undo_redo->get_version() == last_saved_version;
}

void EPASAnimationEditorDocument::_show_error(const String &p_error) {
	emit_signal("show_error", p_error);
}

void EPASAnimationEditorDocument::_draw_timeline_frames() {
}

void EPASAnimationEditorDocument::_rebuild_keyframe_cache() {
	for (int i = 0; i < keyframe_cache.size(); i++) {
		memdelete(keyframe_cache[i]);
	}
	keyframe_cache.clear();

	ERR_FAIL_COND(!animation->get_editor_animation().is_valid());

	keyframe_cache.resize(animation->get_editor_animation()->get_keyframe_count());
	for (int i = 0; i < animation->get_editor_animation()->get_keyframe_count(); i++) {
		AnimationKeyframeCache *kc = memnew(AnimationKeyframeCache(animation->get_editor_animation()->get_keyframe(i)));
		keyframe_cache.set(i, kc);
	}
}

void EPASAnimationEditorDocument::add_keyframe(Ref<EPASKeyframe> p_keyframe) {
	AnimationKeyframeCache *kc = memnew(AnimationKeyframeCache(p_keyframe));
	keyframe_cache.push_back(kc);
	animation->get_editor_animation()->add_keyframe(p_keyframe);
}

void EPASAnimationEditorDocument::remove_keyframe(Ref<EPASKeyframe> p_keyframe) {
	for (int i = 0; i < keyframe_cache.size(); i++) {
		if (keyframe_cache[i]->keyframe == p_keyframe) {
			memdelete(keyframe_cache[i]);
			keyframe_cache.remove_at(i);
			break;
		}
	}
	animation->get_editor_animation()->erase_keyframe(p_keyframe);
}

EPASAnimationEditorDocument::AnimationKeyframeCache *EPASAnimationEditorDocument::get_keyframe_at_time(int p_frame_time) const {
	for (int i = 0; i < keyframe_cache.size(); i++) {
		if (keyframe_cache[i]->frame_time == p_frame_time) {
			return keyframe_cache[i];
		}
	}
	return nullptr;
}

EPASAnimationEditorDocument::AnimationKeyframeCache *EPASAnimationEditorDocument::get_keyframe(int p_frame_idx) const {
	ERR_FAIL_INDEX_V(p_frame_idx, keyframe_cache.size(), nullptr);
	return keyframe_cache[p_frame_idx];
}

UndoRedo *EPASAnimationEditorDocument::get_undo_redo() {
	return undo_redo;
}

Ref<EPASEditorAnimation> EPASAnimationEditorDocument::get_animation() const {
	return animation;
}

Skeleton3D *EPASAnimationEditorDocument::get_skeleton() const {
	return editing_skeleton;
}

void EPASAnimationEditorDocument::apply_constraints_to_current_frame() {
	Ref<EPASPose> current_pose = get_current_pose();
	if (!current_pose.is_valid()) {
		return;
	}
	apply_constraints(current_pose);
}

Ref<EPASPose> EPASAnimationEditorDocument::get_current_pose() const {
	// Returns the current pose that the user is hovering, if any
	int32_t current_frame = get_playback_position() * FPS;
	EPASAnimationEditorDocument::AnimationKeyframeCache *kf = get_keyframe_at_time(current_frame);
	if (kf) {
		return kf->keyframe->get_pose();
	}
	return Ref<EPASPose>();
}

int32_t EPASAnimationEditorDocument::get_length() const {
	return animation->get_editor_animation()->get_length() * 60.0f;
}

void EPASAnimationEditorDocument::load_model(const String &p_path) {
	undo_redo->clear_history();
	if (current_model) {
		current_model->queue_free();
		current_model = nullptr;
		editing_skeleton = nullptr;
	}

	Ref<PackedScene> scene = ResourceLoader::load(p_path);
	if (scene.is_null()) {
		_show_error("Loaded file was not a PackedScene!");
	}

	Node *node = scene->instantiate();
	Node3D *node_3d = Object::cast_to<Node3D>(node);
	if (!node_3d) {
		if (node) {
			node->queue_free();
		}
		_show_error("Model was not a 3D scene");
		return;
	}
	TypedArray<Node> skel_candidate = node_3d ? node_3d->find_children("*", "Skeleton3D") : TypedArray<Node>();
	Skeleton3D *skel = nullptr;
	if (skel_candidate.size() > 0) {
		skel = Object::cast_to<Skeleton3D>(skel_candidate[0]);
	}
	if (!skel) {
		if (node) {
			node->queue_free();
		}
		_show_error("Model did not contain any skeleton!");
		return;
	}

	document_3d_root->add_child(node_3d);
	epas_controller->set_skeleton_path(skel->get_path());
	epas_controller->set_playback_process_mode(EPASController::MANUAL);
	editing_skeleton = skel;
	current_model = node_3d;
	for (int i = 0; i < editing_skeleton->get_child_count(); i++) {
		GeometryInstance3D *gi = Object::cast_to<GeometryInstance3D>(editing_skeleton->get_child(i));
		if (gi) {
			model_visuals.append(gi);
		}
	}
}

void EPASAnimationEditorDocument::apply_constraints(const Ref<EPASPose> &p_pose) {
	if (!editing_skeleton) {
		return;
	}
	for (int i = 0; i < ik_constraints.size(); i++) {
		Ref<EPASAnimationEditorIKJoint> ik_constraint = ik_constraints[i];

		int target_bone_idx = editing_skeleton->find_bone(ik_constraint->get_ik_target_bone_name());
		int magnet_bone_idx = editing_skeleton->find_bone(ik_constraint->get_ik_magnet_bone_name());

		int c_bone_idx = editing_skeleton->find_bone(ik_constraint->get_ik_tip_bone_name());
		int b_bone_idx = editing_skeleton->get_bone_parent(c_bone_idx);
		int a_bone_idx = editing_skeleton->get_bone_parent(b_bone_idx);

		ERR_FAIL_COND_MSG(c_bone_idx == -1 || b_bone_idx == -1 || a_bone_idx == -1, "Invalid IK constraint");

		Transform3D a_bone_trf = editing_skeleton->get_bone_global_pose(a_bone_idx);
		Transform3D b_bone_trf = editing_skeleton->get_bone_pose(b_bone_idx);
		Transform3D c_bone_trf = editing_skeleton->get_bone_pose(c_bone_idx);
		Transform3D target_bone_trf = editing_skeleton->get_bone_global_pose(target_bone_idx);
		Vector3 magnet_target_pos = editing_skeleton->get_bone_global_pose(magnet_bone_idx).origin;

		Ref<FABRIKSolver> fabrik_solver = ik_constraint->get_fabrik_solver();
		fabrik_solver->set_joint_transform(0, a_bone_trf);
		fabrik_solver->set_joint_transform(1, b_bone_trf);
		fabrik_solver->set_joint_transform(2, c_bone_trf);
		fabrik_solver->calculate_distances();
		fabrik_solver->set_target_position(target_bone_trf.origin);
		fabrik_solver->set_pole_position(magnet_target_pos);

		fabrik_solver->solve(10);

		String a_bone_name = editing_skeleton->get_bone_name(a_bone_idx);
		String b_bone_name = editing_skeleton->get_bone_name(b_bone_idx);
		String c_bone_name = ik_constraint->get_ik_tip_bone_name();

		int a_parent_transform_idx = editing_skeleton->get_bone_parent(a_bone_idx);

		ERR_FAIL_COND(a_parent_transform_idx == -1);

		Transform3D a_parent_global_transform = editing_skeleton->get_bone_global_pose(a_parent_transform_idx);
		Quaternion a_bone_result_local = (a_parent_global_transform.affine_inverse() * fabrik_solver->get_joint_transform(0)).basis.get_rotation_quaternion();
		Quaternion b_bone_result_local = fabrik_solver->get_joint_transform(1).basis.get_rotation_quaternion();

		const int bone_indices[] = { a_bone_idx, b_bone_idx };
		const Quaternion *bone_rots[] = { &a_bone_result_local, &b_bone_result_local };

		for (int j = 0; j < 2; j++) {
			String bone_name = editing_skeleton->get_bone_name(bone_indices[j]);
			if (!p_pose->has_bone(bone_name)) {
				p_pose->create_bone(bone_name);
			}
			p_pose->set_bone_rotation(bone_name, *bone_rots[j]);
		}

		if (!p_pose->has_bone(c_bone_name)) {
			p_pose->create_bone(c_bone_name);
		}
		// Align the tip of rotation
		Transform3D b_global_trf = p_pose->calculate_bone_global_transform(b_bone_name, editing_skeleton, epas_controller->get_base_pose());
		Quaternion c_rot = b_global_trf.basis.get_rotation_quaternion().inverse() * target_bone_trf.basis.get_rotation_quaternion();
		p_pose->set_bone_rotation(c_bone_name, c_rot);
	}
}

Vector<Ref<EPASAnimationEditorIKJoint>> EPASAnimationEditorDocument::get_ik_constraints() const {
	return ik_constraints;
}

Ref<EPASEditorAnimationNode> EPASAnimationEditorDocument::get_animation_node() const {
	return epas_animation_node;
}

void EPASAnimationEditorDocument::set_rt_ik_enabled(bool p_rt_ik_enabled) {
	rt_ik_enabled = p_rt_ik_enabled;
}

bool EPASAnimationEditorDocument::get_rt_ik_enabled() const {
	return rt_ik_enabled;
}

void EPASAnimationEditorDocument::update(float p_delta) {
	epas_controller->advance(p_delta);
	if (rt_ik_enabled) {
		epas_blend_node->set_blend_amount(0.0f);
		Ref<EPASPose> output_pose = epas_controller->get_output_pose()->duplicate();
		apply_constraints(output_pose);
		epas_pose_node->set_pose(output_pose);
		epas_blend_node->set_blend_amount(1.0f);
		epas_controller->advance(0.0f);
		epas_blend_node->set_blend_amount(0.0f);
	}
}

EPASController *EPASAnimationEditorDocument::get_epas_controller() const {
	return epas_controller;
}

float EPASAnimationEditorDocument::get_playback_position() const {
	return epas_animation_node->get_time();
}

void EPASAnimationEditorDocument::pause() {
	epas_animation_node->set_playback_mode(EPASAnimationNode::MANUAL);
	epas_animation_node->set_looping_enabled(false);
}

void EPASAnimationEditorDocument::play() {
	epas_animation_node->set_playback_mode(EPASAnimationNode::AUTOMATIC);
	epas_animation_node->set_looping_enabled(true);
}

bool EPASAnimationEditorDocument::is_playing() const {
	return epas_animation_node->get_playback_mode() == EPASAnimationNode::AUTOMATIC;
}

void EPASAnimationEditorDocument::load_placeholder_scene(const String &p_path) {
	Ref<PackedScene> ps = ResourceLoader::load(p_path);
	if (ps.is_valid()) {
		Node *node_c = ps->instantiate();
		if (!node_c) {
			return;
		}
		Node3D *node = Object::cast_to<Node3D>(node_c);
		if (node) {
			document_3d_root->add_child(node);
			placeholder_scene = node;
		} else {
			_show_error("Scene was not a 3D scene, are you sure you are doing this right?");
			node_c->queue_free();
		}
	}
}

void EPASAnimationEditorDocument::add_warp_point_handle(const Ref<EPASWarpPoint> &p_warp_point) {
	ERR_FAIL_COND(!p_warp_point.is_valid());
	Ref<EPASAnimationEditorWarpPointSelectable> handle;
	handle.instantiate(p_warp_point, this);
	handle->type = EPASAnimationEditorSelection::WARP_POINT;
	handle->group = EPASAnimationEditorSelection::WARP_POINT_GROUP;
	selection_handles.push_back(handle);
}

void EPASAnimationEditorDocument::remove_warp_point_handle(const Ref<EPASWarpPoint> &p_warp_point) {
	for (int i = 0; i < selection_handles.size(); i++) {
		if (selection_handles[i]->type == EPASAnimationEditorSelection::WARP_POINT) {
			if (selection_handles[i]->warp_point == p_warp_point) {
				selection_handles.remove_at(i);
				break;
			}
		}
	}
}

void EPASAnimationEditorDocument::add_selection_handle(const Ref<EPASAnimationEditorSelection> &p_selection_handle) {
	selection_handles.push_back(p_selection_handle);
}

Ref<EPASAnimationEditorSelection> EPASAnimationEditorDocument::get_selection_handle_for_event(const Ref<EPASAnimationEvent> &p_event) {
	Ref<EPASDirectionalAnimationEvent> directional_ev = p_event;
	if (directional_ev.is_valid()) {
		Ref<EPASAnimationEditorDirectionalEventSelection> selection;
		selection.instantiate(this, directional_ev);
		return selection;
	}
	return Ref<EPASAnimationEditorSelection>();
}

void EPASAnimationEditorDocument::set_interpolation_method(EPASAnimation::InterpolationMethod p_interp_method) {
	epas_animation_node->set_interpolation_method(p_interp_method);
	animation->set_editor_interpolation_method(p_interp_method);
}

void EPASAnimationEditorDocument::open_file(const String &p_path) {
	Error err;
	animation = ResourceLoader::load(p_path, "EPASEditorAnimation", ResourceFormatLoader::CACHE_MODE_REUSE, &err);
	if (err == OK) {
		if (animation.is_valid()) {
			undo_redo->clear_history();
			if (!animation->get_editor_animation().is_valid()) {
				// Copy all keyframes over
				Ref<EPASAnimation> editor_animation;
				editor_animation.instantiate();
				for (int i = 0; i < animation->get_keyframe_count(); i++) {
					Ref<EPASKeyframe> og_kf = animation->get_keyframe(i);
					Ref<EPASKeyframe> kf;
					kf.instantiate();
					kf->set_time(og_kf->get_time());
					kf->set_pose(og_kf->get_pose()->duplicate());
					editor_animation->add_keyframe(kf);
				}
				animation->set_editor_animation(editor_animation);
			}
			animation->clear_keyframes();
			epas_animation_node->set_interpolation_method(animation->get_editor_interpolation_method());
			if (animation->has_meta("__editor_model_path")) {
				String model_path = animation->get_meta("__editor_model_path");
				load_model(model_path);
			}
			set_animation(animation);
			if (animation->get_meta("__editor_humanoid_ik", false)) {
				create_eirteam_humanoid_ik();
			}
			if (animation->has_meta("__editor_group_visibility")) {
				Array group_vis = animation->get_meta("__editor_group_visibility");
				if (group_vis.size() <= group_visibility.size()) {
					for (int i = 0; i < group_vis.size(); i++) {
						group_visibility.set(i, group_vis[i]);
					}
				}
			}

			for (int i = 0; i < animation->get_event_count(); i++) {
				Ref<EPASAnimationEditorSelection> handle = get_selection_handle_for_event(animation->get_event(i));
				if (handle.is_valid()) {
					add_selection_handle(handle);
				}
			}
		} else {
			_show_error("Selected file was not an EPAS animation file.");
		}
	} else {
		_show_error("Error loading animation: " + String(error_names[err]));
	}
}

void EPASAnimationEditorDocument::set_animation(Ref<EPASEditorAnimation> p_animation) {
	animation = p_animation;
	epas_animation_node->set_animation(p_animation);
	for (int i = 0; i < p_animation->get_warp_point_count(); i++) {
		add_warp_point_handle(p_animation->get_warp_point(i));
	}
	Skeleton3D *skel = get_skeleton();
	if (skel) {
		// Add selection handles
		for (int i = 0; i < skel->get_bone_count(); i++) {
			Ref<EPASAnimationEditorBoneSelectable> selection_handle;
			selection_handle.instantiate(this, Ref<EPASAnimationEditorIKJoint>(), i);
			String bone_name = skel->get_bone_name(i);
			if (bone_name.begins_with("palm") || bone_name.begins_with("f_")) {
				selection_handle->group = EPASAnimationEditorSelection::FK_FINGER_GROUP;
			} else if (bone_name.ends_with(".L")) {
				selection_handle->group = EPASAnimationEditorSelection::FK_LEFT_GROUP;
			} else if (bone_name.ends_with(".R")) {
				selection_handle->group = EPASAnimationEditorSelection::FK_RIGHT_GROUP;
			} else {
				selection_handle->group = EPASAnimationEditorSelection::FK_CENTER_GROUP;
			}
			selection_handles.push_back(selection_handle);
		}
	}
	_rebuild_keyframe_cache();
}

void EPASAnimationEditorDocument::bake_animation() {
	const int constexpr static ANIMATION_SAMPLE_FRAMERATE = 30;

	const float animation_length = animation->get_editor_animation()->get_length();
	int animation_sample_count = animation_length * ANIMATION_SAMPLE_FRAMERATE;
	for (int i = 0; i < animation_sample_count; i++) {
		float time = (i / (float)(animation_sample_count - 1)) * animation_length;
		// Bake animation
		Ref<EPASKeyframe> kf;
		kf.instantiate();
		kf->set_time(time);

		epas_animation_node->seek(time);
		epas_controller->advance(0.0f);

		Ref<EPASPose> pose = epas_controller->get_output_pose()->duplicate();

		apply_constraints(pose);

		kf->set_pose(pose);
		animation->add_keyframe(kf);
	}
}

void EPASAnimationEditorDocument::save_to_file(const String &p_path) {
	ERR_FAIL_COND(animation.is_null());
	Array group_vis;
	for (int i = 0; i < group_visibility.size(); i++) {
		group_vis.push_back(group_visibility[i]);
	}
	animation->set_meta("__editor_group_visibility", group_vis);
	animation->clear_keyframes();
	Ref<EPASAnimation> editor_animation = animation->get_editor_animation();

	bake_animation();

	if (current_model) {
		animation->set_meta("__editor_model_path", current_model->get_scene_file_path());
	}
	if (ik_constraints.size() > 0) {
		animation->set_meta("__editor_humanoid_ik", true);
	}
	int err_code = ResourceSaver::save(animation, p_path, ResourceSaver::FLAG_CHANGE_PATH);
	ERR_FAIL_COND_MSG(err_code != OK, vformat("Error saving animation, %s", error_names[err_code]));
	animation->set_path(p_path);
	last_saved_version = undo_redo->get_version();
}

bool EPASAnimationEditorDocument::get_group_visibility(int p_group) const {
	ERR_FAIL_INDEX_V(p_group, group_visibility.size(), false);
	return group_visibility[p_group];
}

void EPASAnimationEditorDocument::set_group_visibility(int p_group, bool p_visible) {
	ERR_FAIL_INDEX(p_group, group_visibility.size());
	group_visibility.set(p_group, p_visible);
}

void EPASAnimationEditorDocument::create_eirteam_humanoid_ik() {
	if (!editing_skeleton) {
		_show_error("You need to load a model before creating an IK rig for it!");
		return;
	}
	int root = editing_skeleton->get_parentless_bones()[0];
	Transform3D root_rest = editing_skeleton->get_bone_rest(root);
	undo_redo->clear_history();
	Vector<String> ik_tips;
	ik_tips.push_back("hand.L");
	ik_tips.push_back("hand.R");
	ik_tips.push_back("foot.L");
	ik_tips.push_back("foot.R");

	Vector<int> bone_handles_to_hide;

	for (int i = 0; i < ik_tips.size(); i++) {
		int tip_idx = editing_skeleton->find_bone(ik_tips[i]);
		ERR_FAIL_COND_MSG(tip_idx == -1, vformat("Can't build IK constraints, bone %s does not exist!", ik_tips[i]));
		bone_handles_to_hide.push_back(tip_idx);
		bone_handles_to_hide.push_back(editing_skeleton->get_bone_parent(tip_idx));
	}

	for (int i = 0; i < selection_handles.size(); i++) {
		Ref<EPASAnimationEditorBoneSelectable> bone_handle = selection_handles[i];
		if (bone_handle.is_valid() && bone_handles_to_hide.has(bone_handle->get_bone_index())) {
			selection_handles.get(i)->hidden = true;
		}
	}

	for (int i = 0; i < ik_tips.size(); i++) {
		// Create the virtual bones
		String ik_target_bone_name = "IK." + ik_tips[i];
		int target_bone_idx = editing_skeleton->find_bone(ik_target_bone_name);

		if (target_bone_idx == -1) {
			editing_skeleton->add_bone(ik_target_bone_name);
			target_bone_idx = editing_skeleton->get_bone_count() - 1;
			editing_skeleton->set_bone_parent(target_bone_idx, root);
		}

		String ik_magnet_bone_name = "IK.Magnet." + ik_tips[i];
		int magnet_bone_idx = editing_skeleton->find_bone(ik_magnet_bone_name);

		if (magnet_bone_idx == -1) {
			editing_skeleton->add_bone(ik_magnet_bone_name);
			magnet_bone_idx = editing_skeleton->get_bone_count() - 1;
			editing_skeleton->set_bone_parent(magnet_bone_idx, root);
		}

		int ik_a_bone_idx = editing_skeleton->find_bone(ik_tips[i]);
		int ik_b_bone_idx = editing_skeleton->get_bone_parent(ik_a_bone_idx);
		int ik_c_bone_idx = editing_skeleton->get_bone_parent(ik_b_bone_idx);
		{
			// Set the rest position for the target bone
			editing_skeleton->set_bone_rest(target_bone_idx, root_rest.affine_inverse() * editing_skeleton->get_bone_global_rest(ik_a_bone_idx));
			editing_skeleton->reset_bone_pose(target_bone_idx);

			Transform3D a_trf = editing_skeleton->get_bone_global_rest(ik_a_bone_idx);
			Transform3D b_trf = editing_skeleton->get_bone_global_rest(ik_b_bone_idx);
			Transform3D c_trf = editing_skeleton->get_bone_global_rest(ik_c_bone_idx);

			Vector3 a_to_b = b_trf.origin - a_trf.origin;
			Vector3 a_to_c = c_trf.origin - a_trf.origin;

			// Use dot product to find out the position of the projection of the middle bone on the hypothenuse (a to c)
			Vector3 a_to_c_magnet_proj = a_to_c.normalized() * (a_to_b.dot(a_to_c) / a_to_c.length());

			const float MAGNET_DISTANCE = 0.5f;
			Vector3 magnet_pos = b_trf.origin + (a_trf.origin + a_to_c_magnet_proj).direction_to(b_trf.origin) * MAGNET_DISTANCE;

			// Set rest position for the magnet bone
			Transform3D magnet_trf;
			magnet_trf.origin = magnet_pos;
			editing_skeleton->set_bone_rest(magnet_bone_idx, root_rest.affine_inverse() * magnet_trf);
			editing_skeleton->reset_bone_pose(magnet_bone_idx);
		}

		// Create the constraint
		Ref<EPASAnimationEditorIKJoint> ik_constraint;
		ik_constraint.instantiate();

		Ref<FABRIKSolver> fabrik_solver;
		fabrik_solver.instantiate();
		fabrik_solver->set_joint_count(3);
		/*fabrik_solver->set_joint_transform(0, editing_skeleton->get_bone_global_rest(ik_a_bone_idx));
		fabrik_solver->set_joint_transform(1, editing_skeleton->get_bone_rest(ik_b_bone_idx));
		fabrik_solver->set_joint_transform(2, editing_skeleton->get_bone_rest(ik_c_bone_idx));
		fabrik_solver->calculate_distances();*/

		fabrik_solver->set_use_pole_constraint(true);
		fabrik_solver->set_joint_hinge_enabled(1, true);
		fabrik_solver->set_joint_rotation_limit_enabled(1, true);
		fabrik_solver->set_joint_rotation_limit_min(1, Vector3(Math::deg_to_rad(10.0f), Math::deg_to_rad(-180.0f), Math::deg_to_rad(-180.0f)));
		fabrik_solver->set_joint_rotation_limit_max(1, Vector3(Math::deg_to_rad(180.0f), Math::deg_to_rad(180.0f), Math::deg_to_rad(180.0f)));

		ik_constraint->set_fabrik_solver(fabrik_solver);

		ik_constraint->set_ik_magnet_bone_name(ik_magnet_bone_name);
		ik_constraint->set_ik_target_bone_name(ik_target_bone_name);
		ik_constraint->set_ik_tip_bone_name(ik_tips[i]);
		ik_constraint->set_use_magnet(false);

		// Create selection handles
		Ref<EPASAnimationEditorBoneSelectable> ik_target_handle;
		ik_target_handle.instantiate(this, ik_constraint, target_bone_idx);
		Ref<EPASAnimationEditorBoneSelectable> ik_magnet_handle;
		ik_magnet_handle.instantiate(this, ik_constraint, target_bone_idx);

		ik_target_handle->type = EPASAnimationEditorSelection::IK_HANDLE;
		ik_target_handle->group = EPASAnimationEditorSelection::IK_GROUP;

		ik_magnet_handle->type = EPASAnimationEditorSelection::IK_HANDLE;
		ik_magnet_handle->group = EPASAnimationEditorSelection::IK_GROUP;

		selection_handles.push_back(ik_target_handle);
		selection_handles.push_back(ik_magnet_handle);
		ik_constraints.push_back(ik_constraint);
	}
}

int EPASAnimationEditorDocument::get_selection_handle_count() const {
	return selection_handles.size();
}

Ref<EPASAnimationEditorSelection> EPASAnimationEditorDocument::get_selection_handle(int p_handle) const {
	ERR_FAIL_INDEX_V(p_handle, selection_handles.size(), Ref<EPASAnimationEditorSelection>());
	return selection_handles[p_handle];
}

String EPASAnimationEditorDocument::get_tab_name() const {
	String path = animation->get_path();
	if (path.is_empty()) {
		return "<unsaved>";
	}
	return path.get_file();
}

Node3D *EPASAnimationEditorDocument::get_3d_root() const {
	return document_3d_root;
}

void EPASAnimationEditorDocument::update_visibility() {
	for (int i = 0; i < model_visuals.size(); i++) {
		switch (visibility_mode) {
			case HIDDEN: {
				document_3d_root->hide();
			} break;
			case VISIBLE: {
				document_3d_root->show();
				model_visuals[i]->set_transparency(0.0f);
			} break;
			case TRANSPARENT: {
				document_3d_root->show();
				model_visuals[i]->set_transparency(0.5f);
			} break;
			default: {
			};
		}
	}
}

EPASAnimationEditorDocument::VisibilityMode EPASAnimationEditorDocument::get_visibility_mode() const {
	return visibility_mode;
}

void EPASAnimationEditorDocument::set_visibility_mode(const VisibilityMode &p_visibility_mode) {
	visibility_mode = p_visibility_mode;
	update_visibility();
}

void EPASAnimationEditorDocument::seek(float p_time) {
	epas_animation_node->seek(p_time);
}

int EPASAnimationEditorDocument::get_frame_count() const {
	return keyframe_cache.size();
}

EPASAnimationEditorDocument::EPASAnimationEditorDocument(Node3D *p_root) {
	group_visibility.resize(EPASAnimationEditorSelection::GROUP_MAX);
	group_visibility.fill(true);
	document_3d_root = memnew(Node3D);
	p_root->add_child(document_3d_root);
	epas_controller = memnew(EPASController);
	document_3d_root->add_child(epas_controller, true);

	epas_animation_node.instantiate();
	epas_animation_node->set_playback_mode(EPASAnimationNode::PlaybackMode::MANUAL);
	epas_animation_node->set_looping_enabled(false);
	epas_blend_node.instantiate();

	epas_controller->connect_node_to_root(epas_blend_node, "Blend Node");
	epas_controller->connect_node(epas_animation_node, epas_blend_node, "Animation", 0);

	epas_pose_node.instantiate();
	epas_controller->connect_node(epas_pose_node, epas_blend_node, "IK Blend", 1);

	undo_redo = memnew(UndoRedo);
}

EPASAnimationEditorDocument::~EPASAnimationEditorDocument() {
	if (undo_redo) {
		memdelete(undo_redo);
	}
	for (int i = 0; i < keyframe_cache.size(); i++) {
		memdelete(keyframe_cache[i]);
	}
	keyframe_cache.clear();
}