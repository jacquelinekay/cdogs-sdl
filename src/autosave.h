/*
 C-Dogs SDL
 A port of the legendary (and fun) action/arcade cdogs.
 
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
#ifndef __autosave
#define __autosave

#include <stddef.h>

#include <cdogs/campaigns.h>
#include <cdogs/sys_config.h>

#define PASSWORD_MAX 16
#define AUTOSAVE_FILE "autosave.json"

typedef struct
{
	campaign_entry_t Campaign;
	char Password[PASSWORD_MAX + 1];
} MissionSave;

typedef struct
{
	MissionSave LastMission;
	MissionSave *Missions;
	size_t NumMissions;
} Autosave;

extern Autosave gAutosave;

void AutosaveInit(Autosave *autosave);
void AutosaveLoad(Autosave *autosave, const char *filename);
void AutosaveSave(Autosave *autosave, const char *filename);
void AutosaveAddMission(Autosave *autosave, MissionSave *mission);
void AutosaveLoadMission(Autosave *autosave, MissionSave *mission, const char *path);

#endif
