/**************************************************************************/
/*  register_types.cpp                                                    */
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

#include "register_types.h"

#include "core/config/project_settings.h"
#include "godot_imgui.h"
#include "scene/main/scene_tree.h"
#include "scene/main/window.h"

GodotImGui *gd_imgui_singleton = nullptr;

void imgui_module_post_init() {
	if (!RenderingDevice::get_singleton()) {
		print_verbose("GodotImGui: RenderingDevice not found, running in OpenGL?");
		return;
	}
	if (Engine::get_singleton()->is_editor_hint()) {
		print_verbose("GodotImGui: Running in the editor, disabling imgui.");
		return;
	}
	gd_imgui_singleton = memnew(GodotImGui);
	SceneTree *st = SceneTree::get_singleton();
	if (st) {
		st->get_root()->add_child(gd_imgui_singleton, true, Node::INTERNAL_MODE_BACK);
	}
	Engine::get_singleton()->add_singleton(Engine::Singleton("GodotImGui", GodotImGui::get_singleton()));
}

void imgui_module_unload() {
	if (gd_imgui_singleton) {
		memdelete(gd_imgui_singleton);
	}
}

void initialize_imgui_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		GDREGISTER_ABSTRACT_CLASS(GodotImGui);
	}
}

void uninitialize_imgui_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
	}
}
