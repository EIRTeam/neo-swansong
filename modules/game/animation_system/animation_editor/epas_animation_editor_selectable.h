/**************************************************************************/
/*  epas_animation_editor_selectable.h                                    */
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

#ifndef EPAS_ANIMATION_EDITOR_SELECTABLE_H
#define EPAS_ANIMATION_EDITOR_SELECTABLE_H

#include "../epas_animation.h"
#include "core/object/ref_counted.h"
#include "epas_animation_editor_ik_joint.h"

class EPASAnimationEditorDocument;

class EPASAnimationEditorSelection : public RefCounted {
public:
	enum SelectionType {
		FK_BONE,
		IK_HANDLE,
		WARP_POINT
	};
	enum SelectionGroup {
		FK_CENTER_GROUP,
		FK_LEFT_GROUP,
		FK_RIGHT_GROUP,
		FK_FINGER_GROUP,
		IK_GROUP,
		WARP_POINT_GROUP,
		GROUP_MAX
	};
	enum TransformChangeMask {
		TRANSLATION = 1,
		ROTATION = 2,
		SCALE = 4,
		ALL = TRANSLATION | ROTATION | SCALE
	};

	SelectionType type = SelectionType::FK_BONE;
	SelectionGroup group = SelectionGroup::FK_CENTER_GROUP;
	bool hidden = false;
	bool is_ik_magnet = false;
	Ref<EPASAnimationEditorIKJoint> ik_joint;
	Ref<EPASWarpPoint> warp_point;

private:
	Ref<EPASAnimationEditorDocument> document;

protected:
	Ref<EPASAnimationEditorDocument> get_document() const;

public:
	virtual Vector3 get_handle_position() const = 0;
	virtual Color get_handle_color() const { return Color("BLACK"); }
	virtual String get_handle_name() const { return "unknown!"; }
	virtual Transform3D get_handle_transform() const { return Transform3D(); }
	virtual void set_handle_transform(const Transform3D &p_trf, BitField<TransformChangeMask> p_change_mask) {}
	virtual void draw() {}
	EPASAnimationEditorSelection(Ref<EPASAnimationEditorDocument> p_document);
};

class EPASAnimationEditorBoneSelectable : public EPASAnimationEditorSelection {
	int bone_idx;
	Ref<EPASAnimationEditorIKJoint> ik_joint;
	bool is_ik_magnet = false;
	void _world_to_bone_trf(int p_bone_idx, const Transform3D &p_world_trf, Transform3D &p_out);

public:
	virtual Vector3 get_handle_position() const override;
	virtual String get_handle_name() const override;
	virtual Color get_handle_color() const override;

	virtual Transform3D get_handle_transform() const override;
	virtual void set_handle_transform(const Transform3D &p_trf, BitField<TransformChangeMask> p_change_mask) override;

	bool get_is_ik_magnet() const { return is_ik_magnet; }
	void set_is_ik_magnet(bool p_is_ik_magnet) { is_ik_magnet = p_is_ik_magnet; }

	int get_bone_index() const;
	void set_bone_index(int p_bone_idx);

	EPASAnimationEditorBoneSelectable(Ref<EPASAnimationEditorDocument> p_document, Ref<EPASAnimationEditorIKJoint> p_ik_joint, int p_bone_idx);
};

class EPASAnimationEditorWarpPointSelectable : public EPASAnimationEditorSelection {
	Ref<EPASWarpPoint> warp_point;

public:
	virtual Vector3 get_handle_position() const override;
	virtual Transform3D get_handle_transform() const override;
	virtual String get_handle_name() const override;
	virtual void set_handle_transform(const Transform3D &p_trf, BitField<TransformChangeMask> p_change_mask) override;
	EPASAnimationEditorWarpPointSelectable(Ref<EPASWarpPoint> p_warp_point, Ref<EPASAnimationEditorDocument> p_document);
};

class EPASAnimationEditorDirectionalEventSelection : public EPASAnimationEditorSelection {
	HBDebugGeometry *debug_geo = nullptr;
	Ref<EPASDirectionalAnimationEvent> event;

public:
	virtual void draw() override;
	virtual Transform3D get_handle_transform() const override;
	virtual void set_handle_transform(const Transform3D &p_trf, BitField<TransformChangeMask> p_change_mask) override;
	virtual Vector3 get_handle_position() const override;
	EPASAnimationEditorDirectionalEventSelection(Ref<EPASAnimationEditorDocument> p_document, Ref<EPASDirectionalAnimationEvent> p_event);
};

#endif // EPAS_ANIMATION_EDITOR_SELECTABLE_H
