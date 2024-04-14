/**************************************************************************/
/*  user_config.cpp                                                       */
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

#include "user_config.h"

#include "scene/main/scene_tree.h"
#include "scene/main/window.h"

CVar HBUserConfig::window_mode_cvar = CVar("window_mode", Variant::INT, 0, "HBUserConfig");
CVar HBUserConfig::render_scale_cvar = CVar("render_scale", Variant::FLOAT, 0, "HBUserConfig");
CVar HBUserConfig::render_scale_mode_cvar = CVar("render_scale_mode", Variant::INT, 0, "HBUserConfig");

void HBUserConfig::_update_window_settings() {
	window_mode_cvar.data->set_value_no_signals(window_mode);
	switch (window_mode) {
		case FULLSCREEN:
		case WINDOWED: {
			SceneTree::get_singleton()->get_root()->set_flag(Window::FLAG_BORDERLESS, false);
		} break;
		case BORDERLESS_WINDOWED: {
			SceneTree::get_singleton()->get_root()->set_flag(Window::FLAG_BORDERLESS, true);
		} break;
	}

	switch (window_mode) {
		case WINDOWED: {
			SceneTree::get_singleton()->get_root()->set_mode(Window::MODE_WINDOWED);
		} break;
		case BORDERLESS_WINDOWED:
		case FULLSCREEN: {
			SceneTree::get_singleton()->get_root()->set_mode(Window::MODE_FULLSCREEN);
		} break;
	}
}

void HBUserConfig::_update_screen_settings() {
	const DisplayServer *ds = DisplayServer::get_singleton();
	const bool is_screen_valid = screen >= ds->get_screen_count() || screen < 0;
	int screen_to_set = screen;
	if (!is_screen_valid) {
		screen_to_set = ds->get_primary_screen();
	}
	SceneTree::get_singleton()->get_root()->set_current_screen(screen_to_set);
}

void HBUserConfig::_update_render_scale() {
	SceneTree::get_singleton()->get_root()->set_scaling_3d_scale(render_scale);
	render_scale_cvar.data->set_value_no_signals(render_scale);
}

void HBUserConfig::_update_render_scale_mode() {
	Viewport::Scaling3DMode scaling_mode = Viewport::SCALING_3D_MODE_BILINEAR;
	switch (render_scale_mode) {
		case BILINEAR: {
			scaling_mode = Viewport::SCALING_3D_MODE_BILINEAR;
		} break;
		case FSR: {
			scaling_mode = Viewport::SCALING_3D_MODE_FSR;
		} break;
		case FSR2: {
			scaling_mode = Viewport::SCALING_3D_MODE_FSR2;
		} break;
	}
	SceneTree::get_singleton()->get_root()->set_scaling_3d_mode(scaling_mode);
	render_scale_mode_cvar.data->set_value_no_signals(scaling_mode);
}

void HBUserConfig::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_window_mode", "window_mode"), &HBUserConfig::set_window_mode);
	ClassDB::bind_method(D_METHOD("get_window_mode"), &HBUserConfig::get_window_mode);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "window_mode", PROPERTY_HINT_ENUM, "Fullscreen,Borderless Windowed,Windowed"), "set_window_mode", "get_window_mode");

	ClassDB::bind_method(D_METHOD("set_render_scale", "render_scale"), &HBUserConfig::set_render_scale);
	ClassDB::bind_method(D_METHOD("get_render_scale"), &HBUserConfig::get_render_scale);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "render_scale"), "set_render_scale", "get_render_scale");

	ClassDB::bind_method(D_METHOD("set_render_scale_mode", "render_scale_mode"), &HBUserConfig::set_render_scale_mode);
	ClassDB::bind_method(D_METHOD("get_render_scale_mode"), &HBUserConfig::get_render_scale_mode);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "render_scale_mode", PROPERTY_HINT_ENUM, "Bilinear,FSR1,FSR2"), "set_render_scale_mode", "get_render_scale_mode");

	ClassDB::bind_method(D_METHOD("get_window_mode_text"), &HBUserConfig::get_window_mode_text);
	ClassDB::bind_method(D_METHOD("get_available_screens"), &HBUserConfig::get_available_screens);
	ClassDB::bind_method(D_METHOD("apply"), &HBUserConfig::apply);
}

String HBUserConfig::get_window_mode_text(WindowMode p_window_mode) const {
	switch (window_mode) {
		case FULLSCREEN: {
			return RTR("Fullscreen");
		} break;
		case BORDERLESS_WINDOWED: {
			return RTR("Borderless windowed");
		} break;
		case WINDOWED: {
			return RTR("Windowed");
		} break;
	}
}

HBUserConfig::HBUserConfig() {
	window_mode_cvar.data->get_signaler()->connect("value_changed", callable_mp(this, &HBUserConfig::set_window_mode));
	render_scale_cvar.data->get_signaler()->connect("value_changed", callable_mp(this, &HBUserConfig::set_render_scale));
	render_scale_mode_cvar.data->get_signaler()->connect("value_changed", callable_mp(this, &HBUserConfig::set_render_scale_mode));
}

void HBUserConfig::set_window_mode(WindowMode p_window_mode) {
	window_mode = p_window_mode;
	_update_window_settings();
}

HBUserConfig::WindowMode HBUserConfig::get_window_mode() const {
	return window_mode;
}

void HBUserConfig::set_render_scale(float p_render_scale) {
	render_scale = p_render_scale;
	_update_render_scale();
}

float HBUserConfig::get_render_scale() const {
	return render_scale;
}

void HBUserConfig::set_render_scale_mode(const RenderScaleMode p_render_scale_mode) {
	render_scale_mode = p_render_scale_mode;
	_update_render_scale_mode();
}

void HBUserConfig::apply() {
	_update_window_settings();
}

TypedArray<Vector2i> HBUserConfig::get_available_screens() const {
	const DisplayServer *ds = DisplayServer::get_singleton();
	int screen_count = ds->get_screen_count();
	TypedArray<Size2i> screens;
	;
	for (int i = 0; i < screen_count; i++) {
		screens.push_back(ds->screen_get_size(i));
	}
	return screens;
}
