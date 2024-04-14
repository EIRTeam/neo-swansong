/**************************************************************************/
/*  user_config.h                                                         */
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

#ifndef USER_CONFIG_H
#define USER_CONFIG_H

#include "../console_system.h"
#include "core/object/class_db.h"

class HBUserConfig : public Object {
	GDCLASS(HBUserConfig, Object);

public:
	enum WindowMode {
		FULLSCREEN,
		BORDERLESS_WINDOWED,
		WINDOWED,
	};

	enum RenderScaleMode {
		BILINEAR,
		FSR,
		FSR2
	};

	WindowMode window_mode = WindowMode::BORDERLESS_WINDOWED;
	RenderScaleMode render_scale_mode = RenderScaleMode::BILINEAR;

	int screen = -1;
	float render_scale = 1.0;

	void _update_window_settings();
	void _update_screen_settings();
	void _update_render_scale();
	void _update_render_scale_mode();

protected:
	static CVar window_mode_cvar;
	static CVar render_scale_cvar;
	static CVar render_scale_mode_cvar;
	static void _bind_methods();

public:
	void set_window_mode(WindowMode p_window_mode);
	WindowMode get_window_mode() const;

	void set_render_scale(float p_render_scale);
	float get_render_scale() const;

	RenderScaleMode get_render_scale_mode() const { return render_scale_mode; }
	void set_render_scale_mode(const RenderScaleMode p_render_scale_mode);

	void apply();
	TypedArray<Vector2i> get_available_screens() const;
	String get_window_mode_text(WindowMode p_window_mode) const;

	HBUserConfig();
};

VARIANT_ENUM_CAST(HBUserConfig::WindowMode);
VARIANT_ENUM_CAST(HBUserConfig::RenderScaleMode);

#endif // USER_CONFIG_H
