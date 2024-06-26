/**************************************************************************/
/*  in_game_ui.h                                                          */
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

#ifndef IN_GAME_UI_H
#define IN_GAME_UI_H

#include "modules/game/agent.h"
#include "scene/gui/control.h"

class HBPlayerAgent;
class HBGameWorld;

class HBInGameUI : public Control {
	GDCLASS(HBInGameUI, Control);

	HBPlayerAgent *player_agent = nullptr;
	GDVIRTUAL3(_player_health_changed, int, int, int)
	GDVIRTUAL1(_agent_entered_combat, HBAgent *)
	GDVIRTUAL1(_agent_exited_combat, HBAgent *)
protected:
	static void _bind_methods();
	void _on_player_damage_received(int p_old_health, int p_new_health);
	void _on_agent_entered_combat(HBAgent *p_agent);
	void _on_agent_exited_combat(HBAgent *p_agent);

public:
	HBPlayerAgent *get_player_agent() const { return player_agent; }
	void set_player_agent(HBPlayerAgent *p_player_agent);

	void set_world(HBGameWorld *p_world);
};

#endif // IN_GAME_UI_H
