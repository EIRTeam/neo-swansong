#include "builder.h"

#include "core/io/resource_loader.h"
#include "scene/3d/light_3d.h"
#include "scene/3d/physics/area_3d.h"
#include "scene/3d/physics/static_body_3d.h"
#include "scene/3d/physics/collision_shape_3d.h"
#include "scene/resources/3d/shape_3d.h"
#include "scene/resources/packed_scene.h"
#include "scene/resources/material.h"
#include "scene/resources/3d/concave_polygon_shape_3d.h"
#include "scene/resources/3d/convex_polygon_shape_3d.h"

#include <tb_loader.h>
#include "tb_loader_singleton.h"

Builder::Builder(TBLoader* loader)
{
	m_loader = loader;
	m_map = std::make_shared<LMMapData>();
}

Builder::~Builder()
{
}

void Builder::load_map(const String& path)
{
	print_line("Building map ", path);

	Ref<FileAccess> f1 = FileAccess::create(FileAccess::ACCESS_RESOURCES);

	if (!f1->file_exists(path)) {
		print_error("Map file does not exist!");
		return;
	}

	// Parse the map from the file
	Ref<FileAccess> f = FileAccess::open(path, FileAccess::ModeFlags::READ);
	LMMapParser parser(m_map);
	parser.load_from_godot_file(f);

	load_and_cache_map_textures();

	// We have to manually set the size of textures
	for (int i = 0; i < m_map->texture_count; i++) {
		auto& tex = m_map->textures[i];

		auto res_texture = texture_from_name(tex.name);
		if (res_texture != nullptr) {
			tex.width = res_texture->get_width();
			tex.height = res_texture->get_height();
		} else {
			// Make sure we don't divide by 0 and create NaN UV's
			tex.width = 1;
			tex.height = 1;
		}
	}

	// Run geometry generator (this also generates UV's, so we do this last)
	LMGeoGenerator geogen(m_map);
	geogen.run();
}

void Builder::build_map() {
	HashMap<StringName, TBLoaderEntity::EntityCompileInfo> built_entities;
	TBLoaderBuildInfo build_info;
	for (int i = 0; i < m_map->entity_count; i++) {
		auto& ent = m_map->entities[i];
		Node3D *output_node = nullptr;
		build_entity(i, ent, ent.get_property("classname"), &output_node, &build_info);
		if (!output_node) {
			continue;
		}
		TBLoaderEntity *tb_entity = Object::cast_to<TBLoaderEntity>(output_node);
		if (tb_entity) {
			TBLoaderEntity::EntityCompileInfo compile_info;
			if (ent.has_property("target")) {
				compile_info.children.push_back(ent.get_property("target"));
			}
			compile_info.node = output_node;
			compile_info.entity = tb_entity;
			built_entities.insert(output_node->get_name(), compile_info);
		}
	}

	for (KeyValue<StringName, TBLoaderEntity::EntityCompileInfo> &kv : built_entities) {
		TBLoaderEntity::EntityCompileInfo *compile_info = &kv.value;
		for (StringName child : compile_info->children) {
			TBLoaderEntity::EntityCompileInfo *target = built_entities.getptr(child);
			if (!target) {
				continue;
			}
			target->parents.push_back(kv.key);
		}
	}

	for (Ref<TBLoaderHook> hook : TBLoaderSingleton::compile_hooks) {
		hook->post_compile_hook(m_loader, &build_info);
	}

	for (KeyValue<StringName, TBLoaderEntity::EntityCompileInfo> &kv : built_entities) {
		kv.value.entity->_editor_build(kv.value, built_entities);
	}
}

void Builder::build_worldspawn(int idx, LMEntity& ent, TBLoaderEntityBuildInfo *p_build_info)
{

	// Decide generated collision type
	ColliderType collider = ColliderType::None;
	ColliderShape collider_shape = ColliderShape::Concave;
	
	bool is_illusionary = strcmp(ent.get_property("classname"), "func_illusionary") == 0;
	
	if (m_loader->m_collision && !is_illusionary) {
		collider = ColliderType::Static;
		collider_shape = ColliderShape::Concave;
	}

	if (!is_illusionary) {
		// Normal geometry

		// Create node for this entity
		auto container_node = memnew(Node3D());
		m_loader->add_child(container_node);
		container_node->set_owner(m_loader->get_owner());
		
		build_entity_mesh(idx, ent, container_node, collider, collider_shape, p_build_info);
		
		// Delete container if we added nothing to it
		if (container_node->get_child_count() == 0) {
			container_node->queue_free();
			return;
		}
		// Find name for entity
		const char* tb_name;
		if (!strcmp(ent.get_property("classname"), "worldspawn")) {
			tb_name = "Default Layer";
		} else {
			tb_name = ent.get_property("_tb_name", nullptr);
		}

		// Add container to loader
		if (tb_name != nullptr) {
			container_node->set_name(tb_name);
		}
		container_node->set_position(lm_transform(ent.center));
	} else {
		// illusionary geometry
		MeshInstance3D *mi = build_entity_mesh(idx, ent, m_loader, collider, collider_shape);
		mi->set_position(lm_transform(ent.center));
	}
}

void Builder::build_entity(int idx, LMEntity& ent, const String& classname, Node3D **p_output_node, TBLoaderBuildInfo *p_build_info)
{
	*p_output_node = nullptr;
	TBLoaderEntityBuildInfo build_info;
	build_info.class_name = classname;
	if (classname == "worldspawn" || classname == "func_group" || classname == "func_illusionary") {
		// Skip worldspawn if the layer is hidden and the "skip hidden layers" option is checked
		if (m_loader->m_skip_hidden_layers) {
			bool is_visible = (ent.get_property_int("_tb_layer_hidden", 0) == 0);
			if (!is_visible) {
				return;
			}
		}
		build_worldspawn(idx, ent, &build_info);
		p_build_info->entities.push_back(build_info);
		if (build_info.collision_shape) {
			p_build_info->worldspawn_collision_shapes.push_back(build_info.collision_shape);
		}
		return;
	}

	if (m_loader->m_entity_common) {
		if (classname == "light") {
			build_entity_light(idx, ent);
			return;
		}

		if (classname == "area") {
			build_entity_area(idx, ent);
			return;
		}

		//TODO: More common entities
	}



	// Try to build using C++ registered entities
	if (build_entity_custom_native(idx, ent, m_map->entity_geo[idx], classname, p_output_node)) {
		return;
	}

	// That didn't work, so let's fall back to NativeScript defined res://entities
	build_entity_custom(idx, ent, m_map->entity_geo[idx], classname);
}

void Builder::build_entity_custom(int idx, LMEntity& ent, LMEntityGeometry& geo, const String& classname)
{
	// m_loader->m_entity_path => "res://entities/"
	// "info_player_start" => "info/player/start.tscn", "info/player_start.tscn", "info_player_start.tscn"
	// "thing" => "thing.tscn"

	auto arr = classname.split("_");
	for (int i = 0; i < arr.size(); i++) {
		String path = m_loader->m_entity_path + "/";
		for (int j = 0; j < arr.size(); j++) {
			if (j > 0) {
				if (j <= i) {
					path = path + "/";
				} else {
					path = path + "_";
				}
			}
			path = path + arr[j];
		}
		path = path + ".tscn";

		if (ResourceLoader::exists(path, "PackedScene")) {
			Ref<PackedScene> scene = ResourceLoader::load(path);
			if (scene == nullptr) {
				print_error("Resource at path '" + path + "' could not be loaded as a PackedScene by the resource loader!");
				return;
			}

			auto instance = scene->instantiate();
			m_loader->add_child(instance);
			instance->set_owner(m_loader->get_owner());

			if (instance->is_class("Node3D")) {
				set_entity_node_common((Node3D*)instance, ent);
				if (ent.brush_count > 0) {
					set_entity_brush_common(idx, (Node3D*)instance, ent);
				}
			}

			for (int j = 0; j < ent.property_count; j++) {
				auto& prop = ent.properties[j];

				auto var = instance->get(prop.key);
				switch (var.get_type()) {
					case Variant::BOOL: instance->set(prop.key, atoi(prop.value) == 1); break;
					case Variant::INT: instance->set(prop.key, (int64_t)atoll(prop.value)); break;
					case Variant::FLOAT: instance->set(prop.key, atof(prop.value)); break; //TODO: Locale?
					case Variant::STRING: instance->set(prop.key, prop.value); break;

					case Variant::STRING_NAME: instance->set(prop.key, StringName(prop.value)); break;
					case Variant::NODE_PATH: instance->set(prop.key, NodePath(prop.value)); break;

					case Variant::VECTOR2: {
						vec2 v = vec2_parse(prop.value);
						instance->set(prop.key, Vector2(v.x, v.y));
						break;
					}
					case Variant::VECTOR2I: {
						vec2 v = vec2_parse(prop.value);
						instance->set(prop.key, Vector2i((int)v.x, (int)v.y));
						break;
					}
					case Variant::VECTOR3: {
						vec3 v = vec3_parse(prop.value);
						instance->set(prop.key, Vector3(v.x, v.y, v.z));
						break;
					}
					case Variant::VECTOR3I: {
						vec3 v = vec3_parse(prop.value);
						instance->set(prop.key, Vector3i((int)v.x, (int)v.y, (int)v.z));
						break;
					}

					case Variant::COLOR: {
						vec3 v = vec3_parse(prop.value);
						instance->set(prop.key, Color(v.x / 255.0f, v.y / 255.0f, v.z / 255.0f));
						break;
					}
					default: {
						ERR_FAIL_MSG(vformat("Couldn't parse entity property of type %s", Variant::get_type_name(var.get_type())));
					} break;
				}
			}
			return;
		}
	}

	print_error("Path to entity resource could not be resolved: " + classname);
}

bool Builder::build_entity_custom_native(int p_idx, LMEntity &p_ent, LMEntityGeometry &p_geo, const String &p_class_name, Node3D **p_output_node) {
	*p_output_node = nullptr;
	Node3D *entity = TBLoaderSingleton::create_entity(p_class_name);
	if (!entity) {
		return false;
	}

	m_loader->add_child(entity);
	entity->set_owner(m_loader->get_owner());
	*p_output_node = entity;

	set_entity_node_common(entity, p_ent);
	if (p_ent.brush_count > 0) {
		set_entity_brush_common(p_idx, entity, p_ent);
	}

	for (int j = 0; j < p_ent.property_count; j++) {
		auto& prop = p_ent.properties[j];

		auto var = entity->get(prop.key);
		switch (var.get_type()) {
			case Variant::BOOL: entity->set(prop.key, atoi(prop.value) == 1); break;
			case Variant::INT: entity->set(prop.key, (int64_t)atoll(prop.value)); break;
			case Variant::FLOAT: entity->set(prop.key, atof(prop.value)); break; //TODO: Locale?
			case Variant::STRING: entity->set(prop.key, prop.value); break;

			case Variant::STRING_NAME: entity->set(prop.key, StringName(prop.value)); break;
			case Variant::NODE_PATH: entity->set(prop.key, NodePath(prop.value)); break; //TODO: More TrenchBroom focused node path conversion?

			case Variant::VECTOR2: {
				vec2 v = vec2_parse(prop.value);
				entity->set(prop.key, Vector2(v.x, v.y));
				break;
			}
			case Variant::VECTOR2I: {
				vec2 v = vec2_parse(prop.value);
				entity->set(prop.key, Vector2i((int)v.x, (int)v.y));
				break;
			}
			case Variant::VECTOR3: {
				vec3 v = vec3_parse(prop.value);
				entity->set(prop.key, Vector3(v.x, v.y, v.z));
				break;
			}
			case Variant::VECTOR3I: {
				vec3 v = vec3_parse(prop.value);
				entity->set(prop.key, Vector3i((int)v.x, (int)v.y, (int)v.z));
				break;
			}

			case Variant::COLOR: {
				vec3 v = vec3_parse(prop.value);
				entity->set(prop.key, Color(v.x / 255.0f, v.y / 255.0f, v.z / 255.0f));
				break;
			}
			default: {
				ERR_FAIL_V_MSG(false, vformat("Couldn't parse entity property of type %s", Variant::get_type_name(var.get_type())));
			}
		}
	}
	return true;
}

void Builder::build_entity_light(int idx, LMEntity& ent)
{
	auto light = memnew(OmniLight3D());

	light->set_bake_mode(Light3D::BAKE_STATIC);
	light->set_param(Light3D::PARAM_RANGE, ent.get_property_double("range", 10));
	light->set_param(Light3D::PARAM_ENERGY, ent.get_property_double("energy", 1));
	light->set_param(Light3D::PARAM_ATTENUATION, ent.get_property_double("attenuation", 1));
	light->set_param(Light3D::PARAM_SPECULAR, ent.get_property_double("specular", 0.5));
	set_entity_node_common(light, ent);

	vec3 color = ent.get_property_vec3("light_color", { 255, 255, 255 });
	light->set_color(Color(color.x / 255.0f, color.y / 255.0f, color.z / 255.0f));

	m_loader->add_child(light);
	light->set_owner(m_loader->get_owner());
}

void Builder::build_entity_area(int idx, LMEntity& ent)
{
	Vector3 center = lm_transform(ent.center);

	// Gather surfaces for the area
	LMSurfaceGatherer surf_gather(m_map);
	surf_gather.surface_gatherer_set_entity_index_filter(idx);
	surf_gather.surface_gatherer_run();

	auto& surfs = surf_gather.out_surfaces;
	if (surfs.surface_count == 0) {
		return;
	}

	for (int i = 0; i < surfs.surface_count; i++) {
		auto& surf = surfs.surfaces[i];
		if (surf.vertex_count == 0) {
			continue;
		}

		// Create the mesh
		Ref<ArrayMesh> mesh = memnew(ArrayMesh());
		add_surface_to_mesh(mesh, surf);

		// Create the area
		auto area = memnew(Area3D());
		m_loader->add_child(area);
		area->set_owner(m_loader->get_owner());
		area->set_position(center);

		// Create collision shape for the area
		add_collider_from_mesh(area, mesh, ColliderShape::Concave);
	}
}

void Builder::set_entity_node_common(Node3D* node, LMEntity& ent)
{
	// Target name
	auto targetname = ent.get_property("targetname", nullptr);
	if (targetname != nullptr) {
		node->set_name(targetname);
	}

	// Position
	if (ent.has_property("origin")) {
		Vector3 origin = lm_transform(ent.get_property_vec3("origin"));
		node->set_position(origin);
	}

	// Brush entities shouldn't be rotated as they are already in mesh space
	if (ent.brush_count == 0) {
		// Rotation
		double pitch = 0;
		double yaw = 0;
		double roll = 0;

		if (ent.has_property("angle")) {
			// "angle" is yaw rotation only
			yaw = ent.get_property_double("angle");
		} else if (ent.has_property("angles")) {
			// "angles" is "pitch yaw roll"
			vec3 angles = ent.get_property_vec3("angles");
			pitch = angles.x;
			yaw = angles.y;
			roll = angles.z;
		} else if (ent.has_property("mangle")) {
			vec3 mangle = ent.get_property_vec3("mangle");
			// "mangle" depends on whether the classname starts with "light"
			const char* classname = ent.get_property("classname");
			if (strstr(classname, "light") == classname) {
				// "yaw pitch roll", if classname starts with "light"
				yaw = mangle.x;
				pitch = mangle.y;
				roll = mangle.z;
			} else {
				// "pitch yaw roll", just like "angles"
				pitch = mangle.x;
				yaw = mangle.y;
				roll = mangle.z;
			}
		}

		node->set_rotation(Vector3(
			Math::deg_to_rad(-pitch),
			Math::deg_to_rad(yaw + 180),
			Math::deg_to_rad(roll)
		));
	}
}

void Builder::set_entity_brush_common(int idx, Node3D* node, LMEntity& ent)
{
	// Position
	Vector3 center = lm_transform(ent.center);
	node->set_position(center);

	// Check what we actually need
	bool need_visual = node->is_class("Node3D");
	ColliderType need_collider = ColliderType::None;
	ColliderShape need_collider_shape = ColliderShape::Concave;

	if (node->is_class("RigidBody3D")) {
		// RigidBody3D requires convex collision meshes
		need_collider = ColliderType::Mesh;
		need_collider_shape = ColliderShape::Convex;

	} else if (node->is_class("Area3D")) {
		// Area3D works best with convex collision meshes
		need_collider = ColliderType::Mesh;
		need_collider_shape = ColliderShape::Convex;

	} else if (node->is_class("CollisionObject3D")) {
		// If it's not a dynamic body, we can just use a concave trimesh collider
		need_collider = ColliderType::Mesh;
		need_collider_shape = ColliderShape::Concave;
	}

	// Stop if we don't need to do anything
	if (!need_visual && need_collider == ColliderType::None) {
		print_error("Brush entity class has no need for visual nor collision: " + node->get_class());
		return;
	}

	build_entity_mesh(idx, ent, node, need_collider, need_collider_shape);
}

Vector3 Builder::lm_transform(const vec3& v)
{
	vec3 sv = vec3_div_double(v, m_loader->m_inverse_scale);
	return Vector3(sv.y, sv.z, sv.x);
}

void Builder::add_collider_from_mesh(Node3D* node, Ref<ArrayMesh>& mesh, ColliderShape colshape, TBLoaderEntityBuildInfo *p_build_info)
{
	Ref<Shape3D> mesh_shape;
	switch (colshape) {
		case ColliderShape::Convex: mesh_shape = mesh->create_convex_shape(); break;
		case ColliderShape::Concave: {
			Ref<ConcavePolygonShape3D> conc = mesh->create_trimesh_shape();
			conc->set_backface_collision_enabled(true);
			mesh_shape = conc;
		} break;
	}

	if (mesh_shape == nullptr) {
		print_error("Unable to create collider shape from mesh!");
		return;
	}

	auto collision_shape = memnew(CollisionShape3D());
	collision_shape->set_shape(mesh_shape);
	node->add_child(collision_shape, true);
	collision_shape->set_owner(m_loader->get_owner());
	if (p_build_info) {
		p_build_info->collision_shape = collision_shape;
	}
}

void Builder::add_surface_to_mesh(Ref<ArrayMesh>& mesh, LMSurface& surf)
{
	PackedVector3Array vertices;
	PackedFloat32Array tangents;
	PackedVector3Array normals;
	PackedVector2Array uvs;
	PackedInt32Array indices;

	// Add all vertices
	for (int k = 0; k < surf.vertex_count; k++) {
		auto& v = surf.vertices[k];

		vertices.push_back(lm_transform(v.vertex));
		tangents.push_back(v.tangent.y);
		tangents.push_back(v.tangent.z);
		tangents.push_back(v.tangent.x);
		tangents.push_back(v.tangent.w);
		normals.push_back(Vector3(v.normal.y, v.normal.z, v.normal.x));
		uvs.push_back(Vector2(v.uv.u, v.uv.v));
	}

	// Add all indices
	for (int k = 0; k < surf.index_count; k++) {
		indices.push_back(surf.indices[k]);
	}

	Array arrays;
	arrays.resize(Mesh::ARRAY_MAX);
	arrays[Mesh::ARRAY_VERTEX] = vertices;
	arrays[Mesh::ARRAY_TANGENT] = tangents;
	arrays[Mesh::ARRAY_NORMAL] = normals;
	arrays[Mesh::ARRAY_TEX_UV] = uvs;
	arrays[Mesh::ARRAY_INDEX] = indices;

	// Create mesh
	mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
}

MeshInstance3D* Builder::build_entity_mesh(int idx, LMEntity& ent, Node3D* parent, ColliderType coltype, ColliderShape colshape, TBLoaderEntityBuildInfo *p_build_info)
{
	// Create instance name based on entity idx
	String instance_name = vformat("entity_%d_geometry", idx);

	auto mesh_instance = memnew(MeshInstance3D());

	parent->add_child(mesh_instance);

	// Set the layers that the mesh instance will be rendered in
	mesh_instance->set_layer_mask(m_loader->get_visual_layer_mask());
	mesh_instance->set_owner(m_loader->get_owner());
	mesh_instance->set_name(instance_name);

	// Create mesh
	Ref<ArrayMesh> mesh = memnew(ArrayMesh());
	Ref<ArrayMesh> collision_mesh = memnew(ArrayMesh());

	// Give mesh to mesh instance
	mesh_instance->set_mesh(mesh);

	for (int i = 0; i < m_map->texture_count; i++) {
		LMTextureData tex = m_map->textures[i];

		// Create material
		Ref<Material> material;

		// Skip processing a surface when it's using the skip material
		if (tex.name == m_loader->get_skip_texture_name()) {
			continue;
		}

		// Attempt to load material
		material = material_from_name(tex.name);

		if (material == nullptr) {
			// Load texture
			auto res_texture = texture_from_name(tex.name);

			// Create material
			if (res_texture != nullptr) {
				Ref<StandardMaterial3D> new_material = memnew(StandardMaterial3D());
				new_material->set_texture(BaseMaterial3D::TEXTURE_ALBEDO, res_texture);
				new_material->set_texture_filter(BaseMaterial3D::TEXTURE_FILTER_LINEAR_WITH_MIPMAPS_ANISOTROPIC);
				if (m_loader->m_filter_nearest) {
					new_material->set_texture_filter(BaseMaterial3D::TEXTURE_FILTER_NEAREST);
				}
				material = new_material;
			}
		}

		// Gather surfaces for this texture
		LMSurfaceGatherer surf_gather(m_map);
		surf_gather.surface_gatherer_set_entity_index_filter(idx);
		surf_gather.surface_gatherer_set_texture_filter(tex.name);
		surf_gather.surface_gatherer_run();

		auto& surfs = surf_gather.out_surfaces;
		if (surfs.surface_count == 0) {
			continue;
		}

		for (int j = 0; j < surfs.surface_count; j++) {
			auto& surf = surfs.surfaces[j];
			if (surf.vertex_count == 0) {
				continue;
			}

			// Add surface to collision mesh
			add_surface_to_mesh(collision_mesh, surf);

			// Skip if the texture specifies that we only want collision (invisible walls)
			if (tex.name == m_loader->get_clip_texture_name()) {
				continue;
			}

			// Add surface to visual mesh
			add_surface_to_mesh(mesh, surf);

			// Give mesh material
			if (material != nullptr) {
				mesh->surface_set_material(mesh->get_surface_count() - 1, material);
			}
		}
	}

	// Unwrap UV2's if needed
	if (m_loader->m_lighting_unwrap_uv2) {
		mesh->lightmap_unwrap(mesh_instance->get_global_transform(), 0.05);
		mesh_instance->set_gi_mode(GeometryInstance3D::GI_MODE_STATIC);
	}

	// Create collisions if needed
	switch (coltype) {
		case ColliderType::Mesh:{
			add_collider_from_mesh(parent, collision_mesh, colshape, p_build_info);
		} break;

		case ColliderType::Static: {		
				StaticBody3D* static_body = memnew(StaticBody3D());
				static_body->set_name(String(mesh_instance->get_name()) + "_col");
				parent->add_child(static_body, true);
				static_body->set_owner(m_loader->get_owner());
				add_collider_from_mesh(static_body, collision_mesh, colshape, p_build_info);
		} break;
		case ColliderType::None: {

		} break;
	}

	return mesh_instance;
}

void Builder::load_and_cache_map_textures()
{
	m_loaded_map_textures.clear();

	// Setup a texture extension list that both Trenchbroom and Godot support
	constexpr int num_extensions = 9;
	constexpr const char* supported_extensions[num_extensions] = { "png", "dds", "tga", "jpg", "jpeg", "bmp", "webp", "exr", "hdr" };

	// Attempt to load and cache textures used by the map
	String tex_path;

	for (int tex_i = 0; tex_i < m_map->texture_count; tex_i++) {
		bool has_loaded_texture = false;
		const LMTextureData& tex = m_map->textures[tex_i];

		// Find the texture with a supported extension - stop when it can be loaded
		for (int ext_i = 0; ext_i < num_extensions; ext_i++) {
			tex_path = texture_path(tex.name, supported_extensions[ext_i]);
			if (ResourceLoader::exists(tex_path, "CompressedTexture2D")) {
				m_loaded_map_textures[tex.name] = ResourceLoader::load(tex_path);
				has_loaded_texture = true;
				break;
			}
		}

		if (!has_loaded_texture && strcmp(tex.name, "__TB_empty") != 0) {
			print_error("Texture cannot be found or is unsupported! - " + m_loader->m_texture_path + tex.name);
			
			if (m_loader->m_texture_path.is_empty()) {
				print_error("texture_path is empty");
			}
		}
	}
}

String Builder::texture_path(const char* name, const char* extension)
{
	return m_loader->m_texture_path + "/" + name + "." + extension;
}

String Builder::material_path(const char* name)
{
	auto root_path = m_loader->m_texture_path + "/" + name;
	String material_path;

	Ref<FileAccess> f = FileAccess::create(FileAccess::ACCESS_RESOURCES);

	if (f->file_exists(root_path + ".material")) {
		material_path = root_path + ".material";
	} else if (f->file_exists(root_path + ".tres")) {
		material_path = root_path + ".tres";
	}

	return material_path;
}

Ref<Texture2D> Builder::texture_from_name(const char* name)
{
	if (!m_loaded_map_textures.has(name)) {
		return nullptr;
	}
	return VariantCaster<Ref<Texture2D>>::cast(m_loaded_map_textures[name]);
}

Ref<Material> Builder::material_from_name(const char* name)
{
	auto path = material_path(name);

	if (!ResourceLoader::exists(path)) {
		return nullptr;
	}
	return ResourceLoader::load(path);
}

void Builder::add_compile_hook(Ref<TBLoaderHook> p_compile_hook) {
	compile_hooks.push_back(p_compile_hook);
}