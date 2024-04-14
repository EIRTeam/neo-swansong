/**************************************************************************/
/*  input_glyph_demo_tool.h                                               */
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

#ifndef INPUT_GLYPH_DEMO_TOOL_H
#define INPUT_GLYPH_DEMO_TOOL_H

#include "modules/modules_enabled.gen.h"

#if defined(MODULE_STEAMWORKS_ENABLED) && defined(MODULE_INPUT_GLYPHS_ENABLED)

#include "modules/imgui/godot_imgui.h"
#include "modules/input_glyphs/input_glyphs.h"
#include "modules/input_glyphs/input_glyphs_constants.h"
#include "modules/input_glyphs/input_glyphs_singleton.h"
#include "modules/steamworks/steam_input.h"
#include <iterator>

class InputGlyphDemoTool : public GodotImGuiTool {
	GDCLASS(InputGlyphDemoTool, GodotImGuiTool);
	struct GlyphDebugInfo {
		InputGlyphsConstants::InputOrigin origin;
		Ref<Texture2D> texture;
		int style;
	};

	int input_type_idx = InputGlyphsConstants::XBOX_ONE_CONTROLLER;
	int glyph_size = InputGlyphSize::GLYPH_SIZE_SMALL;

	// 3 themes, knockout light and dark
	// we also add 3 spaces per-theme for the different variants of ABXY
	// (GLYPH_STYLE_NEUTRAL_COLOR_ABXY, GLYPH_STYLE_SOLID_ABXY, GLYPH_STYLE_NEUTRAL_COLOR_ABXY + GLYPH_STYLE_SOLID_ABXY)
	static const int GLYPH_COUNT = InputGlyphsConstants::INPUT_ORIGIN_COUNT + (4 * 3);
	static const int THEME_COUNT = 3;

	const char *THEME_NAMES[THEME_COUNT] = {
		"Knockout",
		"Light",
		"Dark",
	};

	GlyphDebugInfo glyph_infos[THEME_COUNT][GLYPH_COUNT];
	bool has_glyphs = false;
	void draw_glyphs(int p_theme_i);
	void reload_glyphs();
	bool open = false;

public:
	bool get_open() const;
	void set_open(bool p_open);

	virtual void draw_ui() override;
	virtual String get_name() const override;
};

#endif // defined(MODULE_STEAMWORKS_ENABLED) && defined(MODULE_INPUT_GLYPHS_ENABLED)
#endif // INPUT_GLYPH_DEMO_TOOL_H
