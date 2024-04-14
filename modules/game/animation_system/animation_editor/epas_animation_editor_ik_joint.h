/**************************************************************************/
/*  epas_animation_editor_ik_joint.h                                      */
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

#ifndef EPAS_ANIMATION_EDITOR_IK_JOINT_H
#define EPAS_ANIMATION_EDITOR_IK_JOINT_H

#include "../../fabrik/fabrik.h"
#include "core/io/resource.h"

class EPASAnimationEditorIKJoint : public Resource {
	GDCLASS(EPASAnimationEditorIKJoint, Resource);
	String ik_tip_bone_name;
	bool use_magnet;
	// We use a virtual bone in the skeleton for simplifying IK code
	String ik_target_bone_name;
	String ik_magnet_bone_name;
	Ref<FABRIKSolver> fabrik_solver;

public:
	bool get_use_magnet() const { return use_magnet; }
	void set_use_magnet(bool p_use_magnet) { use_magnet = p_use_magnet; }

	String get_ik_tip_bone_name() const { return ik_tip_bone_name; }
	void set_ik_tip_bone_name(const String &p_ik_tip_bone_name) { ik_tip_bone_name = p_ik_tip_bone_name; }

	String get_ik_target_bone_name() const { return ik_target_bone_name; }
	void set_ik_target_bone_name(const String &p_ik_target_bone_name) { ik_target_bone_name = p_ik_target_bone_name; }

	String get_ik_magnet_bone_name() const { return ik_magnet_bone_name; }
	void set_ik_magnet_bone_name(const String &p_ik_magnet_bone_name) { ik_magnet_bone_name = p_ik_magnet_bone_name; }

	Ref<FABRIKSolver> get_fabrik_solver() const { return fabrik_solver; }
	void set_fabrik_solver(const Ref<FABRIKSolver> &p_fabrik_solver) { fabrik_solver = p_fabrik_solver; }
};

#endif // EPAS_ANIMATION_EDITOR_IK_JOINT_H
