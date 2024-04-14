/**************************************************************************/
/*  debug_geometry.cpp                                                    */
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

#include "debug_geometry.h"
#include "scene/resources/3d/primitive_meshes.h"
#include "scene/resources/3d/shape_3d.h"

Ref<StandardMaterial3D> HBDebugGeometry::get_debug_material() {
	if (debug_material.is_valid()) {
		return debug_material;
	}

	debug_material.instantiate();
	Ref<StandardMaterial3D> mat;
	mat.instantiate();
	mat->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
	mat->set_flag(BaseMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
	mat->set_flag(BaseMaterial3D::FLAG_DISABLE_DEPTH_TEST, true);
	debug_material = mat;
	return debug_material;
}

Ref<StandardMaterial3D> HBDebugGeometry::get_arrow_material() {
	if (arrow_material.is_valid()) {
		return arrow_material;
	}

	arrow_material.instantiate();
	Ref<StandardMaterial3D> mat;
	mat.instantiate();
	mat->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
	mat->set_flag(BaseMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
	mat->set_flag(BaseMaterial3D::FLAG_DISABLE_DEPTH_TEST, true);
	mat->set_cull_mode(BaseMaterial3D::CULL_DISABLED);
	arrow_material = mat;
	return arrow_material;
}

void HBDebugGeometry::_draw_arrow(const Vector3 &p_from, const Vector3 &p_to, const Color &p_color) {
	if (p_from == p_to) {
		return;
	}
	current_group->immediate_mesh->surface_set_color(p_color);
	current_group->immediate_mesh->surface_add_vertex(p_from);
	current_group->immediate_mesh->surface_add_vertex(p_to);

	Vector3 dir = p_from.direction_to(p_to);

	Vector3 normal = (fabs(dir.x) + fabs(dir.y) > CMP_EPSILON) ? Vector3(-dir.y, dir.x, 0).normalized() : Vector3(0, -dir.z, dir.y).normalized();

	for (int i = 0; i < 4; i++) {
		float prog = i / 4.0f;
		float prog_plus_one = (i + 1) / 4.0f;
		Vector3 v_curr = p_to + normal.rotated(dir, prog * Math_TAU) * 0.1f;
		Vector3 v_next = p_to + normal.rotated(dir, prog_plus_one * Math_TAU) * 0.1f;
		v_curr -= dir * 0.1f;
		v_next -= dir * 0.1f;
		current_group->immediate_mesh->surface_add_vertex(v_curr);
		current_group->immediate_mesh->surface_add_vertex(v_next);
		current_group->immediate_mesh->surface_add_vertex(p_to);
		current_group->immediate_mesh->surface_add_vertex(v_curr);
	}
}

Ref<Mesh> HBDebugGeometry::generate_arrow_mesh(float p_length, const Color &p_color) const {
	const float ARROW_HEAD_RADIUS = 0.025f;
	const float ARROW_HEAD_HEIGHT = 0.075f;
	const float BODY_RADIUS = 0.01f;
	const int ARROW_RESOLUTION = 16;

	// We can sometimes skip the body

	float head_height = MIN(p_length, ARROW_HEAD_HEIGHT);
	float body_length = MAX(p_length - head_height, 0.0);

	PackedVector3Array vertex_array;
	PackedInt32Array index_array;

	const int head_vertex_count = ARROW_RESOLUTION + 1; // +1 for the peak of the arrow head
	const int head_index_count = ARROW_RESOLUTION * 3;

	const int body_vertex_count = body_length > 0 ? ARROW_RESOLUTION * 2 : 0;
	const int body_index_count = body_length > 0 ? ARROW_RESOLUTION * 6 : 0;

	// Head
	vertex_array.resize(head_vertex_count + body_vertex_count);
	index_array.resize(head_index_count + body_index_count);

	Vector3 *vertex_ptrw = vertex_array.ptrw();
	int *index_ptrw = index_array.ptrw();

	const int ARROW_PEAK_VERTEX_IDX = ARROW_RESOLUTION;

	vertex_ptrw[ARROW_PEAK_VERTEX_IDX] = Vector3(0.0f, body_length + head_height, 0.0f);
	const Vector3 UP = Vector3(0.0f, 1.0f, 0.0f);

	for (int i = 0; i < ARROW_RESOLUTION; i++) {
		vertex_ptrw[i] = Vector3(ARROW_HEAD_RADIUS, body_length, 0.0).rotated(UP, Math_TAU * (i / (float)ARROW_RESOLUTION));
		int next_point_idx = (i + 1) % ARROW_RESOLUTION;
		index_ptrw[(i * 3) + 0] = i;
		index_ptrw[(i * 3) + 1] = ARROW_PEAK_VERTEX_IDX;
		index_ptrw[(i * 3) + 2] = next_point_idx;
	}

	// Body

	if (body_length > 0) {
		const int body_vtx_offset = head_vertex_count;
		const int body_idx_offset = head_index_count;
		for (int i = 0; i < ARROW_RESOLUTION; i++) {
			const int point_down_idx = body_vtx_offset + (i * 2);
			const int point_up_idx = point_down_idx + 1;
			vertex_ptrw[point_down_idx] = Vector3(BODY_RADIUS, 0.0, 0.0).rotated(UP, Math_TAU * (i / (float)ARROW_RESOLUTION));
			vertex_ptrw[point_up_idx] = Vector3(BODY_RADIUS, body_length, 0.0).rotated(UP, Math_TAU * (i / (float)ARROW_RESOLUTION));

			const int next_point_down_idx = body_vtx_offset + (((i + 1) * 2) % (ARROW_RESOLUTION * 2));
			const int next_point_up_idx = next_point_down_idx + 1;

			const int idx_start = body_idx_offset + (i * 6);
			index_ptrw[idx_start] = point_down_idx;
			index_ptrw[idx_start + 1] = point_up_idx;
			index_ptrw[idx_start + 2] = next_point_up_idx;

			index_ptrw[idx_start + 3] = point_down_idx;
			index_ptrw[idx_start + 4] = next_point_up_idx;
			index_ptrw[idx_start + 5] = next_point_down_idx;
		}
	}

	Array arr;
	arr.resize(Mesh::ARRAY_MAX);
	arr[Mesh::ARRAY_VERTEX] = vertex_array;
	arr[Mesh::ARRAY_INDEX] = index_array;
	PackedColorArray colors;
	colors.resize(vertex_array.size());
	colors.fill(p_color);

	Ref<ArrayMesh> mesh;
	mesh.instantiate();
	BitField<Mesh::ArrayFormat> array_fmt;
	array_fmt.set_flag(Mesh::ArrayFormat::ARRAY_FORMAT_INDEX);
	array_fmt.set_flag(Mesh::ArrayFormat::ARRAY_FORMAT_VERTEX);
	array_fmt.set_flag(Mesh::ArrayFormat::ARRAY_FORMAT_COLOR);
	mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arr, TypedArray<Array>(), Dictionary(), array_fmt);
	mesh->surface_set_material(0, const_cast<HBDebugGeometry *>(this)->get_arrow_material());
	return mesh;
}

void HBDebugGeometry::_bind_methods() {
	ClassDB::bind_method(D_METHOD("clear"), &HBDebugGeometry::clear);
	ClassDB::bind_method(D_METHOD("add_group", "group_name"), &HBDebugGeometry::add_group);
	ClassDB::bind_method(D_METHOD("set_current_group", "group_name"), &HBDebugGeometry::set_current_group);
	ClassDB::bind_method(D_METHOD("debug_line", "from", "to", "color"), &HBDebugGeometry::debug_line);
	ClassDB::bind_method(D_METHOD("debug_sphere", "position", "radius", "color"), &HBDebugGeometry::debug_sphere);
	ClassDB::bind_method(D_METHOD("debug_arrow", "from", "to", "color"), &HBDebugGeometry::debug_arrow);
	ClassDB::bind_method(D_METHOD("set_group_visible", "group_name", "visible"), &HBDebugGeometry::set_group_visible);
	ClassDB::bind_method(D_METHOD("get_group_visible", "group_name"), &HBDebugGeometry::get_group_visible);
}

void HBDebugGeometry::add_group(const StringName &p_group_name) {
	for (int i = 0; i < groups.size(); i++) {
		ERR_FAIL_COND_MSG(groups[i].group_name == p_group_name, vformat("Group %s already exists!", p_group_name));
	}
	DebugGeometryGroup group;
	group.immediate_mesh_instance = memnew(MeshInstance3D);
	group.immediate_mesh_instance->set_material_override(get_debug_material());
	group.immediate_mesh.instantiate();
	group.immediate_mesh_instance->set_mesh(group.immediate_mesh);

	add_child(group.immediate_mesh_instance, false, INTERNAL_MODE_BACK);
	group.group_name = p_group_name;

	groups.push_back(group);

	// Group likely changed address because we resized the group vector
	set_current_group(current_group_name);
}

void HBDebugGeometry::set_current_group(const StringName &p_group_name) {
	DebugGeometryGroup *group = nullptr;
	for (int i = 0; i < groups.size(); i++) {
		group = &groups.write[i];
	}
	ERR_FAIL_COND_MSG(!group, vformat("Group %s does not exist!", p_group_name));

	current_group_name = p_group_name;
	current_group = group;
}

void HBDebugGeometry::clear() {
	current_group->immediate_mesh->clear_surfaces();
	if (current_group->sphere_multi_mesh) {
		current_group->sphere_multi_mesh->get_multimesh()->set_visible_instance_count(0);
		current_group->curr_sphere_instance = 0;
	}

	for (KeyValue<Ref<Shape3D>, MultiMeshInstance3D *> kv : current_group->shape_map) {
		memdelete(kv.value);
	}
	current_group->shape_map.clear();

	for (MeshInstance3D *mi : current_group->arrow_meshes) {
		mi->queue_free();
	}
	current_group->arrow_meshes.clear();
}

void HBDebugGeometry::debug_raycast(const PhysicsDirectSpaceState3D::RayParameters &p_params, const Color &p_color) {
	current_group->immediate_mesh->surface_begin(Mesh::PRIMITIVE_LINES);

	_draw_arrow(p_params.from, p_params.to, p_color);

	current_group->immediate_mesh->surface_end();
}

void HBDebugGeometry::debug_line(const Vector3 &p_from, const Vector3 &p_to, const Color &p_color) {
	current_group->immediate_mesh->surface_begin(Mesh::PRIMITIVE_LINES);
	current_group->immediate_mesh->surface_set_color(p_color);
	current_group->immediate_mesh->surface_add_vertex(p_from);
	current_group->immediate_mesh->surface_add_vertex(p_to);
	current_group->immediate_mesh->surface_end();
}

void HBDebugGeometry::debug_arrow(const Vector3 &p_from, const Vector3 &p_to, const Color &p_color) {
	Ref<ArrayMesh> mesh = generate_arrow_mesh(p_from.distance_to(p_to), p_color);
	MeshInstance3D *mi = memnew(MeshInstance3D);
	add_child(mi);
	Transform3D trf;
	trf.origin = p_from;
	trf.basis = Quaternion(Vector3(0.0, 1.0, 0.0), p_from.direction_to(p_to));
	mi->set_global_transform(trf);
	current_group->arrow_meshes.push_back(mi);
	mi->set_mesh(mesh);
}

void HBDebugGeometry::debug_shape(Ref<Shape3D> p_shape, const Transform3D &p_trf, const Color &p_color) {
	ERR_FAIL_COND(!p_shape.is_valid());
	if (!current_group->shape_map.has(p_shape)) {
		Ref<MultiMesh> mm;
		mm.instantiate();

		Ref<ArrayMesh> debug_mesh = p_shape->get_debug_mesh();

		mm->set_transform_format(MultiMesh::TRANSFORM_3D);
		mm->set_use_colors(true);
		mm->set_mesh(debug_mesh);

		MultiMeshInstance3D *mmi = memnew(MultiMeshInstance3D);
		mmi->set_material_override(get_debug_material());
		mmi->set_multimesh(mm);

		mm->set_instance_count(SPHERE_INSTANCE_COUNT);
		mm->set_visible_instance_count(0);

		current_group->shape_map.insert(p_shape, mmi);
		add_child(mmi);
	}
	Ref<MultiMesh> mm = current_group->shape_map[p_shape]->get_multimesh();
	mm->set_visible_instance_count(mm->get_visible_instance_count() + 1);
	mm->set_instance_transform(mm->get_visible_instance_count() - 1, p_trf);
	mm->set_instance_color(mm->get_visible_instance_count() - 1, p_color);
}

void HBDebugGeometry::debug_sphere(const Vector3 &p_position, float p_radius, const Color &p_color) {
	if (!current_group->sphere_multi_mesh) {
		current_group->sphere_multi_mesh = memnew(MultiMeshInstance3D);
		current_group->sphere_multi_mesh->set_visible(current_group->visible);
		add_child(current_group->sphere_multi_mesh);
		Ref<MultiMesh> multi_mesh;
		multi_mesh.instantiate();
		Ref<SphereMesh> sm;
		sm.instantiate();
		sm->set_radius(1.0f);
		sm->set_height(2.0f);
		multi_mesh->set_mesh(sm);
		multi_mesh->set_use_colors(true);
		multi_mesh->set_transform_format(MultiMesh::TRANSFORM_3D);
		multi_mesh->set_instance_count(SPHERE_INSTANCE_COUNT);
		multi_mesh->set_visible_instance_count(0);
		current_group->sphere_multi_mesh->set_multimesh(multi_mesh);
		print_line("DEBUG MAT", get_debug_material());
		current_group->sphere_multi_mesh->set_material_override(get_debug_material());
	}
	Ref<MultiMesh> mm = current_group->sphere_multi_mesh->get_multimesh();

	mm->set_visible_instance_count(MIN(current_group->curr_sphere_instance + 1, SPHERE_INSTANCE_COUNT));
	int mm_idx = current_group->curr_sphere_instance;

	Transform3D trf;
	trf.origin = p_position;
	trf.basis.scale(Vector3(1.0f, 1.0f, 1.0f) * p_radius);
	mm->set_instance_transform(mm_idx, trf);
	mm->set_instance_color(mm_idx, p_color);
	current_group->curr_sphere_instance = (current_group->curr_sphere_instance + 1) % SPHERE_INSTANCE_COUNT;
}

void HBDebugGeometry::debug_cast_motion(const Ref<Shape3D> &p_shape, const PhysicsDirectSpaceState3D::ShapeParameters &p_shape_cast_3d, const Color &p_color) {
	if (!p_shape_cast_3d.motion.is_zero_approx()) {
		current_group->immediate_mesh->surface_begin(Mesh::PRIMITIVE_LINES);
		_draw_arrow(p_shape_cast_3d.transform.origin, p_shape_cast_3d.transform.origin + p_shape_cast_3d.motion, p_color);
		current_group->immediate_mesh->surface_end();
	}
	debug_shape(p_shape, p_shape_cast_3d.transform, p_color);
}

void HBDebugGeometry::set_group_visible(const StringName &p_group_name, bool p_visible) {
	DebugGeometryGroup *group = nullptr;
	for (int i = 0; i < groups.size(); i++) {
		group = &groups.write[i];
	}

	ERR_FAIL_COND_MSG(!group, vformat("Group %s does not exist!", p_group_name));

	group->visible = p_visible;
	group->immediate_mesh_instance->set_visible(p_visible);
	if (group->sphere_multi_mesh) {
		group->sphere_multi_mesh->set_visible(p_visible);
	}
	for (KeyValue<Ref<Shape3D>, MultiMeshInstance3D *> kv : group->shape_map) {
		kv.value->set_visible(p_visible);
	}
	for (MeshInstance3D *mi : group->arrow_meshes) {
		mi->set_visible(p_visible);
	}
}

bool HBDebugGeometry::get_group_visible(const StringName &p_group_name) const {
	const DebugGeometryGroup *group = nullptr;
	for (int i = 0; i < groups.size(); i++) {
		group = &groups[i];
	}

	ERR_FAIL_COND_V_MSG(!group, false, vformat("Group %s does not exist!", p_group_name));
	return group->visible;
}

int HBDebugGeometry::get_group_count() const {
	return groups.size();
}

StringName HBDebugGeometry::get_group_name(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, groups.size(), "");
	return groups[p_idx].group_name;
}

HBDebugGeometry::HBDebugGeometry() {
	current_group_name = "default";
	add_group("default");
}
