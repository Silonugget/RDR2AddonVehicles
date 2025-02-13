// Licensed under the MIT License.

#include "Menu.hpp"
#include "Drawing.h"
#include "../common.hpp"

void playSoundFrontend(const char* name, const char* soundset)
{
	AUDIO::_STOP_SOUND_WITH_NAME(name, soundset);
	AUDIO::PLAY_SOUND_FRONTEND(name, soundset, true, 0);
}


void CNativeMenu::Update()
{
	CheckInput();
	HandleInput();
	DisableCommonInputs();

	if (m_IsOpen)
	{
		if (m_CurrentSubmenu == nullptr) return;

		Drawing::DrawMenu();

		for (int i = 0; i < m_CurrentSubmenu->GetNumberOfOptions(); i++) {
			Option* option = m_CurrentSubmenu->GetOption(i);
			if (option != nullptr) {
				Drawing::DrawOption(option);
			}
			else {
				PRINT_ERROR("Option index ", i, " in submenu ", (int)m_CurrentSubmenuID, " is null");
			}
		}

		MAP::DISPLAY_RADAR(false); // Always hide radar when the menu is open
	}

	if (m_CurrentSubmenu != nullptr) {
		m_CurrentSubmenuID = m_CurrentSubmenu->ID;
	}

	HUD::_UI_PROMPT_SET_VISIBLE(m_SelectPrompt, m_IsOpen);
	HUD::_UI_PROMPT_SET_ENABLED(m_SelectPrompt, m_IsOpen);
	HUD::_UI_PROMPT_SET_VISIBLE(m_BackPrompt, m_IsOpen);
	HUD::_UI_PROMPT_SET_ENABLED(m_BackPrompt, m_IsOpen);
}


void CNativeMenu::SetEnabled(bool bEnabled, bool bPlaySounds)
{
	m_IsOpen = bEnabled;

	if (bPlaySounds) {
		if (m_IsOpen) {
			playSoundFrontend("MENU_ENTER", "HUD_PLAYER_MENU");
		}
		else {
			playSoundFrontend("MENU_CLOSE", "HUD_PLAYER_MENU");
		}
	}

	if (m_IsOpen) {
		MAP::DISPLAY_RADAR(false);
	}
	else {
		MAP::DISPLAY_RADAR(true);
	}
}


void CNativeMenu::CheckInput()
{
	m_ControlIndex = PAD::IS_USING_KEYBOARD_AND_MOUSE(0) ? 0 : 2;

	m_OpenKeyPressed	= IsKeyJustUp(VK_F7) || PAD::IS_DISABLED_CONTROL_PRESSED(2, INPUT_FRONTEND_RB) && PAD::IS_CONTROL_JUST_RELEASED(2, INPUT_CONTEXT_X);
	m_EnterKeyPressed	= PAD::IS_CONTROL_JUST_RELEASED(m_ControlIndex, INPUT_GAME_MENU_ACCEPT);
	m_BackKeyPressed	= PAD::IS_CONTROL_JUST_RELEASED(m_ControlIndex, INPUT_GAME_MENU_CANCEL);
	m_UpKeyPressed		= PAD::IS_CONTROL_JUST_PRESSED(m_ControlIndex, INPUT_GAME_MENU_UP);
	m_DownKeyPressed	= PAD::IS_CONTROL_JUST_PRESSED(m_ControlIndex, INPUT_GAME_MENU_DOWN);
	m_LeftKeyPressed	= PAD::IS_DISABLED_CONTROL_JUST_PRESSED(m_ControlIndex, INPUT_GAME_MENU_LEFT);
	m_RightKeyPressed	= PAD::IS_DISABLED_CONTROL_JUST_PRESSED(m_ControlIndex, INPUT_GAME_MENU_RIGHT);
	//m_LMBPressed		= _NAMESPACE30::_0xF7F51A57349739F2();

	if (HUD::_UI_PROMPT_IS_VALID(m_SelectPrompt) && HUD::_UI_PROMPT_IS_ACTIVE(m_SelectPrompt)) {
		m_SelectPromptCompleted = HUD::_UI_PROMPT_HAS_STANDARD_MODE_COMPLETED(m_SelectPrompt, 0);
	}
	else {
		m_SelectPromptCompleted = m_EnterKeyPressed;
	}

	if (HUD::_UI_PROMPT_IS_VALID(m_BackPrompt) && HUD::_UI_PROMPT_IS_ACTIVE(m_SelectPrompt)) {
		m_BackPromptCompleted = HUD::_UI_PROMPT_HAS_STANDARD_MODE_COMPLETED(m_BackPrompt, 0);
	}
	else {
		m_BackPromptCompleted = m_BackKeyPressed;
	}
}





void CNativeMenu::HandleEnterPressed()
{
	Option* option = GetSelectedOption();
	if (option == nullptr) return;

	if (option->IsRegularOption) {
		option->Execute();
	}
	else if (option->IsBoolOption) {
		BoolOption* boolOption = option->As<BoolOption*>();
		if (boolOption->pBoolPointer != nullptr) {
			*boolOption->pBoolPointer ^= TRUE; // XOR
		}
		else {
			PRINT_ERROR("Bad boolean pointer for option index ", m_SelectionIndex, ". Executing assigned function, but boolean pointer will not be switched");
		}

		boolOption->Execute();
	}
	else if (option->IsVectorOption) {
		VectorOption* vecOption = option->As<VectorOption*>();
		vecOption->ExecuteVectorFunction(false, true);
	}
	else if (option->IsSubmenuOption) {
		SubmenuOption* submenuOption = option->As<SubmenuOption*>();
		if (DoesSubmenuExist(submenuOption->SubmenuID)) {
			//g_Menu->GoToSubmenu(submenuOption->SubmenuID);
			m_PreviousSubmenus.push_back(m_CurrentSubmenu->ID);
			m_LastSubmenuSelections[m_CurrentSubmenu->ID] = m_SelectionIndex;
			m_CurrentSubmenu = &m_SubmenusMap[submenuOption->SubmenuID];
			m_SelectionIndex = 0;
		}
		else {
			PRINT_ERROR("Submenu ID ", (int)submenuOption->SubmenuID, " doesn't exist, make sure this submenu is initialized!");
		}
	}

	playSoundFrontend("SELECT", "HUD_SHOP_SOUNDSET");
}


void CNativeMenu::HandleBackPressed()
{
	if (m_CurrentSubmenu->ID <= eSubmenuID::Submenu_EntryMenu) {
		playSoundFrontend("BACK", "HUD_SHOP_SOUNDSET");
		SetEnabled(false);
		return;
	}

	if (m_PreviousSubmenus.size() > 0) {
		//g_Menu->GoToSubmenu(m_PreviousSubmenus[m_PreviousSubmenus.size() - 1]);
		m_CurrentSubmenu = &m_SubmenusMap[m_PreviousSubmenus[m_PreviousSubmenus.size() - 1]];
		m_PreviousSubmenus.pop_back();
		m_SelectionIndex = m_LastSubmenuSelections[m_CurrentSubmenu->ID];
		m_LastSubmenuSelections.erase(m_CurrentSubmenu->ID);
	}
	else {
		m_CurrentSubmenu = &m_SubmenusMap[eSubmenuID::Submenu_EntryMenu];
		m_SelectionIndex = m_LastSubmenuSelections[eSubmenuID::Submenu_EntryMenu];
	}

	playSoundFrontend("BACK", "HUD_SHOP_SOUNDSET");
}


void CNativeMenu::HandleUpKeyPressed()
{
	if (GetSelectedOption() == nullptr) return;
	int numOptions = m_CurrentSubmenu->GetNumberOfOptions();

	if (IsKeyDownLong(VK_SHIFT)) {
		m_SelectionIndex -= 10;
		if (m_SelectionIndex < 0) {
			m_SelectionIndex = 0; // DONT reset to the BOTTOM of the page
		}
	}
	else {
		m_SelectionIndex--;
	}

	// First check
	if (m_SelectionIndex < 0) { m_SelectionIndex = numOptions - 1; }
	// Skip over empty option
	if (GetSelectedOption()->IsEmptyOption) { m_SelectionIndex--; }
	// Second check
	if (m_SelectionIndex < 0) { m_SelectionIndex = numOptions - 1; }

	playSoundFrontend("NAV_UP", "Ledger_Sounds");
}


void CNativeMenu::HandleDownKeyPressed()
{
	if (GetSelectedOption() == nullptr) return;
	int numOptions = m_CurrentSubmenu->GetNumberOfOptions();

	if (IsKeyDownLong(VK_SHIFT)) {
		m_SelectionIndex += 10;
		if (m_SelectionIndex > numOptions - 1) {
			m_SelectionIndex = numOptions - 1; // DONT reset to the TOP of the page
		}	
	}
	else {
		m_SelectionIndex++;
	}

	// First check
	if (m_SelectionIndex > numOptions - 1) { m_SelectionIndex = 0; }
	// Skip over empty option
	if (GetSelectedOption()->IsEmptyOption) { m_SelectionIndex++; }
	// Second check
	if (m_SelectionIndex > numOptions - 1) { m_SelectionIndex = 0; }

	playSoundFrontend("NAV_DOWN", "Ledger_Sounds");
}


void CNativeMenu::HandleLeftKeyPressed()
{
	Option* option = GetSelectedOption();
	if (option == nullptr) return;

	if (option->IsVectorOption) {
		option->As<VectorOption*>()->ExecuteVectorFunction(true, false);
		playSoundFrontend("NAV_LEFT", "PAUSE_MENU_SOUNDSET");
	}
}


void CNativeMenu::HandleRightKeyPressed()
{
	Option* option = GetSelectedOption();
	if (option == nullptr) return;

	if (option->IsVectorOption) {
		option->As<VectorOption*>()->ExecuteVectorFunction(false, true);
		playSoundFrontend("NAV_RIGHT", "PAUSE_MENU_SOUNDSET");
	}
}


void CNativeMenu::HandleInput()
{
	if (m_OpenKeyPressed) {
		m_IsOpen = !m_IsOpen;
		SetEnabled(m_IsOpen);
	}

	if (m_IsOpen) {
		// !m_OpenKeyPressed is a fix where opening with RB + A would enter/leave a submenu prematurely
		if ((m_SelectPromptCompleted || m_LMBPressed) && !m_OpenKeyPressed) {
			this->HandleEnterPressed();
		}

		if (m_BackPromptCompleted && !m_OpenKeyPressed) {
			this->HandleBackPressed();
		}

		if (m_UpKeyPressed) {
			this->HandleUpKeyPressed();
		}

		if (m_DownKeyPressed) {
			this->HandleDownKeyPressed();
		}

		if (m_LeftKeyPressed) {
			this->HandleLeftKeyPressed();
		}

		if (m_RightKeyPressed) {
			this->HandleRightKeyPressed();
		}

		//HandleMouseInput();
	}
}


void CNativeMenu::DisableCommonInputs()
{
	if (m_IsOpen) {
		if (m_ControlIndex == INPUT_GROUP_GAMEPAD) {
			*getGlobalPtr(1900383 + 316) = 2; // Disables horse whistling this frame. Not sure if this is safe, but it works.
			PAD::DISABLE_CONTROL_ACTION(INPUT_GROUP_GAMEPAD, INPUT_WHISTLE, false);
			PAD::DISABLE_CONTROL_ACTION(INPUT_GROUP_GAMEPAD, INPUT_WHISTLE_HORSEBACK, false);
		}

		PAD::DISABLE_CONTROL_ACTION(m_ControlIndex, INPUT_SELECT_RADAR_MODE, false);
		PAD::DISABLE_CONTROL_ACTION(m_ControlIndex, INPUT_REVEAL_HUD, false);
		PAD::DISABLE_CONTROL_ACTION(m_ControlIndex, INPUT_PLAYER_MENU, false);
		PAD::DISABLE_CONTROL_ACTION(m_ControlIndex, INPUT_OPEN_JOURNAL, false);
		PAD::DISABLE_CONTROL_ACTION(m_ControlIndex, INPUT_OPEN_SATCHEL_MENU, false);
		PAD::DISABLE_CONTROL_ACTION(m_ControlIndex, INPUT_OPEN_SATCHEL_HORSE_MENU, false);
		PAD::DISABLE_CONTROL_ACTION(m_ControlIndex, INPUT_OPEN_CRAFTING_MENU, false);
		PAD::DISABLE_CONTROL_ACTION(m_ControlIndex, INPUT_OPEN_EMOTE_WHEEL, false);
		PAD::DISABLE_CONTROL_ACTION(m_ControlIndex, INPUT_OPEN_WHEEL_MENU, false);
		PAD::DISABLE_CONTROL_ACTION(m_ControlIndex, INPUT_EXPAND_RADAR, false);
		PAD::DISABLE_CONTROL_ACTION(m_ControlIndex, INPUT_INTERACTION_MENU, false);
		PAD::DISABLE_CONTROL_ACTION(m_ControlIndex, INPUT_HUD_SPECIAL, false);
	}
}


void CNativeMenu::RegisterUIPrompts()
{
	auto reg = [](Prompt &_prompt, const char* text, Hash control) 
	{ 
		_prompt = HUD::_UI_PROMPT_REGISTER_BEGIN();
		HUD::_UI_PROMPT_SET_CONTROL_ACTION(_prompt, control);
		HUD::_UI_PROMPT_SET_PRIORITY(_prompt, 2); // PP_High
		HUD::_UI_PROMPT_SET_TEXT(_prompt, MISC::VAR_STRING(10, "LITERAL_STRING", text));
		HUD::_UI_PROMPT_SET_STANDARD_MODE(_prompt, true);
		// Allows multiple prompts of the same type to be shown (kPromptAttrib_ManualResolved) (?)
		HUD::_UI_PROMPT_SET_ATTRIBUTE(_prompt, 34, true);
		HUD::_UI_PROMPT_REGISTER_END(_prompt);

		HUD::_UI_PROMPT_SET_VISIBLE(_prompt, false);
		HUD::_UI_PROMPT_SET_ENABLED(_prompt, false);
	};

	if (m_SelectPrompt == NULL)
		reg(m_SelectPrompt, "Select", INPUT_GAME_MENU_ACCEPT);

	if (m_BackPrompt == NULL)
		reg(m_BackPrompt, "Back", INPUT_GAME_MENU_CANCEL);
}

