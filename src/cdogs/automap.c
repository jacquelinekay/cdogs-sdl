/*
    C-Dogs SDL
    A port of the legendary (and fun) action/arcade cdogs.
    Copyright (C) 1995 Ronny Wester
    Copyright (C) 2003 Jeremy Chin 
    Copyright (C) 2003 Lucas Martin-King 

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
#include "automap.h"

#include <stdio.h>
#include <string.h>

#include "config.h"
#include "defs.h"
#include "pics.h"
#include "map.h"
#include "actors.h"
#include "blit.h"
#include "sounds.h"
#include "text.h"
#include "gamedata.h"
#include "input.h"
#include "mission.h"
#include "keyboard.h"
#include "objs.h"
#include "drawtools.h"
#include "sys_specifics.h"


#define MAP_XOFFS   60
#define MAP_YOFFS   10

#define MAP_FACTOR  2

#define WALL_COLOR   		85
#define FLOOR_COLOR  		88
#define DOOR_COLOR   		36
#define YELLOW_DOOR_COLOR   16
#define GREEN_DOOR_COLOR	11
#define BLUE_DOOR_COLOR		14
#define RED_DOOR_COLOR		134

#define EXIT_COLOR  255
#define KEY_COLOR   255



static void DisplayPlayer(TActor * player)
{
	int x, y;
	struct CharacterDescription *c;
	int pic;

	if (player) {
		c = &characterDesc[player->character];
		pic = cHeadPic[c->facePic][DIRECTION_DOWN][STATE_IDLE];
		x = MAP_XOFFS +
		    MAP_FACTOR * player->tileItem.x / TILE_WIDTH;
		y = MAP_YOFFS +
		    MAP_FACTOR * player->tileItem.y / TILE_HEIGHT;
		x -= PicWidth(gPics[pic]) / 2;
		y -= PicHeight(gPics[pic]) / 2;
		DrawTTPic(x, y, gPics[pic], c->table, gRLEPics[pic]);
	}
}

void DrawCross(TTileItem * t, unsigned char color)
{
	unsigned char *scr = GetDstScreen();

	scr += MAP_XOFFS + MAP_FACTOR * t->x / TILE_WIDTH;
	scr += (MAP_YOFFS + MAP_FACTOR * t->y / TILE_HEIGHT) * gGraphicsDevice.cachedConfig.ResolutionWidth;
	*scr = color;
	*(scr - 1) = color;
	*(scr + 1) = color;
	*(scr - gGraphicsDevice.cachedConfig.ResolutionWidth) = color;
	*(scr + gGraphicsDevice.cachedConfig.ResolutionWidth) = color;
}

static void DisplayObjective(TTileItem * t, int objectiveIndex)
{
	DrawCross(t, gMission.objectives[objectiveIndex].color);
}

static void DisplayExit(void)
{
	unsigned char *scr = GetDstScreen();
	int i;
	int x1, x2, y1, y2;

	if (!CanCompleteMission(&gMission))
	{
		return;
	}

	x1 = MAP_FACTOR * gMission.exitLeft / TILE_WIDTH + MAP_XOFFS;
	y1 = MAP_FACTOR * gMission.exitTop / TILE_HEIGHT + MAP_YOFFS;
	x2 = MAP_FACTOR * gMission.exitRight / TILE_WIDTH + MAP_XOFFS;
	y2 = MAP_FACTOR * gMission.exitBottom / TILE_HEIGHT + MAP_YOFFS;

	for (i = x1; i <= x2; i++)
	{
		*(scr + i + y1 * gGraphicsDevice.cachedConfig.ResolutionWidth) = EXIT_COLOR;
		*(scr + i + y2 * gGraphicsDevice.cachedConfig.ResolutionWidth) = EXIT_COLOR;
	}
	for (i = y1 + 1; i < y2; i++)
	{
		*(scr + x1 + i * gGraphicsDevice.cachedConfig.ResolutionWidth) = EXIT_COLOR;
		*(scr + x2 + i * gGraphicsDevice.cachedConfig.ResolutionWidth) = EXIT_COLOR;
	}
}

static void DisplaySummary(void)
{
	int i, y, x, x2;
	char sScore[20];
	//unsigned char *scr = GetDstScreen();
	unsigned char color;

	y = gGraphicsDevice.cachedConfig.ResolutionHeight - 5 - CDogsTextHeight(); // 10 pixels from bottom

	for (i = 0; i < gMission.missionData->objectiveCount; i++) {
		if (gMission.objectives[i].required > 0 ||
		    gMission.objectives[i].done > 0) {
			// Objective color dot
			color = gMission.objectives[i].color;

			x = 5;
			Draw_Rect(x, (y + 3), 2, 2, color);

			x += 5;
			x2 = x + CDogsTextWidth(gMission.missionData->objectives[i].description) + 5;

			sprintf(sScore, "(%d)", gMission.objectives[i].done);

			if (gMission.objectives[i].required <= 0) {
				CDogsTextStringWithTableAt(x, y,
						      gMission.missionData->objectives[i].description,
						      &tablePurple);
				CDogsTextStringWithTableAt(x2, y, sScore, &tablePurple);
			} else if (gMission.objectives[i].done >= gMission.objectives[i].required) {
				CDogsTextStringWithTableAt(x, y,
						      gMission.missionData->objectives[i].description,
						      &tableFlamed);
				CDogsTextStringWithTableAt(x2, y, sScore, &tableFlamed);
			} else {
				CDogsTextStringAt(x, y, gMission.missionData->objectives[i].description);
				CDogsTextStringAt(x2, y, sScore);
			}
			y -= (CDogsTextHeight() + 1);
		}
	}
}

static int MapLevel(int x, int y)
{
	int l;

	l = MapAccessLevel(x - 1, y);
	if (l)
		return l;
	l = MapAccessLevel(x + 1, y);
	if (l)
		return l;
	l = MapAccessLevel(x, y - 1);
	if (l)
		return l;
	return MapAccessLevel(x, y + 1);
}

unsigned char DoorColor(int x, int y)
{
	int l = MapLevel(x, y);

	switch (l) {
	case FLAGS_KEYCARD_YELLOW:
		return YELLOW_DOOR_COLOR;
	case FLAGS_KEYCARD_GREEN:
		return GREEN_DOOR_COLOR;
	case FLAGS_KEYCARD_BLUE:
		return BLUE_DOOR_COLOR;
	case FLAGS_KEYCARD_RED:
		return RED_DOOR_COLOR;
	default:
		return DOOR_COLOR;
	}
}

void DrawDot(TTileItem * t, unsigned char color)
{
	unsigned int x, y;

	x = MAP_XOFFS + MAP_FACTOR * t->x / TILE_WIDTH;
	y = MAP_YOFFS + MAP_FACTOR * t-> y / TILE_HEIGHT;

	Draw_Rect(x, y, 2, 2, color);
}

void DisplayAutoMap(int showAll)
{
	int x, y, i, j;
	TTile *tile;
	unsigned char *p;
	unsigned char *screen;
	TTileItem *t;
	int obj;

	screen = p = GetDstScreen();
	// Draw faded green overlay
	for (x = 0;
		x < gGraphicsDevice.cachedConfig.ResolutionWidth * gGraphicsDevice.cachedConfig.ResolutionHeight;
		x++)
	{
		p[x] = tableGreen[p[x] & 0xFF];
	}

	screen += MAP_YOFFS * gGraphicsDevice.cachedConfig.ResolutionWidth + MAP_XOFFS;
	for (y = 0; y < YMAX; y++)
		for (i = 0; i < MAP_FACTOR; i++) {
			for (x = 0; x < XMAX; x++)
				if (AutoMap(x, y) || showAll) {
					tile = &Map(x, y);
					for (j = 0; j < MAP_FACTOR; j++)
						if ((tile->flags & IS_WALL) != 0)
							*screen++ = WALL_COLOR;
						else if ((tile->flags & NO_WALK)
							 != 0)
							*screen++ = DoorColor(x, y);
						else
							*screen++ = FLOOR_COLOR;
				} else
					screen += MAP_FACTOR;
			screen += gGraphicsDevice.cachedConfig.ResolutionWidth - XMAX * MAP_FACTOR;
		}

	for (y = 0; y < YMAX; y++)
		for (x = 0; x < XMAX; x++) {
			t = Map(x, y).things;
			while (t) {
				if ((t->flags & TILEITEM_OBJECTIVE) != 0) {
					obj =
					    ObjectiveFromTileItem(t->
								  flags);
					if ((gMission.missionData->
					     objectives[obj].
					     flags & OBJECTIVE_HIDDEN) == 0
					    || showAll) {
						if ((gMission.missionData->
						     objectives[obj].
						     flags &
						     OBJECTIVE_POSKNOWN) !=
						    0 || AutoMap(x, y)
						    || showAll)
							DisplayObjective(t,
									 obj);
					}
				} else if (t->kind == KIND_OBJECT && t->data && AutoMap(x, y)) {
					TObject *o = t->data;

					if (o->objectIndex == OBJ_KEYCARD_RED)
						DrawDot(t, RED_DOOR_COLOR);
					else if (o->objectIndex == OBJ_KEYCARD_BLUE)
						DrawDot(t, BLUE_DOOR_COLOR);
					else if (o->objectIndex == OBJ_KEYCARD_GREEN)
						DrawDot(t, GREEN_DOOR_COLOR);
					else if (o->objectIndex ==  OBJ_KEYCARD_YELLOW)
						DrawDot(t, YELLOW_DOOR_COLOR);
				}

				t = t->next;
			}
		}


	DisplayPlayer(gPlayer1);
	DisplayPlayer(gPlayer2);

	DisplayExit();
	DisplaySummary();

	CopyToScreen();
}
