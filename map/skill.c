/* スキル関係 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "timer.h"

#include "skill.h"
#include "map.h"
#include "clif.h"
#include "pc.h"
#include "mob.h"
#include "battle.h"
#include "party.h"
#include "itemdb.h"

#define SKILLUNITTIMER_INVERVAL	100


/* スキル番号＝＞ステータス異常番号変換テーブル */
int SkillStatusChangeTable[]={	/* skill.hのenumのSC_***とあわせること */
/* 0- */
	-1,-1,-1,-1,-1,-1,
	SC_PROVOKE,			/* プロボック */
	-1, 1,-1,
/* 10- */
	SC_SIGHT,			/* サイト */
	-1,-1,-1,-1,
	SC_FREEZE,			/* フロストダイバー */
	SC_STONE,			/* ストーンカース */
	-1,-1,-1,
/* 20- */
	-1,-1,-1,-1,
	SC_RUWACH,			/* ルアフ */
	-1,-1,-1,-1,
	SC_INCREASEAGI,		/* 速度増加 */
/* 30- */
	SC_DECREASEAGI,		/* 速度減少 */
	-1,
	SC_SIGNUMCRUCIS,	/* シグナムクルシス */
	SC_ANGELUS,			/* エンジェラス */
	SC_BLESSING,		/* ブレッシング */
	-1,-1,-1,-1,-1,
/* 40- */
	-1,-1,-1,-1,-1,
	SC_CONCENTRATE,		/* 集中力向上 */
	-1,-1,-1,-1,
/* 50- */
	-1,
	SC_HIDDING,			/* ハイディング */
	-1,-1,-1,-1,-1,-1,-1,-1,
/* 60- */
	SC_TWOHANDQUICKEN,	/* 2HQ */
	-1,-1,-1,-1,-1,
	SC_IMPOSITIO,		/* インポシティオマヌス */
	SC_SUFFRAGIUM,		/* サフラギウム */
	SC_ASPERSIO,		/* アスペルシオ */
	SC_BENEDICTIO,		/* 聖体降福 */
/* 70- */
	-1,-1,-1,
	SC_KYRIE,			/* キリエエレイソン */
	SC_MAGNIFICAT,		/* マグニフィカート */
	SC_GLORIA,			/* グロリア */
	SC_DIVINA,			/* レックスディビーナ */
	-1,
	SC_AETERNA,			/* レックスエーテルナ */
	-1,
/* 80- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 90- */
	-1,-1,
	SC_QUAGMIRE,		/* クァグマイア */
	-1,-1,-1,-1,-1,-1,-1,
/* 100- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 110- */
	-1,
	SC_ADRENALINE,		/* アドレナリンラッシュ */
	SC_WEAPONPERFECTION,/* ウェポンパーフェクション */
	SC_OVERTHRUST,		/* オーバートラスト */
	SC_MAXIMIZEPOWER,	/* マキシマイズパワー */
	-1,-1,-1,-1,-1,
/* 120- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 130- */
	-1,-1,-1,-1,-1,
	SC_CLOAKING,		/* クローキング */
	SC_STAN,			/* ソニックブロー */
	-1,
	SC_ENCPOISON,		/* エンチャントポイズン */
	SC_POISONREACT,		/* ポイズンリアクト */
/* 140- */
	SC_POISON,			/* ベノムダスト */
	-1,-1,
	SC_TRICKDEAD,		/* 死んだふり */
	-1,-1,-1,-1,-1,-1,
/* 150- */
	-1,-1,-1,-1,-1,
	SC_LOUD,			/* ラウドボイス */
	-1,
	SC_ENERGYCOAT,		/* エナジーコート */
	-1,-1,
/* 160- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 210- */
	-1,-1,-1,-1,-1,
	SC_STRIPWEAPON,
	SC_STRIPSHIELD,
	SC_STRIPARMOR,
	SC_STRIPHELM,
	-1,
/* 220- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 230- */
	-1,-1,-1,-1,
	SC_CP_WEAPON,
	SC_CP_SHIELD,
	SC_CP_ARMOR,
	SC_CP_HELM,
	-1,-1,
/* 240- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,
	SC_AUTOGUARD,
/* 250- */
	-1,
	SC_REFLECTSHIELD,
	-1,-1,-1,
	SC_DEVOTION,
	SC_PROVIDENCE,
	SC_DEFENDER,
	SC_SPEARSQUICKEN,
	-1,
/* 260- */
	-1,-1,-1,-1,-1,-1,-1,-1,
	SC_STEELBODY,
	-1,
/* 270- */
	SC_EXPLOSIONSPIRITS,-1,-1,-1,-1,
	SC_CASTCANCEL,
	-1,
	SC_SPELLBREAKER,
	SC_FREECAST,
	-1,
/* 280- */
	SC_FLAMELAUNCHER,
	SC_FROSTWEAPON,
	SC_LIGHTNINGLOADER,
	SC_SEISMICWEAPON,
	-1,
	SC_VOLCANO,
	SC_DELUGE,
	-1,-1,-1,
/* 290- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 300- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,
	SC_DRUMBATTLE,
/* 310- */
	-1,-1,-1,
	SC_SIEGFRIED,
	-1,-1,-1,-1,-1,
	SC_WHISTLE,
/* 320- */
	SC_ASSNCROS,
	SC_POEMBRAGI,
	SC_APPLEIDUN,
	-1,-1,-1,-1,-1,-1,
	SC_FORTUNE,
/* 330- */
	SC_SERVICE4U,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,
};

/* スキルデータベース */
struct skill_db skill_db[MAX_SKILL_DB];

/* アイテム作成データベース */
struct skill_produce_db skill_produce_db[MAX_SKILL_PRODUCE_DB];

int skill_get_range( int id ){ return skill_db[id].range; }
int	skill_get_hit( int id ){ return skill_db[id].hit; }
int	skill_get_inf( int id ){ return skill_db[id].inf; }
int	skill_get_pl( int id ){ return skill_db[id].pl; }
int	skill_get_nk( int id ){ return skill_db[id].nk; }
int	skill_get_max( int id ){ return skill_db[id].max; }
int	skill_get_sp( int id ,int lv ){ return skill_db[id].sp[lv-1]; }
int	skill_get_num( int id ,int lv ){ return skill_db[id].num[lv-1]; }
int	skill_get_cast( int id ,int lv ){ return skill_db[id].cast[lv-1]; }
int	skill_get_delay( int id ,int lv ){ return skill_db[id].delay[lv-1]; }
int	skill_get_inf2( int id ){ return skill_db[id].inf2; }


/* プロトタイプ */
struct skill_unit_group *skill_unitsetting( struct block_list *src, int skillid,int skilllv,int x,int y,int flag);

/* スキルユニットIDを返す（これもデータベースに入れたいな） */
int skill_get_unit_id(int id,int flag)
{
	switch(id){
	case MG_SAFETYWALL:		return 0x7e;				/* セイフティウォール */
	case MG_FIREWALL:		return 0x7f;				/* ファイアーウォール */
	case AL_WARP:			return (flag==0)?0x81:0x80;	/* ワープポータル */
	case PR_BENEDICTIO:		return 0x82;				/* 聖体降福 */
	case PR_SANCTUARY:		return 0x83;				/* サンクチュアリ */
	case PR_MAGNUS:			return 0x84;				/* マグヌスエクソシズム */
	case AL_PNEUMA:			return 0x85;				/* ニューマ */
	case WZ_METEOR:			return 0x86;				/* メテオストーム */
	case WZ_VERMILION:		return 0x86;				/* ロードオブヴァーミリオン */
	case WZ_STORMGUST:		return 0x86;				/* ストームガスト(とりあえずLoVと同じで処理) */
	case CR_GRANDCROSS:		return 0x86;				/* グランドクロス */
	case WZ_FIREPILLAR:		return (flag==0)?0x87:0x88;	/* ファイアーピラー */
	case HT_TALKIEBOX:		return (flag==0)?0x99:0x8c;	/* トーキーボックス */
	case WZ_ICEWALL:		return 0x8d;				/* アイスウォール */
	case WZ_QUAGMIRE:		return 0x8e;				/* クァグマイア */
	case HT_BLASTMINE:		return 0x8f;				/* ブラストマイン */
	case HT_SKIDTRAP:		return 0x90;				/* スキッドトラップ */
	case HT_ANKLESNARE:		return 0x91;				/* アンクルスネア */
	case AS_VENOMDUST:		return 0x92;				/* ベノムダスト */
	case HT_LANDMINE:		return 0x93;				/* ランドマイン */
	case HT_SHOCKWAVE:		return 0x94;				/* ショックウェーブトラップ */
	case HT_SANDMAN:		return 0x95;				/* サンドマン */
	case HT_FLASHER:		return 0x96;				/* フラッシャー */
	case HT_FREEZINGTRAP:	return 0x97;				/* フリージングトラップ */
	case HT_CLAYMORETRAP:	return 0x98;				/* クレイモアートラップ */
	case SA_DELUGE:			return 0x9b;				/* デリュージ */
	case BD_LULLABY:		return 0x9e;				/* 子守歌 */
	case BD_RICHMANKIM:		return 0x9f;				/* ニヨルドの宴 */
	case BD_ETERNALCHAOS:	return 0xa0;				/* 永遠の混沌 */
	case BD_DRUMBATTLEFIELD:return 0xa1;					/* 戦太鼓の響き */
	case BD_RINGNIBELUNGEN:	return 0xa2;				/* ニーベルングの指輪 */
	case BD_ROKISWEIL:		return 0xa3;				/* ロキの叫び */
	case BD_INTOABYSS:		return 0xa4;				/* 深淵の中に */
	case BD_SIEGFRIED:		return 0xa5;				/* 不死身のジークフリード */
	case BA_DISSONANCE:		return 0xa6;				/* 不協和音 */
	case BA_WHISTLE:		return 0xa7;				/* 口笛 */
	case BA_ASSASSINCROSS:	return 0xa8;				/* 夕陽のアサシンクロス */
	case BA_POEMBRAGI:		return 0xa9;				/* ブラギの詩 */
	case BA_APPLEIDUN:		return 0xaa;				/* イドゥンの林檎 */
	case DC_UGLYDANCE:		return 0xab;				/* 自分勝手なダンス */
	case DC_HUMMING:		return 0xac;				/* ハミング */
	case DC_DONTFORGETME:	return 0xad;				/* 私を忘れないで… */
	case DC_FORTUNEKISS:	return 0xae;				/* 幸運のキス */
	case DC_SERVICEFORYOU:	return 0xaf;				/* サービスフォーユー */
	case RG_GRAFFITI:		return 0xb0;				/* グラフィティ */
	}
	return 0;
	/*
	0x89,0x8a,0x8b 表示無し
	0x9a 炎属性の詠唱みたいなエフェクト
	0x9b 水属性の詠唱みたいなエフェクト
	0x9c 風属性の詠唱みたいなエフェクト
	0x9d 白い小さなエフェクト
	*/
}



int skill_check_condition( struct map_session_data *sd );

int skill_castend_damage_id( struct block_list* src, struct block_list *bl,int skillid,int skilllv,unsigned int tick,int flag );


/*==========================================
 * スキル追加効果
 *------------------------------------------
 */
int skill_additional_effect( struct block_list* src, struct block_list *bl,int skillid,int skilllv,unsigned int tick)
{
	struct map_session_data *sd=NULL;
	struct map_session_data *sd2=NULL;
	int skill;
	int rate;
	
	if(src->type==BL_PC)
		sd=(struct map_session_data *)src;
	if(bl->type==BL_PC)
		sd2=(struct map_session_data *)bl;

	switch(skillid){
	case 0:					/* 通常攻撃 */
		/* 自動鷹 */
		if( sd && pc_isfalcon(sd) && (skill=pc_checkskill(sd,HT_BLITZBEAT))>0 &&
			rand()%1000 <= sd->paramc[5]*10/3+1 ) {
			int lv=(sd->status.job_level+9)/10;
			skill_castend_damage_id(src,bl,HT_BLITZBEAT,(skill<lv)?skill:lv,tick,0xf00000);
		}
		// スナッチャー
		if(sd && (skill=pc_checkskill(sd,RG_SNATCHER)) > 0)
			if((skill*15 + 55) + pc_checkskill(sd,TF_STEAL)*10 > rand()%1000)
				if(pc_steal_item(sd,bl))
					clif_skill_nodamage(src,bl,TF_STEAL,0,1);
		break;

	case SM_BASH:			/* バッシュ（急所攻撃） */
		if( sd && (skill=pc_checkskill(sd,SM_FATALBLOW))>0 ){
			if( rand()%100 < 6*(skilllv-5) )
				skill_status_change_start(bl,SC_STAN,skilllv,5000);
		}
		break;

	case TF_POISON:			/* インベナム */
		if(battle_get_elem_type(bl)!=9 && rand()%100< 2*skilllv+10 )
			skill_status_change_start(bl,SC_POISON,skilllv,0);
		break;

	case AS_SONICBLOW:		/* ソニックブロー */
		if( rand()%100 < 2*skilllv+10 )
			skill_status_change_start(bl,SC_STAN,skilllv,5000);
		break;

	case MG_FROSTDIVER:		/* フロストダイバー */
	case WZ_FROSTNOVA:		/* フロストノヴァ */
	case WZ_STORMGUST:		/* ストームガスト */
	case HT_FREEZINGTRAP:	/* フリージングトラップ */
	case BA_FROSTJOKE:		/* 寒いジョーク */
/*
		rate=battle_get_lv(src)/2 +battle_get_int(src)/3+ skilllv*2 +15 -battle_get_mdef(bl);
		if(rate>95)rate=95;
*/
		rate=skilllv*3+35;
		if( rand()%100 < rate )
			skill_status_change_start(bl,SC_FREEZE,skilllv,0);
		break;

	case HT_LANDMINE:		/* ランドマイン */
		if( rand()%100 < 5*skilllv+30 ){
			skill_status_change_start(bl,SC_STAN,skilllv,skilllv*500+1000);
		}
		break;

	case HT_ANKLESNARE:		/* アンクルスネア */
		{
			int sec=(battle_get_mode(bl)&0x20)?1000:5000;
			skill_status_change_start(bl,SC_ANKLE,skilllv,skilllv*sec);
		}
		break;

	case HT_SANDMAN:		/* サンドマン */
		if( rand()%100 < 5*skilllv+30 ){
			skill_status_change_start(bl,SC_SLEEP,skilllv,30000);
		}
		break;

	case TF_SPRINKLESAND:	/* 砂まき */
		if( rand()%100 < 15 )
			skill_status_change_start(bl,SC_BLIND,1,0);
		break;

	case TF_THROWSTONE:		/* 石投げ */
		if( rand()%100 < 5 )
			skill_status_change_start(bl,SC_STAN,1,3000);
		break;

	case CR_HOLYCROSS:		/* 砂まき */
		if( rand()%100 < 3*skilllv )
			skill_status_change_start(bl,SC_BLIND,1,0);
		break;

#if 0
	case BA_FROSTJOKE:		/* 寒いジョーク */
		if( rand()%100 < 15+5*skilllv )
			skill_status_change_start(bl,SC_FREEZE,1,0);
		break;
#endif

	/* MOBの追加効果付きスキル */
	case NPC_POISON:
	case NPC_BLINDATTACK:
	case NPC_SILENCEATTACK:
	case NPC_STUNATTACK:
	case NPC_PETRIFYATTACK:
	case NPC_CURSEATTACK:
	case NPC_SLEEPATTACK:
		{
			const int sc[]={
				SC_POISON,	SC_BLIND,	SC_SILENCE,	SC_STAN,
				SC_STONE,	SC_CURSE,	SC_SLEEP	};
			const int sc2[]={
				6000,		0,			6000,			1000,
				1000,		0,			6000	};
			skill_status_change_start(bl,
				sc[skillid-NPC_POISON],skilllv,sc2[skillid-NPC_POISON]*skilllv);
		}
		break;
	}

	if(sd){	/* カードによる追加効果 */
		int i;
		for(i=SC_STONE;i<=SC_BLIND;i++){
			if( sd->addeff[i-SC_STONE]>0 && rand()%100<sd->addeff[i-SC_STONE] ){
				printf("skill_addeff: cardによる異常発動 %d %d\n",i,sd->addeff[i-SC_STONE]);
				skill_status_change_start(bl,i,1,5);
			}
		}
	}

	return 0;
}

/*=========================================================================
 スキル攻撃吹き飛ばし処理
-------------------------------------------------------------------------*/
int skill_blown( struct block_list *src, struct block_list *target,int count)
{
	static const int dirx[8]={0,-1,-1,-1,0,1,1,1};
	static const int diry[8]={1,1,0,-1,-1,-1,0,1};
	int dx=0,dy=0,nx,ny;
	int x=target->x,y=target->y;
	int ret;
	int moveblock;
	struct map_session_data *sd=NULL;
	struct mob_data *md=NULL;
	
	if(target->type==BL_PC)
		sd=(struct map_session_data *)target;
	else if(target->type==BL_MOB)
		md=(struct mob_data *)target;

	if(!(count&0x10000 && (sd||md))){	/* 指定なしなら位置関係から方向を求める */
		dx=target->x-src->x; dx=(dx>0)?1:((dx<0)?-1: 0);
		dy=target->y-src->y; dy=(dy>0)?1:((dy<0)?-1: 0);
	}
	if(dx==0 && dy==0){
		int dir=(sd)?sd->dir:md->dir;
		if(dir>=0 && dir<8){
			dx=-dirx[dir];
			dy=-diry[dir];
		}
	}

	ret=path_blownpos(target->m,x,y,dx,dy,count&0xff);
	nx=ret>>16;
	ny=ret&0xffff;
	moveblock=( x/BLOCK_SIZE != nx/BLOCK_SIZE || y/BLOCK_SIZE != ny/BLOCK_SIZE);

	if(sd){
		sd->to_x=nx;
		sd->to_y=ny;
	}else if(md){
		md->to_x=nx;
		md->to_y=ny;
	}
	if(count&0x20000){
		if(sd){
			clif_walkok(sd);
			clif_movechar(sd);
		}
		if(md)
			clif_fixmobpos(md);
	}

	if(sd)	/* 画面外に出たので消去 */
		map_foreachinmovearea(clif_pcoutsight,
			target->m,x-AREA_SIZE,y-AREA_SIZE,
			x+AREA_SIZE,y+AREA_SIZE,dx,dy,0,sd);
	if(md)
		map_foreachinmovearea(clif_moboutsight,
			target->m,x-AREA_SIZE,y-AREA_SIZE,
			x+AREA_SIZE,y+AREA_SIZE,dx,dy,BL_PC,md);

	if(moveblock) map_delblock(target);
	target->x=nx;
	target->y=ny;
	if(moveblock) map_addblock(target);


	skill_unit_move(target,gettick(),count+2);	/* スキルユニットの判定 */

	if(sd)	/* 画面内に入ってきたので表示 */
		map_foreachinmovearea(clif_pcinsight,
			target->m,nx-AREA_SIZE,ny-AREA_SIZE,
			nx+AREA_SIZE,ny+AREA_SIZE,-dx,-dy,0,sd);
	if(md)
		map_foreachinmovearea(clif_mobinsight,
			target->m,nx-AREA_SIZE,ny-AREA_SIZE,
			nx+AREA_SIZE,ny+AREA_SIZE,-dx,-dy,BL_PC,md);
	return 0;
}


/*
 * =========================================================================
 * スキル攻撃効果処理まとめ
 * flagの説明。16進図
 * 	00XRTTff
 *  ff	= magicで計算に渡される）
 *	TT	= パケットのtype部分(0でデフォルト）
 *  X   = パケットのスキルLv
 *  R	= 予約（skill_area_subで使用する）
 *-------------------------------------------------------------------------
 */
int skill_attack( int attack_type, struct block_list* src, struct block_list *dsrc,
	 struct block_list *bl,int skillid,int skilllv,unsigned int tick,int flag )
{
	struct Damage dmg;
	int type=-1,lv=(flag>>20)&0xf;
	dmg=battle_calc_attack(attack_type, src,bl, skillid,skilllv, flag&0xff );
	
	if(lv==15)lv=-1;

	if( flag&0xff00 )
		type=(flag&0xff00)>>8;

	if(dmg.damage + dmg.damage2 <= 0)
		dmg.blewcount = 0;

	if( dmg.blewcount ){	/* 吹き飛ばし処理とそのパケット */
		skill_blown(dsrc,bl,dmg.blewcount);
		clif_skill_damage2(dsrc,bl,tick,dmg.amotion,dmg.dmotion,
			dmg.damage, dmg.div_, skillid, (lv!=0)?lv:skilllv, type );
	} else			/* スキルのダメージパケット */
		clif_skill_damage(dsrc,bl,tick,dmg.amotion,dmg.dmotion,
			dmg.damage, dmg.div_, skillid, (lv!=0)?lv:skilllv, type );

	/* 実際にダメージ処理を行う */
	battle_damage(src,bl,(dmg.damage+dmg.damage2));
	/* ダメージがあるなら追加効果判定 */
	if((dmg.damage+dmg.damage2)>0)
		skill_additional_effect(src,bl,skillid,skilllv,tick);

	if(bl->type==BL_MOB && src!=bl)	/* スキル使用条件のMOBスキル */
		mobskill_use((struct mob_data *)bl,tick,MSC_SKILLUSED|(skillid<<16));

	return (dmg.damage+dmg.damage2);	/* 与ダメを返す */
}


/*==========================================
 * スキル範囲攻撃用(map_foreachinareaから呼ばれる)
 * flagについて：16進図を確認
 * MSB <- 00fTffff ->LSB
 *	T	=ターゲット選択用(BCT_*)
 *  ffff=自由に使用可能
 *  0	=予約。0に固定
 *------------------------------------------
 */
static int skill_area_temp[8];	/* 一時変数。必要なら使う。 */
typedef int (*SkillFunc)(struct block_list *,struct block_list *,int,int,unsigned int,int);
int skill_area_sub( struct block_list *bl,va_list ap )
{
	struct block_list *src;
	int skill_id,skill_lv,flag;
	unsigned int tick;
	SkillFunc func;
	
	if(bl->type!=BL_PC && bl->type!=BL_MOB)
		return 0;

	src=va_arg(ap,struct block_list *);
	skill_id=va_arg(ap,int);
	skill_lv=va_arg(ap,int);
	tick=va_arg(ap,unsigned int);
	flag=va_arg(ap,int);
	func=va_arg(ap,SkillFunc);

	if(battle_check_target(src,bl,flag))
		func(src,bl,skill_id,skill_lv,tick,flag);
	return 0;
}
/*=========================================================================
 * 範囲スキル使用処理小分けここから
 */
/* 対象の数をカウントする。（skill_area_temp[0]を初期化しておくこと） */
int skill_area_sub_count(struct block_list *src,struct block_list *target,int skillid,int skilllv,unsigned int tick,int flag)
{
	skill_area_temp[0]++;
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
static int skill_timerskill(int tid, unsigned int tick, int id,int data )
{
	struct map_session_data* sd=NULL/*,*target_sd=NULL*/;
	struct block_list *bl;
	struct skill_timerskill skl;
	int i;
	
	if( (sd=map_id2sd(id))==NULL )
		return 0;

	sd->skilltimerskill[0].timer = -1;
	memcpy(&skl, &sd->skilltimerskill[0],sizeof(struct skill_timerskill));
	for(i=1;i<sd->skill_timer_count;i++)
		memcpy(&sd->skilltimerskill[i-1],&sd->skilltimerskill[i],sizeof(struct skill_timerskill));
	sd->skill_timer_count--;
	if(sd->skill_timer_count < 0)
		sd->skill_timer_count = 0;

	if(skl.target_id) {
		bl=map_id2bl(skl.target_id);
		if(bl==NULL || bl->prev == NULL)
			return 0;
		if(sd->bl.m != bl->m || pc_isdead(sd))
			return 0;

		skill_attack(skl.type,&sd->bl,&sd->bl,bl,skl.skill_id,skl.skill_lv,tick,data);
	}
	else {
		if(sd->bl.m != skl.map)
			return 0;

		switch(skl.skill_id) {
			case WZ_METEOR:
				clif_skill_poseffect(&sd->bl,skl.skill_id,skl.skill_lv,skl.x,skl.y,tick);
				skill_unitsetting(&sd->bl,skl.skill_id,skl.skill_lv,skl.x,skl.y,0);
				break;
		}
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int skill_addtimerskill(struct map_session_data *sd,int tick,int interval,
	int target,int x,int y,int skill_id,int skill_lv,int type,int flag)
{
	if(sd->skill_timer_count < MAX_SKILLTIMERSKILL) {
		sd->skilltimerskill[sd->skill_timer_count].timer = add_timer(tick + interval, skill_timerskill, sd->bl.id, flag );
		sd->skilltimerskill[sd->skill_timer_count].src_id = sd->bl.id;
		sd->skilltimerskill[sd->skill_timer_count].target_id = target;
		sd->skilltimerskill[sd->skill_timer_count].skill_id = skill_id;
		sd->skilltimerskill[sd->skill_timer_count].skill_lv = skill_lv;
		sd->skilltimerskill[sd->skill_timer_count].map = sd->bl.m;
		sd->skilltimerskill[sd->skill_timer_count].x = x;
		sd->skilltimerskill[sd->skill_timer_count].y = y;
		sd->skilltimerskill[sd->skill_timer_count].type = type;
		sd->skill_timer_count++;
		return 0;
	}

	return 1;
}

/*==========================================
 *
 *------------------------------------------
 */
int skill_cleartimerskill(struct map_session_data *sd)
{
	int i;
	for(i=0;i<sd->skill_timer_count;i++) {
		if(sd->skilltimerskill[i].timer != -1) {
			delete_timer(sd->skilltimerskill[i].timer, skill_timerskill);
			sd->skilltimerskill[i].timer = -1;
		}
	}
	sd->skill_timer_count = 0;

	return 0;
}

/* 範囲スキル使用処理小分けここまで
 * -------------------------------------------------------------------------
 */


/*==========================================
 * スキル使用（詠唱完了、ID指定攻撃系）
 * （スパゲッティに向けて１歩前進！(ダメポ)）
 *------------------------------------------
 */
int skill_castend_damage_id( struct block_list* src, struct block_list *bl,int skillid,int skilllv,unsigned int tick,int flag )
{
	struct map_session_data *sd=NULL;
	int i;
	
	if(src->type==BL_PC)
		sd=(struct map_session_data *)src;

	switch(skillid)
	{
	/* 武器攻撃系スキル */
	case SM_BASH:			/* バッシュ */
	case MC_MAMMONITE:		/* メマーナイト */
	case AC_DOUBLE:			/* ダブルストレイフィング */
	case AS_SONICBLOW:		/* ソニックブロー */
	case AS_GRIMTOOTH:		/* グリムトゥース */
	case KN_PIERCE:			/* ピアース */
	case KN_SPEARBOOMERANG:	/* スピアブーメラン */
	case TF_POISON:			/* インベナム */
	case TF_SPRINKLESAND:	/* 砂まき */
	case AC_CHARGEARROW:	/* チャージアロー */
	case KN_SPEARSTAB:		/* スピアスタブ */
	case RG_INTIMIDATE:		/* インティミデイト */
	case BA_MUSICALSTRIKE:	/* ミュージカルストライク */
	case DC_THROWARROW:		/* 矢撃ち */
	case BA_DISSONANCE:		/* 不協和音 */
	case MO_INVESTIGATE:	/* 発勁 */
	case MO_EXTREMITYFIST:	/* 阿修羅覇鳳拳 */
	case MO_CHAINCOMBO:		/* 連打掌 */
	case MO_COMBOFINISH:	/* 猛龍拳 */
	case CR_HOLYCROSS:		/* ホーリークロス */
	/* 以下MOB専用 */
	/* 単体攻撃、SP減少攻撃、遠距離攻撃、防御無視攻撃、多段攻撃 */
	case NPC_PIERCINGATT:
	case NPC_MENTALBREAKER:
	case NPC_RANGEATTACK:
	case NPC_CRITICALSLASH:
	case NPC_COMBOATTACK:
	/* 必中攻撃、毒攻撃、暗黒攻撃、沈黙攻撃、スタン攻撃 */
	case NPC_GUIDEDATTACK:
	case NPC_POISON:
	case NPC_BLINDATTACK:
	case NPC_SILENCEATTACK:
	case NPC_STUNATTACK:
	/* 石化攻撃、呪い攻撃、睡眠攻撃、ランダムATK攻撃 */
	case NPC_PETRIFYATTACK:
	case NPC_CURSEATTACK:
	case NPC_SLEEPATTACK:
	case NPC_RANDOMATTACK:
	/* 水属性攻撃、地属性攻撃、火属性攻撃、風属性攻撃 */
	case NPC_WATERATTACK:
	case NPC_GROUNDATTACK:
	case NPC_FIREATTACK:
	case NPC_WINDATTACK:
	/* 毒属性攻撃、聖属性攻撃、闇属性攻撃、念属性攻撃、SP減少攻撃 */
	case NPC_POISONATTACK:
	case NPC_HOLYATTACK:
	case NPC_DARKNESSATTACK:
	case NPC_TELEKINESISATTACK:
	case NPC_LICK:
		if(skillid == MO_EXTREMITYFIST)
			skill_status_change_end(src, SC_EXPLOSIONSPIRITS, -1);
		skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
		break;
	case MO_FINGEROFFENSIVE:	/* 指弾 */
		skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
		for(i=1;i<sd->spiritball_old;i++)
			skill_addtimerskill(sd,tick,i*250,bl->id,0,0,skillid,skilllv,BF_WEAPON,flag);
		sd->skillcanmove_tick = tick + (sd->spiritball_old-1)*250;

		break;

	/* 武器系範囲攻撃スキル */
	case AC_SHOWER:			/* アローシャワー */
	case SM_MAGNUM:			/* マグナムブレイク */
	case KN_BOWLINGBASH:	/* ボウリングバッシュ */
	case MC_CARTREVOLUTION:	/* カートレヴォリューション */
	case NPC_SPLASHATTACK:	/* スプラッシュアタック */
		if(flag&3){
			/* 個別にダメージを与える */
			if(bl->id!=skill_area_temp[1]){
				int dist=0;
				if(skillid==SM_MAGNUM){	/* マグナムブレイクなら中心からの距離を計算 */
					int dx=abs( bl->x - skill_area_temp[2] );
					int dy=abs( bl->y - skill_area_temp[3] );
					dist=((dx>dy)?dx:dy);
				}
				skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,
					0x0500|dist  );
			}
		}else{
			int ar=1;
			int x=bl->x,y=bl->y;
			if( skillid==SM_MAGNUM ){
				x=src->x;
				y=src->y;
			}else if(skillid==NPC_SPLASHATTACK)	/* スプラッシュアタックは範囲7*7 */
				ar=3;
			else if(skillid==KN_BOWLINGBASH){/*ボウリングバッシュを仮実装してみる（吹き飛ばしはここでやる） */
				int i;	/* 他人から聞いた動きなので間違ってる可能性大＆効率が悪いっす＞＜ */
				for(i=0;i<4;i++){
					skill_blown(src,bl,1|0x20000);
					skill_area_temp[0]=0;
					map_foreachinarea(skill_area_sub,
						bl->m,bl->x-1,bl->y-1,bl->x+1,bl->y+1,0,
						src,skillid,skilllv,tick, flag|BCT_ENEMY ,
						skill_area_sub_count);
					if(skill_area_temp[0]>1)break;
				}
			/*	if(i==4)break; */
				x=bl->x;y=bl->y;
			}
			skill_area_temp[1]=bl->id;
			skill_area_temp[2]=x;
			skill_area_temp[3]=y;
			/* まずターゲットに攻撃を加える */
			skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,0);
			/* その後ターゲット以外の範囲内の敵全体に処理を行う */
			map_foreachinarea(skill_area_sub,
				bl->m,x-ar,y-ar,x+ar,y+ar,0,
				src,skillid,skilllv,tick, flag|BCT_ENEMY|1,
				skill_castend_damage_id);
		}
		break;

	/* 魔法系スキル */
	case MG_SOULSTRIKE:			/* ソウルストライク */
	case MG_COLDBOLT:			/* コールドボルト */
	case MG_FIREBOLT:			/* ファイアーボルト */
	case MG_LIGHTNINGBOLT:		/* ライトニングボルト */
	case WZ_EARTHSPIKE:			/* アーススパイク */
	case AL_HEAL:				/* ヒール */
	case AL_HOLYLIGHT:			/* ホーリーライト */
	case ALL_RESURRECTION:		/* リザレクション */
	case PR_TURNUNDEAD:			/* ターンアンデッド */
	case MG_FROSTDIVER:			/* フロストダイバー */
	case WZ_JUPITEL:			/* ユピテルサンダー */
	case NPC_MAGICALATTACK:		/* MOB:魔法打撃攻撃 */
		skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,flag);
		break;

	case WZ_WATERBALL:			/* ウォーターボール */
		skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,flag);
		if(skilllv>1)
			skill_status_change_start(src,SC_WATERBALL,skilllv,bl->id);
		break;

	case PR_BENEDICTIO:			/* 聖体降福 */
		if(battle_get_race(bl)==1 || battle_get_race(bl)==6)
			skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,flag);
		break;

	/* 魔法系範囲攻撃スキル */
	case MG_NAPALMBEAT:			/* ナパームビート */
	case MG_FIREBALL:			/* ファイヤーボール */
	case MG_THUNDERSTORM:		/* サンダーストーム(flag=2のみ) */
	case WZ_HEAVENDRIVE:		/* ヘブンズドライブ(flag=2のみ) */
		if(flag&3){
			/* 個別にダメージを与える */
			if(bl->id!=skill_area_temp[1]){
				if(skillid==MG_FIREBALL){	/* ファイヤーボールなら中心からの距離を計算 */
					int dx=abs( bl->x - skill_area_temp[2] );
					int dy=abs( bl->y - skill_area_temp[3] );
					skill_area_temp[0]=((dx>dy)?dx:dy);
				}
				skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,
					skill_area_temp[0]| ((flag&2)?0:0x0500)  );
			}
		}else{
			int ar=(skillid==MG_NAPALMBEAT)?1:2;
			skill_area_temp[1]=bl->id;
			if(skillid==MG_NAPALMBEAT){	/* ナパームでは先に数える */
				skill_area_temp[0]=0;
				map_foreachinarea(skill_area_sub,
					bl->m,bl->x-1,bl->y-1,bl->x+1,bl->y+1,0,
					src,skillid,skilllv,tick, flag|BCT_ENEMY ,
					skill_area_sub_count);
			}else{
				skill_area_temp[2]=bl->x;
				skill_area_temp[3]=bl->y;
			}
			/* まずターゲットに攻撃を加える */
			skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,
					skill_area_temp[0] );
			/* その後ターゲット以外の範囲内の敵全体に処理を行う */
			map_foreachinarea(skill_area_sub,
				bl->m,bl->x-ar,bl->y-ar,bl->x+ar,bl->y+ar,0,
				src,skillid,skilllv,tick, flag|BCT_ENEMY|1,
				skill_castend_damage_id);
			break;
		}
		break;

	case WZ_FROSTNOVA:			/* フロストノヴァ */
		/* 個別にダメージを与える */
		if(bl->id!=skill_area_temp[1] &&
			(bl->x!=skill_area_temp[2] || bl->y!=skill_area_temp[3]) )
			skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,0 );
		break;


	/* その他 */
	case HT_BLITZBEAT:			/* ブリッツビート */
		if(flag&1){
			/* 個別にダメージを与える */
			if(bl->id!=skill_area_temp[1])
				skill_attack(BF_MISC,src,src,bl,skillid,skilllv,tick,flag&0xf00000 );
		}else{
			skill_area_temp[1]=bl->id;
			/* まずターゲットに攻撃を加える */
			skill_attack(BF_MISC,src,src,bl,skillid,skilllv,tick,flag&0xf00000 );
			/* その後ターゲット以外の範囲内の敵全体に処理を行う */
			map_foreachinarea(skill_area_sub,
				bl->m,bl->x-1,bl->y-1,bl->x+1,bl->y+1,0,
				src,skillid,skilllv,tick, flag|BCT_ENEMY|1,
				skill_castend_damage_id);
			break;
		}
		break;

	case TF_THROWSTONE:			/* 石投げ */
	case NPC_SMOKING:			/* スモーキング */
		skill_attack(BF_MISC,src,src,bl,skillid,skilllv,tick,0 );
		break;

	case NPC_SELFDESTRUCTION:	/* 自爆 */
		if(flag&1){
			/* 個別にダメージを与える */
			if(src->type==BL_MOB){
				struct mob_data *md=(struct mob_data *)src;
				md->hp=skill_area_temp[0];
				if(bl->id!=skill_area_temp[1])
					skill_attack(BF_MISC,src,src,bl,skillid,skilllv,tick,flag );
				md->hp=1;
			}
		}else{
			skill_area_temp[0]=battle_get_hp(src);
			skill_area_temp[1]=bl->id;
			/* まずターゲットに攻撃を加える */
			skill_attack(BF_MISC,src,src,bl,skillid,skilllv,tick,flag );
			/* その後ターゲット以外の範囲内の敵全体に処理を行う */
			map_foreachinarea(skill_area_sub,
				bl->m,bl->x-5,bl->y-5,bl->x+5,bl->y+5,0,
				src,skillid,skilllv,tick, flag|BCT_ALL|1,
				skill_castend_damage_id);
			battle_damage(src,src,1);
			break;
		}
		break;

	/* HP吸収/HP吸収魔法 */
	case NPC_BLOODDRAIN:
	case NPC_ENERGYDRAIN:
		battle_heal(NULL,src,
			skill_attack( (skillid==NPC_BLOODDRAIN)?BF_WEAPON:BF_MAGIC,
				src,src,bl,skillid,skilllv,tick,flag), 0 );
		break;
	}

	return 0;
}



/*==========================================
 * スキル使用（詠唱完了、ID指定支援系）
 *------------------------------------------
 */
int skill_castend_nodamage_id( struct block_list *src, struct block_list *bl,int skillid,int skilllv,unsigned int tick,int flag )
{
	struct map_session_data *sd=NULL;
	struct map_session_data *dstsd=NULL;
	struct mob_data *md=NULL;
	int i;

	if(src->type==BL_PC)
		sd=(struct map_session_data *)src;
	else if(src->type==BL_MOB)
		md=(struct mob_data *)src;

	if(bl->type==BL_PC)
		dstsd=(struct map_session_data *)bl; 

	switch(skillid)
	{
#if 0
	case SM_RECOVERY:			/* SP回復向上 */
		{
        int a=0,i,bonus,c=0;
		bonus=100+(skilllv*10);
        if(skilllv==1){a=500 > 550;}
        else if(skilllv==2){a=500 > 551;}
        else if(skilllv==3){a=500 > 551;}
        else if(skilllv==4){a=500 > 551;}
		else if(skilllv==5){a=500 > 551;}
		else if(skilllv==6){a=500 > 551;}
		else if(skilllv==7){a=500 > 551;}
		else if(skilllv==8){a=500 > 551;}
		else if(skilllv==9){a=500 > 551;}
        else if(skilllv==10){a=500 > 551;}
        if(a>0){
           for(i=0;i<MAX_INVENTORY;i++){
          if(sd->status.inventory[i].nameid==a){
                 pc_delitem(sd,i,1,0);
                 c=1;
                 if(skilllv==1){a=rand()%15+30;}
                 else if(skilllv==2){a=rand()%20+70;}
                 else if(skilllv==3){a=rand()%60+175;}
                 else if(skilllv==4){a=rand()%80+350;}
                 else if(skilllv==5){a=rand()%80+350;}
				 else if(skilllv==6){a=rand()%80+350;}
				 else if(skilllv==7){a=rand()%80+350;}
				 else if(skilllv==8){a=rand()%80+350;}
				 else if(skilllv==9){a=rand()%80+350;}
				 else if(skilllv==10){a=rand()%80+350;}
                 if(a>1){
                    clif_skill_nodamage(src,bl,skillid,(bonus*a)/100,1);
                    battle_heal(NULL,bl,(bonus*a)/100,0);
                 }
          }
       }
       if(c==0){clif_displaymessage(sd->fd,"Potions.");}
#endif

	case AL_HEAL:				/* ヒール */
		{
			int heal=skill_calc_heal( src, skilllv );
			if( bl->type==BL_PC &&
				pc_check_equip_dcard((struct map_session_data *)bl,4128) )
				heal=0;	/* 黄金蟲カード（ヒール量０） */
			clif_skill_nodamage(src,bl,skillid,heal,1);
			battle_heal(NULL,bl,heal,0);
		}
		break;

	case ALL_RESURRECTION:		/* リザレクション */
		if(bl->type==BL_PC){
			int per=0;
			struct map_session_data *tsd=(struct map_session_data*)bl;

			if( (map[bl->m].flag&MF_PVP) && tsd->pvp_point<=0 )
				break;			/* PVPで復活不可能状態 */

			if(pc_isdead(tsd)){	/* 死亡判定 */
				clif_skill_nodamage(src,bl,skillid,0,1);
				switch(skilllv){
				case 2: per=15; break;
				case 3: per=30; break;
				case 4: per=50; break;
				}
				tsd->status.hp=tsd->status.max_hp*per/100;
				if(tsd->status.hp<=0) tsd->status.hp=1;
				if( pc_check_equip_dcard(tsd,4144) ){	/* オシリスカード */
					tsd->status.hp=tsd->status.max_hp;
					tsd->status.sp=tsd->status.max_sp;
				}
				pc_setstand(tsd);
				if(battle_config.ghost_time > 0)
					pc_setghosttimer(tsd,battle_config.ghost_time);
				clif_updatestatus(tsd,SP_HP);
				clif_resurrection(&tsd->bl,1);
			}
		}
		break;

	case AL_INCAGI:			/* 速度増加 */
	case AL_DECAGI:			/* 速度減少 */
	case AL_BLESSING:		/* ブレッシング */
	case KN_TWOHANDQUICKEN:	/* ツーハンドクイッケン */
	case CR_SPEARQUICKEN:	/* スピアクイッケン */
	case PR_IMPOSITIO:		/* イムポシティオマヌス */
	case PR_ASPERSIO:		/* アスペルシオ */
	case PR_KYRIE:			/* キリエエレイソン */
	case PR_LEXDIVINA:		/* レックスディビーナ */
	case PR_LEXAETERNA:		/* レックスエーテルナ */
	case AS_ENCHANTPOISON:	/* エンチャントポイズン */
	case AS_POISONREACT:	/* ポイズンリアクト */
	case AC_CONCENTRATION:	/* 集中力向上 */
	case MC_LOUD:			/* ラウドボイス */
	case MG_ENERGYCOAT:		/* エナジーコート */
	case SM_PROVOKE:		/* プロボック */
	case SM_ENDURE:			/* インデュア */
	case PR_SUFFRAGIUM:		/* サフラギウム */
	case MG_SIGHT:			/* サイト */
	case AL_RUWACH:			/* ルアフ */
	case PR_BENEDICTIO:		/* 聖体降福 */
	case CR_PROVIDENCE:		/* プロヴィデンス */
	case SA_FLAMELAUNCHER:	/* フレイムランチャー */
	case SA_FROSTWEAPON:	/* フロストウェポン */
	case SA_LIGHTNINGLOADER:/* ライトニングローダー */
	case SA_SEISMICWEAPON:	/* サイズミックウェポン */
	case MO_EXPLOSIONSPIRITS:	// 爆裂波動
	case MO_STEELBODY:		// 金剛
#if 0
	case CR_AUTOGUARD:		/* オートガード */
	case CR_DEFENDER:		/* ディフェンダー */
	case SA_CASTCANCEL:		/* キャストキャンセル */
	case SA_FREECAST:		/* フリーキャスト */
	case SA_VOLCANO:		/* ボルケーノ */
	case SA_DELUGE:			/* デリュージ */
	case SA_VIOLENTGALE:	/* バイオレントゲイル */
	case SA_LANDPROTECTOR:	/* ランドプロテクター */
	case BA_FROSTJOKE:		/* 寒いジョーク */
#endif
		clif_skill_nodamage( (skillid==PR_KYRIE)?bl:src,bl,skillid,0,1);
		skill_status_change_start( bl,
			SkillStatusChangeTable[skillid], skilllv, 0 );
		if(skillid==SM_PROVOKE && bl->type==BL_MOB)
			mob_target((struct mob_data *)bl,src,skill_get_range(skillid));
		break;
	case MO_CALLSPIRITS:	// 気功
		clif_skill_nodamage(src,bl,skillid,0,1);
		pc_addspiritball(sd,60*10*1000,skilllv);
		break;
	case MO_ABSORBSPIRITS:	// 気奪
		if(sd && dstsd) {
			if(sd == dstsd || map[sd->bl.m].flag&MF_PVP) {
				if(dstsd->spiritball > 0) {
					clif_skill_nodamage(src,bl,skillid,0,1);
					i = dstsd->spiritball * 7;
					pc_delspiritball(dstsd,dstsd->spiritball,0);
					if(i > 0x7FFF)
						i = 0x7FFF;
					if(sd->status.sp + i > sd->status.max_sp) {
						i = sd->status.max_sp - sd->status.sp;
						sd->status.sp = sd->status.max_sp;
					}
					else
						sd->status.sp += i;
					clif_heal(sd->fd,SP_SP,i);
				}
				else
					clif_skill_nodamage(src,bl,skillid,0,0);
			}
			else
				clif_skill_nodamage(src,bl,skillid,0,0);
		}
		break;

	case BS_HAMMERFALL:		/* ハンマーフォール */
		clif_skill_nodamage(src,bl,skillid,0,1);
		if( rand()%100 < (20+ 10*skilllv) ) {
			skill_status_change_start(bl,SC_STAN,skilllv,10000);
		}
		break;

	/* パーティスキル */
	case AL_ANGELUS:		/* エンジェラス */
	case PR_MAGNIFICAT:		/* マグニフィカート */
	case PR_GLORIA:			/* グロリア */
	case BS_ADRENALINE:		/* アドレナリンラッシュ */
	case BS_WEAPONPERFECT:	/* ウェポンパーフェクション */
	case BS_OVERTHRUST:		/* オーバートラスト */
		if( sd==NULL || sd->status.party_id==0 || (flag&1) ){
			/* 個別の処理 */
			clif_skill_nodamage(bl,bl,skillid,0,1);
			skill_status_change_start( bl,
				SkillStatusChangeTable[skillid], skilllv, 0 );
		}else{
			/* パーティ全体への処理 */
			party_foreachsamemap(skill_area_sub,
				sd,1,
				src,skillid,skilllv,tick, flag|BCT_PARTY|1,
				skill_castend_nodamage_id);
		}
		break;

	/*（付加と解除が必要） */
	case BS_MAXIMIZE:		/* マキシマイズパワー */
	case NV_TRICKDEAD:		/* 死んだふり */
	case TF_HIDING:			/* ハイディング */
	case AS_CLOAKING:		/* クローキング */
		clif_skill_nodamage(src,bl,skillid,0,1);
		{
			int sc=SkillStatusChangeTable[skillid];
			if( (battle_get_sc_data(bl))[sc].timer==-1 )
				/* 付加する */
				skill_status_change_start(bl, sc, skilllv, 0);
			else
				/* 解除する */
				skill_status_change_end(bl, sc, -1);

			if(skillid==AS_CLOAKING)
				skill_check_cloaking(bl);
		}
		break;

	/* 対地スキル */
	case BD_ETERNALCHAOS:		/* 永遠の混沌 */
	case BD_DRUMBATTLEFIELD:	/* 戦太鼓の響き */
	case BD_RINGNIBELUNGEN:		/* ニーベルングの指輪 */
	case BD_SIEGFRIED:			/* 不死身のジークフリード */
	case BA_WHISTLE:			/* 口笛 */
	case BA_ASSASSINCROSS:		/* 夕陽のアサシンクロス */
	case BA_APPLEIDUN:			/* イドゥンの林檎 */
	case DC_FORTUNEKISS:		/* 幸運のキス */
	case DC_SERVICEFORYOU:		/* サービスフォーユー */
#if 0
	case BD_LULLABY:			/* 子守唄 */
	case BD_RICHMANKIM:			/* ニヨルドの宴 */
	case BD_ETERNALCHAOS:		/* 永遠の混沌 */
	case BD_DRUMBATTLEFIELD:	/* 戦太鼓の響き */
	case BD_RINGNIBELUNGEN:		/* ニーベルングの指輪 */
	case BD_ROKISWEIL:			/* ロキの叫び */
	case BD_INTOABYSS:			/* 深淵の中に */
	case BD_SIEGFRIED:			/* 不死身のジークフリード */
	case BA_DISSONANCE:			/* 不協和音 */
	case BA_WHISTLE:			/* 口笛 */
	case BA_ASSASSINCROSS:		/* 夕陽のアサシンクロス */
	case BA_POEMBRAGI:			/* ブラギの詩 */
	case BA_APPLEIDUN:			/* イドゥンの林檎 */
	case DC_UGLYDANCE:			/* 自分勝手なダンス */
	case DC_HUMMING:			/* ハミング */
	case DC_DONTFORGETME:		/* 私を忘れないで… */
	case DC_FORTUNEKISS:		/* 幸運のキス */
	case DC_SERVICEFORYOU:		/* サービスフォーユー */
#endif
		skill_unitsetting(src,skillid,skilllv,src->x,src->y,0);
		break;

	case TF_STEAL:			// スティール
		if(pc_steal_item(sd,bl)) {
			clif_skill_nodamage(src,bl,skillid,0,1);
			mob_target((struct mob_data *)bl,src,skill_get_range(skillid));
		}
		else
			clif_skill_nodamage(src,bl,skillid,0,0);
		break;

	case RG_STEALCOIN:		// スティールコイン
		if(pc_steal_coin(sd,bl)) {
			clif_skill_nodamage(src,bl,skillid,0,1);
			mob_target((struct mob_data *)bl,src,skill_get_range(skillid));
		}
		else
			clif_skill_nodamage(src,bl,skillid,0,0);
		break;

	case MG_STONECURSE:			/* ストーンカース */
		clif_skill_nodamage(src,bl,skillid,0,1);
		if( rand()%100 < skilllv*4+20 )
			skill_status_change_start(bl,SC_STONE,skilllv,0);
		break;

	case NV_FIRSTAID:			/* 応急手当 */
		clif_skill_nodamage(src,bl,skillid,5,1);
		battle_heal(NULL,bl,5,0);
		break;

	case AL_CURE:				/* キュアー */
		clif_skill_nodamage(src,bl,skillid,0,1);
		skill_status_change_end(bl, SC_SILENCE	, -1 );
		skill_status_change_end(bl, SC_BLIND	, -1 );
		skill_status_change_end(bl, SC_CONFUSION, -1 );
		break;

	case TF_DETOXIFY:			/* 解毒 */
		clif_skill_nodamage(src,bl,skillid,0,1);
		skill_status_change_end(bl, SC_POISON	, -1 );
		break;

	case PR_STRECOVERY:			/* リカバリー */
		clif_skill_nodamage(src,bl,skillid,0,1);
		skill_status_change_end(bl, SC_FREEZE	, -1 );
		skill_status_change_end(bl, SC_STONE	, -1 );
		skill_status_change_end(bl, SC_SLEEP	, -1 );
		skill_status_change_end(bl, SC_STAN		, -1 );
		break;

	case WZ_ESTIMATION:			/* モンスター情報 */
		if(src->type==BL_PC){
			clif_skill_nodamage(src,bl,skillid,0,1);
			clif_skill_estimation((struct map_session_data *)src,bl);
		}
		break;

	case MC_IDENTIFY:			/* アイテム鑑定 */
		if(sd!=NULL)
			clif_item_identify_list(sd);
		break;

	case MC_VENDING:			/* 露店開設 */
		if(sd!=NULL)
			clif_openvendingreq(sd,2+sd->skilllv);
		break;

	case AL_TELEPORT:			/* テレポート */
		clif_skill_nodamage(src,bl,skillid,0,1);
		if( sd ){
			if(map[sd->bl.m].flag&MF_NOTELEPORT)	/* テレポ禁止 */
				break;
			if( sd->skilllv==1 )
				pc_randomwarp(sd,3);
			else{
				clif_skill_warppoint(sd,sd->skillid,"Random",
					sd->status.save_point.map,"","");
			}
		}else if( bl->type==BL_MOB )
			mob_warp((struct mob_data *)bl,-1,-1,3);
		break;

	case WZ_FROSTNOVA:			/* フロストノヴァ */
		skill_area_temp[1]=bl->id;
		skill_area_temp[2]=bl->x;
		skill_area_temp[3]=bl->y;
		/* まずターゲットにエフェクトを出す */
/*		clif_skill_nodamage(src,bl,skillid,0,1);
		clif_skill_damage(src,bl,tick,battle_get_amotion(src),0,-1,1,skillid,skilllv,6);
		skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick, ); */
		/* その後ターゲット以外の範囲内の敵全体に処理を行う */
		map_foreachinarea(skill_area_sub,
			bl->m,bl->x-1,bl->y-1,bl->x+1,bl->y+1,0,
			src,skillid,skilllv,tick, flag|BCT_ENEMY|1,
			skill_castend_damage_id);
		break;

	case RG_STRIPWEAPON:		/* ストリップウェポン */
		/*{
		int i,a=0,b=0;
		if(skilllv==1){a=1100 > 2000;}
        else if(skilllv==2){a=1100 > 2000;}
        else if(skilllv==3){a=1100 > 2000;}
        else if(skilllv==4){a=1100 > 2000;}
        else if(skilllv==5){a=1100 > 2000;}
		if(a>0){
           for(i=0;i<MAX_INVENTORY;i++){
          if(sd->status.inventory[i].nameid==a){
                 pc_unequipitem(sd,i);
                 b=1;
                 if(skilllv==1){a=rand()%7;}
                 else if(skilllv==2){a=rand()%9;}
                 else if(skilllv==3){a=rand()%11;}
                 else if(skilllv==4){a=rand()%13;}
                 else if(skilllv==5){a=rand()%15;}
		}
       if(b==0){clif_displaymessage(sd->fd,"Nothing Equiped.");}*/
		break;

	case RG_STRIPSHIELD:		/* ストリップシールド */
		/*{
		int i,a=0,b=0;
		if(skilllv==1){a=2100 > 2200;}
        else if(skilllv==2){a=2100 > 2200;}
        else if(skilllv==3){a=2100 > 2200;}
        else if(skilllv==4){a=2100 > 2200;}
        else if(skilllv==5){a=2100 > 2200;}
		if(a>0){
           for(i=0;i<MAX_INVENTORY;i++){
          if(sd->status.inventory[i].nameid==a){
                 pc_unequipitem(sd,i);
                 b=1;
                 if(skilllv==1){a=rand()%7;}
                 else if(skilllv==2){a=rand()%9;}
                 else if(skilllv==3){a=rand()%11;}
                 else if(skilllv==4){a=rand()%13;}
                 else if(skilllv==5){a=rand()%15;}
		}
       if(b==0){clif_displaymessage(sd->fd,"Nothing Equiped.");}*/
		break;
	case RG_STRIPARMOR:			/* ストリップアーマー */
		/*{
		int i,a=0,b=0;
		if(skilllv==1){a=2300 > 2400;}
        else if(skilllv==2){a=2300 > 2400;}
        else if(skilllv==3){a=2300 > 2400;}
        else if(skilllv==4){a=2300 > 2400;}
        else if(skilllv==5){a=2300 > 2400;}
		if(a>0){
           for(i=0;i<MAX_INVENTORY;i++){
          if(sd->status.inventory[i].nameid==a){
                 pc_unequipitem(sd,i);
                 b=1;
                 if(skilllv==1){a=rand()%7;}
                 else if(skilllv==2){a=rand()%9;}
                 else if(skilllv==3){a=rand()%11;}
                 else if(skilllv==4){a=rand()%13;}
                 else if(skilllv==5){a=rand()%15;}
		}
       if(b==0){clif_displaymessage(sd->fd,"Nothing Equiped.");}*/
		break;
	case RG_STRIPHELM:			/* ストリップヘルム */
		/*{
		int i,a=0,b=0;
		if(skilllv==1){a=2200 > 2300;}
        else if(skilllv==2){a=2200 > 2300;}
        else if(skilllv==3){a=2200 > 2300;}
        else if(skilllv==4){a=2200 > 2300;}
        else if(skilllv==5){a=2200 > 2300;}
		if(a>0){
           for(i=0;i<MAX_INVENTORY;i++){
          if(sd->status.inventory[i].nameid==a){
                 pc_unequipitem(sd,i);
                 b=1;
                 if(skilllv==1){a=rand()%7;}
                 else if(skilllv==2){a=rand()%9;}
                 else if(skilllv==3){a=rand()%11;}
                 else if(skilllv==4){a=rand()%13;}
                 else if(skilllv==5){a=rand()%15;}
		}
       if(b==0){clif_displaymessage(sd->fd,"Nothing Equiped.");}*/
		break;
	case RG_INTIMIDATE:			/* インティミデイト */
		if( sd ){
			if(map[sd->bl.m].flag&MF_NOTELEPORT)	/* テレポ禁止 */
				break;
			if( sd->skilllv==1 )
				pc_randomwarp(sd,3);
			else{
				clif_skill_warppoint(sd,sd->skillid,"Random",
					sd->status.save_point.map,"","");
			}
		}else if( bl->type==BL_MOB )
			mob_warp((struct mob_data *)bl,-1,-1,3);
		break;
	/* PotionPitcher added by Tato [17/08/03] */
	case AM_POTIONPITCHER:		/* ポーションピッチャー */
		{
        int a=0,i,bonus,c=0;
        bonus=100+(skilllv*10);
        if(skilllv==1){a=501;}
        else if(skilllv==2){a=500 > 503;}
        else if(skilllv==3){a=500 > 504;}
        else if(skilllv==4){a=500 > 506;}
        else if(skilllv==5){a=500 > 507;}
        if(a>0){
           for(i=0;i<MAX_INVENTORY;i++){
          if(sd->status.inventory[i].nameid==a){
                 pc_delitem(sd,i,1,0);
                 c=1;
                 if(a==501){a=rand()%15+30;}
                 else if(a==502){a=rand()%20+70;}
                 else if(a==503){a=rand()%60+175;}
                 else if(a==504){a=rand()%80+350;}
                 else if(a==506){a=1;}
                 if(a>1){
                    clif_skill_nodamage(src,bl,skillid,(bonus*a)/100,1);
                    battle_heal(NULL,bl,(bonus*a)/100,0);
                 }else{
                    skill_status_change_end(bl, SC_POISON , -1 );
                    skill_status_change_end(bl, SC_SILENCE , -1 );
                    skill_status_change_end(bl, SC_BLIND , -1 );
                    skill_status_change_end(bl, SC_CONFUSION , -1 );
                 }
          }
       }
       if(c==0){clif_displaymessage(sd->fd,"Not enough Potions.");}
        }
	}
		break;
	case AM_CP_WEAPON:
		break;
	case AM_CP_SHIELD:
		break;
	case AM_CP_ARMOR:
		break;
	case AM_CP_HELM:
		break;
	case SA_DISPELL:			/* ディスペル */
		clif_skill_nodamage(src,bl,skillid,0,1);
		skill_status_change_end(bl, SC_SILENCE	, -1 );
		skill_status_change_end(bl, SC_BLIND	, -1 );
		skill_status_change_end(bl, SC_CONFUSION, -1 );
		skill_status_change_end(bl, SC_FREEZE	, -1 );
		skill_status_change_end(bl, SC_STONE	, -1 );
		skill_status_change_end(bl, SC_SLEEP	, -1 );
		skill_status_change_end(bl, SC_STAN		, -1 );
		skill_status_change_end(bl, SC_POISON	, -1 );
		break;

	case TF_BACKSLIDING:		/* バックステップ */
		if(sd)
			pc_stop_walking(sd);
		skill_blown(src,bl,5|0x10000);
		clif_skill_nodamage(src,bl,skillid,0,1);
		clif_fixpos(src);
		break;

	/* ランダム属性変化、水属性変化、地、火、風 */
	case NPC_ATTRICHANGE:
	case NPC_CHANGEWATER:
	case NPC_CHANGEGROUND:
	case NPC_CHANGEFIRE:
	case NPC_CHANGEWIND:
	/* 毒、聖、念、闇 */
	case NPC_CHANGEPOISON:
	case NPC_CHANGEHOLY:
	case NPC_CHANGEDARKNESS:
	case NPC_CHANGETELEKINESIS:
		if(md){
			clif_skill_nodamage(src,bl,skillid,0,1);
			md->def_ele=skill_get_pl(skillid);
			if(md->def_ele==0)			/* ランダム変化、ただし、*/
				md->def_ele=rand()%9;	/* 不死属性は除く */
			md->def_ele+=(1+rand()%4)*20;	/* 属性レベルはランダム */
		}
		break;

	case NPC_SUICIDE:			/* 自決 */
		if(md){
			clif_skill_nodamage(src,bl,skillid,0,1);
			mob_damage(NULL,md,md->hp);
		}
		break;

	case NPC_SUMMONSLAVE:		/* 手下召喚 */
	case NPC_SUMMONMONSTER:		/* MOB召喚 */
		if(md)
			mob_summonslave(md,mob_db[md->class].skill[md->skillidx].val1,
				skilllv,(skillid==NPC_SUMMONSLAVE)?1:0);
		break;

	case NPC_EMOTION:			/* エモーション */
		clif_emotion(&md->bl,mob_db[md->class].skill[md->skillidx].val1);
		break;
	}
	return 0;
}


/*==========================================
 * スキル使用（詠唱完了、ID指定）
 *------------------------------------------
 */
int skill_castend_id( int tid, unsigned int tick, int id,int data )
{
	struct map_session_data* sd=NULL/*,*target_sd=NULL*/;
	struct block_list *bl;
	
	if( (sd=map_id2sd(id))==NULL )
		return 0;
	
	if( sd->skilltimer != tid )	/* タイマIDの確認 */
		return 0;
	sd->skilltimer=-1;
	
	bl=map_id2bl(sd->skilltarget);
	if(bl==NULL || bl->prev==NULL)
		return 0;
	if(sd->bl.m != bl->m || pc_isdead(sd))
		return 0;
	
	if(!skill_check_condition( sd ))		/* 使用条件チェック */
		return 0;
		
	printf("skill castend skill=%d\n",sd->skillid);

	if( (skill_get_inf(sd->skillid)&1) &&	// 彼我敵対関係チェック
		battle_check_target(&sd->bl,bl, BCT_ENEMY)<=0 )
		return 0;

	switch( skill_get_nk(sd->skillid) )
	{
	/* 攻撃系/吹き飛ばし系 */
	case 0:	case 2:
		skill_castend_damage_id(&sd->bl,bl,sd->skillid,sd->skilllv,tick,0);
		break;
	case 1:/* 支援系 */
		if( (sd->skillid==AL_HEAL || sd->skillid==ALL_RESURRECTION)&& battle_get_elem_type(bl)==9 )
			skill_castend_damage_id(&sd->bl,bl,sd->skillid,sd->skilllv,tick,0);
		else
			skill_castend_nodamage_id(&sd->bl,bl,sd->skillid,sd->skilllv,tick,0);
		break;
	}
	

	return 0;
}

/*==========================================
 * スキル使用（詠唱完了、場所指定の実際の処理）
 *------------------------------------------
 */
int skill_castend_pos2( struct block_list *src, int x,int y,int skillid,int skilllv,unsigned int tick,int flag)
{
	struct map_session_data *sd=NULL;
	int i,tmpx,tmpy;

	if(src->type==BL_PC)
		sd=(struct map_session_data *)src;
	if(skillid != WZ_METEOR)
		clif_skill_poseffect(src,skillid,skilllv,x,y,tick);

	switch(skillid)
	{
	case MG_THUNDERSTORM:		/* サンダーストーム */
	case WZ_HEAVENDRIVE:		/* ヘヴンズドライブ */
		skill_area_temp[1]=src->id;
		skill_area_temp[2]=x;
		skill_area_temp[3]=y;
		map_foreachinarea(skill_area_sub,
			src->m,x-2,y-2,x+2,y+2,0,
			src,skillid,skilllv,tick, flag|BCT_ENEMY|2,
			skill_castend_damage_id);
		break;

	case PR_BENEDICTIO:			/* 聖体降福 */
		skill_area_temp[1]=src->id;
		map_foreachinarea(skill_area_sub,
			src->m,x-1,y-1,x+1,y+1,0,
			src,skillid,skilllv,tick, flag|BCT_NOENEMY|1,
			skill_castend_nodamage_id);
		map_foreachinarea(skill_area_sub,
			src->m,x-1,y-1,x+1,y+1,0,
			src,skillid,skilllv,tick, flag|BCT_ENEMY|1,
			skill_castend_damage_id);
		break;

	case BS_HAMMERFALL:			/* ハンマーフォール */
		skill_area_temp[1]=src->id;
		skill_area_temp[2]=x;
		skill_area_temp[3]=y;
		map_foreachinarea(skill_area_sub,
			src->m,x-2,y-2,x+2,y+2,0,
			src,skillid,skilllv,tick, flag|BCT_ENEMY|2,
			skill_castend_nodamage_id);
		break;

	case MG_SAFETYWALL:			/* セイフティウォール */
	case MG_FIREWALL:			/* ファイヤーウォール */
	case AL_PNEUMA:				/* ニューマ */
	case WZ_ICEWALL:			/* アイスウォール */
	case WZ_FIREPILLAR:			/* ファイアピラー */
	case WZ_QUAGMIRE:			/* クァグマイア */
	case WZ_VERMILION:			/* ロードオブヴァーミリオン */
	case WZ_STORMGUST:			/* ストームガスト */
	case PR_SANCTUARY:			/* サンクチュアリ */
	case PR_MAGNUS:				/* マグヌスエクソシズム */
	case HT_SKIDTRAP:			/* スキッドトラップ */
	case HT_LANDMINE:			/* ランドマイン */
	case HT_ANKLESNARE:			/* アンクルスネア */
	case HT_SHOCKWAVE:			/* ショックウェーブトラップ */
	case HT_SANDMAN:			/* サンドマン */
	case HT_FLASHER:			/* フラッシャー */
	case HT_FREEZINGTRAP:		/* フリージングトラップ */
	case HT_BLASTMINE:			/* ブラストマイン */
	case HT_CLAYMORETRAP:		/* クレイモアートラップ */
	case AS_VENOMDUST:			/* ベノムダスト */
		skill_unitsetting(src,skillid,skilllv,x,y,0);
		break;

	case WZ_METEOR:				//メテオストーム
		for(i=0;i<2+(skilllv>>1);i++) {
			int j = 0,c;
			do {
				tmpx=x + (rand()%7 - 3);
				tmpy=y + (rand()%7 - 3);
				j++;
			} while(((c=map_getcell(src->m,x,y))==1 || c==5) && j<100);
			if(j >= 100)
				continue;
			if(i==0) {
				clif_skill_poseffect(src,skillid,skilllv,tmpx,tmpy,tick);
				skill_unitsetting(src,skillid,skilllv,tmpx,tmpy,0);
			}
			else
				skill_addtimerskill(sd,tick,i*1000,0,tmpx,tmpy,skillid,skilllv,BF_MAGIC,flag);
		}
		break;

	case AL_WARP:				/* ワープポータル */
		if(map[sd->bl.m].flag&MF_NOTELEPORT)	/* テレポ禁止 */
			break;
		clif_skill_warppoint(sd,sd->skillid,sd->status.save_point.map,
			(sd->skilllv>1)?sd->status.memo_point[0].map:"",
			(sd->skilllv>2)?sd->status.memo_point[1].map:"",
			(sd->skilllv>3)?sd->status.memo_point[2].map:"");
		break;
	case MO_BODYRELOCATION:
		pc_movepos(sd,x,y);
		break;
	}
	return 0;
}

/*==========================================
 * スキル使用（詠唱完了、map指定）
 *------------------------------------------
 */
int skill_castend_map( struct map_session_data *sd,int skill_num, const char *map)
{
	int x=0,y=0;
	
	if( sd==NULL || pc_isdead(sd))
		return 0;

	if( sd->opt1>0 || sd->status.option&6 || sd->sc_data[SC_DIVINA].timer!=-1 )
		return 0;	/* 異常や沈黙など */
	
	if( skill_num != sd->skillid)	/* 不正パケットらしい */
		return 0;

	pc_stopattack(sd);

	printf("skill castend skill =%d map=%s\n",skill_num,map);

	if(strcmp(map,"cancel")==0)
		return 0;

	switch(skill_num){
	case AL_TELEPORT:		/* テレポート */
		if(strcmp(map,"Random")==0)
			pc_randomwarp(sd,3);
		else
			pc_setpos(sd,sd->status.save_point.map,
				sd->status.save_point.x,sd->status.save_point.y,3);
		break;

	case AL_WARP:			/* ワープポータル */
		{
			const struct point *p[]={
				&sd->status.save_point,&sd->status.memo_point[0],
				&sd->status.memo_point[1],&sd->status.memo_point[2],
			};
			struct skill_unit_group *group;
			int i;
			for(i=0;i<sd->skilllv;i++){
				if(strcmp(map,p[i]->map)==0){
					x=p[i]->x;
					y=p[i]->y;
					break;
				}
			}
			if(x==0 || y==0)	/* 不正パケット？ */
				return 0;

			group=skill_unitsetting(&sd->bl,sd->skillid,sd->skilllv,sd->skillx,sd->skilly,0);
			group->valstr=malloc(16);
			if(group->valstr==NULL){
				printf("skill_castend_map: out of memory !\n");
				exit(0);
			}
			memcpy(group->valstr,map,16);
			group->val2=(x<<16)|y;
		}
		break;

	case RG_INTIMIDATE:			/* インティミデイト */
		if(strcmp(map,"Random")==0)
			pc_randomwarp(sd,3);
		else
			pc_setpos(sd,sd->status.save_point.map,
				sd->status.save_point.x,sd->status.save_point.y,3);
		break;
	}

	return 0;
}

/*==========================================
 * スキルユニット設定処理
 *------------------------------------------
 */
struct skill_unit_group *skill_unitsetting( struct block_list *src, int skillid,int skilllv,int x,int y,int flag)
{
	struct skill_unit_group *group;
	int i,count=1,limit=10000,val1=skilllv,val2=0;
	int target=BCT_ENEMY,interval=1000,range=0;
	int dir=0;

	switch(skillid){	/* 設定 */

	case MG_SAFETYWALL:			/* セイフティウォール */
		limit=5000*skilllv;
		val2=skilllv+1;
		target=(battle_config.defnotenemy)?BCT_NOENEMY:BCT_ALL;
		break;

	case MG_FIREWALL:			/* ファイヤーウォール */
		dir=map_calc_dir(src,x,y);
		if(dir&1) count=5;
		else count=3;
		limit=1000*(val2=(4+skilllv));
		interval=250;
		break;

	case AL_PNEUMA:				/* ニューマ */
		target=(battle_config.defnotenemy)?BCT_NOENEMY:BCT_ALL;
		range=1;
		break;

	case AL_WARP:				/* ワープポータル */
		target=BCT_ALL;
		val1=skillid+6;
		if(flag==0)
			limit=2000;
		else
			limit=5000*(1+skilllv);
		break;

	case PR_SANCTUARY:			/* サンクチュアリ */
		count=21;
		limit=1000*(3*skilllv+1);
		val1=skilllv+3;
		val2=(skilllv>6)?777:skilllv*100;
		target=BCT_ALL;
		range=1;
		break;

	case PR_MAGNUS:				/* マグヌスエクソシズム */
		count=33;
		limit=1000*(skilllv+4);
		interval=3000;
		break;

	case WZ_FIREPILLAR:			/* ファイアーピラー */
		if(flag==0)
			limit=30000;
		else
			limit=1000;
		interval=2000;
		val1=skilllv+2;
		range=1;
		break;
	case WZ_METEOR:				/* メテオストーム */
		limit=500;
		interval=500;
		range=2;
		count=2*(skilllv);
		break;

	case WZ_VERMILION:			/* ロードオブヴァーミリオン */
		limit=3500;
		interval=1500;
		range=4;
		break;

	case WZ_ICEWALL:			/* アイスウォール */
		limit=4000*(1+skilllv);
		count=5;
		break;

	case WZ_STORMGUST:			/* ストームガスト */
		limit=2000+skilllv*300;
		interval=450;
		range=4;
		break;

	case WZ_QUAGMIRE:			/* クァグマイア */
		limit=5000*skilllv;
		interval=200;
		count=25;
		break;

	case HT_SKIDTRAP:			/* スキッドトラップ */
		limit=60000*(6-skilllv);
		range=1;
		break;

	case HT_LANDMINE:			/* ランドマイン */
		limit=40000*(6-skilllv);
		range=1;
		break;

	case HT_ANKLESNARE:			/* アンクルスネア */
		limit=50000*(6-skilllv);
		range=1;
		val1=skilllv*5000;
		interval=val1+5000;
		break;

	case HT_SHOCKWAVE:			/* ショックウェーブトラップ */
		limit=40000*(6-skilllv);
		range=1;
		val1=skilllv*15+10;
		break;

	case HT_SANDMAN:			/* サンドマン */
		limit=30000*(6-skilllv);
		range=1;
		break;

	case HT_FLASHER:			/* フラッシャー */
		limit=30000*(6-skilllv);
		range=1;
		break;

	case HT_FREEZINGTRAP:		/* フリージングトラップ */
		limit=30000*(6-skilllv);
		range=1;
		break;

	case HT_BLASTMINE:			/* ブラストマイン */
		limit=5000*(6-skilllv);
		interval=3000;
		range=1;
		break;

	case HT_CLAYMORETRAP:		/* クレイモアートラップ */
		limit=20000*(skilllv);
		interval=3000;
		range=1;
		break;

	case AS_VENOMDUST:			/* ベノムダスト */
		limit=5000*skilllv;
		interval=1000;
		count=5;
		break;

	case CR_GRANDCROSS:			/* グランドクロス */
		count=33;
		limit=1000*(skilllv+4);
		interval=3000;
		break;

	case SA_VOLCANO:			/* ボルケーノ */
		range=skilllv+4;
		target=BCT_ALL;
		break;

	case SA_DELUGE:				/* デリュージ */
		range=skilllv+4;
		target=BCT_ALL;
		break;

	case BD_DRUMBATTLEFIELD:	/* 戦太鼓の響き */
		count=81;
		limit=40000*(6-skilllv);
		val1=skilllv*15+10;
		target=BCT_ALL;
		break;

	case BD_SIEGFRIED:			/* 不死身のジークフリード */
		count=81;
		target=BCT_ALL;
		break;

	case BA_DISSONANCE:			/* 不協和音 */
		count=81;
		target=BCT_ENEMY;
		break;

	case BA_WHISTLE:			/* 口笛 */
		count=81;
		limit=40000*(6-skilllv);
		range=9;
		val1=skilllv*15+10;
		target=BCT_ALL;
		break;

	case BA_APPLEIDUN:			/* イドゥンの林檎 */
		count=81;
		target=BCT_ALL;
		break;

	case DC_UGLYDANCE:			/* 自分勝手なダンス */
		count=81;
		limit=40000*(6-skilllv);
		range=5;
		val1=skilllv*15+10;
		target=BCT_ALL;
		break;

	case DC_FORTUNEKISS:		/* 幸運のキス */
		count=81;
		limit=40000*(6-skilllv);
		val1=skilllv*15+10;
		target=BCT_ALL;
		break;

	case DC_SERVICEFORYOU:		/* サービスフォーユー */
		count=81;
		limit=40000*(6-skilllv);
		val1=skilllv*15+10;
		target=BCT_ALL;
		break;
	};

	group=skill_initunitgroup(src,count,skillid,skilllv,
		skill_get_unit_id(skillid,flag&1));
	group->limit=limit;
	group->val1=val1;
	group->val2=val2;
	group->target_flag=target;
	group->interval=interval;
	group->range=range;

	for(i=0;i<count;i++){
		struct skill_unit *unit;
		int ux=x,uy=y,val1=skilllv,val2=0,limit=group->limit,alive=1;
		int range=0;
		switch(skillid){	/* 設定 */
		case MG_FIREWALL:		/* ファイヤーウォール */
		{
				if(dir&1){	/* 斜め配置 */
					static const int dx[][5]={
						{ 1,1,0,0,-1 }, { -1,-1,0,0,1 },
					},dy[][5]={
						{ 1,0,0,-1,-1 }, { 1,0,0,-1,-1 },
					};
					ux+=dx[(dir>>1)&1][i];
					uy+=dy[(dir>>1)&1][i];
				}else{	/* 上下配置 */
					if(dir%4==0)	/* 上下 */
						ux+=i-1;
					else			/* 左右 */
						uy+=i-1;
				}
				val2=group->val2;
			}
			break;

		case PR_SANCTUARY:		/* サンクチュアリ */
			{
				static const int dx[]={
					-1,0,1, -2,-1,0,1,2, -2,-1,0,1,2, -2,-1,0,1,2, -1,0,1 };
				static const int dy[]={
					-2,-2,-2, -1,-1,-1,-1,-1, 0,0,0,0,0, 1,1,1,1,1, 2,2,2, };
				ux+=dx[i];
				uy+=dy[i];
			}
			break;

		case PR_MAGNUS:			/* マグヌスエクソシズム */
			{
				static const int dx[]={ -1,0,1, -1,0,1, -3,-2,-1,0,1,2,3,
					-3,-2,-1,0,1,2,3, -3,-2,-1,0,1,2,3, -1,0,1, -1,0,1, };
				static const int dy[]={
					-3,-3,-3, -2,-2,-2, -1,-1,-1,-1,-1,-1,-1,
					0,0,0,0,0,0,0, 1,1,1,1,1,1,1, 2,2,2, 3,3,3 };
				ux+=dx[i];
				uy+=dy[i];
			}
			break;

		case WZ_ICEWALL:		/* アイスウォール */
			{
				static const int dirx[8]={0,-1,-1,-1,0,1,1,1};
				static const int diry[8]={1,1,0,-1,-1,-1,0,1};
				dir=map_calc_dir(src,x,y);
				ux+=(2-i)*diry[dir];
				uy+=(i-2)*dirx[dir];
				val2=map_getcell(src->m,ux,uy);
				if(val2==5 || val2==1)
					alive=0;
				else
					map_setcell(src->m,ux,uy,5);
			}
			break;

		case WZ_QUAGMIRE:		/* クァグマイア */
			ux+=(i%5-2);
			uy+=(i/5-2);
			break;

		case AS_VENOMDUST:		/* ベノムダスト */
			{
				static const int dx[]={-1,0,0,0,1};
				static const int dy[]={0,-1,0,1,0};
				ux+=dx[i];
				uy+=dy[i];
			}
			break;

		/* ダンスなど */
		case BD_LULLABY:		/* 子守歌 */
		case BD_RICHMANKIM:		/* ニヨルドの宴 */
		case BD_ETERNALCHAOS:	/* 永遠の混沌 */
		case BD_DRUMBATTLEFIELD:/* 戦太鼓の響き */
		case BD_RINGNIBELUNGEN:	/* ニーベルングの指輪 */
		case BD_ROKISWEIL:		/* ロキの叫び */
		case BD_INTOABYSS:		/* 深淵の中に */
		case BD_SIEGFRIED:		/* 不死身のジークフリード */
		case BA_DISSONANCE:		/* 不協和音 */
		case BA_WHISTLE:		/* 口笛 */
		case BA_ASSASSINCROSS:	/* 夕陽のアサシンクロス */
		case BA_POEMBRAGI:		/* ブラギの詩 */
		case BA_APPLEIDUN:		/* イドゥンの林檎 */
		case DC_UGLYDANCE:		/* 自分勝手なダンス */
		case DC_HUMMING:		/* ハミング */
		case DC_DONTFORGETME:	/* 私を忘れないで… */
		case DC_FORTUNEKISS:	/* 幸運のキス */
		case DC_SERVICEFORYOU:	/* サービスフォーユー */
			ux+=(i%9-4);
			uy+=(i/9-4);
			if(i==40)
				range=4;	/* 中心の場合は範囲を4にオーバーライド */
			else
				range=-1;	/* 中心じゃない場合は範囲を-1にオーバーライド */
			break;
		}
		if(alive){
			unit=skill_initunit(group,i,ux,uy);
			unit->val1=val1;
			unit->val2=val2;
			unit->limit=limit;
			unit->range=range;
		}
	}
	return group;
}



/*==========================================
 * スキルユニットの発動イベント
 *------------------------------------------
 */
int skill_unit_onplace(struct skill_unit *src,struct block_list *bl,unsigned int tick)
{
	struct skill_unit_group *sg= src->group;
	struct block_list *ss=map_id2bl(sg->src_id);
	struct skill_unit_group_tickset *ts;
	int diff,goflag;
	
	
	if( bl->prev==NULL || !src->alive)
		return 0;
	if( bl->type!=BL_PC && bl->type!=BL_MOB )
		return 0;

	if(ss==NULL)
		return 0;

	ts=skill_unitgrouptickset_search( bl, sg->group_id);
	diff=DIFF_TICK(tick,ts->tick);
	goflag=(diff>sg->interval || diff<0);
	if(!goflag)
		return 0;
	ts->tick=tick;
	ts->group_id=sg->group_id;

	switch(sg->unit_id){
	case 0x83:	/* サンクチュアリ */
		{
			int *list=sg->vallist;
			int i,ei=0;
			
			if( battle_get_hp(bl)>=battle_get_max_hp(bl) &&
				battle_get_elem_type(bl)!=9 )
				break;

			for(i=0;i<16;i++)	/* 人数制限の計算 */
				if(list[i]==0)
					ei=i;
				else if(list[i]==bl->id)
					break;
			if(i==16){
				list[ei]=bl->id;
				if((sg->val1--)<=0){
					skill_delunitgroup(sg);
					return 0;
				}
			}
			if( battle_get_elem_type(bl)!=9){
				int heal=sg->val2;
				if( bl->type==BL_PC &&
					pc_check_equip_dcard((struct map_session_data *)bl,4128) )
					heal=0;	/* 黄金蟲カード（ヒール量０） */
				clif_skill_nodamage(&src->bl,bl,/*sg->skill_id*/28,heal,1);
				battle_heal(NULL,bl,heal,0);
			}else
				skill_attack(BF_MAGIC,ss,&src->bl,bl,
					sg->skill_id,sg->skill_lv,tick,0);
		}
		break;

	case 0x84:	/* マグヌスエクソシズム */
		{
			int race=battle_get_race(bl);
			if( race!=1 && race!=6 )
				return 0;
			skill_attack(BF_MAGIC,ss,&src->bl,bl,
				sg->skill_id,sg->skill_lv,tick,0);
		}
		break;

	case 0x85:	/* ニューマ */
	case 0x7e:	/* セイフティウォール */
		{
			struct skill_unit *unit2;
			struct status_change *sc_data=battle_get_sc_data(bl);
			int type=(sg->unit_id==0x85)?SC_PNEUMA:SC_SAFETYWALL;
			if(sc_data[type].timer==-1)
				skill_status_change_start(bl,type,src->group->val1,(int)src);
			else if((unit2=(struct skill_unit *)sc_data[type].val2)!=src){
				if( sg->unit_id==0x85 && DIFF_TICK(sg->tick,unit2->group->tick)>0 )
					skill_status_change_start(bl,type,src->group->val1,(int)src);
				if( sg->unit_id==0x7e && sg->val1 < unit2->group->val1 )
					skill_status_change_start(bl,type,src->group->val1,(int)src);
				ts->tick-=sg->interval;
			}
		} break;

	case 0x86:	/* ロードオブヴァーミリオン(＆ストームガスト) */
		skill_attack(BF_MAGIC,ss,&src->bl,bl,
			sg->skill_id,sg->skill_lv,tick,0);
		break;

	case 0x7f:	/* ファイヤーウォール */
		if( (src->val2--)>0)
			skill_attack(BF_MAGIC,ss,&src->bl,bl,
				sg->skill_id,sg->skill_lv,tick,0);
		if( src->val2<=0 )
			skill_delunit(src);
		break;

	case 0x87:	/* ファイアーピラー(発動前) */
		skill_delunit(src);
		skill_unitsetting(ss,sg->skill_id,sg->skill_lv,src->bl.x,src->bl.y,1);
		break;

	case 0x88:	/* ファイアーピラー(発動後) */
		skill_attack(BF_MAGIC,ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
		break;

	case 0x90:	/* スキッドトラップ */
		if(sg->val2==0){
			battle_stopwalking(bl);
			skill_blown(&src->bl,bl,sg->skill_lv|0x30000);
			sg->limit=DIFF_TICK(tick,sg->tick)+3000;
			sg->val2=bl->id;
		}break;

	case 0x93:	/* ランドマイン */
		skill_attack(BF_MISC,ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
		skill_delunit(src);
		break;

	case 0x8f:	/* ブラストマイン */
	case 0x97:	/* フリージングトラップ */
	case 0x98:	/* クレイモアートラップ */
		skill_attack((sg->unit_id==0x97)?BF_WEAPON:BF_MISC,
			ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,(sg->val2)?0x0500:0);
		if(sg->val2==0){
			sg->limit=DIFF_TICK(tick,sg->tick)+500;
			sg->val2=bl->id;
		}
		break;

	case 0x95:	/* サンドマン */
	case 0x96:	/* フラッシャー */
	case 0x94:	/* ショックウェーブトラップ */
	case 0x91:	/* アンクルスネア */
		skill_additional_effect(ss,bl,sg->skill_id,sg->skill_lv,tick);
		if(sg->val2==0){
			sg->limit=DIFF_TICK(tick,sg->tick)
				+((sg->unit_id==0x91)?sg->val1/((battle_get_mode(bl)&0x20)?5:1):500);
			sg->val2=bl->id;
		}
		break;

	case 0x80:	/* ワープポータル(発動後) */
		if(bl->type==BL_PC){
			if((sg->val1--)>0){
				pc_setpos((struct map_session_data *)bl,
					sg->valstr,sg->val2>>16,sg->val2&0xffff,0);
			}else
				skill_delunitgroup(sg);
		}
		break;

	case 0x8e:	/* クァグマイア */
	case 0x92:	/* ベノムダスト */
		{
			int type=SkillStatusChangeTable[sg->skill_id];
			if( battle_get_sc_data(bl)[type].timer==-1 )
				skill_status_change_start(bl,type,src->group->val1,(int)src);
		}
		break;

	case 0x9e:	/* 子守唄 */
	case 0x9f:	/* ニヨルドの宴 */
	case 0xa0:	/* 永遠の混沌 */
	case 0xa1:	/* 戦太鼓の響き */
	case 0xa2:	/* ニーベルングの指輪 */
	case 0xa3:	/* ロキの叫び */
	case 0xa4:	/* 深淵の中に */
	case 0xa5:	/* 不死身のジークフリード */
	case 0xa6:	/* 不協和音 */
	case 0xa7:	/* 口笛 */
	case 0xa8:	/* 夕陽のアサシンクロス */
	case 0xa9:	/* ブラギの詩 */
	case 0xaa:	/* イドゥンの林檎 */
	case 0xab:	/* 自分勝手なダンス */
	case 0xac:	/* ハミング */
	case 0xad:	/* 私を忘れないで… */
	case 0xae:	/* 幸運のキス */
	case 0xaf:	/* サービスフォーユー */
		{
			struct skill_unit *unit2;
			struct status_change *sc_data=battle_get_sc_data(bl);
			int type=SkillStatusChangeTable[sg->skill_id];
			if(sc_data[type].timer==-1)
				skill_status_change_start(bl,type,src->group->val1,(int)src);
			else if((unit2=(struct skill_unit *)sc_data[type].val4)!=src){
				if( DIFF_TICK(sg->tick,unit2->group->tick)>0 )
					skill_status_change_start(bl,type,0,(int)src);
/*				if( sg->unit_id==0x7e && sg->val1 < unit2->group->val1 )
					skill_status_change_start(bl,type,0,(int)src);*/
				ts->tick-=sg->interval;
			}
		} break;

/*	default:
		printf("skill_unit_onplace: Unknown skill unit id=%d block=%d\n",sg->unit_id,bl->id);
		break;*/
	}
	return 0;
}
/*==========================================
 * スキルユニットから離脱する(もしくはしている)場合
 *------------------------------------------
 */
int skill_unit_onout(struct skill_unit *src,struct block_list *bl,unsigned int tick)
{
	struct skill_unit_group *sg= src->group;

	if( bl->prev==NULL || !src->alive)
		return 0;
	if( bl->type!=BL_PC && bl->type!=BL_MOB )
		return 0;

	switch(sg->unit_id){
	case 0x83:	/* サンクチュアリ */
		{
			int i,*list=sg->vallist;
			for(i=0;i<list[i];i++)
				if(list[i]==bl->id)
					list[i]=0;
		}
		break;

	case 0x7e:	/* セイフティウォール */
	case 0x85:	/* ニューマ */
	case 0x8e:	/* クァグマイア */
		{
			struct skill_unit *unit2;
			struct status_change *sc_data=battle_get_sc_data(bl);
			int type=
				(sg->unit_id==0x85)?SC_PNEUMA:
				((sg->unit_id==0x7e)?SC_SAFETYWALL:
				 SC_QUAGMIRE);
			if(sc_data[type].timer!=-1 && (unit2=(struct skill_unit *)sc_data[type].val2)==src){
				skill_status_change_end(bl,type,-1);
			}
		} break;

	case 0x91:	/* アンクルスネア */
		{
			struct block_list *target=map_id2bl(sg->val2);
			if( target==bl )
				skill_status_change_end(bl,SC_ANKLE,-1);
			sg->limit=DIFF_TICK(tick,sg->tick)+1000;
		}
		break;

	case 0x9e:	/* 子守唄 */
	case 0x9f:	/* ニヨルドの宴 */
	case 0xa0:	/* 永遠の混沌 */
#if 0
	case 0xa1:	/* 戦太鼓の響き */
#endif
	case 0xa2:	/* ニーベルングの指輪 */
	case 0xa3:	/* ロキの叫び */
#if 0
	case 0xa4:	/* 深淵の中に */
#endif
	case 0xa5:	/* 不死身のジークフリード */
#if 0
	case 0xa6:	/* 不協和音 */
	case 0xa7:	/* 口笛 */
#endif
	case 0xa8:	/* 夕陽のアサシンクロス */
#if 0
	case 0xa9:	/* ブラギの詩 */
#endif
	case 0xaa:	/* イドゥンの林檎 */
#if 0
	case 0xab:	/* 自分勝手なダンス */
	case 0xac:	/* ハミング */
#endif
	case 0xad:	/* 私を忘れないで… */
	case 0xae:	/* 幸運のキス */
#if 0
	case 0xaf:	/* サービスフォーユー */
#endif
		{
			struct skill_unit *unit2;
			struct status_change *sc_data=battle_get_sc_data(bl);
			int type=SkillStatusChangeTable[sg->skill_id];
			if(sc_data[type].timer!=-1 && (unit2=(struct skill_unit *)sc_data[type].val4)==src){
				skill_status_change_end(bl,type,-1);
			}
		}
		break;

/*	default:
		printf("skill_unit_onout: Unknown skill unit id=%d block=%d\n",sg->unit_id,bl->id);
		break;*/
	}
	skill_unitgrouptickset_delete(bl,sg->group_id);
	return 0;
}
/*==========================================
 * スキルユニットの削除イベント
 *------------------------------------------
 */
int skill_unit_ondelete(struct skill_unit *src,struct block_list *bl,unsigned int tick)
{
	struct skill_unit_group *sg= src->group;
	
	if( bl->prev==NULL || !src->alive)
		return 0;
	if( bl->type!=BL_PC && bl->type!=BL_MOB )
		return 0;

	switch(sg->unit_id){
	case 0x85:	/* ニューマ */
	case 0x7e:	/* セイフティウォール */
	case 0x8e:	/* クァグマイヤ */
	case 0x9e:	/* 子守唄 */
	case 0x9f:	/* ニヨルドの宴 */
	case 0xa0:	/* 永遠の混沌 */
	case 0xa1:	/* 戦太鼓の響き */
	case 0xa2:	/* ニーベルングの指輪 */
	case 0xa3:	/* ロキの叫び */
	case 0xa4:	/* 深淵の中に */
	case 0xa5:	/* 不死身のジークフリード */
	case 0xa6:	/* 不協和音 */
	case 0xa7:	/* 口笛 */
	case 0xa8:	/* 夕陽のアサシンクロス */
	case 0xa9:	/* ブラギの詩 */
	case 0xaa:	/* イドゥンの林檎 */
	case 0xab:	/* 自分勝手なダンス */
	case 0xac:	/* ハミング */
	case 0xad:	/* 私を忘れないで… */
	case 0xae:	/* 幸運のキス */
	case 0xaf:	/* サービスフォーユー */
		return skill_unit_onout(src,bl,tick);

/*	default:
		printf("skill_unit_ondelete: Unknown skill unit id=%d block=%d\n",sg->unit_id,bl->id);
		break;*/
	}
	skill_unitgrouptickset_delete(bl,sg->group_id);
	return 0;
}
/*==========================================
 * スキルユニットの限界イベント
 *------------------------------------------
 */
int skill_unit_onlimit(struct skill_unit *src,unsigned int tick)
{
	struct skill_unit_group *sg= src->group;
	

	switch(sg->unit_id){
	case 0x81:	/* ワープポータル(発動前) */
		{
			struct skill_unit_group *group=
				skill_unitsetting(map_id2bl(sg->src_id),sg->skill_id,sg->skill_lv,
					src->bl.x,src->bl.y,1);
			group->valstr=malloc(16);
			if(group->valstr==NULL){
				printf("skill_unit_ondelete: out of memory !\n");
				exit(0);
			}
			memcpy(group->valstr,sg->valstr,16);
			group->val2=sg->val2;
		}
		break;

	case 0x8d:	/* アイスウォール */
		map_setcell(src->bl.m,src->bl.x,src->bl.y,src->val2);
		break;
			
	}
	return 0;
}
/*==========================================
 * スキルユニットのダメージイベント
 *------------------------------------------
 */
int skill_unit_ondamaged(struct skill_unit *src,struct block_list *bl,
	int damage,unsigned int tick)
{
	struct skill_unit_group *sg= src->group;
	
	switch(sg->unit_id){
	case 0x8d:	/* アイスウォール */
		sg->limit-=damage;
		break;
	}
	return 0;
}


/*---------------------------------------------------------------------------- */



/*==========================================
 * スキル使用（詠唱完了、場所指定）
 *------------------------------------------
 */
int skill_castend_pos( int tid, unsigned int tick, int id,int data )
{
	struct map_session_data* sd=NULL/*,*target_sd=NULL*/;
	
	if( (sd=map_id2sd(id))==NULL )
		return 0;
	
	if( sd->skilltimer != tid )	/* タイマIDの確認 */
		return 0;
	sd->skilltimer=-1;

	if(pc_isdead(sd))
		return 0;
	
	if(!skill_check_condition( sd ))		/* 使用条件チェック */
		return 0;

	printf("skill castend skill=%d\n",sd->skillid);

	skill_castend_pos2(&sd->bl,sd->skillx,sd->skilly,sd->skillid,sd->skilllv,tick,0);

	return 0;
}

/*==========================================
 * スキル使用条件（偽で使用失敗）
 *------------------------------------------
 */
int skill_check_condition( struct map_session_data *sd )
{
	int j=0,sp=0,zeny=0,spiritball=0;
	int	i[3]={0,0,0},
		item_id[3]={0,0,0},
		item_amount[3]={0,0,0};

	if(sd->sc_data[SC_STEELBODY].timer!=-1) {
		clif_skill_fail(sd,sd->skillid,0,0);
		return 0;
	}

	if(sd->skillitem==sd->skillid) {	/* アイテムの場合無条件成功 */
		sd->skillitem = sd->skillitemlv = -1;
	}else{

		sp=skill_get_sp(sd->skillid, sd->skilllv);	/* 消費SP */
		if(sd->dsprate!=100)
			sp=sp*sd->dsprate/100;	/* 消費SP修正 */
		
		/* スキルごとの使用条件 */
		switch(sd->skillid)
		{
		case SA_ABRACADABRA:
			item_amount[0]+=1;
		case SA_VOLCANO:
		case SA_DELUGE:
		case SA_VIOLENTGALE:
			item_id[0]=715;		//	ygem = 715;
			item_amount[0]+=1;
			break;

		case SA_DISPELL:
			item_id[1]=715;		//	ygem = 715;
			item_amount[1]+=1;
		case MG_STONECURSE:		// ストーンカース
		case AS_VENOMDUST:		// ベナムダスト
			item_id[0]=716;		//	rgem = 716;
			item_amount[0]+=1;
			break;

		case SA_LANDPROTECTOR:
			item_id[1]=715;
			item_amount[1]+=1;
		case MG_SAFETYWALL:		// セイフティウォール
		case AL_WARP:			// ワープポータル
		case ALL_RESURRECTION:	// リザレクション
		case PR_SANCTUARY:		// サンクチュアリ
		case PR_MAGNUS:			// マグヌスエクソシズム
		case WZ_FIREPILLAR:		// ファイアーピラー
			item_id[0]=717;		//	bgem = 717;
			item_amount[0]+=1;
			break;

		case SA_FROSTWEAPON:
			item_id[0]=991;
			item_amount[0]+=1;
			break;

		case SA_LIGHTNINGLOADER:
			item_id[0]=992;
			item_amount[0]+=1;
			break;

		case SA_SEISMICWEAPON:
			item_id[0]=993;
			item_amount[0]+=1;
			break;

		case MC_MAMMONITE:		/* メマーナイト */
			zeny = sd->skilllv*100;
			if( sd->status.zeny<zeny ) {
				clif_skill_fail(sd,sd->skillid,5,0);	/* Zeny不足：失敗通知 */
				return 0;
			}
			break;

		case MC_VENDING:		// 露店開設	
			if(!pc_iscarton(sd)) {
				clif_skill_fail(sd,sd->skillid,0,0);
				return 0;
			}
			break;


		case AC_DOUBLE:		// ダブルストレイフィング
		case AC_SHOWER:		// アローシャワー
		case AC_CHARGEARROW:		// チャージアロー
			if( sd->status.weapon != 11) {
				clif_skill_fail(sd,sd->skillid,6,0);	// 弓
			     	return 0; 
			}
			break;

		case KN_BRANDISHSPEAR:	// ブランディッシュスピア
			if(!pc_isriding(sd)) {
				clif_skill_fail(sd,sd->skillid,0,0);
				return 0;		// ,ペコペコ状態		
			}
		case KN_PIERCE:			// ピアース
		case KN_SPEARSTAB:		// スピアスタブ
		case KN_SPEARBOOMERANG:	// スピアブーメラン
			if(sd->status.weapon != 4 && sd->status.weapon != 5) {
				clif_skill_fail(sd,sd->skillid,6,0);	// 窓
				return 0; 
			}
			break;

		case KN_TWOHANDQUICKEN:	// ツーハンドクイッケン
			if( sd->status.weapon != 3) {
				clif_skill_fail(sd,sd->skillid,6,0);	// 両手剣
				return 0;
			}
			break;
			
		case BS_ADRENALINE:		// アドレナリンラッシュ
		case BS_HAMMERFALL:		// ハンマーフォール
			if( sd->status.weapon != 6 && sd->status.weapon != 7 && sd->status.weapon != 8) {
				clif_skill_fail(sd,sd->skillid,6,0);	// 斧・でメイス
				return 0;
			}
			break;

		case BS_MAXIMIZE:		/* マキシマイズパワー */
		case NV_TRICKDEAD:		/* 死んだふり */
		case TF_HIDING:			/* ハイディング */
		case AS_CLOAKING:		/* クローキング */
			if(sd->sc_data[SkillStatusChangeTable[sd->skillid]].timer!=-1)
				sp=0;			/* 解除する場合はSP消費しない */
			break;

		case HT_BLITZBEAT:		/* ブリッツビート */
			if(!pc_isfalcon(sd)) {		/* 鷹がいない */
				clif_skill_fail(sd,sd->skillid,0,0);
				return 0;
			}
			break;

		case AS_GRIMTOOTH:		/* グリムトゥース */
			if(!pc_ishiding(sd)) {		// ハイディング状態
				clif_skill_fail(sd,sd->skillid,0,0);
				return 0;
			}
			if( sd->status.weapon != 16)  {
				clif_skill_fail(sd,sd->skillid,6,0);    	// カタール消費
				return 0; 
			}
			break;

		case CR_SPEARQUICKEN:	// スピアクイッケン
			if( sd->status.weapon != 5) {
				clif_skill_fail(sd,sd->skillid,6,0);	// 両手槍
				return 0;
			}
			break;

		case MO_EXTREMITYFIST:					// 阿修羅覇鳳拳
			if( sd->sc_data[SC_EXPLOSIONSPIRITS].timer == -1) {
				clif_skill_fail(sd,sd->skillid,0,0);
				return 0;
			}
			else
				spiritball = 5;									// 氣球
			break;

		case MO_FINGEROFFENSIVE:	//指弾
			spiritball = sd->skilllv;							// 氣球
			if (sd->spiritball > 0 && sd->spiritball < spiritball) {
				spiritball = sd->spiritball;
				sd->spiritball_old = sd->spiritball;	
			}
			else sd->spiritball_old = sd->skilllv;	
			break;
		case MO_INVESTIGATE:		//発勁
		case MO_COMBOFINISH:
			spiritball = 1;									// 氣球
			break;

		case MO_BODYRELOCATION:
			if(!pc_can_reach(sd,sd->skillx,sd->skilly)) {
				clif_skill_fail(sd,sd->skillid,0,0);
				return 0;
			} else
				spiritball = 1;									// 氣球
			break;

		
		case MO_STEELBODY:						// 金剛
		case MO_EXPLOSIONSPIRITS:				// 爆裂波動
				spiritball = 5;									// 氣球
			break;
		}

		if( sp>0 && sd->status.sp < sp) {			/* SPチェック */
			clif_skill_fail(sd,sd->skillid,1,0);		/* SP不足：失敗通知 */
			return 0;
		}
		
		if( spiritball > 0 && sd->spiritball < spiritball) {
			clif_skill_fail(sd,sd->skillid,0,0);		// 氣球不足
			return 0;
		}

		if(!pc_check_equip_dcard(sd,4132) && 
			(item_id[0] || item_id[1] || item_id[2])) {	// ミストレスカード
			for(j=0;j<3;j++) {
				if(item_id[j] == 0 || item_amount[j] == 0)
					continue;
				if((i[j]=pc_search_inventory(sd,item_id[j])) == -1 ||
					sd->status.inventory[i[j]].amount < item_amount[j]) {	// ジェムストーンなし
					if(item_id[j] == 716 || item_id[j] == 717)
						clif_skill_fail(sd,sd->skillid,(6+(item_id[j]-715)),0);
					else
						clif_skill_fail(sd,sd->skillid,0,0);
					return 0;
				}
			}
			for(j=0;j<3;j++) {
				if(i[j] == 0)
					continue;
				pc_delitem(sd,i[j],item_amount[j],0);		// ジェムストーン消費
			}
		}

		if(sp) {					/* SP消費 */
			sd->status.sp-=sp;
			clif_updatestatus(sd,SP_SP);
		}
		if(zeny) {					/* Zeny消費 */
			sd->status.zeny -= zeny;
			clif_updatestatus(sd,SP_ZENY);
		}
		if(spiritball)			// 氣球消費
			pc_delspiritball(sd,spiritball,0);
	}
	return 1;
}


/*==========================================
 * 詠唱時間計算
 *------------------------------------------
 */
int skill_castfix( struct block_list *bl, int time )
{
	struct status_change *sc_data;
	int dex=battle_get_dex(bl);
	int castrate=100;
	if(time==0)
		return 0;
	if(bl->type==BL_PC)
		castrate=((struct map_session_data *)bl)->castrate;
	
	time=time*castrate*(150- dex)/15000;
	time=time*battle_config.cast_rate/100;
	
	if( (sc_data = battle_get_sc_data(bl))[SC_SUFFRAGIUM].timer!=-1 ){
		/* サフラギウム */
		time=time*(100-sc_data[SC_SUFFRAGIUM].val1*15)/100;
		skill_status_change_end( bl, SC_SUFFRAGIUM, -1);
	}
	return (time>0)?time:0;
}
/*==========================================
 * ディレイ計算
 *------------------------------------------
 */
int skill_delayfix( struct block_list *bl, int time )
{
	if(time<=0)
		return 0;
	
	if( battle_config.delay_dependon_dex )	/* dexの影響を計算する */
		time=time*(150- battle_get_dex(bl))/150;

	time=time*battle_config.delay_rate/100;
	
	return (time>0)?time:0;
}
/*==========================================
 * スキル使用（ID指定）
 *------------------------------------------
 */
int skill_use_id( struct map_session_data *sd, int target_id,
	int skill_num, int skill_lv)
{
	int casttime=0,delay=0,tick;
	struct map_session_data* target_sd=NULL;
	int target_fd=-1;
	int forcecast=0;
	struct block_list *bl;

	bl=map_id2bl(target_id);
	if(bl==NULL){
/*		printf("skill target not found %d\n",target_id); */
		return 0;
	}
	if(sd->bl.m != bl->m || pc_isdead(sd))
		return 0;

	/* 沈黙や異常（ただし、グリムなどの判定をする） */
	if( sd->opt1>0 || sd->status.option&6 || sd->sc_data[SC_DIVINA].timer!=-1 ){
		if( (sd->status.option&4) && skill_num==AS_CLOAKING );	/* クローキング中 */
		else if( (sd->status.option&2) && (skill_num==TF_HIDING || skill_num==AS_GRIMTOOTH) );	/* ハイディング中 */
		else
			return 0;
	}

	if( skill_num==HT_BLITZBEAT && !(sd->status.option&0x10) ){	/* 鷹がいない */
		return 0;
	}

	/* 射程と障害物チェック */
	if(!battle_check_range(&sd->bl,bl->x,bl->y,skill_get_range(skill_num)))
		return 0;

	if(bl->type==BL_PC){
		target_sd=(struct map_session_data*)bl;
		target_fd=target_sd->fd;
	}
	pc_stopattack(sd);

	casttime=skill_castfix(&sd->bl, skill_get_cast( skill_num,skill_lv) );
	delay=skill_delayfix(&sd->bl, skill_get_delay( skill_num,skill_lv) );

	sd->skillcastcancel=1;

	switch(skill_num){	/* 何か特殊な処理が必要 */
	case AL_HEAL:	/* ヒール */
		if( battle_get_elem_type(bl)==9)
			forcecast=1;	/* ヒールアタックなら詠唱エフェクト有り */
		break;
	case ALL_RESURRECTION:	/* リザレクション */
		if( battle_get_elem_type(bl)==9){	/* 敵がアンデッドなら */
			forcecast=1;	/* ターンアンデットと同じ詠唱時間 */
			casttime=skill_castfix(&sd->bl, skill_get_cast(PR_TURNUNDEAD,skill_lv) );
		}
		break;
	/* 詠唱キャンセルされないもの */
	case KN_BRANDISHSPEAR:	/* ブランディッシュスピア */
	case KN_BOWLINGBASH:	/* ボウリングバッシュ */
	case AC_CHARGEARROW:	/* チャージアロー */
	case RG_STRIPWEAPON:	/* ストリップウエポン */
	case RG_STRIPSHIELD:	/* ストリップシールド */
	case RG_STRIPARMOR:	/* ストリップアーマー */
	case RG_STRIPHELM:	/* ストリップヘルム */
	case CR_GRANDCROSS:	/* グランドクロス */
	case MO_CALLSPIRITS:	// 気功
	case MO_INVESTIGATE:	/* 発勁 */
	case MO_STEELBODY:	/* 金剛*/
		sd->skillcastcancel=0;
		break;
	case MO_FINGEROFFENSIVE:	/* 指弾 */
		casttime += casttime * ((skill_lv > sd->spiritball)? sd->spiritball:skill_lv);
		sd->skillcastcancel=0;
		break;
	}

	printf("skill use target_id=%d skill=%d lv=%d cast=%d\n"
		,target_id,skill_num,skill_lv,casttime);

	if(sd->skillitem == skill_num)
		casttime = delay = 0;

	if( casttime>0 || forcecast ){ /* 詠唱が必要 */
		struct mob_data *md;
		clif_skillcasting( &sd->bl,
			sd->bl.id, target_id, 0,0, skill_num,casttime);

		/* 詠唱反応モンスター */
		if( bl->type==BL_MOB && mob_db[(md=(struct mob_data *)bl)->class].mode&0x10 &&
			md->state.state!=MS_ATTACK && sd->ghost_timer == -1){
				md->target_id=sd->bl.id;
				md->state.targettype = ATTACKABLE;
				md->min_chase=13;
		}
	}

	if( casttime<=0 )	/* 詠唱の無いものはキャンセルされない */
		sd->skillcastcancel=0;

	sd->skilltarget	= target_id;
/*	sd->cast_target_bl	= bl; */
	sd->skillx		= 0;
	sd->skilly		= 0;
	sd->skillid		= skill_num;
	sd->skilllv		= skill_lv;
	tick=gettick();
	sd->canmove_tick = tick + casttime + delay;
	sd->skillcanmove_tick = tick;
	if(casttime > 0)
		sd->skilltimer = add_timer( tick+casttime, skill_castend_id, sd->bl.id, 0 );
	else {
		sd->skilltimer = 0;
		skill_castend_id(sd->skilltimer,tick,sd->bl.id,0);
	}

	return 0;
}

/*==========================================
 * スキル使用（場所指定）
 *------------------------------------------
 */
int skill_use_pos( struct map_session_data *sd,
	int skill_x, int skill_y, int skill_num, int skill_lv)
{
	int casttime=0,delay=0,tick;
	
	if(pc_isdead(sd))
		return 0;

	if( sd->opt1>0 || sd->status.option&6 || sd->sc_data[SC_DIVINA].timer!=-1 )
		return 0;	/* 異常や沈黙など */

	/* 射程と障害物チェック */
	if(!battle_check_range(&sd->bl,skill_x,skill_y,skill_get_range(skill_num)))
		return 0;
	
	pc_stopattack(sd);

	casttime=skill_castfix(&sd->bl, skill_get_cast( skill_num,skill_lv) );
	delay=skill_delayfix(&sd->bl, skill_get_delay( skill_num,skill_lv) );

	sd->skillcastcancel=1;

	printf("skill use target_pos=(%d,%d) skill=%d lv=%d cast=%d\n",
		skill_x,skill_y,skill_num,skill_lv,casttime);

	if(sd->skillitem == skill_num)
		casttime = delay = 0;

	if( casttime>0 )	/* 詠唱が必要 */
		clif_skillcasting( &sd->bl,
			sd->bl.id, 0, skill_x,skill_y, skill_num,casttime);

	if( casttime<=0 )	/* 詠唱の無いものはキャンセルされない */
		sd->skillcastcancel=0;

	sd->skillx			= skill_x;
	sd->skilly			= skill_y;
	sd->skilltarget	= 0;
/*	sd->cast_target_bl	= NULL; */
	sd->skillid		= skill_num;
	sd->skilllv			= skill_lv;
	tick=gettick();
	sd->canmove_tick = tick + casttime + delay;
	sd->skillcanmove_tick = tick;
	if(casttime > 0)
		sd->skilltimer = add_timer( tick+casttime, skill_castend_pos, sd->bl.id, 0 );
	else {
		sd->skilltimer = 0;
		skill_castend_pos(sd->skilltimer,tick,sd->bl.id,0);
	}

	return 0;
}



/*==========================================
 * スキル詠唱キャンセル
 *------------------------------------------
 */
int skill_castcancel( struct block_list *bl )
{
	if(bl->type==BL_PC){
		struct map_session_data *sd=(struct map_session_data *)bl;
		unsigned long tick=gettick();
		sd->canmove_tick=tick;
		sd->skillcanmove_tick = tick;
		if( sd->skilltimer!=-1){
			if( skill_get_inf( sd->skillid )&2 )
				delete_timer( sd->skilltimer, skill_castend_pos );
			else
				delete_timer( sd->skilltimer, skill_castend_id );
			sd->skilltimer=-1;
			clif_skillcastcancel(bl);
		}

		return 0;
	}else if(bl->type==BL_MOB){
		struct mob_data *md=(struct mob_data *)bl;
		if( md->skilltimer!=-1 ){
			if( skill_get_inf( md->skillid )&2 )
				delete_timer( md->skilltimer, mobskill_castend_pos );
			else
				delete_timer( md->skilltimer, mobskill_castend_id );
			md->skilltimer=-1;
			clif_skillcastcancel(bl);
		}
		return 0;
	}
	return 1;
}




/*----------------------------------------------------------------------------
 * ステータス異常
 *----------------------------------------------------------------------------
 */

/*==========================================
 * ステータス異常タイマー範囲処理
 *------------------------------------------
 */
int skill_status_change_timer_sub(struct block_list *bl, va_list ap )
{
	struct block_list *src;
	int type;
	unsigned int tick;
	src=va_arg(ap,struct block_list*);
	type=va_arg(ap,int);
	tick=va_arg(ap,unsigned int);
	
	if(bl->type!=BL_PC && bl->type!=BL_MOB)
		return 0;
	
	switch( type ){
	case SC_SIGHT:	/* サイト */
	case SC_RUWACH:	/* ルアフ */
		if( (*battle_get_option(bl))&6 ){
			skill_status_change_end( bl, SC_HIDDING, -1);
			skill_status_change_end( bl, SC_CLOAKING, -1);
			if( type==SC_RUWACH && battle_check_target( src,bl, BCT_ENEMY ))
				skill_attack(BF_MAGIC,src,src,bl,(type==SC_SIGHT)?10:24,1,tick,0);
		}

		break;
	}
	return 0;
}

/*==========================================
 * ステータス異常終了
 *------------------------------------------
 */
int skill_status_change_end( struct block_list* bl , int type,int tid )
{
	struct status_change* sc_data;
	int opt_flag=0;
	short *sc_count, *option, *opt1, *opt2;
	
	sc_data=battle_get_sc_data(bl);
	sc_count=battle_get_sc_count(bl);
	option=battle_get_option(bl);
	opt1=battle_get_opt1(bl);
	opt2=battle_get_opt2(bl);
	
	if(bl->type!=BL_PC && bl->type!=BL_MOB) {
		printf("skill_status_change_end: neither MOB nor PC !\n");
		return 0;
	}
	
	
	if((*sc_count)>0 && sc_data[type].timer!=-1 &&
		(sc_data[type].timer==tid || tid==-1) ){
		
		if(tid==-1)	/* タイマから呼ばれていないならタイマ削除をする */
			delete_timer(sc_data[type].timer,skill_status_change_timer);
	
		/* 該当の異常を正常に戻す */
		sc_data[type].timer=-1;
		(*sc_count)--;

		if(bl->type==BL_PC && (type<64 || type == SC_EXPLOSIONSPIRITS))	/* アイコン消去 */
			clif_status_change(bl,type,0);

		switch(type){	/* 正常に戻るときなにか処理が必要 */
		case SC_STONE:
		case SC_FREEZE:
		case SC_STAN:
		case SC_SLEEP:
			*opt1=0;
			opt_flag=1;
			break;

		case SC_POISON:
		case SC_CURSE:
		case SC_SILENCE:
		case SC_BLIND:
			*opt2&=~(1<<(type-SC_POISON));
			opt_flag=1;
			break;

		case SC_HIDDING:
		case SC_CLOAKING:
			*option&=~((type==SC_HIDDING)?2:4);
			opt_flag =1 ;
			break;

		case SC_SIGHT:
		case SC_RUWACH:
			*option&=~1;
			opt_flag=1;
			break;
		}
		
		if(opt_flag)	/* optionの変更を伝える */
			clif_changeoption(bl);

		if(bl->type==BL_PC)
			pc_calcstatus((struct map_session_data *)bl,0);	/* ステータス再計算 */
	}

	return 0;
}
/*==========================================
 * ステータス異常終了タイマー
 *------------------------------------------
 */
int skill_status_change_timer(int tid, unsigned int tick, int id, int data)
{
	int type=data;
	struct block_list *bl;
	struct map_session_data *sd=NULL;
	struct status_change *sc_data;
	short *sc_count;
	
	bl=map_id2bl(id);
	if(bl==NULL)
		return 0;
	
	if(bl->type==BL_PC)
		sd=(struct map_session_data *)bl;

	sc_data=battle_get_sc_data(bl);
	sc_count=battle_get_sc_count(bl);
	
	switch(type){	/* 特殊な処理になる場合 */
	case SC_MAXIMIZEPOWER:	/* マキシマイズパワー */
	case SC_CLOAKING:		/* クローキング */
		if(sd){
			if( sd->status.sp > 0 ){	/* SP切れるまで持続 */
				sd->status.sp--;
				clif_updatestatus(sd,SP_SP);
				sc_data[type].timer=add_timer(	/* タイマー再設定 */
					sc_data[type].val2+tick, skill_status_change_timer,
					bl->id, data);
				return 0;
			}
		}
		break;

	case SC_HIDDING:		/* ハイディング */
		if(sd){		/* SPがあって、時間制限の間は持続 */
			if( sd->status.sp > 0 && (--sc_data[type].val2)>0 ){
				if(sc_data[type].val2 % (sc_data[type].val1+3) ==0 ){
					sd->status.sp--;
					clif_updatestatus(sd,SP_SP);
				}
				sc_data[type].timer=add_timer(	/* タイマー再設定 */
					1000+tick, skill_status_change_timer,
					bl->id, data);
				return 0;
			}
		}
		break;

	case SC_SIGHT:	/* サイト */
	case SC_RUWACH:	/* ルアフ */
		{
			const int range=AREA_SIZE;
			map_foreachinarea( skill_status_change_timer_sub,
				bl->m, bl->x-range, bl->y-range, bl->x+range,bl->y+range,0,
				bl,type,tick);
	
			if( (--sc_data[type].val2)>0 ){
				sc_data[type].timer=add_timer(	/* タイマー再設定 */
					250+tick, skill_status_change_timer,
					bl->id, data);
				return 0;
			}
		}
		break;

	case SC_PROVOKE:	/* プロボック/オートバーサーク */
		if(sc_data[type].val2!=0){	/* オートバーサーク（１秒ごとにHPチェック） */
			if(sd!=NULL && sd->status.hp>sd->status.max_hp/4)	/* 停止 */
				break;
			if(sc_data[type].timer==tid)
				sc_data[type].timer=add_timer( 1000+tick,
					skill_status_change_timer, bl->id, data );
		}
		break;

	case SC_WATERBALL:	/* ウォーターボール */
		{
			struct block_list *target=map_id2bl(sc_data[type].val2);
			if(target==NULL || target->prev==NULL)
				break;
			skill_attack(BF_MAGIC,bl,bl,target,86,sc_data[type].val1,tick,0);
			if((--sc_data[type].val3)>0)
				sc_data[type].timer=add_timer( 100+tick,
					skill_status_change_timer, bl->id, data );
		}
		break;

	/* 時間切れ無し？？ */
	case SC_AETERNA:
	case SC_TRICKDEAD:
	case SC_STONE:
	case SC_BLIND:
	case SC_RIDING:
	case SC_FALCON:
	case SC_WEIGHT50:
	case SC_WEIGHT90:
		if(sc_data[type].timer==tid)
			sc_data[type].timer=add_timer( 1000*600+tick,
				skill_status_change_timer, bl->id, data );
		return 0;
	}

	return skill_status_change_end( bl,type,tid );
}
/*==========================================
 * ステータス異常開始
 *------------------------------------------
 */
int skill_status_change_start(struct block_list *bl,int type,int val1,int val2)
{
	struct map_session_data *sd = NULL;
	struct mob_data *md = NULL;
	struct status_change* sc_data;
	int tick = 0;
	short *sc_count, *option, *opt1, *opt2;
	int opt_flag = 0;
	int val3=0,val4=val2;
	
	sc_data=battle_get_sc_data(bl);
	sc_count=battle_get_sc_count(bl);
	option=battle_get_option(bl);
	opt1=battle_get_opt1(bl);
	opt2=battle_get_opt2(bl);
	
	
	if(bl->type==BL_MOB){
		md=(struct mob_data *)bl;
		if(mob_db[md->class].mode & 0x20 &&
			(type==SC_STONE || type==SC_FREEZE || type==SC_STAN || type==SC_SLEEP)){
			/* ボスには効かない */
			return 0;
		}
		if(type==SC_STONE || type==SC_FREEZE || type==SC_STAN || type==SC_SLEEP)
			mob_stop_walking(md);
	}else if(bl->type==BL_PC){
		sd=(struct map_session_data *)bl;
		
		if(SC_STONE<=type && type<=SC_BLIND){	/* カードによる耐性 */
			if(sd->reseff[type-SC_STONE] && rand()%100<sd->reseff[type-SC_STONE]){
				printf("skill_sc_start: cardによる異常耐性発動\n");
				return 0;
			}
		}
		if(type==SC_STONE || type==SC_FREEZE || type==SC_STAN || type==SC_SLEEP)
			pc_stop_walking(sd);
	}else{
		printf("skill_status_change_start: neither MOB nor PC !\n");
		return 0;
	}
	
	
	if(sc_data[type].timer != -1){	/* すでに同じ異常になっている場合タイマ解除 */
		(*sc_count)--;
		delete_timer(sc_data[type].timer, skill_status_change_timer);
		sc_data[type].timer = -1;
	}
	
	switch(type){	/* 異常の種類ごとの処理 */
		case SC_PROVOKE:			/* プロボック */
			if(val2==0)	tick = 1000 * 30;
			else		tick = 1000;/* (オートバーサーク) */
			break;
		case SC_ENDURE:				/* インデュア */
			tick = 1000 * (7 + val1*3);
			break;
		case SC_CONCENTRATE:		/* 集中力向上 */
			tick = 1000 * (40 + val1 * 20);
			break;
		case SC_BLESSING:			/* ブレッシング */
			tick = 1000 * (40 + val1 * 20);
			break;
		case SC_ANGELUS:			/* アンゼルス */
			tick = 1000 * (30 * val1);
			break;
		case SC_INCREASEAGI:		/* 速度上昇 */
			tick = 1000 * (40 + val1 * 20);
			break;
		case SC_DECREASEAGI:		/* 速度減少 */
			tick = 1000 * (30 + val1 * 10);
			break;
		case SC_TWOHANDQUICKEN:		/* 2HQ */
			tick = 1000 * (30 * val1);
			break;
		case SC_ADRENALINE:			/* アドレナリンラッシュ */
			tick = 1000 * (30 * val1);
			break;
		case SC_WEAPONPERFECTION:	/* ウェポンパーフェクション */
			tick = 1000 * (30 * val1);
			break;
		case SC_OVERTHRUST:			/* オーバースラスト */
			tick = 1000 * (20 * val1);
			break;
		case SC_MAXIMIZEPOWER:		/* マキシマイズパワー(SPが1減る時間,val2にも) */
			tick = 1000 * val1;
			val2 = tick;
			break;
		case SC_ENCPOISON:			/* エンチャントポイズン */
			tick = 1000 * (15 + val1 * 15);
			val2=((val1 - 1) / 2) + 3;	/* 毒付与確率 */
			if( sc_data[SC_ASPERSIO].timer!=-1 )	/* アスペ解除 */
				skill_status_change_end(bl,SC_ASPERSIO,-1);
			break;
		case SC_IMPOSITIO:			/* インポシティオマヌス */
			tick = 1000 * 60;
			break;
		case SC_SUFFRAGIUM:			/* サフラギム */
			tick = 1000 * (40 - val1 * 10);
			break;
		case SC_ASPERSIO:			/* アスペルシオ */
			tick = 1000 * (30 + val1 * 30);
			if( sc_data[SC_ENCPOISON].timer!=-1 )	/* EP解除 */
				skill_status_change_end(bl,SC_ENCPOISON,-1);
			break;
		case SC_BENEDICTIO:			/* 聖体 */
			tick = 1000 * 40 * val1;
			break;
		case SC_KYRIE:				/* キリエエレイソン */
			tick = 1000 * 120;
			val2 = ((sd)?sd->status.max_hp:mob_db[md->class].max_hp) *
				 (val1 * 2 + 10) / 100;/* 耐久度 */
			val1 = (val1 / 2 + 5);	/* 回数 */
			break;
		case SC_MAGNIFICAT:			/* マグニフィカート */
			tick = 1000 * 20 * val1;
			break;
		case SC_GLORIA:				/* グロリア */
			tick = 1000 * (5 + val1 * 5);
			break;
		case SC_AETERNA:			/* エーテルナ */
			tick = 1000 * 600;		/* とりあえず10分にしてみる（後はタイマ処理で延長する） */
			break;
		case SC_ENERGYCOAT:			/* エナジーコート */
			tick = 1000 * 6 * 50;
			break;
		case SC_LOUD:				/* ラウドボイス */
			tick = 1000 * 60 * 5;
			break;
		case SC_TRICKDEAD:			/* 死んだふり */
			tick = 1000 * 600;		/* とりあえず10分にしてみる（後はタイマ処理で延長する） */
			break;
		case SC_QUAGMIRE:			/* クァグマイア */
			tick = 5000 * val1;
			break;
		case SC_SIGNUMCRUCIS:		/* シグナムクルシス */
			tick = 5000 * 60 * 25;
			break;
		case SC_FLAMELAUNCHER:		/* フレームランチャー */
			tick = 1000 * (15 + val1 * 15);
			break;
		case SC_FROSTWEAPON:		/* フロストウェポン */
			tick = 1000 * (15 + val1 * 15);
			break;
		case SC_LIGHTNINGLOADER:	/* ライトニングローダー */
			tick = 1000 * (15 + val1 * 15);
			break;
		case SC_SEISMICWEAPON:		/* サイズミックウェポン */
			tick = 1000 * (15 + val1 * 15);
			break;
		case SC_PROVIDENCE:			/* プロヴィデンス */
			tick = 1000 * 180;
			val2=val1*5;
			break;
		case SC_DEFENDER:			/* ディフェンダー */
			tick = 1000 * 180;
			val2=val1*15+5;
			break;
		case SC_SPEARSQUICKEN:		/* スピアクイッケン */
			tick = 1000 * 300;
			val2=80-val1;
			break;
		case SC_ASSNCROS:			/* 夕陽のアサシンクロス */
			tick = 1000 * 120;
			val2=90-val1;
			break;
		case SC_WHISTLE:			/* 口笛 */
			tick = 1000 * 60;
			break;
		case SC_APPLEIDUN:			/* イドゥンの林檎 */
			tick = 1000 * 60 * 3;
			val2 = val1*2+5;
			break;
		case SC_SERVICE4U:			/* サービスフォーユー */
			tick = 1000 * 60 * 3;
			val2 = val1+10;
			break;
		case SC_HUMMING:			/* ハミング */
			tick = 1000 * 60;
			val2 = val1*2;
			break;
		case SC_DONTFORGETME:		/* 私を忘れないで */
			tick = 1000 * 180;
			val2 = 100+val1*3;
			val3 = 100+val1*2;
			break;
		case SC_FORTUNE:			/* 幸運のキス */
			tick = 1000 * 60 * 2;
			break;
		case SC_DRUMBATTLE:			/* 戦太鼓の響き */
			tick = 1000 * 60;
			val2 = (val1+1)*25;
			val3 = (val1+1)*2;
			break;
		case SC_ETERNALCHAOS:		/* エターナルカオス */
			tick = 1000* 60;
			break;
		case SC_NIBELUNGEN:			/* ニーベルングの指輪 */
			tick = 1000 * 60;
			val2 = (val1+2)*50;
			break;
		case SC_SIEGFRIED:			/* 不死身のジークフリード */
			tick = 1000 * 60;
			val2 = (val1+3)*10;
			break;
		case SC_EXPLOSIONSPIRITS:	// 爆裂波動
			tick = 1000 * 60 * 3;
			val2 = 75 + 25*val1;
			break;
		case SC_STEELBODY:			// 金剛
			tick = 1000 * 30 * val1;
			break;

		/* option1 */
		case SC_SPEEDPOTION0:		/* 増速ポーション */
		case SC_SPEEDPOTION1:
		case SC_SPEEDPOTION2:
			tick = 1000 * 60 * 30;
			val2 = 100- 5*(2+type-SC_SPEEDPOTION0);
			break;

		case SC_STONE:				/* 石化 */
			if( (tick=val2)<=0 )
				tick = 1000 * 600;		/* とりあえず10分にしてみる（後はタイマ処理で延長する） */
			break;
		case SC_SLEEP:				/* 睡眠 */
			tick = val2;
			break;
		case SC_FREEZE:				/* 凍結 */
			if( (tick=val2)<=0 )
				tick = 1000 * 3 * val1;
			break;
		case SC_STAN:				/* スタン（val2にミリ秒セット） */
			tick = val2;
			break;

		/* option2 */
		case SC_POISON:				/* 毒 */
			if( (tick=val2)<=0 )
				tick = 1000 * (10 + rand()%50);	/* 適当な長さ */
			break;
		case SC_SILENCE:			/* 沈黙（レックスデビーナ） */
			if( (tick=val2)<=0 )
				tick = 1000 * ( (val1>6)?60: 25 + val1 * 5);
			break;
		case SC_BLIND:				/* 暗黒 */
			if( (tick=val2)<=0 )
				tick = 1000*600;
			break;
			
		/* option */
		case SC_HIDDING:		/* ハイディング */
			tick = 1000;			/* １秒ずつ時間チェック */
			val2 = 30 * val1;		/* 持続時間 */
			break;
		case SC_CLOAKING:		/* クローキング */
			tick = 1000 * (3 + val1 );	/* SPが1減る時間(val2にもセット) */
			val2 = tick;
			break;
		case SC_SIGHT:			/* サイト/ルアフ */
		case SC_RUWACH:
			tick=10;
			val2=40;	/* 250*40=10秒 */
			break;

		/* セーフティウォール、ニューマ */
		case SC_SAFETYWALL:	case SC_PNEUMA:
			tick=((struct skill_unit *)val2)->group->limit;
			break;

		/* アンクル */
		case SC_ANKLE:
			tick=val2;
			break;

		/* ウォーターボール */
		case SC_WATERBALL:
			tick=100;
			val3= (val1|1)*(val1|1)-1;
			break;

		/* スキルじゃない/時間に関係しない */
		case SC_RIDING:	case SC_FALCON:	case SC_WEIGHT50:	case SC_WEIGHT90:
			tick=600*1000;
			break;

		default:
			printf("UnknownStatusChange [%d]\n", type);
			return 0;
	}

	if(bl->type==BL_PC && (type<64 || type == SC_EXPLOSIONSPIRITS))	/* アイコン表示パケット */
		clif_status_change(bl,type,1);

	/* optionの変更 */
	switch(type){
		case SC_STONE:	case SC_FREEZE:	case SC_STAN:	case SC_SLEEP:
			battle_stopattack(bl);	/* 攻撃停止 */
			{
				/* 同時に掛からないステータス異常を解除 */
				int i;
				for(i = SC_STONE; i <= SC_SLEEP; i++){
					if(sc_data[i].timer != -1){
						(*sc_count)--;
						delete_timer(sc_data[i].timer, skill_status_change_timer);
						sc_data[i].timer = -1;
					}
				}
			}
			*opt1 = type - SC_STONE + 1;
			opt_flag = 1;
			break;
		case SC_POISON:	case SC_CURSE:	case SC_SILENCE:	case SC_BLIND:
			*opt2|= 1<<(type-SC_POISON);
			opt_flag = 1;
			break;
		case SC_HIDDING:	case SC_CLOAKING:
			battle_stopattack(bl);	/* 攻撃停止 */
			*option|= ((type==SC_HIDDING)?2:4);
			opt_flag =1 ;
			break;
		case SC_SIGHT:	case SC_RUWACH:
			*option|=1;
			opt_flag=1;
			break;
	}

	if(opt_flag)	/* optionの変更 */
		clif_changeoption(bl);

	(*sc_count)++;	/* ステータス異常の数 */

	sc_data[type].val1 = val1;
	sc_data[type].val2 = val2;
	sc_data[type].val3 = val3;
	sc_data[type].val4 = val4;
	/* タイマー設定 */
	sc_data[type].timer = add_timer(
		gettick() + tick, skill_status_change_timer, bl->id, type);

	if(bl->type==BL_PC)
		pc_calcstatus(sd,0);	/* ステータス再計算 */

	return 0;
}
/*==========================================
 * ステータス異常全解除
 *------------------------------------------
 */
int skill_status_change_clear(struct block_list *bl)
{
	struct status_change* sc_data;
	short *sc_count, *option, *opt1, *opt2;
	int i;
	
	sc_data=battle_get_sc_data(bl);
	sc_count=battle_get_sc_count(bl);
	option=battle_get_option(bl);
	opt1=battle_get_opt1(bl);
	opt2=battle_get_opt2(bl);
	
	if(*sc_count == 0)
		return 0;
	for(i = 0; i < MAX_STATUSCHANGE; i++){
		if(sc_data[i].timer != -1){	/* 異常があるならタイマーを削除する */
			delete_timer(sc_data[i].timer, skill_status_change_timer);
			sc_data[i].timer = -1;
			if( bl->type==BL_PC && i<64 )
				clif_status_change(bl,i,0);
		}
	}
	*sc_count = 0;
	*opt1 = 0;
	*opt2 = 0;
	*option &= 0xfff8;

	if( bl->type==BL_PC )
		clif_changeoption(bl);

	return 0;
}


/* クローキング検査（周りに移動不可能地帯があるか） */
int skill_check_cloaking(struct block_list *bl)
{
	static int dx[]={-1, 0, 1,-1, 1,-1, 0, 1};
	static int dy[]={-1,-1,-1, 0, 0, 1, 1, 1};
	int end=1,i;
	for(i=0;i<sizeof(dx)/sizeof(dx[0]);i++){
		int c=map_getcell(bl->m,bl->x+dx[i],bl->y+dy[i]);
		if(c==1 || c==5) end=0;
	}
	if(end){
		skill_status_change_end(bl, SC_CLOAKING, -1);
		*battle_get_option(bl)&=~4;	/* 念のための処理 */
	}
	return end;
}





/*
 *----------------------------------------------------------------------------
 * スキルユニット
 *----------------------------------------------------------------------------
 */


/*==========================================
 * スキルユニット初期化
 *------------------------------------------
 */
struct skill_unit *skill_initunit(struct skill_unit_group *group,int idx,int x,int y)
{
	struct skill_unit *unit=&group->unit[idx];
	
	if(!unit->alive)
		group->alive_count++;
	
	unit->bl.id=map_addobject(&unit->bl);
	unit->bl.type=BL_SKILL;
	unit->bl.m=group->map;
	unit->bl.x=x;
	unit->bl.y=y;
	unit->group=group;
	unit->val1=unit->val2=0;
	unit->alive=1;

	map_addblock(&unit->bl);
	clif_skill_setunit(unit);
	return unit;
}

int skill_unit_timer_sub_ondelete( struct block_list *bl, va_list ap );
/*==========================================
 * スキルユニット削除
 *------------------------------------------
 */
int skill_delunit(struct skill_unit *unit)
{
	struct skill_unit_group *group=unit->group;
	int range;
/*	printf("delunit %d\n",unit->bl.id); */

	if(!unit->alive || group==NULL)
		return 0;

	/* onlimitイベント呼び出し */
	skill_unit_onlimit( unit,gettick() );

	/* ondeleteイベント呼び出し */
	range=unit->group->range;
	map_foreachinarea( skill_unit_timer_sub_ondelete, unit->bl.m,
		unit->bl.x-range,unit->bl.y-range,unit->bl.x+range,unit->bl.y+range,0,
		&unit->bl,gettick() );

	clif_skill_delunit(unit);
	
	
	unit->group=NULL;
	unit->alive=0;
	map_delobjectnofree(unit->bl.id);
	if(group->alive_count>0 && (--group->alive_count)<=0)
		skill_delunitgroup(group);
	
	return 0;
}
/*==========================================
 * スキルユニットグループ初期化
 *------------------------------------------
 */
static int skill_unit_group_newid=10;
struct skill_unit_group *skill_initunitgroup(struct block_list *src,
	int count,int skillid,int skilllv,int unit_id)
{
	int i;
	struct skill_unit_group *group=NULL, *list=NULL;
	int maxsug=0;
	
	if(src->type==BL_PC){
		list=((struct map_session_data *)src)->skillunit;
		maxsug=MAX_SKILLUNITGROUP;
	}else if(src->type==BL_MOB){
		list=((struct mob_data *)src)->skillunit;
		maxsug=MAX_MOBSKILLUNITGROUP;
	}
	if(list){
		for(i=0;i<maxsug;i++)	/* 空いているもの検索 */
			if(list[i].group_id==0){
				group=&list[i];
				break;
			}

		if(group==NULL){	/* 空いてないので古いもの検索 */
			int j=0;
			unsigned maxdiff=0,x,tick=gettick();
			for(i=0;i<maxsug;i++)
				if((x=DIFF_TICK(tick,list[i].tick))>maxdiff){
					maxdiff=x;
					j=i;
				}
			skill_delunitgroup(&list[j]);
			group=&list[j];
		}
	}

	if(group==NULL){
		printf("skill_initunitgroup: error unit group !\n");
		exit(0);
	}

	group->src_id=src->id;
	group->party_id=battle_get_party_id(src);
	group->guild_id=battle_get_guild_id(src);
	group->group_id=skill_unit_group_newid++;
	if(skill_unit_group_newid<=0)
		skill_unit_group_newid=10;

	group->unit=malloc(sizeof(struct skill_unit)*count);
	if(group->unit==NULL){
		printf("skill_initunitgroup: out of memory! \n");
		exit(0);
	}
	memset(group->unit,0,sizeof(struct skill_unit)*count);

	group->unit_count=count;
	group->val1=group->val2=0;
	group->skill_id=skillid;
	group->skill_lv=skilllv;
	group->unit_id=unit_id;
	group->map=src->m;
	group->range=0;
	group->limit=10000;
	group->interval=1000;
	group->tick=gettick();
	memset(group->vallist,0,sizeof(group->vallist));
	group->valstr=NULL;

	return group;
}


/*==========================================
 * スキルユニットグループ削除
 *------------------------------------------
 */
int skill_delunitgroup(struct skill_unit_group *group)
{
	int i;
	if(group->unit_count<=0)
		return 0;

/*	printf("delunitgroup %d\n",group->group_id); */

	group->alive_count=-1;
	if(group->unit!=NULL){
		for(i=0;i<group->unit_count;i++)
			if(group->unit[i].alive)
				skill_delunit(&group->unit[i]);
	}
	if(group->valstr!=NULL){
		map_freeblock(group->valstr);
		group->valstr=NULL;
	}
	
	map_freeblock(group->unit);	/* free()の替わり */
	group->unit=NULL;
	group->src_id=0;
	group->group_id=0;
	group->unit_count=0;
	return 0;
}


/*==========================================
 * スキルユニットグループ全削除
 *------------------------------------------
 */
int skill_clear_unitgroup(struct block_list *src)
{
	struct skill_unit_group *group=NULL;
	int maxsug=0;
	if(src->type==BL_PC){
		group=((struct map_session_data *)src)->skillunit;
		maxsug=MAX_SKILLUNITGROUP;
	}else if(src->type==BL_MOB){
		group=((struct mob_data *)src)->skillunit;
		maxsug=MAX_MOBSKILLUNITGROUP;
	}
	if(group){
		int i;
		for(i=0;i<maxsug;i++)
			if(group[i].group_id>0)
				skill_delunitgroup(&group[i]);
	}
	return 0;
}


/*==========================================
 * スキルユニットグループの被影響tick検索
 *------------------------------------------
 */
struct skill_unit_group_tickset *skill_unitgrouptickset_search(
	struct block_list *bl,int group_id)
{
	int i,j=0,k,s=group_id%MAX_SKILLUNITGROUPTICKSET;
	struct skill_unit_group_tickset *set=NULL;
	if(bl->type==BL_PC){
		set=((struct map_session_data *)bl)->skillunittick;
	}else{
		set=((struct mob_data *)bl)->skillunittick;
	}
	if(set==NULL)
		return 0;
	for(i=0;i<MAX_SKILLUNITGROUPTICKSET;i++)
		if( set[(k=(i+s)%MAX_SKILLUNITGROUPTICKSET)].group_id == group_id )
			return &set[k];
		else if( set[k].group_id==0 )
			j=k;

	return &set[j];
}


/*==========================================
 * スキルユニットグループの被影響tick削除
 *------------------------------------------
 */
int skill_unitgrouptickset_delete(struct block_list *bl,int group_id)
{
	int i,s=group_id%MAX_SKILLUNITGROUPTICKSET;
	struct skill_unit_group_tickset *set=NULL,*ts;
	if(bl->type==BL_PC){
		set=((struct map_session_data *)bl)->skillunittick;
	}else{
		set=((struct mob_data *)bl)->skillunittick;
	}
	
	if(set!=NULL){
	
		for(i=0;i<MAX_SKILLUNITGROUPTICKSET;i++)
			if( (ts=&set[(i+s)%MAX_SKILLUNITGROUPTICKSET])->group_id == group_id )
				ts->group_id=0;

	}
	return 0;
}


/*==========================================
 * スキルユニットタイマー発動処理用(foreachinarea)
 *------------------------------------------
 */
int skill_unit_timer_sub_onplace( struct block_list *bl, va_list ap )
{
	struct block_list *src;
	unsigned int tick;
	src=va_arg(ap,struct block_list*);
	tick=va_arg(ap,unsigned int);


	if( ((struct skill_unit *)src)->alive &&
		battle_check_target(src,bl,((struct skill_unit *)src)->group->target_flag )>0 )
			skill_unit_onplace( (struct skill_unit *)src, bl, tick );
	return 0;
}


/*==========================================
 * スキルユニットタイマー削除処理用(foreachinarea)
 *------------------------------------------
 */
int skill_unit_timer_sub_ondelete( struct block_list *bl, va_list ap )
{
	struct block_list *src;
	unsigned int tick;
	src=va_arg(ap,struct block_list*);
	tick=va_arg(ap,unsigned int);

	if( ((struct skill_unit *)src)->alive &&
		battle_check_target(src,bl,((struct skill_unit *)src)->group->target_flag )>0 )
			skill_unit_ondelete( (struct skill_unit *)src, bl, tick );
	return 0;
}


/*==========================================
 * スキルユニットタイマー処理用(foreachobject)
 *------------------------------------------
 */
int skill_unit_timer_sub( struct block_list *bl, va_list ap )
{
	struct skill_unit *unit=(struct skill_unit *)bl;
	struct skill_unit_group *group;
	int range;
	unsigned int tick;
	tick=va_arg(ap,unsigned int);

	if(!unit->alive)
		return 0;

	group=unit->group;
	range=(unit->range!=0)?unit->range:group->range;

	/* onplaceイベント呼び出し */
	if(unit->alive && unit->range>=0)
		map_foreachinarea( skill_unit_timer_sub_onplace, bl->m,
			bl->x-range,bl->y-range,bl->x+range,bl->y+range,0,
			bl,tick );

	/* 時間切れ削除 */
	if(unit->alive &&
		(DIFF_TICK(tick,group->tick)>=group->limit ||
		 DIFF_TICK(tick,group->tick)>=unit->limit) ){
		skill_delunit(unit);
	}

	return 0;
}


/*==========================================
 * スキルユニットタイマー処理
 *------------------------------------------
 */
int skill_unit_timer( int tid,unsigned int tick,int id,int data)
{
	map_foreachobject( skill_unit_timer_sub, BL_SKILL, tick );
	return 0;
}


/*==========================================
 * スキルユニット移動時処理用(foreachinarea)
 *------------------------------------------
 */
int skill_unit_move_sub( struct block_list *bl, va_list ap )
{
	struct skill_unit *unit=(struct skill_unit *)bl;
	struct skill_unit_group *group;
	struct block_list *src;
	int range;
	unsigned int tick;
	src=va_arg(ap,struct block_list*);
	tick=va_arg(ap,unsigned int);

	if(!unit->alive || src->prev==NULL)
		return 0;

	group=unit->group;
	range=(unit->range!=0)?unit->range:group->range;

	if( range<0 || battle_check_target(bl,src,group->target_flag )<=0 )
		return 0;

	if( src->x >= bl->x-range && src->x <= bl->x+range &&
		src->y >= bl->y-range && src->y <= bl->y+range )
		skill_unit_onplace( unit, src, tick );
	else
		skill_unit_onout( unit, src, tick );

	return 0;
}


/*==========================================
 * スキルユニット移動時処理
 *------------------------------------------
 */
int skill_unit_move( struct block_list *bl,unsigned int tick,int range)
{
	if(bl->prev==NULL)
		return 0;

	if(range<5)range=5;
	map_foreachinarea( skill_unit_move_sub,
		bl->m,bl->x-range,bl->y-range,bl->x+range,bl->y+range,BL_SKILL,
		bl,tick );

	return 0;
}


/*----------------------------------------------------------------------------
 * アイテム合成
 *----------------------------------------------------------------------------
 */

/*==========================================
 * アイテム合成可能判定
 *------------------------------------------
 */
int skill_can_produce_mix( struct map_session_data *sd, int nameid, int trigger )
{
	int i,j,equip;

	if(nameid<=0)
		return 0;

	for(i=0;i<MAX_SKILL_PRODUCE_DB;i++){
		if(skill_produce_db[i].nameid == nameid )
			break;
	}
	if( i == MAX_SKILL_PRODUCE_DB )	/* データベースにない */
		return 0;

	if(trigger>=0){
		equip = itemdb_isequip(nameid);
		if(trigger==16){
			if(equip)	/* 溶鉱炉＊武器はだめ */
				return 0;
		}else{
			if(!equip)		/* 鎚＊鉄鉱系はだめ */
				return 0;
			if( itemdb_wlv(nameid) > trigger )	/* 武器Lv判定 */
				return 0;
		}
	}
	if( (j=skill_produce_db[i].req_skill)>0 && pc_checkskill(sd,j)<=0 )
		return 0;		/* スキルが足りない */

	for(j=0;j<5;j++){
		int id,x,y;
		if( (id=skill_produce_db[i].mat_id[j])==0 )	/* これ以上は材料要らない */
			break;
		for(y=0,x=0;y<MAX_INVENTORY;y++)
			if( sd->status.inventory[y].nameid == id )
				x+=sd->status.inventory[y].amount;
		if(x<skill_produce_db[i].mat_amount[j])	/* アイテムが足りない */
			return 0;
	}
	return i+1;
}


/*==========================================
 * アイテム合成可能判定
 *------------------------------------------
 */
int skill_produce_mix( struct map_session_data *sd,
	int nameid, int slot1, int slot2, int slot3 )
{
	int slot[3];
	int i,sc,ele,idx,equip,wlv,make_per;

	if( !(idx=skill_can_produce_mix(sd,nameid,-1)) )	/* 条件不足 */
		return 0;
	idx--;
	slot[0]=slot1;
	slot[1]=slot2;
	slot[2]=slot3;

	/* 埋め込み処理 */
	for(i=0,sc=0,ele=0;i<3;i++){
		int j;
		if( slot[i]<=0 )
			continue;
		j = pc_search_inventory(sd,slot[i]);
		if(j < 0)	/* 不正パケット(アイテム存在)チェック */
			continue;
		if(slot[i]==1000){	/* 星のかけら */
			pc_delitem(sd,j,1,1);
			sc++;
		}
		if(slot[i]>=994 && slot[i]<=997 && ele==0){	/* 属性石 */
			static const int ele_table[4]={3,1,4,2};
			pc_delitem(sd,j,1,1);
			ele=ele_table[slot[i]-994];
		}
	}

	/* 材料消費 */
	for(i=0;i<5;i++){
		int j,id,x;
		if( (id=skill_produce_db[idx].mat_id[i])<=0 )
			break;
		x=skill_produce_db[idx].mat_amount[i];	/* 必要な個数 */
		do{	/* ２つ以上のインデックスにまたがっているかもしれない */
			int y=0;
			j = pc_search_inventory(sd,id);
			
			if(j >= 0){
				y = sd->status.inventory[j].amount;
				if(y>x)y=x;	/* 足りている */
				pc_delitem(sd,j,y,0);
			}else
				printf("skill_produce_mix: material item error\n");

			x-=y;	/* まだ足りない個数を計算 */
		}while( j>=0 && x>0 );	/* 材料を消費するか、エラーになるまで繰り返す */
	}

	/* 確率判定 */
	equip = itemdb_isequip(nameid);
	if(!equip) {
		if(nameid == 998)
			make_per = 2000 + sd->status.base_level*30 + sd->paramc[4]*20 + sd->paramc[5]*10 + pc_checkskill(sd,skill_produce_db[idx].req_skill)*600;
		else if(nameid == 985)
			make_per = 1000 + sd->status.base_level*30 + sd->paramc[4]*20 + sd->paramc[5]*10 + (pc_checkskill(sd,skill_produce_db[idx].req_skill)-1)*500;
		else
			make_per = 1000 + sd->status.base_level*30 + sd->paramc[4]*20 + sd->paramc[5]*10 + pc_checkskill(sd,skill_produce_db[idx].req_skill)*500;
	}
	else {
		int add_per;
		if(pc_search_inventory(sd,989) >= 0) add_per = 750;
		else if(pc_search_inventory(sd,988) >= 0) add_per = 500;
		else if(pc_search_inventory(sd,987) >= 0) add_per = 250;
		else if(pc_search_inventory(sd,986) >= 0) add_per = 0;
		else add_per = -500;
		if(ele) add_per -= 500;
		add_per -= sc*500;
		wlv = itemdb_wlv(nameid);
		make_per = ((250 + sd->status.base_level*15 + sd->paramc[4]*10 + sd->paramc[5]*5 + pc_checkskill(sd,skill_produce_db[idx].req_skill)*500 +
			add_per) * (100 - (wlv - 1)*20))/100 + pc_checkskill(sd,107)*100 + ((wlv >= 3)? pc_checkskill(sd,97)*100 : 0);
	}

	if(make_per < 1) make_per = 1;

	if( battle_config.wp_rate!=100 )	/* 確率補正 */
		make_per=make_per*battle_config.wp_rate/100;

	/* debug code */
	/*printf("make success percent = %.2lf\n",(double)make_per/100.); */
	
	if(rand()%10000 < make_per){
		/* 成功 */
		struct item tmp_item;
		memset(&tmp_item,0,sizeof(tmp_item));
		tmp_item.nameid=nameid;
		tmp_item.amount=1;
		tmp_item.identify=1;
		if(equip){	/* 武器の場合 */
			tmp_item.card[0]=0x00ff; /* 製造武器フラグ */
			tmp_item.card[1]=((sc*5)<<8)+ele;	/* 属性とつよさ */
			*((unsigned long *)(&tmp_item.card[2]))=sd->char_id;	/* キャラID */
		}
		clif_produceeffect(sd,0,nameid);/* 製造エフェクトパケット */
		clif_misceffect(&sd->bl,3); /* 他人にも成功を通知（精錬成功エフェクトと同じでいいの？） */
		if(pc_additem(sd,&tmp_item,1))
			map_addflooritem(&tmp_item,1,sd->bl.m,sd->bl.x,sd->bl.y);
	}
	else {
		/* 失敗 */
		clif_produceeffect(sd,1,nameid);/* 製造エフェクトパケット */
		clif_misceffect(&sd->bl,2); /* 他人にも失敗を通知 */
	}
	return 0;
}


/*----------------------------------------------------------------------------
 * 初期化系
 */

/*==========================================
 * スキル関係ファイル読み込み
 * skill_db.txt スキルデータ
 * cast_db.txt スキルの詠唱時間とディレイデータ
 * produce_db.txt アイテム作成スキル用データ
 *------------------------------------------
 */
int skill_readdb(void)
{
	int i,j,k;
	FILE *fp;
	char line[1024],*p;

	/* スキルデータベース */
	memset(skill_db,0,sizeof(skill_db));
	fp=fopen("db/skill_db.txt","r");
	if(fp==NULL){
		printf("can't read db/skill_db.txt\n");
		return 1;
	}
	while(fgets(line,1020,fp)){
		char *split[50], *split2[16];
		if(line[0]=='/' && line[1]=='/')
			continue;
		for(j=0,p=line;j<10 && p;j++){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(split[9]==NULL || j<10)
			continue;

		i=atoi(split[0]);
		if(i<0 || i>MAX_SKILL_DB)
			continue;

/*		printf("skill id=%d\n",i); */
		skill_db[i].range=atoi(split[1]);
		skill_db[i].hit=atoi(split[2]);
		skill_db[i].inf=atoi(split[3]);
		skill_db[i].pl=atoi(split[4]);
		skill_db[i].nk=atoi(split[5]);
		skill_db[i].max=atoi(split[6]);

		memset(split2,0,sizeof(split2));
		for(j=0,p=split[7];j<10 && p;j++){
			split2[j]=p;
			p=strchr(p,':');
			if(p) *p++=0;
		}
		for(k=0;k<10;k++)
			skill_db[i].sp[k]=(split2[k]?atoi(split2[k]):atoi(split2[0]));

		memset(split2,0,sizeof(split2));
		for(j=0,p=split[8];j<10 && p;j++){
			split2[j]=p;
			p=strchr(p,':');
			if(p) *p++=0;
		}
		for(k=0;k<10;k++)
			skill_db[i].num[k]=(split2[k]?atoi(split2[k]):atoi(split2[0]));
		
		skill_db[i].inf2=atoi(split[9]);

	}
	fclose(fp);
	printf("read db/skill_db.txt done\n");

	/* キャスティングデータベース */
/*	for(i=0;i<MAX_SKILL;i++){
		for(j=0;j<10;j++){
			skill_db[i].cast[j]=1000;
			skill_db[i].delay[j]=1000;
		}
	}*/
	fp=fopen("db/cast_db.txt","r");
	if(fp==NULL){
		printf("can't read db/cast_db.txt\n");
		return 1;
	}
	while(fgets(line,1020,fp)){
		char *split[50], *split2[16];
		if(line[0]=='/' && line[1]=='/')
			continue;
		for(j=0,p=line;j<3 && p;j++){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(split[2]==NULL || j<3)
			continue;

		i=atoi(split[0]);
		if(i<0 || i>MAX_SKILL_DB)
			continue;

		memset(split2,0,sizeof(split2));
		for(j=0,p=split[1];j<10 && p;j++){
			split2[j]=p;
			p=strchr(p,':');
			if(p) *p++=0;
		}
		for(k=0;k<10;k++)
			skill_db[i].cast[k]=(split2[k]?atoi(split2[k]):atoi(split2[0]));

		memset(split2,0,sizeof(split2));
		for(j=0,p=split[2];j<10 && p;j++){
			split2[j]=p;
			p=strchr(p,':');
			if(p) *p++=0;
		}
		for(k=0;k<10;k++)
			skill_db[i].delay[k]=(split2[k]?atoi(split2[k]):atoi(split2[0]));
	}
	fclose(fp);
	printf("read db/cast_db.txt done\n");

	memset(skill_produce_db,0,sizeof(skill_produce_db));
	fp=fopen("db/produce_db.txt","r");
	if(fp==NULL){
		printf("can't read db/produce_db.txt\n");
		return 1;
	}
	k=0;
	while(fgets(line,1020,fp)){
		char *split[16];
		int x,y;
		if(line[0]=='/' && line[1]=='/')
			continue;
		memset(split,0,sizeof(split));
		if(line[0]=='/' && line[1]=='/')
			continue;
		for(j=0,p=line;j<13 && p;j++){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(split[0]==NULL)
			continue;
		i=atoi(split[0]);
		if(i<=0)
			continue;

		skill_produce_db[k].nameid=i;
		skill_produce_db[k].req_skill=atoi(split[2]);

		for(x=3,y=0;split[x] && split[x+1] && y<5;x+=2,y++){
			skill_produce_db[k].mat_id[y]=atoi(split[x]);
			skill_produce_db[k].mat_amount[y]=atoi(split[x+1]);
		}
		k++;
		if(k >= MAX_SKILL_PRODUCE_DB)
			break;
	}
	fclose(fp);
	printf("read db/produce_db.txt done (count=%d)\n",k);

	return 0;
}


/*==========================================
 * スキル関係初期化処理
 *------------------------------------------
 */
int do_init_skill(void)
{
	skill_readdb();

	add_timer_interval(gettick()+1000,skill_unit_timer,0,0,SKILLUNITTIMER_INVERVAL);
	add_timer_func_list(skill_unit_timer,"skill_unit_timer");
	add_timer_func_list(skill_timerskill,"skill_timerskill");

	return 0;
}


