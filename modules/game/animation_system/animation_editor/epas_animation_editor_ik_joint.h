#ifndef EPAS_ANIMATION_EDITOR_IK_JOINT_H
#define EPAS_ANIMATION_EDITOR_IK_JOINT_H

#include "core/io/resource.h"
#include "../../fabrik/fabrik.h"

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
