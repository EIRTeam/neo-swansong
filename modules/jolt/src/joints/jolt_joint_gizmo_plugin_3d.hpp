#pragma once

#ifdef GDJ_CONFIG_EDITOR

#include "jolt_joint_gizmo_plugin_3d.gen.hpp"

#if defined(GDEXTENSION) || defined(GDMODULE_IMPL)

class JoltJointGizmoPlugin3D final : public EditorNode3DGizmoPlugin {
	GDCLASS_NO_WARN(JoltJointGizmoPlugin3D, EditorNode3DGizmoPlugin)

private:
	static void _bind_methods();

public:
	JoltJointGizmoPlugin3D() = default;

	explicit JoltJointGizmoPlugin3D(EditorInterface* p_editor_interface);

	bool _has_gizmo(Node3D* p_node) GDEX_CONST_EX_ONLY override;

	Ref<EditorNode3DGizmo> _create_gizmo(Node3D* p_node) GDEX_CONST_EX_ONLY override;

	String _get_gizmo_name() const override;

	void _redraw(const Ref<EditorNode3DGizmo>& p_gizmo) GDEX_OVERRIDE_EX_ONLY;

	void redraw_gizmos();

#ifndef GDEXTENSION
	void redraw(EditorNode3DGizmo *p_gizmo) override {
		Ref<EditorNode3DGizmo> gizmo = Ref<EditorNode3DGizmo>(p_gizmo);
		_redraw(gizmo);
	};
#endif

private:
	void _create_materials();

	void _create_redraw_timer(const Ref<EditorNode3DGizmo>& p_gizmo);

	mutable HashSetJolt<Ref<EditorNode3DGizmo>> gizmos;

	EditorInterface* editor_interface = nullptr;

	bool initialized = false;
};

#endif // GDJ_CONFIG_EDITOR
#endif