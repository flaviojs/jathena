//atcommand.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "socket.h"
#include "timer.h"

#include "clif.h"
#include "chrif.h"
#include "intif.h"
#include "itemdb.h"
#include "map.h"
#include "pc.h"
#include "skill.h"
#include "mob.h"
#include "pet.h"
#include "battle.h"
#include "party.h"
#include "guild.h"
#include "atcommand.h"

#define OPTION_HIDE 0x40
#define STATE_BLIND 0x10

static char msg_table[200][1024];	/* Server message */

#define ATCOMMAND_FUNC(x) int atcommand_ ## x (const int fd, struct map_session_data* sd, const char* command, const char* message)
//ATCOMMAND_FUNC(broadcast);
//ATCOMMAND_FUNC(localbroadcast);
ATCOMMAND_FUNC(rurap);
ATCOMMAND_FUNC(rura);
ATCOMMAND_FUNC(where);
ATCOMMAND_FUNC(jumpto);
ATCOMMAND_FUNC(jump);
ATCOMMAND_FUNC(who);
ATCOMMAND_FUNC(save);
ATCOMMAND_FUNC(load);
ATCOMMAND_FUNC(speed);
ATCOMMAND_FUNC(storage);
ATCOMMAND_FUNC(guildstorage);
ATCOMMAND_FUNC(option);
ATCOMMAND_FUNC(hide);
ATCOMMAND_FUNC(jobchange);
ATCOMMAND_FUNC(die);
ATCOMMAND_FUNC(kill);
ATCOMMAND_FUNC(alive);
ATCOMMAND_FUNC(kami);
ATCOMMAND_FUNC(heal);
ATCOMMAND_FUNC(item);
ATCOMMAND_FUNC(item2);
ATCOMMAND_FUNC(itemreset);
ATCOMMAND_FUNC(itemcheck);
ATCOMMAND_FUNC(baselevelup);
ATCOMMAND_FUNC(joblevelup);
ATCOMMAND_FUNC(help);
ATCOMMAND_FUNC(gm);
ATCOMMAND_FUNC(pvpoff);
ATCOMMAND_FUNC(pvpon);
ATCOMMAND_FUNC(gvgoff);
ATCOMMAND_FUNC(gvgon);
ATCOMMAND_FUNC(model);
ATCOMMAND_FUNC(go);
ATCOMMAND_FUNC(monster);
ATCOMMAND_FUNC(killmonster);
ATCOMMAND_FUNC(killmonster2);
ATCOMMAND_FUNC(refine);
ATCOMMAND_FUNC(produce);
ATCOMMAND_FUNC(memo);
ATCOMMAND_FUNC(gat);
ATCOMMAND_FUNC(packet);
ATCOMMAND_FUNC(statuspoint);
ATCOMMAND_FUNC(skillpoint);
ATCOMMAND_FUNC(zeny);
ATCOMMAND_FUNC(param);
ATCOMMAND_FUNC(guildlevelup);
ATCOMMAND_FUNC(makepet);
ATCOMMAND_FUNC(petfriendly);
ATCOMMAND_FUNC(pethungry);
ATCOMMAND_FUNC(petrename);
ATCOMMAND_FUNC(recall);
ATCOMMAND_FUNC(character_job);
ATCOMMAND_FUNC(revive);
ATCOMMAND_FUNC(character_stats);
ATCOMMAND_FUNC(character_option);
ATCOMMAND_FUNC(character_save);
ATCOMMAND_FUNC(night);
ATCOMMAND_FUNC(day);
ATCOMMAND_FUNC(doom);
ATCOMMAND_FUNC(doommap);
ATCOMMAND_FUNC(raise);
ATCOMMAND_FUNC(raisemap);
ATCOMMAND_FUNC(character_baselevel);
ATCOMMAND_FUNC(character_joblevel);
ATCOMMAND_FUNC(kick);
ATCOMMAND_FUNC(kickall);
ATCOMMAND_FUNC(allskill);
ATCOMMAND_FUNC(questskill);
ATCOMMAND_FUNC(lostskill);
ATCOMMAND_FUNC(spiritball);
ATCOMMAND_FUNC(party);
ATCOMMAND_FUNC(guild);
ATCOMMAND_FUNC(agitstart);
ATCOMMAND_FUNC(agitend);
ATCOMMAND_FUNC(mapexit);

/*==========================================
 *AtCommandInfo atcommand_info[]構造体の定義
 *------------------------------------------
 */
static AtCommandInfo atcommand_info[] = {
	{ AtCommand_RuraP,					"@rura+",			0, atcommand_rurap },
	{ AtCommand_Rura,					"@rura",			0, atcommand_rura },
	{ AtCommand_Where,					"@where",			0, atcommand_where },
	{ AtCommand_JumpTo,					"@jumpto",			0, atcommand_jumpto },
	{ AtCommand_Jump,					"@jump",			0, atcommand_jump },
	{ AtCommand_Who,					"@who",				0, atcommand_who },
	{ AtCommand_Save,					"@save",			0, atcommand_save },
	{ AtCommand_Load,					"@load",			0, atcommand_load },
	{ AtCommand_Speed,					"@speed",			0, atcommand_speed },
	{ AtCommand_Storage,				"@storage",			0, atcommand_storage },
	{ AtCommand_GuildStorage,			"@gstorage",		0, atcommand_guildstorage },
	{ AtCommand_Option,					"@option",			0, atcommand_option },
	{ AtCommand_Hide,					"@hide",			0, atcommand_hide },
	{ AtCommand_JobChange,				"@jobchange",		0, atcommand_jobchange },
	{ AtCommand_Die,					"@die",				0, atcommand_die },
	{ AtCommand_Kill,					"@kill",			0, atcommand_kill },
	{ AtCommand_Alive,					"@alive",			0, atcommand_alive },
	{ AtCommand_Kami,					"@kami",			0, atcommand_kami },
	{ AtCommand_KamiB,					"@kamib",			0, atcommand_kami },
	{ AtCommand_Heal,					"@heal",			0, atcommand_heal },
	{ AtCommand_Item,					"@item",			0, atcommand_item },
	{ AtCommand_Item2,					"@item2",			0, atcommand_item2 },
	{ AtCommand_ItemReset,				"@itemreset",		0, atcommand_itemreset },
	{ AtCommand_ItemCheck,				"@itemcheck",		0, atcommand_itemcheck },
	{ AtCommand_BaseLevelUp,			"@lvup",			0, atcommand_baselevelup },
	{ AtCommand_JobLevelUp,				"@joblvup",			0, atcommand_joblevelup },
	{ AtCommand_H,						"@h",				0, atcommand_help },
	{ AtCommand_Help,					"@help",			0, atcommand_help },
	{ AtCommand_GM,						"@gm",				0, atcommand_gm },
	{ AtCommand_PvPOff,					"@pvpoff",			0, atcommand_pvpoff },
	{ AtCommand_PvPOn,					"@pvpon",			0, atcommand_pvpon },
	{ AtCommand_GvGOff,					"@gvgoff",			0, atcommand_gvgoff },
	{ AtCommand_GvGOn,					"@gvgon",			0, atcommand_gvgon },
	{ AtCommand_Model,					"@model",			0, atcommand_model },
	{ AtCommand_Go,						"@go",				0, atcommand_go },
	{ AtCommand_Monster,				"@monster",			0, atcommand_monster },
	{ AtCommand_KillMonster,			"@killmonster",		0, atcommand_killmonster },
	{ AtCommand_KillMonster2,			"@killmonster2",	0, atcommand_killmonster2 },
	{ AtCommand_Refine,					"@refine",			0, atcommand_refine },
	{ AtCommand_Produce,				"@produce",			0, atcommand_produce },
	{ AtCommand_Memo,					"@memo",			0, atcommand_memo },
	{ AtCommand_GAT,					"@gat",				0, atcommand_gat },
	{ AtCommand_Packet,					"@packet",			0, atcommand_packet },
	{ AtCommand_StatusPoint,			"@stpoint",			0, atcommand_statuspoint },
	{ AtCommand_SkillPoint,				"@skpoint",			0, atcommand_skillpoint },
	{ AtCommand_Zeny,					"@zeny",			0, atcommand_zeny },
//	{ AtCommand_Param,					"@param",			0, atcommand_param },
	{ AtCommand_Strength,				"@str",				0, atcommand_param },
	{ AtCommand_Agility,				"@agi",				0, atcommand_param },
	{ AtCommand_Vitality,				"@vit",				0, atcommand_param },
	{ AtCommand_Intelligence,			"@int",				0, atcommand_param },
	{ AtCommand_Dexterity,				"@dex",				0, atcommand_param },
	{ AtCommand_Luck,					"@luk",				0, atcommand_param },
	{ AtCommand_GuildLevelUp,			"@guildlvup",		0, atcommand_guildlevelup },
	{ AtCommand_MakePet,				"@makepet",			0, atcommand_makepet },
	{ AtCommand_PetFriendly,			"@petfriendly",		0, atcommand_petfriendly },
	{ AtCommand_PetHungry,				"@pethungry",		0, atcommand_pethungry },
	{ AtCommand_PetRename,				"@petrename",		0, atcommand_petrename },
	{ AtCommand_Recall,					"@recall",			0, atcommand_recall },
	{ AtCommand_CharacterJob,			"@charjob",			0, atcommand_character_job },
	{ AtCommand_Revive,					"@revive",			0, atcommand_revive },
	{ AtCommand_CharacterStats,			"@charstats",		0, atcommand_character_stats },
	{ AtCommand_CharacterOption,		"@charoption",		0, atcommand_character_option },
	{ AtCommand_CharacterSave,			"@charsave",		0, atcommand_character_save },
	{ AtCommand_Night,					"@night",			0, atcommand_night },
	{ AtCommand_Day,					"@day",				0, atcommand_day },
	{ AtCommand_Doom,					"@doom",			0, atcommand_doom },
	{ AtCommand_DoomMap,				"@doommap",			0, atcommand_doommap },
	{ AtCommand_Raise,					"@raise",			0, atcommand_raise },
	{ AtCommand_RaiseMap,				"@raisemap",		0, atcommand_raisemap },
	{ AtCommand_CharacterBaseLevel,		"@charbaselvl",		0, atcommand_character_baselevel },
	{ AtCommand_CharacterJobLevel,		"@charjlvl",		0, atcommand_character_joblevel },
	{ AtCommand_Kick,					"@kick",			0, atcommand_kick },
	{ AtCommand_KickAll,				"@kickall",			0, atcommand_kickall },
	{ AtCommand_AllSkill,				"@allskill",		0, atcommand_allskill },
	{ AtCommand_QuestSkill,				"@questskill",		0, atcommand_questskill },
	{ AtCommand_LostSkill,				"@lostskill",		0, atcommand_lostskill },
	{ AtCommand_SpiritBall,				"@spiritball",		0, atcommand_spiritball },
	{ AtCommand_Party,					"@party",			0, atcommand_party },
	{ AtCommand_Guild,					"@guild",			0, atcommand_guild },
	{ AtCommand_AgitStart,				"@agitstart",		0, atcommand_agitstart },
	{ AtCommand_AgitEnd,				"@agitend",			0, atcommand_agitend },
	{ AtCommand_MapExit,				"@mapexit",			0, atcommand_mapexit },
	// add here
	{ AtCommand_MapMove,				"@mapmove",			0, NULL },
	{ AtCommand_Broadcast,				"@broadcast",		0, NULL },
	{ AtCommand_LocalBroadcast,			"@local_broadcast",	0, NULL },
	{ AtCommand_Unknown,				NULL,				0, NULL }
};

/*==========================================
 *get_atcommand_level @コマンドの必要レベルを取得
 *------------------------------------------
 */
int get_atcommand_level(const AtCommandType type)
{
	int i = 0;
	for (i = 0; atcommand_info[i].type != AtCommand_None; i++)
		if (atcommand_info[i].type == type)
			return atcommand_info[i].level;
	return 99;
}

/*==========================================
 *is_atcommand @コマンドに存在するかどうか確認する
 *------------------------------------------
 */
AtCommandType
is_atcommand(const int fd, struct map_session_data* sd, const char* message)
{
	const char* str = message;
	int s_flag = 0;
	AtCommandInfo info;
	AtCommandType type;
	
	if (!message || !*message)
		return AtCommand_None;
	memset(&info, 0, sizeof info);
	str += strlen(sd->status.name);
	while (*str && (isspace(*str) || (s_flag == 0 && *str == ':'))) {
		if (*str == ':')
			s_flag = 1;
		str++;
	}
	if (!*str)
		return AtCommand_None;
	type = atcommand(pc_isGM(sd), str, &info);
	if (type != AtCommand_None) {
		char command[25];
		char output[100];
		const char* p = str;
		memset(command, '\0', sizeof command);
		while (*p && !isspace(*p))
			p++;
		if (p - str > sizeof command) // too long
			return AtCommand_Unknown;
		strncpy(command, str,
			(p - str > sizeof command) ?  sizeof command : p - str);
		if (isspace(*p))
			p++;
		if (type == AtCommand_Unknown) {
 			snprintf(output, sizeof output, "%s is Unknown Command.", command);
 			clif_displaymessage(fd, output);
		} else {
			if (info.proc(fd, sd, command, p) != 0) {
				// 異常終了
 				snprintf(output, sizeof output, "%s failed.", command);
 				clif_displaymessage(fd, output);
			}
		}
		return info.type;
	}
	return AtCommand_None;
}

/*==========================================
 * 
 *------------------------------------------
 */
AtCommandType
atcommand(
	const int level, const char* message, struct AtCommandInfo* info)
{
	const char* p = message;
	if (!info)
		return AtCommand_None;
	if (!p || !*p) {
		fprintf(stderr, "at command message is empty\n");
		return AtCommand_None;
	}
	if (!*p || *p != '@')
		return AtCommand_None;
	
	memset(info, 0, sizeof(AtCommandInfo));
	{
		char command[101];
		int i = 0;
		sscanf(p, "%100s", command);
		command[100] = '\0';
		
		while (atcommand_info[i].type != AtCommand_Unknown) {
			if (strcmpi(command, atcommand_info[i].command) == 0 &&
				level >= atcommand_info[i].level)
				break;
			i++;
		}
		if (atcommand_info[i].type == AtCommand_Unknown)
			return AtCommand_Unknown;
		memcpy(info, &atcommand_info[i], sizeof atcommand_info[i]);
	}
	
	return info->type;
}

//struct Atcommand_Config atcommand_config;

/*==========================================
 * 
 *------------------------------------------
 */
static int atkillmonster_sub(struct block_list *bl,va_list ap)
{
	int flag = va_arg(ap,int);
	if (flag)
		mob_damage(NULL,(struct mob_data *)bl,((struct mob_data *)bl)->hp,2);
	else
		mob_delete((struct mob_data *)bl);
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
/* Read Message Data */
int msg_config_read(const char *cfgName)
{
	int i,msg_number;
	char line[1024],w1[512],w2[1024];
	FILE *fp;

	fp=fopen(cfgName,"r");
	if (fp==NULL) {
		printf("file not found: %s\n",cfgName);
		return 1;
						}
	while(fgets(line,1020,fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;
		i=sscanf(line,"%d: %[^\r\n]",&msg_number,w2);
		if (i!=2) {
			if (sscanf(line,"%s: %[^\r\n]",w1,w2) !=2)
				continue;
			if (strcmpi(w1,"import") == 0) {
				msg_config_read(w2);
					}
			continue;
				}
		if (msg_number>=0&&msg_number<=200)
			strcpy(msg_table[msg_number],w2);
		//printf("%d:%s\n",msg_number,msg);
			}
	fclose(fp);
	return 0;
}

/*
static AtCommandInfo* getAtCommandInfoByType(const AtCommandType type)
{
	int i = 0;
	for (i = 0; atcommand_info[i].type != AtCommand_None; i++)
		if (atcommand_info[i].type == type)
			return &atcommand_info[i];
	return NULL;
}
*/

/*==========================================
 * 
 *------------------------------------------
 */
static AtCommandInfo*
get_atcommandinfo_byname(const char* name)
{
	int i = 0;
	for (i = 0; atcommand_info[i].type != AtCommand_Unknown; i++)
		if (strcmpi(atcommand_info[i].command + 1, name) == 0)
			return &atcommand_info[i];
	return NULL;
}

/*==========================================
 * 
 *------------------------------------------
 */
int atcommand_config_read(const char *cfgName)
{
	int i;
	char line[1024],w1[1024],w2[1024];
	
	if (battle_config.atc_gmonly > 0) {
		AtCommandInfo* p = NULL;
		FILE* fp = fopen(cfgName,"r");
		if (fp == NULL) {
			printf("file not found: %s\n",cfgName);
			return 1;
		}
		while (fgets(line, 1020, fp)) {
			if (line[0] == '/' && line[1] == '/')
				continue;
			i=sscanf(line,"%1020[^:]:%1020s",w1,w2);
			if (i!=2)
				continue;
			p = get_atcommandinfo_byname(w1);
			if (p != NULL)
				p->level = atoi(w2);
			
			if (strcmpi(w1, "import") == 0)
				atcommand_config_read(w2);
			}
		fclose(fp);
		}

	return 0;
}

// @ command 処理関数群

// @rura+
/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_rurap(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char map[100];
	char character[100];
	int x = 0, y = 0;
	struct map_session_data *pl_sd = NULL;
	
	if (!message || !*message)
		return -1;
	memset(character, '\0', sizeof character);
	if (sscanf(message, "%99s %d %d %99[^\n]", map, &x, &y, character) < 4)
		return -1;
	pl_sd = map_nick2sd(character);
	if (pl_sd == NULL) {
		clif_displaymessage(fd, msg_table[3]);
		return -1;
			}
	if (pc_isGM(sd) > pc_isGM(pl_sd)) {
		if (x >= 0 && x < 400 && y >= 0 && y < 400) {
			if (pc_setpos(pl_sd, map, x, y, 3) == 0) {
				clif_displaymessage(pl_sd->fd, msg_table[0]);
			} else {
				clif_displaymessage(fd, msg_table[1]);
		}
		} else {
			clif_displaymessage(fd,msg_table[2]);
			}
		}
	
	return 0;
}

// @rura
/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_rura(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char map[100];
	int x = 0, y = 0;
	
	if (!message || !*message)
		return -1;
	
	if (sscanf(message, "%99s %d %d", map, &x, &y) < 1)
		return -1;
			if (x >= 0 && x < 400 && y >= 0 && y < 400) {
		clif_displaymessage(fd,
			(pc_setpos((struct map_session_data*)sd, map, x, y, 3) == 0) ?
				msg_table[0] : msg_table[1]);
	} else {
		clif_displaymessage(fd, msg_table[2]);
			}
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_where(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char character[100];
	char output[200];
	struct map_session_data *pl_sd = NULL;
	if (!message || !*message)
		return -1;
	memset(character, '\0', sizeof character);
	if (sscanf(message, "%99[^\n]", character) < 1)
		return -1;
	if ((pl_sd = map_nick2sd(character)) == NULL) {
		snprintf(output, sizeof output, "%s %d %d",
			sd->mapname, sd->bl.x, sd->bl.y);
		clif_displaymessage(fd, output);
		return -1;
	}
	snprintf(output, sizeof output, "%s %s %d %d",
		character, pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y);
	clif_displaymessage(fd, output);
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_jumpto(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char character[100];
	struct map_session_data *pl_sd = NULL;
	
	if (!message || !*message)
		return -1;
	
	memset(character, '\0', sizeof character);
	if (sscanf(message, "%99[^\n]", character) < 1)
		return -1;
	if ((pl_sd = map_nick2sd(character)) != NULL) {
		char output[200];
		pc_setpos((struct map_session_data*)sd,
			pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y, 3);
		snprintf(output, sizeof output, msg_table[4], character);
		clif_displaymessage(fd, output);
	} else {
		clif_displaymessage(fd, msg_table[3]);
		}
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_jump(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int x = 0, y = 0;
	
	if (sscanf(message, "%d %d", &x, &y) < 2)
		return -1;
	if (x >= 0 && x < 400 && y >= 0 && y < 400) {
		char output[200];
		pc_setpos((struct map_session_data*)sd, (char*)sd->mapname, x, y, 3);
		snprintf(output, sizeof output, msg_table[5], x, y);
		clif_displaymessage(fd, output);
	} else {
		clif_displaymessage(fd, msg_table[2]);
				}
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_who(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	int i = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) &&
			pl_sd->state.auth) {
			clif_displaymessage(fd, pl_sd->status.name);
			}
		}
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_save(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	pc_setsavepoint(sd, sd->mapname, sd->bl.x, sd->bl.y);
	if (sd->status.pet_id > 0 && sd->pd)
		intif_save_petdata(sd->status.account_id, &sd->pet);
			pc_makesavestatus(sd);
			chrif_save(sd);
			storage_storage_save(sd);
	clif_displaymessage(fd, msg_table[6]);
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_load(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	pc_setpos(sd, sd->status.save_point.map,
		sd->status.save_point.x, sd->status.save_point.y, 0);
	clif_displaymessage(fd, msg_table[7]);
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_speed(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int speed = DEFAULT_WALK_SPEED;
	if (!message || !*message)
		return -1;
	speed = atoi(message);
	if (speed > MIN_WALK_SPEED && speed < MAX_WALK_SPEED) {
		sd->speed = speed;
				//sd->walktimer = x;
				//この文を追加 by れあ
		clif_updatestatus(sd, SP_SPEED);
		clif_displaymessage(fd, msg_table[8]);
		}
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_storage(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
			storage_storageopen(sd);
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_guildstorage(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	if (sd->status.guild_id > 0)
				storage_guild_storageopen(sd);
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_option(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int param1 = 0, param2 = 0, param3 = 0;
	
	if (!message || !*message)
		return -1;
	
	if (sscanf(message, "%d %d %d", &param1, &param2, &param3) < 1)
		return -1;
	sd->opt1 = param1;
	sd->opt2 = param2;
	if (!(sd->status.option & CART_MASK) && param3 & CART_MASK) {
				clif_cart_itemlist(sd);
				clif_cart_equiplist(sd);
		clif_updatestatus(sd, SP_CARTINFO);
			}
	sd->status.option = param3;
			clif_changeoption(&sd->bl);
			clif_displaymessage(fd,msg_table[9]);
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_hide(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	if (sd->status.option & OPTION_HIDE) {
		sd->status.option &= ~OPTION_HIDE;
		clif_displaymessage(fd, msg_table[10]);
	} else {
		sd->status.option |= OPTION_HIDE;
		clif_displaymessage(fd, msg_table[11]);
			}
			clif_changeoption(&sd->bl);
	
	return 0;
}

/*==========================================
 * 転職する upperを指定すると転生や養子にもなれる
 *------------------------------------------
 */
int
atcommand_jobchange(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int job = 0, upper = -1;
	if (!message || !*message)
		return -1;
	if (sscanf(message, "%d %d", &job, &upper) < 1)
		return -1;

	if ((job >= 0 && job < MAX_PC_CLASS)) {
		if(pc_jobchange(sd, job, upper) == 0)
			clif_displaymessage(fd, msg_table[12]);
	}
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_die(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	pc_damage(NULL, sd, sd->status.hp + 1);
	clif_displaymessage(fd, msg_table[13]);
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_kill(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char character[100];
	struct map_session_data *pl_sd = NULL;
	
	if (!message || !*message)
		return -1;
	memset(character, '\0', sizeof character);
	sscanf(message, "%99[^\n]", character);
	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (pc_isGM(sd) > pc_isGM(pl_sd)) {
			pc_damage(NULL, pl_sd, pl_sd->status.hp + 1);
			clif_displaymessage(fd, msg_table[14]);
			}
	} else {
		clif_displaymessage(fd, msg_table[15]);
		}
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_alive(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	sd->status.hp = sd->status.max_hp;
	sd->status.sp = sd->status.max_sp;
			pc_setstand(sd);
	if (battle_config.pc_invincible_time > 0)
		pc_setinvincibletimer(sd, battle_config.pc_invincible_time);
	clif_updatestatus(sd, SP_HP);
	clif_updatestatus(sd, SP_SP);
	clif_resurrection(&sd->bl, 1);
	clif_displaymessage(fd, msg_table[16]);
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_kami(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char output[200];
	
	if (!message || !*message)
		return -1;
	sscanf(message, "%199[^\n]", output);
	intif_GMmessage(output,
		strlen(output) + 1,
		(*(command + 5) == 'b') ? 0x10 : 0);
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_heal(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int hp = 0, sp = 0;
	
	if (!message || !*message)
		return -1;
	if (sscanf(message, "%d %d", &hp, &sp) < 1)
		return -1;
	if (hp <= 0 && sp <= 0) {
		hp = sd->status.max_hp-sd->status.hp;
		sp = sd->status.max_sp-sd->status.sp;
	} else {
		if (sd->status.hp + hp > sd->status.max_hp) {
			hp = sd->status.max_hp - sd->status.hp;
		}
		if (sd->status.sp + sp > sd->status.max_sp) {
			sp = sd->status.max_sp - sd->status.sp;
		}
		}
	clif_heal(fd, SP_HP, (hp > 0x7fff) ? 0x7fff : hp);
	clif_heal(fd, SP_SP, (sp > 0x7fff) ? 0x7fff : sp);
	pc_heal(sd, hp, sp);
	clif_displaymessage(fd, msg_table[17]);
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_item(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char item_name[100];
	int number = 0, item_id = 0, flag = 0;
			struct item item_tmp;
			struct item_data *item_data;

	if (!message || !*message)
		return -1;

	if (sscanf(message, "%99s %d", item_name, &number) < 1)
		return -1;
	if (number <= 0)
		number = 1;
	
	if ((item_id = atoi(item_name)) > 0) {
		if (battle_config.item_check) {
			item_id =
				(((item_data = itemdb_exists(item_id)) &&
				 itemdb_available(item_id)) ? item_id : 0);
		} else {
			item_data = itemdb_search(item_id);
				}
	} else if ((item_data = itemdb_searchname(item_name)) != NULL) {
		item_id = (!battle_config.item_check ||
			itemdb_available(item_data->nameid)) ? item_data->nameid : 0;
	}
	
	if (item_id > 0) {
		int loop = 1, get_count = number,i;
		if (item_data->type == 4 || item_data->type == 5 ||
			item_data->type == 7 || item_data->type == 8) {
			loop = number;
			get_count = 1;
		}
		for (i = 0; i < loop; i++) {
			memset(&item_tmp, 0, sizeof(item_tmp));
			item_tmp.nameid = item_id;
			item_tmp.identify = 1;
			if ((flag = pc_additem((struct map_session_data*)sd,
					&item_tmp, get_count)))
				clif_additem((struct map_session_data*)sd, 0, 0, flag);
				}
		clif_displaymessage(fd, msg_table[18]);
			} else {
		clif_displaymessage(fd, msg_table[19]);
			}
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_item2(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
			struct item item_tmp;
			struct item_data *item_data;
	char item_name[100];
	int item_id = 0, number = 0;
	int identify = 0, refine = 0, attr = 0;
	int c1 = 0, c2 = 0, c3 = 0, c4 = 0;
	int flag = 0;
	
	if (sscanf(message, "%99s %d %d %d %d %d %d %d %d", item_name, &number,
		&identify, &refine, &attr, &c1, &c2, &c3, &c4) >= 9) {
		if (number <= 0)
			number = 1;
		
		if ((item_id = atoi(item_name)) > 0) {
			if (battle_config.item_check) {
				item_id =
					(((item_data = itemdb_exists(item_id)) &&
					  itemdb_available(item_id)) ? item_id : 0);
			} else {
				item_data = itemdb_search(item_id);
					}
		} else if ((item_data = itemdb_searchname(item_name)) != NULL) {
			item_id =
				(!battle_config.item_check ||
				 itemdb_available(item_data->nameid)) ? item_data->nameid : 0;
		}
		
		if (item_id > 0) {
			int loop = 1, get_count = number, i = 0;
			
			if (item_data->type == 4 || item_data->type == 5 ||
				item_data->type == 7 || item_data->type == 8) {
				loop = number;
				get_count = 1;
				if (item_data->type == 7) {
					identify = 1;
					refine = 0;
				}
				if (item_data->type == 8)
					refine = 0;
				if (refine > 10)
					refine = 10;
				} else {
				identify = 1;
				refine = attr = 0;
				}
			for (i = 0; i < loop; i++) {
				memset(&item_tmp, 0, sizeof(item_tmp));
				item_tmp.nameid = item_id;
				item_tmp.identify = identify;
				item_tmp.refine = refine;
				item_tmp.attribute = attr;
				item_tmp.card[0] = c1;
				item_tmp.card[1] = c2;
				item_tmp.card[2] = c3;
				item_tmp.card[3] = c4;
				if ((flag = pc_additem(sd, &item_tmp, get_count)))
					clif_additem(sd, 0, 0, flag);
			}
			clif_displaymessage(fd, msg_table[18]);
		} else {
			clif_displaymessage(fd, msg_table[19]);
		}
	} else {
		return -1;
			}
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_itemreset(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int i = 0;
	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].amount &&
			sd->status.inventory[i].equip == 0)
			pc_delitem(sd, i, sd->status.inventory[i].amount, 0);
		}
	clif_displaymessage(fd, msg_table[20]);
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_itemcheck(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
			pc_checkitem(sd);
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_baselevelup(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int level = 0, i = 0;
	
	if (!message || !*message)
		return -1;
	
	level = atoi(message);
	if (level >= 1) {
		for (i = 1; i <= level; i++)
				sd->status.status_point += (sd->status.base_level + i + 14) / 5 ;
		sd->status.base_level += level;
		clif_updatestatus(sd, SP_BASELEVEL);
		clif_updatestatus(sd, SP_NEXTBASEEXP);
		clif_updatestatus(sd, SP_STATUSPOINT);
		pc_calcstatus(sd, 0);
		pc_heal(sd, sd->status.max_hp, sd->status.max_sp);
		clif_misceffect(&sd->bl, 0);
		clif_displaymessage(fd, msg_table[21]);
	} else if (level < 0 && sd->status.base_level + level > 0) {
		sd->status.base_level += level;
		clif_updatestatus(sd, SP_BASELEVEL);
		clif_updatestatus(sd, SP_NEXTBASEEXP);
		pc_calcstatus(sd, 0);
		clif_displaymessage(fd, msg_table[22]);
							}
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_joblevelup(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int up_level = 50, level = 0;
	//転生や養子の場合の元の職業を算出する
	struct pc_base_job s_class = pc_calc_base_job(sd->status.class);

	if (!message || !*message)
		return -1;
	level = atoi(message);
	if (s_class.job == 0)
		up_level -= 40;
	if ((s_class.job == 23) || (s_class.upper == 1 && s_class.type == 2)) //スパノビと転生二次職はJobレベルの最高が70
		up_level += 20;
	if (sd->status.job_level == up_level) {
		clif_displaymessage(fd, msg_table[23]);
	} else if (level >= 1) {
		if (sd->status.job_level + level > up_level)
			level = up_level - sd->status.job_level;
		sd->status.job_level += level;
		clif_updatestatus(sd, SP_JOBLEVEL);
		clif_updatestatus(sd, SP_NEXTJOBEXP);
		sd->status.skill_point += level;
		clif_updatestatus(sd, SP_SKILLPOINT);
		pc_calcstatus(sd, 0);
		clif_misceffect(&sd->bl, 1);
		clif_displaymessage(fd, msg_table[24]);
	} else if (level < 0 && sd->status.job_level + level > 0) {
		sd->status.job_level += level;
		clif_updatestatus(sd, SP_JOBLEVEL);
		clif_updatestatus(sd, SP_NEXTJOBEXP);
		pc_calcstatus(sd, 0);
		clif_displaymessage(fd, msg_table[25]);
						}
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_help(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char buf[BUFSIZ];
	FILE* fp = fopen(help_txt, "r");
	if (fp != NULL) {
		int i = 0;
		clif_displaymessage(fd, msg_table[26]);
		while (fgets(buf, sizeof buf - 20, fp) != NULL) {
			for (i = 0; buf[i] != '\0'; i++) {
				if (buf[i] == '\r' || buf[i] == '\n') {
					buf[i] = '\0';
					}
				}
			clif_displaymessage(fd, buf);
		}
		fclose(fp);
	} else
		clif_displaymessage(fd, msg_table[27]);
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_gm(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char password[100];
	
	if (!message || !*message)
		return -1;
	
	sscanf(message, "%99[^\n]", password);
	if (sd->status.party_id)
		clif_displaymessage(fd, msg_table[28]);
	else if (sd->status.guild_id)
		clif_displaymessage(fd, msg_table[29]);
	else {
		if (sd->status.pet_id > 0 && sd->pd)
			intif_save_petdata(sd->status.account_id, &sd->pet);
				pc_makesavestatus(sd);
				chrif_save(sd);
				storage_storage_save(sd);
		clif_displaymessage(fd, msg_table[30]);
		chrif_changegm(sd->status.account_id, password, strlen(password) + 1);
		}

	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_pvpoff(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	int i = 0;
	
	if (map[sd->bl.m].flag.pvp) {
				map[sd->bl.m].flag.pvp = 0;
		clif_send0199(sd->bl.m, 0);
		for (i = 0; i < fd_max; i++) {	//人数分ループ
			if (session[i] && (pl_sd = session[i]->session_data) &&
				pl_sd->state.auth) {
				if (sd->bl.m == pl_sd->bl.m) {
					clif_pvpset(pl_sd, 0, 0, 2);
					if (pl_sd->pvp_timer != -1) {
						delete_timer(pl_sd->pvp_timer, pc_calc_pvprank_timer);
								pl_sd->pvp_timer = -1;
							}
						}
					}
				}
		clif_displaymessage(fd, msg_table[31]);
		}

	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_pvpon(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	int i = 0;
	
	if (!map[sd->bl.m].flag.pvp) {
				map[sd->bl.m].flag.pvp = 1;
		clif_send0199(sd->bl.m, 1);
		for (i = 0; i < fd_max; i++) {
			if (session[i] && (pl_sd = session[i]->session_data) &&
				pl_sd->state.auth) {
				if (sd->bl.m == pl_sd->bl.m && pl_sd->pvp_timer == -1) {
					pl_sd->pvp_timer = add_timer(gettick() + 200,
						pc_calc_pvprank_timer, pl_sd->bl.id, 0);
					pl_sd->pvp_rank = 0;
					pl_sd->pvp_lastusers = 0;
					pl_sd->pvp_point = 5;
						}
					}
				}
		clif_displaymessage(fd, msg_table[32]);
		}

	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_gvgoff(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	if (map[sd->bl.m].flag.gvg) {
				map[sd->bl.m].flag.gvg = 0;
		clif_send0199(sd->bl.m, 0);
		clif_displaymessage(fd, msg_table[33]);
		}
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_gvgon(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	if (!map[sd->bl.m].flag.gvg) {
				map[sd->bl.m].flag.gvg = 1;
		clif_send0199(sd->bl.m, 3);
		clif_displaymessage(fd, msg_table[34]);
			}
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_model(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int hair_style = 0, hair_color = 0, cloth_color = 0;
	if (!message || !*message)
		return -1;
	if (sscanf(message, "%d %d %d", &hair_style, &hair_color, &cloth_color) < 1)
		return -1;
	if (hair_style >= MIN_HAIR_STYLE && hair_style < MAX_HAIR_STYLE &&
		hair_color >= MIN_HAIR_COLOR && hair_color < MAX_HAIR_COLOR &&
		cloth_color >= MIN_CLOTH_COLOR && cloth_color <= MAX_CLOTH_COLOR) {
				//服の色変更
		if (cloth_color != 0 &&
			sd->status.sex == 1 &&
			(sd->status.class == 12 ||  sd->status.class == 17)) {
					//服の色未実装職の判定
					clif_displaymessage(fd,msg_table[35]);
				} else {
			pc_changelook(sd, LOOK_HAIR, hair_style);
			pc_changelook(sd, LOOK_HAIR_COLOR, hair_color);
			pc_changelook(sd, LOOK_CLOTHES_COLOR, cloth_color);
			clif_displaymessage(fd, msg_table[36]);
				}
			} else {
		clif_displaymessage(fd, msg_table[37]);
		}

	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_go(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int town = 0;
			struct { char map[16]; int x,y; } data[] = {
				{	"prontera.gat",	156, 191	},	//	0=プロンテラ
				{	"morocc.gat",	156,  93	},	//	1=モロク
				{	"geffen.gat",	119,  59	},	//	2=ゲフェン
				{	"payon.gat",	 89, 122	},	//	3=フェイヨン
				{	"alberta.gat",	192, 147	},	//	4=アルベルタ
				{	"izlude.gat",	128, 114	},	//	5=イズルード
				{	"aldebaran.gat",140, 131	},	//	6=アルデバラン
				{	"xmas.gat",		147, 134	},	//	7=ルティエ
				{	"comodo.gat",	209, 143	},	//	8=コモド
				{	"yuno.gat",		157,  51	},	//	9=ジュノー
				{	"amatsu.gat",	198,  84	},	//	10=アマツ
				{	"gonryun.gat",	160, 120	},	//	11=ゴンリュン
			};
	
	if (!message || !*message)
		return -1;
	
	town = atoi(message);
	if (town >= 0 && town < sizeof(data) / sizeof(data[0])) {
		pc_setpos((struct map_session_data*)sd,
			data[town].map, data[town].x, data[town].y, 3);
			} else {
		clif_displaymessage(fd, msg_table[38]);
		}
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_monster(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char name[100];
	char monster[100];
	int mob_id = 0;
	int number = 0;
	int x = 0;
	int y = 0;
	int count = 0;
	int i = 0;
	
	if (!message || !*message)
		return -1;
	if (sscanf(message, "%99s %99s %d %d %d", name, monster,
		&number, &x, &y) < 3)
		return -1;
	
	if ((mob_id = atoi(monster)) == 0)
		mob_id = mobdb_searchname(monster);
	if (number <= 0)
		number = 1;
	if (battle_config.etc_log)
		printf("%s monster=%s name=%s id=%d count=%d (%d,%d)\n",
			command, monster, name, mob_id, number, x, y);
	
	for (i = 0; i < number; i++) {
		int mx = 0, my = 0;
		if (x <= 0)
			mx = sd->bl.x + (rand() % 10 - 5);
		else
			mx = x;
		if (y <= 0)
			my = sd->bl.y + (rand() % 10 - 5);
		else
			my = y;
		count +=
			(mob_once_spawn((struct map_session_data*)sd,
				"this", mx, my, name, mob_id, 1, "") != 0) ?
			1 : 0;
				}
	if (count != 0) {
					clif_displaymessage(fd,msg_table[39]);
	} else {
					clif_displaymessage(fd,msg_table[40]);
				}
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
void
atcommand_killmonster_sub(
	const int fd, struct map_session_data* sd, const char* message,
	const int drop)
{
	int map_id = 0;
	
	if (!message || !*message) {
		map_id = sd->bl.m;
	} else {
		char map_name[100];
		sscanf(message, "%99s", map_name);
		if (strstr(map_name, ".gat") == NULL && strlen(map_name) < 16) {
			strcat(map_name, ".gat");
			}
		if ((map_id = map_mapname2mapid(map_name)) < 0)
			map_id = sd->bl.m;
						}
	map_foreachinarea(atkillmonster_sub, map_id, 0, 0,
		map[map_id].xs, map[map_id].ys, BL_MOB, drop);
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_killmonster(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	atcommand_killmonster_sub(fd, sd, message, 1);
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_killmonster2(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	atcommand_killmonster_sub(fd, sd, message, 0);
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_refine(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int i = 0, position = 0, refine = 0, current_position = 0;
	struct map_session_data* p = (struct map_session_data*)sd;
	
	if (!message || !*message)
		return -1;
	
	if (sscanf(message, "%d %d", &position, &refine) >= 2) {
		for (i = 0; i < MAX_INVENTORY; i++) {
			if (sd->status.inventory[i].nameid &&	// 該当個所の装備を精錬する
			    (sd->status.inventory[i].equip & position ||
				(sd->status.inventory[i].equip && !position))) {
				if (refine <= 0 || refine > 10)
					refine = 1;
				if (sd->status.inventory[i].refine < 10) {
					p->status.inventory[i].refine += refine;
					if (sd->status.inventory[i].refine > 10)
						p->status.inventory[i].refine = 10;
					current_position = sd->status.inventory[i].equip;
					pc_unequipitem((struct map_session_data*)sd, i, 0);
					clif_refine(fd, (struct map_session_data*)sd,
						0, i, sd->status.inventory[i].refine);
					clif_delitem((struct map_session_data*)sd, i, 1);
					clif_additem((struct map_session_data*)sd, i, 1, 0);
					pc_equipitem((struct map_session_data*)sd, i,
						current_position);
					clif_misceffect((struct block_list*)&sd->bl, 3);
					}
				}
			}
		}
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_produce(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char item_name[100];
	int item_id = 0, attribute = 0, star = 0;
	int flag = 0;
	if (!message || !*message)
		return -1;
	
	if (sscanf(message, "%99s %d %d", item_name, &attribute, &star) > 0) {
		if ((item_id = atoi(item_name)) == 0) {
			struct item_data *item_data = itemdb_searchname(item_name);
			if (item_data)
				item_id = item_data->nameid;
		}
		if (itemdb_exists(item_id) &&
			(item_id <= 500 || item_id > 1099) &&
			(item_id < 4001 || item_id > 4148) &&
			(item_id < 7001 || item_id > 10019) &&
			itemdb_isequip(item_id)) {
					struct item tmp_item;
			if (attribute < MIN_ATTRIBUTE || attribute > MAX_ATTRIBUTE)
				attribute = ATTRIBUTE_NORMAL;
			if (star < MIN_STAR || star > MAX_STAR)
				star = 0;
			memset(&tmp_item, 0, sizeof tmp_item);
			tmp_item.nameid = item_id;
			tmp_item.amount = 1;
			tmp_item.identify = 1;
			tmp_item.card[0] = 0x00ff;
			tmp_item.card[1] = ((star * 5) << 8) + attribute;
			*((unsigned long *)(&tmp_item.card[2])) = sd->char_id;
			clif_produceeffect(sd, 0, item_id); // 製造エフェクトパケット
			clif_misceffect(&sd->bl, 3); // 他人にも成功を通知
			if ((flag = pc_additem(sd, &tmp_item, 1)))
				clif_additem(sd, 0, 0, flag);
		} else {
			if (battle_config.error_log)
				printf("@produce NOT WEAPON [%d]\n", item_id);
		}
		}

	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_memo(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int position = 0;
	
	if (!message || !*message)
		return -1;
	
	position = atoi(message);
	if (position < MIN_PORTAL_MEMO || position > MAX_PORTAL_MEMO)
		position = MIN_PORTAL_MEMO;
	pc_memo(sd, position+MIN_PORTAL_MEMO);
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_gat(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char output[BUFSIZ];
	int y = 0;
	for (y = 2; y >= -2; y--) {
		snprintf(output, sizeof output,
			"%s (x= %d, y= %d) %02X %02X %02X %02X %02X",
			map[sd->bl.m].name, sd->bl.x - 2, sd->bl.y + y,
			map_getcell(sd->bl.m, sd->bl.x - 2, sd->bl.y + y),
			map_getcell(sd->bl.m, sd->bl.x - 1, sd->bl.y + y),
			map_getcell(sd->bl.m, sd->bl.x,     sd->bl.y + y),
			map_getcell(sd->bl.m, sd->bl.x + 1, sd->bl.y + y),
			map_getcell(sd->bl.m, sd->bl.x + 2, sd->bl.y + y));
		clif_displaymessage(fd, output);
			}
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_packet(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int x = 0, y = 0;
	
	if (!message || !*message)
		return -1;
	
	if (sscanf(message,"%d %d", &x, &y) < 2)
			return 1;
	clif_status_change(&sd->bl, x, y);
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_statuspoint(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int point = 0;
	
	if (!message || !*message)
		return -1;
	point = atoi(message);
	if (point > 0 || sd->status.status_point + point >= 0) {
		sd->status.status_point += point;
		clif_updatestatus(sd, SP_STATUSPOINT);
	} else {
		clif_displaymessage(fd, msg_table[41]);
		}

	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_skillpoint(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int point = 0;
	if (!message || !*message)
		return -1;
	point = atoi(message);
	if (point > 0 || sd->status.skill_point + point >= 0) {
		sd->status.skill_point += point;
		clif_updatestatus(sd, SP_SKILLPOINT);
	} else {
		clif_displaymessage(fd, msg_table[41]);
		}

	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_zeny(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int zeny = 0;
	
	if (!message || !*message)
		return -1;
	zeny = atoi(message);
	if (zeny > 0 || sd->status.zeny + zeny >= 0) {
		if (sd->status.zeny + zeny > MAX_ZENY)
			zeny = MAX_ZENY - sd->status.zeny;
		sd->status.zeny += zeny;
		clif_updatestatus(sd, SP_ZENY);
	} else {
		clif_displaymessage(fd, msg_table[41]);
		}
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_param(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int i = 0, value = 0, index = -1, new_value = 0;
	
	const char* param[] = {
		"@str", "@agi", "@vit", "@int", "@dex", "@luk", NULL
	};
	short* status[] = {
		&sd->status.str,  &sd->status.agi, &sd->status.vit,
		&sd->status.int_, &sd->status.dex, &sd->status.luk
	};
	
	if (!message || !*message)
		return -1;
	value = atoi(message);
	
	for (i = 0; param[i] != NULL; i++) {
		if (strcmpi(command, param[i]) == 0) {
			index = i;
			break;
		}
	}
	if (index < 0 || index > MAX_STATUS_TYPE)
		return -1;
	
	new_value = (int)(*status[index]) + value;
	if (new_value < 1)
		value = 1 - *status[index];
	if (new_value > battle_config.max_parameter)
		value = battle_config.max_parameter - *status[index];
	*status[index] += value;
	
	clif_updatestatus(sd, SP_STR + index);
	clif_updatestatus(sd, SP_USTR + index);
	pc_calcstatus(sd, 0);
	clif_displaymessage(fd, msg_table[42]);
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_guildlevelup(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int level = 0;
	struct guild *guild_info = NULL;
	
	if (!message || !*message)
		return -1;
	level = atoi(message);
	if (sd->status.guild_id <= 0 ||
		(guild_info = guild_search(sd->status.guild_id)) == NULL) {
		clif_displaymessage(fd, msg_table[43]);
		return 0;
	}
	if (strcmp(sd->status.name, guild_info->master) != 0) {
		clif_displaymessage(fd, msg_table[44]);
		return 0;
		}

	if (guild_info->guild_lv + level >= 1 &&
		guild_info->guild_lv + level <= MAX_GUILDLEVEL)
		intif_guild_change_basicinfo(guild_info->guild_id,
			GBI_GUILDLV, &level, 2);
	else
		clif_displaymessage(fd, msg_table[45]);
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_makepet(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int id = 0, pet_id = 0;

	if (!message || !*message)
		return -1;
			
	id = atoi(message);
	pet_id = search_petDB_index(id, PET_CLASS);
	if (pet_id < 0)
		pet_id = search_petDB_index(id, PET_EGG);
	if (pet_id >= 0) {
		sd->catch_target_class = pet_db[pet_id].class;
		intif_create_pet(
			sd->status.account_id, sd->status.char_id,
			pet_db[pet_id].class, mob_db[pet_db[pet_id].class].lv,
			pet_db[pet_id].EggID, 0, pet_db[pet_id].intimate,
			100, 0, 1, pet_db[pet_id].jname);
	} else {
		return -1;
		}

	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_petfriendly(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int friendly = 0;

	if (!message || !*message)
		return -1;

	friendly = atoi(message);
	if (sd->status.pet_id > 0 && sd->pd) {
		if (friendly >= 0 && friendly <= 1000) {
					int t = sd->pet.intimate;
			sd->pet.intimate = friendly;
					clif_send_petstatus(sd);
			if (battle_config.pet_status_support) {
				if ((sd->pet.intimate > 0 && t <= 0) ||
					(sd->pet.intimate <= 0 && t > 0)) {
					if (sd->bl.prev != NULL)
						pc_calcstatus(sd, 0);
							else
						pc_calcstatus(sd, 2);
					}
				}
		} else {
			return -1;
			}
		}

	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_pethungry(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int hungry = 0;
	
	if (!message || !*message)
		return -1;
	
	hungry = atoi(message);
	if (sd->status.pet_id > 0 && sd->pd) {
		if (hungry >= 0 && hungry <= 100) {
			sd->pet.hungry = hungry;
					clif_send_petstatus(sd);
		} else {
			return -1;
				}
			}

	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_petrename(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	if (sd->status.pet_id > 0 && sd->pd) {
				sd->pet.rename_flag = 0;
		intif_save_petdata(sd->status.account_id, &sd->pet);
				clif_send_petstatus(sd);
			}

	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_recall(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char character[100];
	struct map_session_data *pl_sd = NULL;
	
	if (!message || !*message)
		return -1;
	
	memset(character, '\0', sizeof character);
	memset(character, '\0', sizeof character);
	sscanf(message, "%99[^\n]", character);
	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (pc_isGM(sd) > pc_isGM(pl_sd)) {
			char output[200];
					pc_setpos(pl_sd, sd->mapname, sd->bl.x, sd->bl.y, 2);
			snprintf(output, sizeof output, msg_table[46], character);
			clif_displaymessage(fd, output);
				}
	} else {
		clif_displaymessage(fd, msg_table[47]);
					}
	
	return 0;
}

/*==========================================
 * 対象キャラクターを転職させる upper指定で転生や養子も可能
 *------------------------------------------
 */
int
atcommand_character_job(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char character[100];
	struct map_session_data* pl_sd = NULL;
	int job = 0, upper = -1;
	
	if (!message || !*message)
		return -1;
	
	memset(character, '\0', sizeof character);
	if (sscanf(message, "%d %d %99[^\n]", &job, &upper, character) < 3){ //upper指定してある
		upper = -1;
		if (sscanf(message, "%d %99[^\n]", &job, character) < 2) //upper指定してない上に何か足りない
			return -1;
	}
	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (pc_isGM(sd) > pc_isGM(pl_sd)) {
			if ((job >= 0 && job < MAX_PC_CLASS)) {
				pc_jobchange(pl_sd, job, upper);
				clif_displaymessage(fd, msg_table[48]);
			} else {
				clif_displaymessage(fd, msg_table[49]);
				}
			}
	} else {
		clif_displaymessage(fd, msg_table[50]);
	}
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_revive(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char character[100];
	struct map_session_data *pl_sd = NULL;
	
	if (!message || !*message)
		return -1;
	memset(character, '\0', sizeof character);
	sscanf(message, "%99[^\n]", character);
	if ((pl_sd = map_nick2sd(character)) != NULL) {
		pl_sd->status.hp = pl_sd->status.max_hp;
				pc_setstand(pl_sd);
		if (battle_config.pc_invincible_time > 0)
			pc_setinvincibletimer(sd, battle_config.pc_invincible_time);
		clif_updatestatus(pl_sd, SP_HP);
		clif_updatestatus(pl_sd, SP_SP);
		clif_resurrection(&pl_sd->bl, 1);
		clif_displaymessage(fd, msg_table[51]);
	} else {
		clif_displaymessage(fd, msg_table[52]);
			}
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_character_stats(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char character[100];
	struct map_session_data *pl_sd = NULL;
	
	if (!message || !*message)
		return -1;
	memset(character, '\0', sizeof character);
	sscanf(message, "%99[^\n]", character);
	if ((pl_sd = map_nick2sd(character)) != NULL) {
		char output[200];
		int i = 0;
		struct {
			const char* format;
			int value;
		} output_table[] = {
			{ "Base Level - %d", pl_sd->status.base_level },
			{ "Job Level - %d", pl_sd->status.job_level },
			{ "Hp - %d", pl_sd->status.hp },
			{ "MaxHp - %d", pl_sd->status.max_hp },
			{ "Sp - %d", pl_sd->status.sp },
			{ "MaxSp - %d", pl_sd->status.max_sp },
			{ "Str - %d", pl_sd->status.str },
			{ "Agi - %d", pl_sd->status.agi },
			{ "Vit - %d", pl_sd->status.vit },
			{ "Int - %d", pl_sd->status.int_ },
			{ "Dex - %d", pl_sd->status.dex },
			{ "Luk - %d", pl_sd->status.luk },
			{ "Zeny - %d", pl_sd->status.zeny },
			{ NULL, 0 }
		};
		snprintf(output, sizeof output, msg_table[53], pl_sd->status.name);
		clif_displaymessage(fd, output);
		for (i = 0; output_table[i].format != NULL; i++) {
			snprintf(output, sizeof output,
				output_table[i].format, output_table[i].value);
			clif_displaymessage(fd, output);
		}
	} else {
				clif_displaymessage(fd,msg_table[54]);
			}
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_character_option(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char character[100];
	int opt1 = 0, opt2 = 0, opt3 = 0;
	struct map_session_data* pl_sd = NULL;
	
	if (!message || !*message)
		return -1;
	memset(character, '\0', sizeof character);
	if (sscanf(message, "%d %d %d %99[^\n]",
			&opt1, &opt2, &opt3, character) < 4)
		return -1;
	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (pc_isGM(sd) > pc_isGM(pl_sd)) {
			pl_sd->opt1 = opt1;
			pl_sd->opt2 = opt2;
			pl_sd->status.option = opt3;
					clif_changeoption(&pl_sd->bl);
			clif_displaymessage(fd, msg_table[55]);
			}
	} else {
		clif_displaymessage(fd, msg_table[56]);
				}
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_character_save(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char map_name[100];
	char character[100];
	struct map_session_data* pl_sd = NULL;
	int x = 0, y = 0;
	
	if (!message || !*message)
		return -1;
	memset(character, '\0', sizeof character);
	if (sscanf(message, "%99s %d %d %99[^\n]", map_name, &x, &y, character) < 4)
		return -1;
	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (pc_isGM(sd) > pc_isGM(pl_sd)) {
			pc_setsavepoint(pl_sd, map_name, x, y);
			clif_displaymessage(fd, msg_table[57]);
			}
	} else {
		clif_displaymessage(fd, msg_table[58]);
		}
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_night(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	int i = 0;
	for(i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) &&
			pl_sd->state.auth) {
			pl_sd->opt2 |= STATE_BLIND;
					clif_changeoption(&pl_sd->bl);
			clif_displaymessage(pl_sd->fd, msg_table[59]);
			}
		}
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_day(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	int i = 0;
	for(i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) &&
			pl_sd->state.auth) {
			pl_sd->opt2 &= ~STATE_BLIND;
					clif_changeoption(&pl_sd->bl);
			clif_displaymessage(pl_sd->fd, msg_table[60]);
				}
			}
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_doom(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	int i = 0;
	for(i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) &&
			pl_sd->state.auth) {
			pl_sd = session[i]->session_data;
			if (pc_isGM(sd) > pc_isGM(pl_sd)) {
				pc_damage(NULL, pl_sd, pl_sd->status.hp + 1);
				clif_displaymessage(pl_sd->fd, msg_table[61]);
		}
			}
		}
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_doommap(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	int i = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) &&
			pl_sd->state.auth && sd->bl.m == pl_sd->bl.m &&
			pc_isGM(sd) > pc_isGM(pl_sd)) {
					pc_damage(NULL, pl_sd, pl_sd->status.hp + 1);
					clif_displaymessage(pl_sd->fd, msg_table[61]);
	}
				}
	clif_displaymessage(fd, msg_table[62]);
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
static void
atcommand_raise_sub(struct map_session_data* sd)
{
	if (sd && sd->state.auth && pc_isdead(sd)) {
		sd->status.hp = sd->status.max_hp;
		sd->status.sp = sd->status.max_sp;
		pc_setstand(sd);
		clif_updatestatus(sd, SP_HP);
		clif_updatestatus(sd, SP_SP);
		clif_resurrection(&sd->bl, 1);
		clif_displaymessage(sd->fd, msg_table[63]);
			}
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_raise(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int i = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i])
			atcommand_raise_sub(session[i]->session_data);
		}
	clif_displaymessage(fd, msg_table[64]);
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_raisemap(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	int i = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) &&
			pl_sd->state.auth && pc_isdead(pl_sd) && sd->bl.m == pl_sd->bl.m)
				atcommand_raise_sub(pl_sd);
	}
	clif_displaymessage(fd, msg_table[64]);
	
	return 0;
}

/*==========================================
 * atcommand_character_baselevel @charbaselvlで対象キャラのレベルを上げる
 *------------------------------------------
*/
int
atcommand_character_baselevel(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	char character[100];
	int level = 0, i = 0;
	
	if (!message || !*message) //messageが空ならエラーを返して終了
		return -1; //エラーを返して終了
	memset(character, '\0', sizeof character);
	if (sscanf(message, "%d %99[^\n]", &level, character) < 2) //messageにLevelとキャラ名が無ければ
		return -1; //エラーを返して終了
	if ((pl_sd = map_nick2sd(character)) != NULL) { //該当名のキャラが存在する
		if (pc_isGM(sd) > pc_isGM(pl_sd)) { //対象キャラのGMレベルが自分より小さい
			if (level >= 1) { //上げるレベルが１より大きい
				for (i = 1; i <= level; i++) //入力されたレベル回ステータスポイントを追加する
					pl_sd->status.status_point += (pl_sd->status.base_level + i + 14) / 5 ;
				pl_sd->status.base_level += level; //対象キャラのベースレベルを上げる
				clif_updatestatus(pl_sd, SP_BASELEVEL); //クライアントに上げたベースレベルを送る
				clif_updatestatus(pl_sd, SP_NEXTBASEEXP); //クライアントに次のベースレベルアップまでの必要経験値を送る
				clif_updatestatus(pl_sd, SP_STATUSPOINT); //クライアントにステータスポイントを送る
				pc_calcstatus(pl_sd, 0); //ステータスを計算しなおす
				pc_heal(pl_sd, pl_sd->status.max_hp, pl_sd->status.max_sp); //HPとSPを完全回復させる
				clif_misceffect(&pl_sd->bl, 0); //ベースレベルアップエフェクトの送信
				clif_displaymessage(fd, msg_table[65]); //レベルを上げたメッセージを表示する
			} else if (level < 0 && pl_sd->status.base_level + level > 0) { //対象キャラのレベルがマイナスで変化結果が0以上なら
				pl_sd->status.base_level += level; //対象キャラのレベルを下げる
				clif_updatestatus(pl_sd, SP_BASELEVEL); //クライアントに下げたベースレベルを送る
				clif_updatestatus(pl_sd, SP_NEXTBASEEXP); //クライアントに次のベースレベルアップまでの必要経験値を送る
				pc_calcstatus(pl_sd, 0); //ステータスを計算しなおす
				clif_displaymessage(fd, msg_table[66]); //レベルを下げたメッセージを表示する
			}
		}
	}
	return 0; //正常終了
}

/*==========================================
 * atcommand_character_joblevel @charjoblvlで対象キャラのJobレベルを上げる
 *------------------------------------------
 */
int
atcommand_character_joblevel(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	char character[100];
	int max_level = 50, level = 0;
	//転生や養子の場合の元の職業を算出する
	struct pc_base_job pl_s_class;
	
	if (!message || !*message)
		return -1;
	memset(character, '\0', sizeof character);
	if (sscanf(message, "%d %99[^\n]", &level, character) < 2)
		return -1;
	if ((pl_sd = map_nick2sd(character)) != NULL) {
		pl_s_class = pc_calc_base_job(pl_sd->status.class);
		if (pc_isGM(sd) > pc_isGM(pl_sd)) {
			if (pl_s_class.job == 0)
				max_level -= 40;
			if ((pl_s_class.job == 23) || (pl_s_class.upper == 1 && pl_s_class.type == 2)) //スパノビと転生職はJobレベルの最高が70
				max_level += 20;
			if (pl_sd->status.job_level == max_level) {
				clif_displaymessage(fd, msg_table[67]);
			} else if (level >= 1) {
				if (pl_sd->status.job_level + level > max_level)
					level = max_level - pl_sd->status.job_level;
				pl_sd->status.job_level += level;
				clif_updatestatus(pl_sd, SP_JOBLEVEL);
				clif_updatestatus(pl_sd, SP_NEXTJOBEXP);
				pl_sd->status.skill_point += level;
				clif_updatestatus(pl_sd, SP_SKILLPOINT);
				pc_calcstatus(pl_sd, 0);
				clif_misceffect(&pl_sd->bl, 1);
				clif_displaymessage(fd, msg_table[68]);
			} else if (level < 0 && sd->status.job_level + level > 0) {
				pl_sd->status.job_level += level;
				clif_updatestatus(pl_sd, SP_JOBLEVEL);
				clif_updatestatus(pl_sd, SP_NEXTJOBEXP);
				pc_calcstatus(pl_sd, 0);
				clif_displaymessage(fd, msg_table[69]);
				}
			}
		}

	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_kick(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	char character[100];
	
	if (!message || !*message)
		return -1;
	memset(character, '\0', sizeof character);
	if (sscanf(message, "%99[^\n]", character) < 1)
		return -1;
	if ((pl_sd = map_nick2sd(character)) == NULL)
		return -1;
	if (pc_isGM(sd) > pc_isGM(pl_sd))
		clif_GM_kick(sd, pl_sd, 1);
			else
		clif_GM_kickack(sd, 0);
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_kickall(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	int i = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] &&
			(pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			if (sd->status.account_id != pl_sd->status.account_id)
				clif_GM_kick(sd, pl_sd, 0);
			}
		}
	clif_GM_kick(sd, sd, 0);
	
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_allskill(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
			pc_allskillup(sd);
	clif_displaymessage(fd, msg_table[76]);
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_questskill(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int skill_id = 0;
	if (!message || !*message)
		return -1;
	skill_id = atoi(message);
	if (skill_get_inf2(skill_id) & 0x01) {
		pc_skill(sd, skill_id, 1, 0);
		clif_displaymessage(fd, msg_table[70]);
		}
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_lostskill(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int skill_id = 0;
	if (!message || !*message)
		return -1;
	skill_id = atoi(message);
	if (skill_id > 0 && skill_id < MAX_SKILL &&
		pc_checkskill(sd, skill_id) == 0)
		return -1;
	sd->status.skill[skill_id].lv = 0;
	sd->status.skill[skill_id].flag = 0;
				clif_skillinfoblock(sd);
	clif_displaymessage(fd, msg_table[71]);

	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_spiritball(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int number = 0;

	if (!message || !*message)
		return -1;
	number = atoi(message);
	if (number >= 0 && number <= 0x7FFF) {
		if (sd->spiritball > 0)
			pc_delspiritball(sd, sd->spiritball, 1);
		sd->spiritball = number;
		clif_spiritball(sd);
	}

	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_party(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char party[100];
	if (!message || !*message)
		return -1;
	if (sscanf(message, "%99[^\n]", party))
		party_create(sd, party);
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_guild(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char guild[100];
	int prev = 0;
	if (!message || !*message)
		return -1;
	if (sscanf(message, "%99[^\n]", guild) == 0)
		return -1;

	prev = battle_config.guild_emperium_check;
	battle_config.guild_emperium_check = 0;
	guild_create(sd, guild);
	battle_config.guild_emperium_check = prev;
	return 0;
}
	
/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_agitstart(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	if (agit_flag == 1) {
		clif_displaymessage(fd, msg_table[73]);
			return 1;
		}
	agit_flag = 1;
	guild_agit_start();
	clif_displaymessage(fd, msg_table[72]);
	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int
atcommand_agitend(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	if (agit_flag == 0) {
		clif_displaymessage(fd, msg_table[75]);
		return 1;
	}
	agit_flag = 0;
	guild_agit_end();
	clif_displaymessage(fd, msg_table[74]);
	return 0;
}

/*==========================================
 * @mapexitでマップサーバーを終了させる
 *------------------------------------------
 */
int
atcommand_mapexit(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	int i = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] &&
			(pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			if (sd->status.account_id != pl_sd->status.account_id)
				clif_GM_kick(sd, pl_sd, 0);
			}
		}
	clif_GM_kick(sd, sd, 0);
	
	exit(1);
}

