/*
    C-Dogs SDL
    A port of the legendary (and fun) action/arcade cdogs.
    Copyright (C) 1995 Ronny Wester
    Copyright (C) 2003 Jeremy Chin 
    Copyright (C) 2003-2007 Lucas Martin-King 

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    This file incorporates work covered by the following copyright and
    permission notice:

    Copyright (c) 2013, Cong Xu
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/
#include "password.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <cdogs/actors.h>
#include <cdogs/blit.h>
#include <cdogs/config.h>
#include <cdogs/defs.h>
#include <cdogs/gamedata.h>
#include <cdogs/grafx.h>
#include <cdogs/input.h>
#include <cdogs/joystick.h>
#include <cdogs/keyboard.h>
#include <cdogs/sounds.h>
#include <cdogs/text.h>

#include "autosave.h"
#include "menu.h"

#define DONE          "Done"


const char *MakePassword(int mission)
{
	static char s[PASSWORD_MAX + 1];
	int i, sum1, sum2, x, count;
	static char *alphabet1 = "0123456789abcdefghijklmnopqrstuvwxyz";
	static char *alphabet2 = "9876543210kjihgfedcbazyxwvutsrqponml";
	char *alphabet = (gOptions.twoPlayers ? alphabet2 : alphabet1);
	int base = strlen(alphabet);

	sum1 = sum2 = 0;
	for (i = 0; i < (int)strlen(gCampaign.Setting.title); i++)
	{
		sum1 += gCampaign.Setting.title[i];
		sum2 ^= gCampaign.Setting.title[i];
	}

	x = ((sum2 << 23) | (mission << 16) | sum1) ^ gCampaign.seed;
	count = 0;
	while (x > 0 && count < PASSWORD_MAX) {
		i = x % base;
		s[count++] = alphabet[i];
		x /= base;
	}
	s[count] = 0;
	return s;
}

static int TestPassword(const char *password)
{
	int i;

	for (i = 0; i < gCampaign.Setting.missionCount; i++)
	{
		if (strcmp(password, MakePassword(i)) == 0)
		{
			return i;
		}
	}
	return -1;
}

static int PasswordEntry(int cmd, char *buffer)
{
	int i, x, y;
	static char letters[] = "abcdefghijklmnopqrstuvwxyz0123456789";
	static int selection = -1;

	// Kludge since Watcom won't let me initialize selection with a strlen()
	if (selection < 0)
		selection = strlen(letters);

	if (cmd & CMD_BUTTON1)
	{
		if (selection == (int)strlen(letters))
		{
			SoundPlay(&gSoundDevice, SND_LAUNCH);
			return 0;
		}

		if (strlen(buffer) < PASSWORD_MAX) {
			int l = strlen(buffer);
			buffer[l + 1] = 0;
			buffer[l] = letters[selection];
			SoundPlay(&gSoundDevice, SND_MACHINEGUN);
		}
		else
		{
			SoundPlay(&gSoundDevice, SND_KILL);
		}
	}
	else if (cmd & CMD_BUTTON2)
	{
		if (buffer[0]) {
			buffer[strlen(buffer) - 1] = 0;
			SoundPlay(&gSoundDevice, SND_BANG);
		}
		else
		{
			SoundPlay(&gSoundDevice, SND_KILL);
		}
	}
	else if (cmd & CMD_LEFT)
	{
		if (selection > 0) {
			selection--;
			SoundPlay(&gSoundDevice, SND_DOOR);
		}
	} else if (cmd & CMD_RIGHT) {
		if (selection < (int)strlen(letters))
		{
			selection++;
			SoundPlay(&gSoundDevice, SND_DOOR);
		}
	} else if (cmd & CMD_UP) {
		if (selection > 9) {
			selection -= 10;
			SoundPlay(&gSoundDevice, SND_DOOR);
		}
	}
	else if (cmd & CMD_DOWN)
	{
		if (selection < (int)strlen(letters) - 9)
		{
			selection += 10;
			SoundPlay(&gSoundDevice, SND_DOOR);
		}
	}
	
	#define ENTRY_COLS	10
	#define	ENTRY_SPACING	12
	
	x = CenterX(((ENTRY_SPACING * (ENTRY_COLS - 1)) + CDogsTextCharWidth('a')));
	y = CenterY(((CDogsTextHeight() * ((strlen(letters) - 1) / ENTRY_COLS) )));
	
	// Draw selection
	for (i = 0; i < (int)strlen(letters); i++)
	{
		CDogsTextGoto(x + (i % ENTRY_COLS) * ENTRY_SPACING,
			 y + (i / ENTRY_COLS) * CDogsTextHeight());

		if (i == selection)
			CDogsTextCharWithTable(letters[i], &tableFlamed);
		else
			CDogsTextChar(letters[i]);
	}
	CDogsTextGoto(x + (i % ENTRY_COLS) * ENTRY_SPACING, y + (i / ENTRY_COLS) * CDogsTextHeight());
	if (i == selection)
		CDogsTextStringWithTable(DONE, &tableFlamed);
	else
		CDogsTextString(DONE);

	return 1;
}

static int EnterCode(void *bkg, const char *password)
{
	int mission = 0;
	int done = 0;
	char buffer[PASSWORD_MAX + 1];

	strcpy(buffer, password);
	while (!done)
	{
		int cmd;
		InputPoll(&gJoysticks, &gKeyboard);
		memcpy(GetDstScreen(), bkg, GraphicsGetMemSize(&gGraphicsDevice.cachedConfig));
		cmd = GetMenuCmd();
		if (!PasswordEntry(cmd, buffer))
		{
			if (!buffer[0])
			{
				mission = 0;
				done = 1;
			} else {
				mission = TestPassword(buffer);
				if (mission > 0)
					done = 1;
				else
				{
					SoundPlay(&gSoundDevice, SND_KILL2);
				}
			}
		}

		#define SYMBOL_LEFT	'\020'
		#define	SYMBOL_RIGHT	'\021'

		CDogsTextGoto(
			CenterX(
				CDogsTextWidth(buffer) +
				CDogsTextCharWidth(SYMBOL_LEFT) +
				CDogsTextCharWidth(SYMBOL_RIGHT)),
				gGraphicsDevice.cachedConfig.ResolutionWidth / 4);
		CDogsTextChar(SYMBOL_LEFT);
		CDogsTextString(buffer);
		CDogsTextChar(SYMBOL_RIGHT);

		CDogsTextStringSpecial(
			"Enter code",
			TEXT_XCENTER | TEXT_TOP,
			0,
			gGraphicsDevice.cachedConfig.ResolutionHeight / 12);
		ShowControls();

		CopyToScreen();
	}

	SoundPlay(&gSoundDevice, SND_SWITCH);

	return mission;
}

typedef enum
{
	RETURN_CODE_CONTINUE,
	RETURN_CODE_START,
	RETURN_CODE_ENTER_CODE
} ReturnCode;

MenuSystem *MenuCreateStart(
	int hasPassword, void *bkg, joysticks_t *joysticks, keyboard_t *keyboard);

int EnterPassword(void *bkg, const char *password)
{
	MenuSystem *startMenu;
	int mission = TestPassword(password);
	int hasPassword = mission > 0;
	startMenu = MenuCreateStart(hasPassword, bkg, &gJoysticks, &gKeyboard);
	for (;;)
	{
		int returnCode;
		MenuLoop(startMenu);
		assert(startMenu->current->type == MENU_TYPE_RETURN);
		returnCode = startMenu->current->u.returnCode;
		if (returnCode == RETURN_CODE_CONTINUE)
		{
			return mission;
		}
		else if (returnCode == RETURN_CODE_START)
		{
			return 0;
		}
		else if (returnCode == RETURN_CODE_ENTER_CODE)
		{
			int enteredMission = EnterCode(bkg, password);
			if (enteredMission > 0)
			{
				return enteredMission;
			}
			MenuReset(startMenu);
		}
	}
}

MenuSystem *MenuCreateStart(
	int hasPassword, void *bkg, joysticks_t *joysticks, keyboard_t *keyboard)
{
	MenuSystem *ms;
	CCALLOC(ms, sizeof *ms);
	MenuSetInputDevices(ms, joysticks, keyboard);
	MenuSetBackground(ms, bkg);
	ms->root = ms->current = MenuCreateNormal("", "", MENU_TYPE_NORMAL, 0);
	if (hasPassword)
	{
		MenuAddSubmenu(ms->root, MenuCreateReturn("Continue", RETURN_CODE_CONTINUE));
	}
	MenuAddSubmenu(ms->root, MenuCreateReturn("Start campaign", RETURN_CODE_START));
	MenuAddSubmenu(ms->root, MenuCreateReturn("Enter code...", RETURN_CODE_ENTER_CODE));
	MenuAddExitType(ms, MENU_TYPE_RETURN);
	return ms;
}
