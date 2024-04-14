/**************************************************************************/
/*  epas_animation_editor_document.h                                      */
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

#ifndef EPAS_ANIMATION_EDITOR_DOCUMENT_H
#define EPAS_ANIMATION_EDITOR_DOCUMENT_H

#include "../epas_blend_node.h"
#include "../epas_controller.h"
#include "../epas_pose_node.h"
#include "core/object/undo_redo.h"
#include "epas_animation_editor_ik_joint.h"
#include "epas_animation_editor_selectable.h"
#include "epas_editor_animation.h"

#include "animation_editor_constants.h"

class EPASAnimationEditorDocument : public RefCounted {
	UndoRedo *undo_redo = nullptr;
	String file_path;

	Ref<EPASEditorAnimation> animation;
	Vector<Ref<EPASAnimationEditorIKJoint>> ik_constraints;
	struct {
		EPASController *animation_controller = nullptr;
	} animation_playback;

	EPASController *epas_controller = nullptr;
	Ref<EPASEditorAnimationNode> epas_animation_node;
	Ref<EPASBlendNode> epas_blend_node;
	Ref<EPASPoseNode> epas_pose_node;
	Node3D *placeholder_scene = nullptr;
	Node3D *current_model = nullptr;
	Node3D *document_3d_root = nullptr;
	HBDebugGeometry *debug_geo = nullptr;

	Vector<GeometryInstance3D *> model_visuals;
	Skeleton3D *editing_skeleton = nullptr;
	Vector<bool> group_visibility;
	Vector<Ref<EPASAnimationEditorSelection>> selection_handles;
	bool rt_ik_enabled = false;
	void _show_error(const String &p_error);
	uint64_t last_saved_version = 0;

public:
	static void _bind_methods();
	// When manipulating keyframes we need to make sure the order of keyframes doesn't change
	// however, keyframes are reordered in the animation when their time is changed, so we have
	// to keep track of them separatedly so their indices don't change
	struct AnimationKeyframeCache {
		Ref<EPASKeyframe> keyframe;
		int32_t frame_time = 0;
		int32_t temporary_frame_time = 0;
		AnimationKeyframeCache(Ref<EPASKeyframe> p_keyframe) {
			keyframe = p_keyframe;
			frame_time = Math::round(keyframe->get_time() * FPS);
			temporary_frame_time = frame_time;
		}
		void apply_temp_frame_time() {
			keyframe->set_time(temporary_frame_time / (float)FPS);
		}
		void apply_frame_time() {
			keyframe->set_time(frame_time / (float)FPS);
		}
	};

	enum VisibilityMode {
		HIDDEN,
		VISIBLE,
		TRANSPARENT,
		VISIBLITY_MODE_MAX
	};

	bool is_unsaved() const;

	VisibilityMode visibility_mode = VisibilityMode::VISIBLE;

	Vector<AnimationKeyframeCache *> keyframe_cache;
	void _draw_timeline_frames();
	void _rebuild_keyframe_cache();
	void _set_frame_time(int p_frame_idx, int32_t p_frame_time);
	void add_keyframe(Ref<EPASKeyframe> p_keyframe);
	void remove_keyframe(Ref<EPASKeyframe> p_keyframe);
	AnimationKeyframeCache *get_keyframe_at_time(int p_frame_time) const;
	AnimationKeyframeCache *get_keyframe(int p_frame_idx) const;
	UndoRedo *get_undo_redo();
	Ref<EPASEditorAnimation> get_animation() const;
	Skeleton3D *get_skeleton() const;
	void apply_constraints_to_current_frame();
	Ref<EPASPose> get_current_pose() const;

	int32_t get_length() const;

	void load_model(const String &p_path);
	void apply_constraints(const Ref<EPASPose> &p_pose);
	Vector<Ref<EPASAnimationEditorIKJoint>> get_ik_constraints() const;
	Ref<EPASEditorAnimationNode> get_animation_node() const;
	void set_rt_ik_enabled(bool p_rt_ik_enabled);
	bool get_rt_ik_enabled() const;
	void update(float p_delta);
	EPASController *get_epas_controller() const;
	float get_playback_position() const;
	void pause();
	void play();
	bool is_playing() const;
	void load_placeholder_scene(const String &p_path);
	void add_warp_point_handle(const Ref<EPASWarpPoint> &p_warp_point);
	void remove_warp_point_handle(const Ref<EPASWarpPoint> &p_warp_point);
	void add_selection_handle(const Ref<EPASAnimationEditorSelection> &p_selection_handle);
	Ref<EPASAnimationEditorSelection> get_selection_handle_for_event(const Ref<EPASAnimationEvent> &p_event);

	void set_interpolation_method(EPASAnimation::InterpolationMethod p_interp_method);
	void open_file(const String &p_path);
	void set_animation(Ref<EPASEditorAnimation> p_animation);
	void bake_animation();
	void save_to_file(const String &p_path);
	bool get_group_visibility(int p_group) const;
	void set_group_visibility(int p_group, bool p_visible);
	void create_eirteam_humanoid_ik();
	int get_selection_handle_count() const;
	Ref<EPASAnimationEditorSelection> get_selection_handle(int p_handle) const;
	String get_tab_name() const;
	Node3D *get_3d_root() const;

	void update_visibility();

	VisibilityMode get_visibility_mode() const;
	void set_visibility_mode(const VisibilityMode &p_visibility_mode);
	void seek(float p_time);
	int get_frame_count() const;

	EPASAnimationEditorDocument(Node3D *p_root);
	~EPASAnimationEditorDocument();
};

#endif // EPAS_ANIMATION_EDITOR_DOCUMENT_H
