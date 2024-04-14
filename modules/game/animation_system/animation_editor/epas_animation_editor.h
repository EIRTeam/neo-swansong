/**************************************************************************/
/*  epas_animation_editor.h                                               */
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

#ifndef EPAS_ANIMATION_EDITOR_H
#define EPAS_ANIMATION_EDITOR_H
#include "core/math/transform_3d.h"
#include "epas_editor_animation.h"
#include "modules/game/animation_system/epas_animation.h"
#include "modules/game/animation_system/epas_animation_node.h"
#include "modules/game/animation_system/epas_blend_node.h"
#include "modules/game/animation_system/epas_pose_node.h"

#ifdef DEBUG_ENABLED
#include "../../fabrik/fabrik.h"
#include "../epas_controller.h"
#include "../epas_editor_camera.h"
#include "../epas_editor_grid.h"
#include "../epas_pose.h"
#include "core/object/ref_counted.h"
#include "core/object/undo_redo.h"
#include "epas_animation_editor_document.h"
#include "epas_animation_editor_ik_joint.h"
#include "epas_animation_editor_selectable.h"
#include "modules/imgui/godot_imgui.h"
#include "scene/2d/multimesh_instance_2d.h"
#include "scene/3d/multimesh_instance_3d.h"
#include "scene/3d/node_3d.h"
#include "scene/3d/skeleton_3d.h"
#include "scene/gui/file_dialog.h"

class EPASAnimationEditor;

class EPASAnimationEventsEditor : public RefCounted {
	GDCLASS(EPASAnimationEventsEditor, RefCounted);
	bool open = false;
	Ref<EPASAnimationEvent> current_event;
	FileDialog *sound_file_dialog = nullptr;
	String get_event_name(const Ref<EPASAnimationEvent> &ev) const;
	void _on_sound_file_selected(String p_file);

public:
	void draw(Ref<EPASAnimation> p_animation, float p_playback_position);
	EPASAnimationEventsEditor();
	~EPASAnimationEventsEditor();
	friend class EPASAnimationEditor;
};

class EPASAnimationCurvesEditor : public RefCounted {
	GDCLASS(EPASAnimationCurvesEditor, RefCounted);
	StringName current_curve;
	bool open = true;

public:
	void draw(Ref<EPASAnimation> p_animation, float p_playback_position);
};

class EPASAnimationEditor : public Control {
	GDCLASS(EPASAnimationEditor, Control);

public:
private:
	int selected_document = -1;

	Vector<Ref<EPASAnimationEditorDocument>> documents;
	ImGui::FrameIndexType current_frame = 0;
	ImGui::FrameIndexType start_frame = 0;
	ImGui::FrameIndexType timeline_length = 100;

	PackedFloat32Array view_matrix;
	PackedFloat32Array projection_matrix;
	// Old trf matrix, for undo_redo
	PackedFloat32Array prev_handle_trf_matrix;
	// Actual TRF matrix the user is manipulating
	PackedFloat32Array current_handle_trf_matrix;
	PackedFloat32Array identity_matrix;
	bool was_using_gizmo = false;

	ImGuizmo::OPERATION guizmo_operation = ImGuizmo::ROTATE;
	ImGuizmo::MODE guizmo_mode = ImGuizmo::LOCAL;

	FileDialog *file_open_dialog;
	FileDialog *model_load_dialog;
	FileDialog *placeholder_load_dialog;
	FileDialog *save_file_dialog;

	EPASEditorCamera *camera;
	Node3D *editor_3d_root;
	EPASEditorGrid *grid;
	MeshInstance3D *solid_ground;

	MultiMeshInstance2D *selection_handle_dots;
	// Spheres that show the location of warp points
	MultiMeshInstance3D *warp_point_spheres;

	Vector<ImGui::FrameIndexType> kf_selection;

	Vector<int> editing_selection_handles;

	Ref<Font> label_font;
	Ref<EPASAnimationCurvesEditor> curves_editor;
	Ref<EPASAnimationEventsEditor> events_editor;
	int currently_hovered_selection_handle = -1;

	struct ui_info_t {
		bool posedbg_window_visible = false;
		bool constraintdbg_window_visible = false;
		bool root_mover_window_visible = false;
		int selected_warp_point = -1;
		Vector3 copy_buffer;
		Ref<EPASPose> pose_copy_buffer;
	} ui_info;

	Ref<EPASAnimationEditorDocument> get_current_document() const;

	void select_document(int p_new_document);
	void _draw_ui();
	void _draw_bone_positions(bool p_selected_only);
	void _update_editing_handle_trf();
	void _world_to_bone_trf(int p_bone_idx, const float *p_world_trf, Transform3D &p_out);
	void _apply_handle_transform(ImGuizmo::OPERATION p_operation, PackedFloat32Array p_delta = PackedFloat32Array());
	bool _is_playing();
	void _show_error(const String &error);
	bool warp_point_sphere_update_queued = false;
	void queue_warp_point_sphere_update();
	void _load_placeholder_scene(const String &p_path);
	void _copy_fk_to_ik();
	void draw_inspector();

	Ref<EPASPose> get_current_pose() const;
	EPASAnimationEditorDocument::AnimationKeyframeCache *get_keyframe(int p_frame_time) const;

	String current_error;

protected:
	void _notification(int p_what);
	virtual void unhandled_input(const Ref<InputEvent> &p_event) override;

public:
	void open_file(const String &p_path);
	void load_model(const String &p_path);
	void set_animation(const Ref<EPASAnimation> &p_animation);
	Ref<EPASAnimation> get_animation() const;
	void save_to_path(const String &p_path);

	EPASAnimationEditor();
	virtual ~EPASAnimationEditor();
};
#endif // DEBUG_ENABLED
#endif // EPAS_ANIMATION_EDITOR_H