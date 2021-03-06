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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <SDL.h>

#include <cdogs/actors.h>
#include <cdogs/ai.h>
#include <cdogs/blit.h>
#include <cdogs/campaigns.h>
#include <cdogs/config.h>
#include <cdogs/draw.h>
#include <cdogs/events.h>
#include <cdogs/files.h>
#include <cdogs/game.h>
#include <cdogs/gamedata.h>
#include <cdogs/grafx.h>
#include <cdogs/hiscores.h>
#include <cdogs/input.h>
#include <cdogs/joystick.h>
#include <cdogs/keyboard.h>
#include <cdogs/mission.h>
#include <cdogs/music.h>
#include <cdogs/objs.h>
#include <cdogs/pics.h>
#include <cdogs/sounds.h>
#include <cdogs/text.h>
#include <cdogs/triggers.h>
#include <cdogs/utils.h>

#include "autosave.h"
#include "credits.h"
#include "mainmenu.h"
#include "password.h"
#include "prep.h"



void DrawObjectiveInfo(int index, int x, int y, struct Mission *mission)
{
	TOffsetPic pic;
	TranslationTable *table = NULL;
	int i = 0;

	switch (mission->objectives[index].type) {
	case OBJECTIVE_KILL:
		i = characterDesc[mission->baddieCount +
				  CHARACTER_OTHERS].facePic;
		table =
		    &characterDesc[mission->baddieCount +
				  CHARACTER_OTHERS].table;
		pic.picIndex = cHeadPic[i][DIRECTION_DOWN][STATE_IDLE];
		pic.dx = cHeadOffset[i][DIRECTION_DOWN].dx;
		pic.dy = cHeadOffset[i][DIRECTION_DOWN].dy;
		break;
	case OBJECTIVE_RESCUE:
		i = characterDesc[CHARACTER_PRISONER].facePic;
		table = &characterDesc[CHARACTER_PRISONER].table;
		pic.picIndex = cHeadPic[i][DIRECTION_DOWN][STATE_IDLE];
		pic.dx = cHeadOffset[i][DIRECTION_DOWN].dx;
		pic.dy = cHeadOffset[i][DIRECTION_DOWN].dy;
		break;
	case OBJECTIVE_COLLECT:
		i = gMission.objectives[index].pickupItem;
		pic = cGeneralPics[i];
		break;
	case OBJECTIVE_DESTROY:
		i = gMission.objectives[index].blowupObject->pic;
		pic = cGeneralPics[i];
		break;
	case OBJECTIVE_INVESTIGATE:
		pic.dx = pic.dy = 0;
		pic.picIndex = -1;
		return;
	default:
		i = gMission.objectives[i].pickupItem;
		pic = cGeneralPics[i];
		break;
	}
	if (pic.picIndex >= 0) {
		if (table)
			DrawTTPic(x + pic.dx, y + pic.dy, gPics[pic.picIndex],
				  table, NULL);
		else
			DrawTPic(x + pic.dx, y + pic.dy, gPics[pic.picIndex],
				 NULL);
	}
}

int MissionDescription(int y, const char *description)
{
	int w, ix, x, lines;
	const char *ws, *word, *p, *s;

#define MAX_BOX_WIDTH (gGraphicsDevice.cachedConfig.ResolutionWidth - (gGraphicsDevice.cachedConfig.ResolutionWidth / 6))

	ix = x = CenterX((MAX_BOX_WIDTH));
	lines = 1;
	CDogsTextGoto(x, y);

	s = ws = word = description;

	while (*s) {
		// Find word
		ws = s;

		while (*s == ' ' || *s == '\n')
			s++;

		word = s;

		while (*s != 0 && *s != ' ' && *s != '\n')
			s++;

		for (w = 0, p = ws; p < s; p++)
			w += CDogsTextCharWidth(*p);

		//if (x + w > MAX_BOX_WIDTH && w < (MAX_BOX_WIDTH - 20)) {
		if (x + w > (MAX_BOX_WIDTH + ix) && w < MAX_BOX_WIDTH) {
			y += CDogsTextHeight();
			x = ix;
			lines++;
			ws = word;
		}

		for (p = ws; p < word; p++)
			x += CDogsTextCharWidth(*p);

		CDogsTextGoto(x, y);

		for (p = word; p < s; p++) {
			CDogsTextChar(*p);
			x += CDogsTextCharWidth(*p);
		}
	}

	return lines;
}

void CampaignIntro(void *bkg)
{
	int y;
	char s[1024];

	debug(D_NORMAL, "\n");

	memcpy(GetDstScreen(), bkg, GraphicsGetMemSize(&gGraphicsDevice.cachedConfig));

	y = gGraphicsDevice.cachedConfig.ResolutionWidth / 4;

	sprintf(s, "%s by %s", gCampaign.Setting.title, gCampaign.Setting.author);
	CDogsTextStringSpecial(s, TEXT_TOP | TEXT_XCENTER, 0, (y - 25));

	MissionDescription(y, gCampaign.Setting.description);

	CopyToScreen();
	GetKey(&gKeyboard);
}

void MissionBriefing(void *bkg)
{
	char s[512];
	int i, y;

	memcpy(GetDstScreen(), bkg, GraphicsGetMemSize(&gGraphicsDevice.cachedConfig));

	y = gGraphicsDevice.cachedConfig.ResolutionWidth / 4;

	sprintf(s, "Mission %d: %s", gMission.index + 1, gMission.missionData->title);
	CDogsTextStringSpecial(s, TEXT_TOP | TEXT_XCENTER, 0, (y - 25));

	// Save password if we're not on the first mission
	if (gMission.index > 0)
	{
		char str[512];
		MissionSave ms;
		ms.Campaign = gCampaign.Entry;
		strcpy(ms.Password, MakePassword(gMission.index));
		AutosaveAddMission(&gAutosave, &ms);
		AutosaveSave(&gAutosave, GetConfigFilePath(AUTOSAVE_FILE));
		sprintf(str, "Password: %s", gAutosave.LastMission.Password);
		CDogsTextStringSpecial(str, TEXT_TOP | TEXT_XCENTER, 0, (y - 15));
	}

	y += CDogsTextHeight() * MissionDescription(y, gMission.missionData->description);

	y += gGraphicsDevice.cachedConfig.ResolutionHeight / 10;

	for (i = 0; i < gMission.missionData->objectiveCount; i++)
	{
		if (gMission.missionData->objectives[i].required > 0)
		{
			CDogsTextStringAt(
				gGraphicsDevice.cachedConfig.ResolutionWidth / 6,
				y,
				gMission.missionData->objectives[i].description);
			DrawObjectiveInfo(
				i,
				gGraphicsDevice.cachedConfig.ResolutionWidth - (gGraphicsDevice.cachedConfig.ResolutionWidth / 6),
				y + 8,
				gMission.missionData);

			y += gGraphicsDevice.cachedConfig.ResolutionHeight / 12;
		}
	}

	CopyToScreen();

	GetKey(&gKeyboard);
}

void Summary(int x, struct PlayerData *data, int character)
{
	char s[50];
	int y = gGraphicsDevice.cachedConfig.ResolutionHeight / 3;

	if (strlen(gAutosave.LastMission.Password) > 0)
	{
		char s1[512];
		sprintf(s1, "Last password: %s", gAutosave.LastMission.Password);
		CDogsTextStringSpecial(
			s1,
			TEXT_BOTTOM | TEXT_XCENTER,
			0,
			gGraphicsDevice.cachedConfig.ResolutionHeight / 12);
	}

	if (data->survived) {
		if (data->hp > 150)
			data->totalScore += (data->hp - 150) * 10;
		else if (data->hp <= 0)
			data->totalScore -= 500;

		if (data->friendlies > 5
		    && data->friendlies > data->kills / 2)
			data->totalScore -= 100 * data->friendlies;
		else if (data->weaponCount == 1 &&
			 data->weapons[0] == GUN_KNIFE &&
			 data->friendlies == 0 && data->kills > 5)
			data->totalScore += 50 * data->kills;
		else if (data->kills == 0 && data->friendlies == 0)
			data->totalScore += 500;
	}

	if (data->survived)
		CDogsTextStringAt(x, y, "Completed mission");
	else
		CDogsTextStringWithTableAt(x, y, "Failed mission", &tableFlamed);

	y += 2 * CDogsTextHeight();
	DisplayPlayer(x, data, character, 0);
	sprintf(s, "Score: %d", data->score);
	CDogsTextStringAt(x, y, s);
	y += CDogsTextHeight();
	sprintf(s, "Total: %d", data->totalScore);
	CDogsTextStringAt(x, y, s);
	y += CDogsTextHeight();
	sprintf(s, "Missions: %d",
		data->missions + (data->survived ? 1 : 0));
	CDogsTextStringAt(x, y, s);
	y += CDogsTextHeight();

	if (data->survived && (data->hp > 150 || data->hp <= 0))
	{
		int maxHealth = (200 * gConfig.Game.PlayerHP) / 100;
		if (data->hp > maxHealth - 50)
		{
			sprintf(s, "Health bonus: %d", (data->hp + 50 - maxHealth) * 10);
		}
		else if (data->hp <= 0)
		{
			sprintf(s, "Resurrection fee: %d", -500);
		}
		CDogsTextStringAt(x, y, s);
		y += CDogsTextHeight();
	}

	if (data->friendlies > 0 && data->friendlies > data->kills / 2) {
		sprintf(s, "Butcher penalty: %d", 100 * data->friendlies);
		CDogsTextStringAt(x, y, s);
		y += CDogsTextHeight();
	} else if (data->weaponCount == 1 &&
			data->weapons[0] == GUN_KNIFE && data->kills > 0) {
		sprintf(s, "Ninja bonus: %d", 50 * data->kills);
		CDogsTextStringAt(x, y, s);
		y += CDogsTextHeight();
	} else if (data->kills == 0 && data->friendlies == 0) {
		sprintf(s, "Friendly bonus: %d", 500);
		CDogsTextStringAt(x, y, s);
		y += CDogsTextHeight();
	}
}

void Bonuses(void)
{
	int i;
	int y = (gGraphicsDevice.cachedConfig.ResolutionHeight / 2) + (gGraphicsDevice.cachedConfig.ResolutionHeight / 10);
	int x = gGraphicsDevice.cachedConfig.ResolutionWidth / 6;
	int done, req, total;
	int access_bonus = 0;
	int index;
	char s[100];

	index = 1;
	for (i = 0; i < gMission.missionData->objectiveCount; i++) {
		done = gMission.objectives[i].done;
		req = gMission.objectives[i].required;
		total = gMission.objectives[i].count;

		if (done > 0 || req > 0) {
			DrawObjectiveInfo(i, x - 26, y + 8, gMission.missionData);
			sprintf(s, "Objective %d: %d of %d, %d required",
				index, done, total, req);
			if (req > 0)
				CDogsTextStringSpecial(s,
						TEXT_LEFT | TEXT_TOP, x, y);
			else
				CDogsTextStringSpecial(s,
						TEXT_LEFT | TEXT_TOP | TEXT_PURPLE,
						x, y);
			if (done < req)
				CDogsTextStringSpecial("Failed",
						TEXT_RIGHT | TEXT_TOP | TEXT_FLAMED, x, y);
			else if (done == total && done > req
					&& (gPlayer1Data.survived
					 || gPlayer2Data.survived)) {
				CDogsTextStringSpecial("Perfect: 500",
						TEXT_RIGHT | TEXT_TOP, x, y);
				if (gPlayer1Data.survived)
					gPlayer1Data.totalScore += 500;
				if (gPlayer2Data.survived)
					gPlayer2Data.totalScore += 500;
			} else if (req > 0)
				CDogsTextStringSpecial("Done", TEXT_RIGHT | TEXT_TOP, x, y);
			else {
				CDogsTextStringSpecial("Bonus!", TEXT_RIGHT | TEXT_TOP, x, y);
			}

			y += 15;
			index++;
		}
	}

	if (gMission.flags & FLAGS_KEYCARD_YELLOW)	access_bonus += 50;
	if (gMission.flags & FLAGS_KEYCARD_GREEN)	access_bonus += 100;
	if (gMission.flags & FLAGS_KEYCARD_BLUE)	access_bonus += 150;
	if (gMission.flags & FLAGS_KEYCARD_RED)		access_bonus += 200;
	if (access_bonus > 0 && (gPlayer1Data.survived || gPlayer2Data.survived))
	{
		sprintf(s, "Access bonus: %d", access_bonus);
		CDogsTextStringAt(x, y, s);
		y += CDogsTextHeight() + 1;
		if (gPlayer1Data.survived)
		{
			gPlayer1Data.totalScore += access_bonus;
		}
		if (gPlayer2Data.survived)
		{
			gPlayer2Data.totalScore += access_bonus;
		}
	}

	i = 60 + gMission.missionData->objectiveCount * 30 - missionTime / 70;

	if (i > 0 && (gPlayer1Data.survived || gPlayer2Data.survived)) {
		sprintf(s, "Time bonus: %d secs x 25 = %d", i, i * 25);
		CDogsTextStringAt(x, y, s);
		if (gPlayer1Data.survived)
			gPlayer1Data.totalScore += i * 25;
		if (gPlayer2Data.survived)
			gPlayer2Data.totalScore += i * 25;
	}
}

void MissionSummary(void *bkg)
{
	memcpy(GetDstScreen(), bkg, GraphicsGetMemSize(&gGraphicsDevice.cachedConfig));

	Bonuses();

	if (gOptions.twoPlayers) {
		Summary(CenterOfLeft(60), &gPlayer1Data, CHARACTER_PLAYER1);
		Summary(CenterOfRight(60), &gPlayer2Data, CHARACTER_PLAYER2);
	} else
		Summary(CenterX(60), &gPlayer1Data, CHARACTER_PLAYER1);

	CopyToScreen();

	GetKey(&gKeyboard);
}

void ShowScore(void *bkg, int score1, int score2)
{
	char s[10];

	memcpy(GetDstScreen(), bkg, GraphicsGetMemSize(&gGraphicsDevice.cachedConfig));

	debug(D_NORMAL, "\n");

	if (gOptions.twoPlayers) {
		DisplayPlayer(CenterOfLeft(60), &gPlayer1Data, CHARACTER_PLAYER1, 0);
		sprintf(s, "Score: %d", score1);
		CDogsTextStringAt(
			CenterOfLeft(CDogsTextWidth(s)),
			gGraphicsDevice.cachedConfig.ResolutionWidth / 3,
			s);

		DisplayPlayer(CenterOfRight(60), &gPlayer2Data, CHARACTER_PLAYER2, 0);
		sprintf(s, "Score: %d", score2);
		CDogsTextStringAt(
			CenterOfRight(CDogsTextWidth(s)),
			gGraphicsDevice.cachedConfig.ResolutionWidth / 3,
			s);
	}
	else
	{
		DisplayPlayer(CenterX(CDogsTextWidth(s)), &gPlayer1Data, CHARACTER_PLAYER1, 0);
	}

	CopyToScreen();
	GetKey(&gKeyboard);
}

void FinalScore(void *bkg, int score1, int score2)
{
	memcpy(GetDstScreen(), bkg, GraphicsGetMemSize(&gGraphicsDevice.cachedConfig));

#define IS_DRAW		"It's a draw!"
#define IS_WINNER	"Winner!"

	if (score1 == score2) {
		DisplayPlayer(CenterOfLeft(60), &gPlayer1Data, CHARACTER_PLAYER1, 0);
		DisplayPlayer(CenterOfRight(60), &gPlayer2Data, CHARACTER_PLAYER2, 0);
		CDogsTextStringAtCenter("It's a draw!");
	} else if (score1 > score2) {
		DisplayPlayer(CenterOfLeft(60), &gPlayer1Data, CHARACTER_PLAYER1, 0);
		CDogsTextStringAt(
			CenterOfLeft(CDogsTextWidth(IS_WINNER)),
			gGraphicsDevice.cachedConfig.ResolutionWidth / 2,
			IS_WINNER);
	}
	else
	{
		DisplayPlayer(CenterOfRight(60), &gPlayer2Data, CHARACTER_PLAYER2, 0);
		CDogsTextStringAt(
			CenterOfRight(CDogsTextWidth(IS_WINNER)),
			gGraphicsDevice.cachedConfig.ResolutionWidth / 2,
			IS_WINNER);
	}
	CopyToScreen();
	GetKey(&gKeyboard);
}


static const char *finalWords1P[] = {
	"Ha, next time I'll use my good hand",
	"Over already? I was just warming up...",
	"There's just no good opposition to be found these days!",
	"Well, maybe I'll just do my monthly reload then",
	"Woof woof",
	"I'll just bury the bones in the back yard, he-he",
	"I just wish they'd let me try bare-handed",
	"Rambo? Who's Rambo?",
	"<in Austrian accent:> I'll be back",
	"Gee, my trigger finger is sore",
	"I need more practice. I think I missed a few shots at times"
};

static const char *finalWords2P[] = {
	"United we stand, divided we conquer",
	"Nothing like good teamwork, is there?",
	"Which way is the camera?",
	"We eat bullets for breakfast and have grenades as dessert",
	"We're so cool we have to wear mittens",
};

#define CONGRATULATIONS "Congratulations, you have completed "

void Victory(void *bkg)
{
	int x, i;
	const char *s;

	memcpy(GetDstScreen(), bkg, GraphicsGetMemSize(&gGraphicsDevice.cachedConfig));

	x = 160 - CDogsTextWidth(CONGRATULATIONS) / 2;
	CDogsTextStringAt(x, 100, CONGRATULATIONS);
	x = 160 - CDogsTextWidth(gCampaign.Setting.title) / 2;
	CDogsTextStringWithTableAt(x, 115, gCampaign.Setting.title, &tableFlamed);

	if (gOptions.twoPlayers) {
		i = sizeof(finalWords2P) / sizeof(char *);
		i = rand() % i;
		s = finalWords2P[i];
		DisplayPlayer(50, &gPlayer1Data, CHARACTER_PLAYER1, 0);
		DisplayPlayer(200, &gPlayer2Data, CHARACTER_PLAYER2, 0);
		if (gPlayer1Data.survived)
			gPlayer1Data.missions++;
		if (gPlayer2Data.survived)
			gPlayer2Data.missions++;
	} else {
		i = sizeof(finalWords1P) / sizeof(char *);
		i = rand() % i;
		s = finalWords1P[i];
		DisplayPlayer(125, &gPlayer1Data, CHARACTER_PLAYER1, 0);
		gPlayer1Data.missions++;
	}

	x = 160 - CDogsTextWidth(s) / 2;
	CDogsTextGoto(x, 140);
	CDogsTextCharWithTable('"', &tableDarker);
	CDogsTextStringWithTable(s, &tablePurple);
	CDogsTextCharWithTable('"', &tableDarker);

	SoundPlay(&gSoundDevice, SND_HAHAHA);

	CopyToScreen();
	GetKey(&gKeyboard);
}


static void PlaceActor(TActor * actor)
{
	int x, y;

	do {
		x = ((rand() % (XMAX * TILE_WIDTH)) << 8);
		y = ((rand() % (YMAX * TILE_HEIGHT)) << 8);
	}
	while (!OKforPlayer(x, y) || !MoveActor(actor, x, y));
}

void InitData(struct PlayerData *data)
{
	data->totalScore = 0;
	data->missions = 0;
}

void DataUpdate(int mission, struct PlayerData *data)
{
	if (!data->survived) {
		data->totalScore = 0;
		data->missions = 0;
	} else
		data->missions++;
	data->lastMission = mission;
}

static void CleanupMission(void)
{
	KillAllActors();
	KillAllMobileObjects(&gMobObjList);
	KillAllObjects();
	FreeTriggersAndWatches();
	gPlayer1 = gPlayer2 = gPrisoner = NULL;
}

static void InitPlayers(int twoPlayers, int maxHealth, int mission)
{
	gPlayer1Data.score = 0;
	gPlayer1Data.kills = gPlayer1Data.friendlies = 0;
	gPlayer1Data.allTime = gPlayer1Data.today = -1;
	gPlayer1Data.lastMission = mission;
	gPlayer1 = AddActor(CHARACTER_PLAYER1);
	gPlayer1->weapon = WeaponCreate(gPlayer1Data.weapons[0]);
	gPlayer1->flags = FLAGS_PLAYER1;
	PlaceActor(gPlayer1);
	gPlayer1->health = maxHealth;

	if (twoPlayers) {
		gPlayer2Data.score = 0;
		gPlayer2Data.kills = gPlayer2Data.friendlies = 0;
		gPlayer2Data.allTime = gPlayer2Data.today = -1;
		gPlayer2Data.lastMission = mission;
		gPlayer2 = AddActor(CHARACTER_PLAYER2);
		gPlayer2->weapon = WeaponCreate(gPlayer2Data.weapons[0]);
		gPlayer2->flags = FLAGS_PLAYER2;
		PlaceActor(gPlayer2);
		gPlayer2->health = maxHealth;
	}
}

static void PlayGameSong(void)
{
	int success = 0;
	// Play a tune
	// Start by trying to play a mission specific song,
	// otherwise pick one from the general collection...
	MusicStop(&gSoundDevice);
	if (strlen(gMission.missionData->song) > 0)
	{
		char buf[CDOGS_PATH_MAX];
		int pathLen;
		pathLen = MAX(strrchr(gCampaign.Entry.path, '\\'), strrchr(gCampaign.Entry.path, '/')) - gCampaign.Entry.path;
		strncpy(buf, gCampaign.Entry.path, pathLen);
		buf[pathLen] = '\0';

		strcat(buf, "/");
		strcat(buf, gMission.missionData->song);
		success = !MusicPlay(&gSoundDevice, buf);
	}
	if (!success && gGameSongs != NULL)
	{
		MusicPlay(&gSoundDevice, gGameSongs->path);
		ShiftSongs(&gGameSongs);
	}
}

static void PlayMenuSong(void)
{
	MusicStop(&gSoundDevice);
	if (gMenuSongs)
	{
		MusicPlay(&gSoundDevice, gMenuSongs->path);
		ShiftSongs(&gMenuSongs);
	}
}


int Game(void *bkg, int mission)
{
	int run, gameOver;
	int allTime, todays;
	int maxHealth;

	maxHealth = 200 * gConfig.Game.PlayerHP / 100;

	do
	{
		SetupMission(mission, 1, &gCampaign);

		SetupMap();

		srand((unsigned int)time(NULL));
		InitializeBadGuys();
		if (IsMissionBriefingNeeded(gCampaign.Entry.mode))
		{
			MissionBriefing(bkg);
		}
		PlayerEquip(bkg);

		InitPlayers(gOptions.twoPlayers, maxHealth, mission);

		CreateEnemies();

		PlayGameSong();

		run = gameloop();

		gameOver =
			(!gPlayer1 && !gPlayer2) ||
			mission == gCampaign.Setting.missionCount - 1;

		gPlayer1Data.survived = gPlayer1 != NULL;
		if (gPlayer1)
			gPlayer1Data.hp = gPlayer1->health;
		gPlayer2Data.survived = gPlayer2 != NULL;
		if (gPlayer2)
			gPlayer2Data.hp = gPlayer2->health;

		CleanupMission();

		PlayMenuSong();
		printf(">> Starting\n");

		if (run) {
			MissionSummary(bkg);
			if (gameOver
			    && (gPlayer1Data.survived
				|| gPlayer2Data.survived))
				Victory(bkg);
		}

		allTime = todays = 0;
		if ((run && !gPlayer1Data.survived) || gameOver) {
			EnterHighScore(&gPlayer1Data);
			allTime = gPlayer1Data.allTime >= 0;
			todays = gPlayer1Data.today >= 0;
		}
		if (run && gOptions.twoPlayers
		    && (!gPlayer2Data.survived || gameOver)) {
			EnterHighScore(&gPlayer2Data);
			allTime = gPlayer2Data.allTime >= 0;
			todays = gPlayer2Data.today >= 0;

			// Check if player 1's position(s) in the list(s) need adjustment...
			if (gPlayer2Data.allTime >= 0 &&
			    gPlayer2Data.allTime <= gPlayer1Data.allTime)
				gPlayer1Data.allTime++;
			if (gPlayer2Data.today >= 0 &&
			    gPlayer2Data.today <= gPlayer1Data.today)
				gPlayer1Data.today++;
		}
		if (allTime && !gameOver)
			DisplayAllTimeHighScores(bkg);
		if (todays && !gameOver)
			DisplayTodaysHighScores(bkg);

		mission++;

		DataUpdate(mission, &gPlayer1Data);
		if (gOptions.twoPlayers)
			DataUpdate(mission, &gPlayer2Data);
	}
	while (run && !gameOver);
	return run;
}

int Campaign(void *bkg)
{
	int mission = 0;

	InitData(&gPlayer1Data);
	InitData(&gPlayer2Data);

	if (IsPasswordAllowed(gCampaign.Entry.mode))
	{
		MissionSave m;
		AutosaveLoadMission(&gAutosave, &m, gCampaign.Entry.path);
		mission = EnterPassword(bkg, m.Password);
	}

	return Game(bkg, mission);
}

void DogFight(void *bkg)
{
	int run;
	int score1 = 0, score2 = 0;
	int twoPlayers = gOptions.twoPlayers;

	InitData(&gPlayer1Data);
	InitData(&gPlayer2Data);

	gOptions.badGuys = 0;
	gOptions.twoPlayers = 1;

	do
	{
		SetupMission(0, 1, &gCampaign);
		SetupMap();

		if (PlayerEquip(bkg))
		{
			srand((unsigned int)time(NULL));
			InitPlayers(YES, 500, 0);
			PlayGameSong();
			run = gameloop();
		}
		else
		{
			run = 0;
		}

		if (gPlayer1 != NULL)
			score1++;
		if (gPlayer2 != NULL)
			score2++;

		CleanupMission();
		PlayMenuSong();

		if (run)
			ShowScore(bkg, score1, score2);

	} while (run && score1 < 5 && score2 < 5);

	gOptions.badGuys = 1;
	gOptions.twoPlayers = twoPlayers;

	if (run)
		FinalScore(bkg, score1, score2);
}

void *MakeBkg(void)
{
	unsigned char *bkg;
	struct Buffer *buffer = NewBuffer();
	unsigned char *p;
	int i;
	TranslationTable randomTintTable;

	CMALLOC(bkg, GraphicsGetMemSize(&gGraphicsDevice.cachedConfig));

	SetupQuickPlayCampaign(&gCampaign.Setting);
	gCampaign.seed = rand();
	SetupMission(0, 1, &gCampaign);
	SetupMap();
	SetDstScreen(bkg);
	SetBuffer(1024, 768, buffer, X_TILES);
	FixBuffer(buffer, 255);
	DrawBuffer(buffer, 0);
	CFREE(buffer);
	KillAllObjects();
	FreeTriggersAndWatches();
	gCampaign.seed = gConfig.Game.RandomSeed;

	p = bkg;
	SetPaletteRanges(15, 12, 10, 0);
	BuildTranslationTables();
	SetRandomTintTable(&randomTintTable, 256);
	for (i = 0; i < GraphicsGetMemSize(&gGraphicsDevice.cachedConfig); i++)
	{
		p[i] = randomTintTable[p[i] & 0xFF];
	}

	return bkg;
}

void MainLoop(credits_displayer_t *creditsDisplayer, custom_campaigns_t *campaigns)
{
	unsigned char *my_screen;

	void *bkg = MakeBkg();
	CCALLOC(my_screen, GraphicsGetMemSize(&gGraphicsDevice.cachedConfig));
	SetDstScreen(my_screen);

	while (MainMenu(bkg, creditsDisplayer, campaigns))
	{
		debug(D_NORMAL, ">> Entering campaign\n");
		if (IsIntroNeeded(gCampaign.Entry.mode))
		{
			CampaignIntro(bkg);
		}

		debug(D_NORMAL, ">> Entering selection\n");
		if (!PlayerSelection(gOptions.twoPlayers, bkg))
		{
			continue;
		}

		debug(D_NORMAL, ">> Starting campaign\n");
		if (gCampaign.Entry.mode == CAMPAIGN_MODE_DOGFIGHT)
		{
			DogFight(bkg);
		}
		else if (Campaign(bkg))
		{
			DisplayAllTimeHighScores(bkg);
			DisplayTodaysHighScores(bkg);
		}
	}
	debug(D_NORMAL, ">> Leaving Main Game Loop\n");
	CFREE(my_screen);
	CFREE(bkg);
}

void PrintTitle(void)
{
	printf("C-Dogs %s, (SDL Port %s)\n", CDOGS_VERSION, CDOGS_SDL_VERSION);

	printf("Original Code Copyright Ronny Wester 1995\n");
	printf("Game Data Copyright Ronny Wester 1995\n");
	printf("SDL Port by Jeremy Chin and Lucas Martin-King, Copyright 2003-2006\n\n");
	printf("%s%s%s%s",
		"C-Dogs SDL comes with ABSOLUTELY NO WARRANTY;\n",
		"see the file COPYING that came with this distibution...\n",
		"This is free software, and you are welcome to redistribute it\n",
		"under certain conditions; for details see COPYING.\n\n");
}

void PrintHelp (void)
{
	printf("%s\n",
		"Video Options:\n"
		"    -fullscreen     Try and use a fullscreen video mode.\n"
		"    -scale=n        Scale the window resolution up by a factor of n\n"
		"                      Factors: 2, 3, 4\n"
		"    -screen=WxH     Set virtual screen width to W x H\n"
		"                      Modes: 320x200, 320x240, 400x300, 640x480, 800x600\n"
		"    -forcemode      Don't check video mode sanity\n"
	);

	printf("%s\n",
		"Sound Options:\n"
		"    -nosound        Disable sound\n"
	);

	printf("%s\n",
		"Control Options:\n"
		"    -nojoystick     Disable joystick(s)\n"
	);

	printf("%s\n",
		"Game Options:\n"
		"    -wait           Wait for a key hit before initialising video.\n"
		"    -shakemult=n    Screen shaking multiplier (0 = disable).\n"
		"    -slices         Display CPU slices [*broken*]\n"
	);

	printf("%s\n",
		"The DEBUG environment variable can be set to show debug information.");

	printf(
		"The DEBUG_LEVEL environment variable can be set to between %d and %d.\n", D_NORMAL, D_MAX
	);
}

int main(int argc, char *argv[])
{
	int i, wait = 0;
	int snd_flag = SDL_INIT_AUDIO;
	int js_flag = SDL_INIT_JOYSTICK;
	int isSoundEnabled = 1;
	credits_displayer_t creditsDisplayer;
	custom_campaigns_t campaigns;
	int forceResolution = 0;

	srand((unsigned int)time(NULL));

	PrintTitle();

	{
		char *dbg;
		if (getenv("DEBUG") != NULL) debug = 1;
		if ((dbg = getenv("DEBUG_LEVEL")) != NULL) {
			debug_level = atoi(dbg);
			if (debug_level < 0) debug_level = 0;
			if (debug_level > D_MAX) debug_level = D_MAX;
		}
	}

	SetupConfigDir();
	ConfigLoadDefault(&gConfig);
	ConfigLoad(&gConfig, GetConfigFilePath(CONFIG_FILE));
	LoadCredits(&creditsDisplayer, &tablePurple, &tableDarker);
	AutosaveInit(&gAutosave);
	AutosaveLoad(&gAutosave, GetConfigFilePath(AUTOSAVE_FILE));

	for (i = 1; i < argc; i++) {
		if ((strlen(argv[i]) > 1 && *(argv[i]) == '-') || *(argv[i]) == '/') {
			if (strcmp(argv[i] + 1, "slices") == 0) {
				printf("Displaying CPU slices\n");
				gOptions.displaySlices = 1;
			}

			if (strstr(argv[i] + 1, "shakemult=")) {
				char *val = strchr(argv[i], '=');
				int nval;
				extern int shakeMultiplier;

				val++;
				nval = atoi(val);
				if (nval < 0) nval = 0;
				printf("Shake multiplier: %d\n", nval);
				shakeMultiplier = nval;
			}
			if (strcmp(argv[i] + 1, "wait") == 0)
				wait = 1;
			if (strcmp(argv[i] + 1, "nosound") == 0) {
				printf("Sound disabled!\n");
				snd_flag = 0;
				isSoundEnabled = 0;
			}
			if (strcmp(argv[i] + 1, "nojoystick") == 0) {
				debug(D_NORMAL, "nojoystick\n");
				js_flag = 0;
			}
			if (strcmp(argv[i] + 1, "fullscreen") == 0)
			{
				gConfig.Graphics.Fullscreen = 1;
			}
			if (strstr(argv[i] + 1, "screen="))
			{
				char *val = strchr(argv[i], '='); val++;
				sscanf(val, "%dx%d",
					&gGraphicsDevice.cachedConfig.ResolutionWidth, &gGraphicsDevice.cachedConfig.ResolutionHeight);
				debug(D_NORMAL, "Video mode %dx%d set...\n",
					gGraphicsDevice.cachedConfig.ResolutionWidth, gGraphicsDevice.cachedConfig.ResolutionHeight);
			}
			if (strcmp(argv[i] + 1, "forcemode") == 0)
			{
				forceResolution = 1;
			}
			if (strstr(argv[i] + 1, "scale=")) {
				int f;
				char *val = strchr(argv[i], '='); val++;
				f = atoi(val);
				if (f >= 1)
				{
					gConfig.Graphics.ScaleFactor = f;
				}
			}
			if (strcmp(argv[i] + 1, "help") == 0 ||
				strcmp(argv[i] + 1, "h") == 0 ||
				strcmp(argv[i] + 1, "-help") == 0) {
				PrintHelp();
				exit(EXIT_SUCCESS);
			}
		}
	}

	debug(D_NORMAL, "Initialising SDL...\n");
	if (SDL_Init(SDL_INIT_TIMER | snd_flag | SDL_INIT_VIDEO | js_flag) != 0) {
		printf("Failed to start SDL!\n");
		return -1;
	}

	printf("Data directory:\t\t%s\n",	GetDataFilePath(""));
	printf("Config directory:\t%s\n\n",	GetConfigFilePath(""));

	i = ReadPics(GetDataFilePath("graphics/cdogs.px"), gPics, PIC_COUNT1, gPalette);
	if (!i) {
		printf("Unable to read CDOGS.PX (%s)\n", GetDataFilePath("graphics/cdogs.px"));
		exit(0);
	}
	if (!AppendPics(GetDataFilePath("graphics/cdogs2.px"), gPics, PIC_COUNT1, PIC_MAX)) {
		printf("Unable to read CDOGS2.PX (%s)\n", GetDataFilePath("graphics/cdogs2.px"));
		exit(0);
	}
	gPalette[0].red = gPalette[0].green = gPalette[0].blue = 0;
	memcpy(origPalette, gPalette, sizeof(origPalette));
	InitializeTranslationTables();

	memset(gCompiledPics, 0, sizeof(gCompiledPics));
	memset(gRLEPics, 0, sizeof(gRLEPics));

	CDogsTextInit(GetDataFilePath("graphics/font.px"), -2);

	if (isSoundEnabled)
	{
		SoundInitialize(&gSoundDevice, &gConfig.Sound);
		if (!gSoundDevice.isInitialised)
		{
			printf("Sound initialization failed!\n");
		}
	}

	LoadHighScores();

	debug(D_NORMAL, "Loading song lists...\n");
	LoadSongs();
	LoadTemplates();

	PlayMenuSong();

	LoadAllCampaigns(&campaigns);

	JoyInit(&gJoysticks);
	KeyInit(&gKeyboard);

	if (wait)
	{
		printf("Press the enter key to continue...\n");
		getchar();
	}

	GraphicsInitialize(&gGraphicsDevice, &gConfig.Graphics, forceResolution);
	if (!gGraphicsDevice.IsInitialized)
	{
		Config defaultConfig;
		printf("Cannot initialise video; trying default config\n");
		ConfigLoadDefault(&defaultConfig);
		gConfig.Graphics = defaultConfig.Graphics;
		GraphicsInitialize(&gGraphicsDevice, &gConfig.Graphics, forceResolution);
	}
	if (!gGraphicsDevice.IsInitialized)
	{
		printf("Video didn't init!\n");
		exit(EXIT_FAILURE);
	} else {
		CDogsSetPalette(gPalette);
		debug(D_NORMAL, ">> Entering main loop\n");
		MainLoop(&creditsDisplayer, &campaigns);
	}
	debug(D_NORMAL, ">> Shutting down...\n");

	JoyTerminate(&gJoysticks);

	GraphicsTerminate(&gGraphicsDevice);

	AutosaveSave(&gAutosave, GetConfigFilePath(AUTOSAVE_FILE));
	ConfigSave(&gConfig, GetConfigFilePath(CONFIG_FILE));
	SaveTemplates();
	FreeSongs(&gMenuSongs);
	FreeSongs(&gGameSongs);
	SaveHighScores();
	UnloadCredits(&creditsDisplayer);
	UnloadAllCampaigns(&campaigns);

	if (isSoundEnabled)
	{
		debug(D_NORMAL, ">> Shutting down sound...\n");
		SoundTerminate(&gSoundDevice, 1);
	}

	debug(D_NORMAL, "SDL_Quit()\n");
	SDL_Quit();

	printf("Bye :)\n");

	exit(EXIT_SUCCESS);
}

