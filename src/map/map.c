// $Id: map.c,v 1.3 2004/09/15 00:20:52 running_pinata Exp $
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#ifndef _WIN32
	#include <netdb.h>
#endif

#include "core.h"
#include "timer.h"
#include "db.h"
#include "grfio.h"
#include "nullpo.h"
#include "malloc.h"
#include "map.h"
#include "chrif.h"
#include "clif.h"
#include "intif.h"
#include "npc.h"
#include "pc.h"
#include "mob.h"
#include "chat.h"
#include "itemdb.h"
#include "storage.h"
#include "skill.h"
#include "trade.h"
#include "party.h"
#include "battle.h"
#include "script.h"
#include "guild.h"
#include "pet.h"
#include "atcommand.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

// 極力 staticでローカルに収める
static struct dbt * id_db;
static struct dbt * map_db;
static struct dbt * nick_db;
static struct dbt * charid_db;

static int users;
static struct block_list *object[MAX_FLOORITEM];
static int first_free_object_id,last_object_id;

#define block_free_max 1048576
static void *block_free[block_free_max];
static int block_free_count=0,block_free_lock=0;

#define BL_LIST_MAX 1048576
static struct block_list *bl_list[BL_LIST_MAX];
static int bl_list_count = 0;

struct map_data map[MAX_MAP_PER_SERVER];
int map_num=0;

int autosave_interval=DEFAULT_AUTOSAVE_INTERVAL;
int agit_flag=0;

extern int packet_parse_time;

struct charid2nick {
	char nick[24];
	int req_id;
	int account_id;
	unsigned long ip;
	unsigned int port;
};
//各マップごとの最小限情報を入れるもの、READ_FROM_BITMAP用
typedef struct{
		char fn[32];//ファイル名
		int xs,ys; //幅と高さ
		int sizeinint;//intでの大きさ、1intに32セルの情報が入てる
		int celltype[MAX_CELL_TYPE];//マップごとにそのタイプのセルがあれば対応する数字が入る、なければ1
						//(タイプ1そのものは0と同じ配列gat_fileused[0]に
		long pos[MAX_CELL_TYPE];//ビットマップファイルでの場所、読み出す時に使う
		}CELL_INFO;


#define READ_FROM_GAT 0 //gatファイルから
#define READ_FROM_BITMAP 1 //ビットマップファイルから
int  map_read_flag=READ_FROM_GAT;//上の判定フラグ,どっちを使うかはmap_athana.conf内のread_map_from_bitmapで指定
					//0ならばREAD_FROM_GAT,1ならばREAD_FROM_BITMAP
int (*map_getcell)(int,int x,int y,CELL_CHK cellchk);
int (*map_getcellp)(struct map_data* m,int x,int y,CELL_CHK cellchk);

char map_bitmap_filename[256]="db/map.info";//ビットマップファイルのデフォルトパス
char motd_txt[256]="conf/motd.txt";
char help_txt[256]="conf/help.txt";

/*==========================================
 * 全map鯖総計での接続数設定
 * (char鯖から送られてくる)
 *------------------------------------------
 */
void map_setusers(int n)
{
	users=n;
}

/*==========================================
 * 全map鯖総計での接続数取得 (/wへの応答用)
 *------------------------------------------
 */
int map_getusers(void)
{
	return users;
}

//
// block削除の安全性確保処理
//

/*==========================================
 * blockをfreeするときfreeの変わりに呼ぶ
 * ロックされているときはバッファにためる
 *------------------------------------------
 */
int map_freeblock( void *bl )
{
	if(block_free_lock==0){
		free(bl);
		bl = NULL;
	}
	else{
		if( block_free_count>=block_free_max ) {
			if(battle_config.error_log)
				printf("map_freeblock: *WARNING* too many free block! %d %d\n",
			block_free_count,block_free_lock);
		}
		else
			block_free[block_free_count++]=bl;
	}
	return block_free_lock;
}
/*==========================================
 * blockのfreeを一時的に禁止する
 *------------------------------------------
 */
int map_freeblock_lock(void)
{
	return ++block_free_lock;
}
/*==========================================
 * blockのfreeのロックを解除する
 * このとき、ロックが完全になくなると
 * バッファにたまっていたblockを全部削除
 *------------------------------------------
 */
int map_freeblock_unlock(void)
{
	if( (--block_free_lock)==0 ){
		int i;
//		if(block_free_count>0) {
//			if(battle_config.error_log)
//				printf("map_freeblock_unlock: free %d object\n",block_free_count);
//		}
		for(i=0;i<block_free_count;i++){
			free(block_free[i]);
			block_free[i] = NULL;
		}
		block_free_count=0;
	}else if(block_free_lock<0){
		if(battle_config.error_log)
			printf("map_freeblock_unlock: lock count < 0 !\n");
	}
	return block_free_lock;
}


//
// block化処理
//
/*==========================================
 * map[]のblock_listから繋がっている場合に
 * bl->prevにbl_headのアドレスを入れておく
 *------------------------------------------
 */
static struct block_list bl_head;

/*==========================================
 * map[]のblock_listに追加
 * mobは数が多いので別リスト
 *
 * 既にlink済みかの確認が無い。危険かも
 *------------------------------------------
 */
int map_addblock(struct block_list *bl)
{
	int m,x,y;

	nullpo_retr(0, bl);

	if(bl->prev != NULL){
			if(battle_config.error_log)
				printf("map_addblock error : bl->prev!=NULL\n");
		return 0;
	}

	m=bl->m;
	x=bl->x;
	y=bl->y;
	if(m<0 || m>=map_num ||
	   x<0 || x>=map[m].xs ||
	   y<0 || y>=map[m].ys)
		return 1;

	if(bl->type==BL_MOB){
		bl->next = map[m].block_mob[x/BLOCK_SIZE+(y/BLOCK_SIZE)*map[m].bxs];
		bl->prev = &bl_head;
		if(bl->next) bl->next->prev = bl;
		map[m].block_mob[x/BLOCK_SIZE+(y/BLOCK_SIZE)*map[m].bxs] = bl;
		map[m].block_mob_count[x/BLOCK_SIZE+(y/BLOCK_SIZE)*map[m].bxs]++;
	} else {
		bl->next = map[m].block[x/BLOCK_SIZE+(y/BLOCK_SIZE)*map[m].bxs];
		bl->prev = &bl_head;
		if(bl->next) bl->next->prev = bl;
		map[m].block[x/BLOCK_SIZE+(y/BLOCK_SIZE)*map[m].bxs] = bl;
		map[m].block_count[x/BLOCK_SIZE+(y/BLOCK_SIZE)*map[m].bxs]++;
		if(bl->type==BL_PC)
			map[m].users++;
	}

	return 0;
}

/*==========================================
 * map[]のblock_listから外す
 * prevがNULLの場合listに繋がってない
 *------------------------------------------
 */
int map_delblock(struct block_list *bl)
{
	int b;
	nullpo_retr(0, bl);

	// 既にblocklistから抜けている
	if(bl->prev==NULL){
		if(bl->next!=NULL){
			// prevがNULLでnextがNULLでないのは有ってはならない
			if(battle_config.error_log)
				printf("map_delblock error : bl->next!=NULL\n");
		}
		return 0;
	}

	b = bl->x/BLOCK_SIZE+(bl->y/BLOCK_SIZE)*map[bl->m].bxs;

	if(bl->type==BL_PC)
		map[bl->m].users--;
	if(bl->next) bl->next->prev = bl->prev;
	if(bl->prev==&bl_head){
		// リストの頭なので、map[]のblock_listを更新する
		if(bl->type==BL_MOB){
			map[bl->m].block_mob[b] = bl->next;
			if((map[bl->m].block_mob_count[b]--) < 0)
				map[bl->m].block_mob_count[b] = 0;
		} else {
			map[bl->m].block[b] = bl->next;
			if((map[bl->m].block_count[b]--) < 0)
				map[bl->m].block_count[b] = 0;
		}
	} else {
		bl->prev->next = bl->next;
	}
	bl->next = NULL;
	bl->prev = NULL;

	return 0;
}

/*==========================================
 * 周囲のPC人数を数える (現在未使用)
 *------------------------------------------
 */
int map_countnearpc(int m,int x,int y)
{
	int bx,by,c=0;
	struct block_list *bl;

	if(map[m].users==0)
		return 0;
	for(by=y/BLOCK_SIZE-AREA_SIZE/BLOCK_SIZE-1;by<=y/BLOCK_SIZE+AREA_SIZE/BLOCK_SIZE+1;by++){
		if(by<0 || by>=map[m].bys)
			continue;
		for(bx=x/BLOCK_SIZE-AREA_SIZE/BLOCK_SIZE-1;bx<=x/BLOCK_SIZE+AREA_SIZE/BLOCK_SIZE+1;bx++){
			if(bx<0 || bx>=map[m].bxs)
				continue;
			bl = map[m].block[bx+by*map[m].bxs];
			for(;bl;bl=bl->next){
				if(bl->type==BL_PC)
					c++;
			}
		}
	}
	return c;
}

/*==========================================
 * セル上のPCとMOBの数を数える (グランドクロス用)
 *------------------------------------------
 */
int map_count_oncell(int m,int x,int y)
{
	int bx,by;
	struct block_list *bl;
	int i,c;
	int count = 0;

	if (x < 0 || y < 0 || (x >= map[m].xs) || (y >= map[m].ys))
		return 1;
	bx = x/BLOCK_SIZE;
	by = y/BLOCK_SIZE;

	bl = map[m].block[bx+by*map[m].bxs];
	c = map[m].block_count[bx+by*map[m].bxs];
	for(i=0;i<c && bl;i++,bl=bl->next){
		if(bl->x == x && bl->y == y && bl->type == BL_PC) count++;
	}
	bl = map[m].block_mob[bx+by*map[m].bxs];
	c = map[m].block_mob_count[bx+by*map[m].bxs];
	for(i=0;i<c && bl;i++,bl=bl->next){
		if(bl->x == x && bl->y == y) count++;
	}
	if(!count) count = 1;
	return count;
}


/*==========================================
 * map m (x0,y0)-(x1,y1)内の全objに対して
 * funcを呼ぶ
 * type!=0 ならその種類のみ
 *------------------------------------------
 */
void map_foreachinarea(int (*func)(struct block_list*,va_list),int m,int x0,int y0,int x1,int y1,int type,...)
{
	int bx,by;
	struct block_list *bl;
	va_list ap;
	int blockcount=bl_list_count,i,c;

	va_start(ap,type);
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 >= map[m].xs) x1 = map[m].xs-1;
	if (y1 >= map[m].ys) y1 = map[m].ys-1;
	if (type == 0 || type != BL_MOB)
		for (by = y0 / BLOCK_SIZE; by <= y1 / BLOCK_SIZE; by++) {
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE;bx++){
				bl = map[m].block[bx+by*map[m].bxs];
				c = map[m].block_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(type && bl->type!=type)
						continue;
					if(bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
			}
		}
	if(type==0 || type==BL_MOB)
		for(by=y0/BLOCK_SIZE;by<=y1/BLOCK_SIZE;by++){
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE;bx++){
				bl = map[m].block_mob[bx+by*map[m].bxs];
				c = map[m].block_mob_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
			}
		}

	if(bl_list_count>=BL_LIST_MAX) {
		if(battle_config.error_log)
			printf("map_foreachinarea: *WARNING* block count too many!\n");
	}

	map_freeblock_lock();	// メモリからの解放を禁止する

	for(i=blockcount;i<bl_list_count;i++)
		if(bl_list[i]->prev)	// 有効かどうかチェック
			func(bl_list[i],ap);

	map_freeblock_unlock();	// 解放を許可する

	va_end(ap);
	bl_list_count = blockcount;
}

/*==========================================
 * 矩形(x0,y0)-(x1,y1)が(dx,dy)移動した時の
 * 領域外になる領域(矩形かL字形)内のobjに
 * 対してfuncを呼ぶ
 *
 * dx,dyは-1,0,1のみとする（どんな値でもいいっぽい？）
 *------------------------------------------
 */
void map_foreachinmovearea(int (*func)(struct block_list*,va_list),int m,int x0,int y0,int x1,int y1,int dx,int dy,int type,...)
{
	int bx,by;
	struct block_list *bl;
	va_list ap;
	int blockcount=bl_list_count,i,c;

	va_start(ap,type);
	if(dx==0 || dy==0){
		// 矩形領域の場合
		if(dx==0){
			if(dy<0){
				y0=y1+dy+1;
			} else {
				y1=y0+dy-1;
			}
		} else if(dy==0){
			if(dx<0){
				x0=x1+dx+1;
			} else {
				x1=x0+dx-1;
			}
		}
		if(x0<0) x0=0;
		if(y0<0) y0=0;
		if(x1>=map[m].xs) x1=map[m].xs-1;
		if(y1>=map[m].ys) y1=map[m].ys-1;
		for(by=y0/BLOCK_SIZE;by<=y1/BLOCK_SIZE;by++){
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE;bx++){
				bl = map[m].block[bx+by*map[m].bxs];
				c = map[m].block_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(type && bl->type!=type)
						continue;
					if(bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
				bl = map[m].block_mob[bx+by*map[m].bxs];
				c = map[m].block_mob_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(type && bl->type!=type)
						continue;
					if(bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
			}
		}
	}else{
		// L字領域の場合

		if(x0<0) x0=0;
		if(y0<0) y0=0;
		if(x1>=map[m].xs) x1=map[m].xs-1;
		if(y1>=map[m].ys) y1=map[m].ys-1;
		for(by=y0/BLOCK_SIZE;by<=y1/BLOCK_SIZE;by++){
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE;bx++){
				bl = map[m].block[bx+by*map[m].bxs];
				c = map[m].block_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(type && bl->type!=type)
						continue;
					if(!(bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1))
						continue;
					if(((dx>0 && bl->x<x0+dx) || (dx<0 && bl->x>x1+dx) ||
						(dy>0 && bl->y<y0+dy) || (dy<0 && bl->y>y1+dy)) &&
						bl_list_count<BL_LIST_MAX)
							bl_list[bl_list_count++]=bl;
				}
				bl = map[m].block_mob[bx+by*map[m].bxs];
				c = map[m].block_mob_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(type && bl->type!=type)
						continue;
					if(!(bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1))
						continue;
					if(((dx>0 && bl->x<x0+dx) || (dx<0 && bl->x>x1+dx) ||
						(dy>0 && bl->y<y0+dy) || (dy<0 && bl->y>y1+dy)) &&
						bl_list_count<BL_LIST_MAX)
							bl_list[bl_list_count++]=bl;
				}
			}
		}

	}

	if(bl_list_count>=BL_LIST_MAX) {
		if(battle_config.error_log)
			printf("map_foreachinarea: *WARNING* block count too many!\n");
	}

	map_freeblock_lock();	// メモリからの解放を禁止する

	for(i=blockcount;i<bl_list_count;i++)
		if(bl_list[i]->prev)	// 有効かどうかチェック
			func(bl_list[i],ap);

	map_freeblock_unlock();	// 解放を許可する

	va_end(ap);
	bl_list_count = blockcount;
}

/*==========================================
 * 床アイテムやエフェクト用の一時obj割り当て
 * object[]への保存とid_db登録まで
 *
 * bl->idもこの中で設定して問題無い?
 *------------------------------------------
 */
int map_addobject(struct block_list *bl)
{
	int i;
	if( bl == NULL ){
		printf("map_addobject nullpo?\n");
		return 0;
	}
	if(first_free_object_id<2 || first_free_object_id>=MAX_FLOORITEM)
		first_free_object_id=2;
	for(i=first_free_object_id;i<MAX_FLOORITEM;i++)
		if(object[i]==NULL)
			break;
	if(i>=MAX_FLOORITEM){
		if(battle_config.error_log)
			printf("no free object id\n");
		return 0;
	}
	first_free_object_id=i;
	if(last_object_id<i)
		last_object_id=i;
	object[i]=bl;
	numdb_insert(id_db,i,bl);
	return i;
}

/*==========================================
 * 一時objectの解放
 *	map_delobjectのfreeしないバージョン
 *------------------------------------------
 */
int map_delobjectnofree(int id)
{
	if(object[id]==NULL)
		return 0;

	map_delblock(object[id]);
	numdb_erase(id_db,id);
//	map_freeblock(object[id]);
	object[id]=NULL;

	if(first_free_object_id>id)
		first_free_object_id=id;

	while(last_object_id>2 && object[last_object_id]==NULL)
		last_object_id--;

	return 0;
}

/*==========================================
 * 一時objectの解放
 * block_listからの削除、id_dbからの削除
 * object dataのfree、object[]へのNULL代入
 *
 * addとの対称性が無いのが気になる
 *------------------------------------------
 */
int map_delobject(int id)
{
	struct block_list *obj=object[id];

	if(obj==NULL)
		return 0;

	map_delobjectnofree(id);
	map_freeblock(obj);

	return 0;
}

/*==========================================
 * 全一時obj相手にfuncを呼ぶ
 *
 *------------------------------------------
 */
void map_foreachobject(int (*func)(struct block_list*,va_list),int type,...)
{
	int i;
	int blockcount=bl_list_count;
	va_list ap;

	va_start(ap,type);

	for(i=2;i<=last_object_id;i++){
		if(object[i]){
			if(type && object[i]->type!=type)
				continue;
			if(bl_list_count>=BL_LIST_MAX) {
				if(battle_config.error_log)
					printf("map_foreachobject: too many block !\n");
			}
			else
				bl_list[bl_list_count++]=object[i];
		}
	}

	map_freeblock_lock();

	for(i=blockcount;i<bl_list_count;i++)
		if( bl_list[i]->prev || bl_list[i]->next )
			func(bl_list[i],ap);

	map_freeblock_unlock();

	va_end(ap);
	bl_list_count = blockcount;
}

/*==========================================
 * 床アイテムを消す
 *
 * data==0の時はtimerで消えた時
 * data!=0の時は拾う等で消えた時として動作
 *
 * 後者は、map_clearflooritem(id)へ
 * map.h内で#defineしてある
 *------------------------------------------
 */
int map_clearflooritem_timer(int tid,unsigned int tick,int id,int data)
{
	struct flooritem_data *fitem;

	fitem = (struct flooritem_data *)object[id];
	if(fitem==NULL || fitem->bl.type!=BL_ITEM || (!data && fitem->cleartimer != tid)){
		if(battle_config.error_log)
			printf("map_clearflooritem_timer : error\n");
		return 1;
	}
	if(data)
		delete_timer(fitem->cleartimer,map_clearflooritem_timer);
	else if(fitem->item_data.card[0] == (short)0xff00)
		intif_delete_petdata(*((long *)(&fitem->item_data.card[1])));
	clif_clearflooritem(fitem,0);
	map_delobject(fitem->bl.id);

	return 0;
}

/*==========================================
 * (m,x,y)の周囲rangeマス内の空き(=侵入可能)cellの
 * 内から適当なマス目の座標をx+(y<<16)で返す
 *
 * 現状range=1でアイテムドロップ用途のみ
 *------------------------------------------
 */
int map_searchrandfreecell(int m,int x,int y,int range)
{
	int free_cell,i,j;

	for(free_cell=0,i=-range;i<=range;i++){
		if(i+y<0 || i+y>=map[m].ys)
			continue;
		for(j=-range;j<=range;j++){
			if(j+x<0 || j+x>=map[m].xs)
				continue;
			if(map_getcell(m,j+x,i+y,CELL_CHKNOPASS))
				continue;
			free_cell++;
		}
	}
	if(free_cell==0)
		return -1;
	free_cell=rand()%free_cell;
	for(i=-range;i<=range;i++){
		if(i+y<0 || i+y>=map[m].ys)
			continue;
		for(j=-range;j<=range;j++){
			if(j+x<0 || j+x>=map[m].xs)
				continue;
			if(map_getcell(m,j+x,i+y,CELL_CHKNOPASS))
				continue;
			if(free_cell==0){
				x+=j;
				y+=i;
				i=range+1;
				break;
			}
			free_cell--;
		}
	}

	return x+(y<<16);
}

/*==========================================
 * (m,x,y)を中心に3x3以内に床アイテム設置
 *
 * item_dataはamount以外をcopyする
 *------------------------------------------
 */
int map_addflooritem(struct item *item_data,int amount,int m,int x,int y,struct map_session_data *first_sd,
	struct map_session_data *second_sd,struct map_session_data *third_sd,int type)
{
	int xy,r;
	unsigned int tick;
	struct flooritem_data *fitem;

	nullpo_retr(0, item_data);

	if((xy=map_searchrandfreecell(m,x,y,1))<0)
		return 0;
	r=rand();

	fitem = (struct flooritem_data *)aCalloc(1,sizeof(*fitem));
	fitem->bl.type=BL_ITEM;
	fitem->bl.prev = fitem->bl.next = NULL;
	fitem->bl.m=m;
	fitem->bl.x=xy&0xffff;
	fitem->bl.y=(xy>>16)&0xffff;
	fitem->first_get_id = 0;
	fitem->first_get_tick = 0;
	fitem->second_get_id = 0;
	fitem->second_get_tick = 0;
	fitem->third_get_id = 0;
	fitem->third_get_tick = 0;

	fitem->bl.id = map_addobject(&fitem->bl);
	if(fitem->bl.id==0){
		free(fitem);
		return 0;
	}

	tick = gettick();
	if(first_sd) {
		fitem->first_get_id = first_sd->bl.id;
		if(type)
			fitem->first_get_tick = tick + battle_config.mvp_item_first_get_time;
		else
			fitem->first_get_tick = tick + battle_config.item_first_get_time;
	}
	if(second_sd) {
		fitem->second_get_id = second_sd->bl.id;
		if(type)
			fitem->second_get_tick = tick + battle_config.mvp_item_first_get_time + battle_config.mvp_item_second_get_time;
		else
			fitem->second_get_tick = tick + battle_config.item_first_get_time + battle_config.item_second_get_time;
	}
	if(third_sd) {
		fitem->third_get_id = third_sd->bl.id;
		if(type)
			fitem->third_get_tick = tick + battle_config.mvp_item_first_get_time + battle_config.mvp_item_second_get_time + battle_config.mvp_item_third_get_time;
		else
			fitem->third_get_tick = tick + battle_config.item_first_get_time + battle_config.item_second_get_time + battle_config.item_third_get_time;
	}

	memcpy(&fitem->item_data,item_data,sizeof(*item_data));
	fitem->item_data.amount=amount;
	fitem->subx=(r&3)*3+3;
	fitem->suby=((r>>2)&3)*3+3;
	fitem->cleartimer=add_timer(gettick()+battle_config.flooritem_lifetime,map_clearflooritem_timer,fitem->bl.id,0);

	map_addblock(&fitem->bl);
	clif_dropflooritem(fitem);

	return fitem->bl.id;
}

/*==========================================
 * charid_dbへ追加(返信待ちがあれば返信)
 *------------------------------------------
 */
void map_addchariddb(int charid, char *name, int account_id, unsigned long ip, int port)
{
	struct charid2nick *p;
	int req=0;
	p=numdb_search(charid_db,charid);
	if(p==NULL){	// データベースにない
		p = (struct charid2nick *)aCalloc(1,sizeof(struct charid2nick));
		p->req_id=0;
	}else
		numdb_erase(charid_db,charid);

	req=p->req_id;
	memcpy(p->nick,name,24);
	p->account_id=account_id;
	p->ip=ip;
	p->port=port;
	p->req_id=0;
	numdb_insert(charid_db,charid,p);
	if(req){	// 返信待ちがあれば返信
		struct map_session_data *sd = map_id2sd(req);
		if(sd!=NULL)
			clif_solved_charname(sd,charid);
	}
	//printf("map add chariddb:%s\n",p->nick);
	return;
}
/*==========================================
 * charid_dbから削除
 *------------------------------------------
 */
void map_delchariddb(int charid)
{
	struct charid2nick *p;
	p=numdb_search(charid_db,charid);
	if(p){	// データベースにあった
		p->ip=0;	//実際に削除すると武器の名前とか取れなくなるのでmap-serverのIPとPortだけ削除
		p->port=0;
//		printf("map delete chariddb:%s\n",p->nick);
	}//else
//		printf("map delete chariddb:notfound %d\n",charid);

	return;
}
/*==========================================
 * charid_dbへ追加（返信要求のみ）
 *------------------------------------------
 */
int map_reqchariddb(struct map_session_data * sd,int charid)
{
	struct charid2nick *p;

	nullpo_retr(0, sd);

	p=numdb_search(charid_db,charid);
	if(p!=NULL)	// データベースにすでにある
		return 0;
	p = (struct charid2nick *)aCalloc(1,sizeof(struct charid2nick));
	p->req_id=sd->bl.id;
	numdb_insert(charid_db,charid,p);
	return 0;
}

/*==========================================
 * id_dbへblを追加
 *------------------------------------------
 */
void map_addiddb(struct block_list *bl)
{
	nullpo_retv(bl);

	numdb_insert(id_db,bl->id,bl);
}

/*==========================================
 * id_dbからblを削除
 *------------------------------------------
 */
void map_deliddb(struct block_list *bl)
{
	nullpo_retv(bl);

	numdb_erase(id_db,bl->id);
}

/*==========================================
 * nick_dbへsdを追加
 *------------------------------------------
 */
void map_addnickdb(struct map_session_data *sd)
{
	nullpo_retv(sd);

	strdb_insert(nick_db,sd->status.name,sd);
}

/*==========================================
 * PCのquit処理 map.c内分
 *
 * quit処理の主体が違うような気もしてきた
 *------------------------------------------
 */
int map_quit(struct map_session_data *sd)
{
	int i;

	nullpo_retr(0, sd);

	if(sd->chatID)	// チャットから出る
		chat_leavechat(sd);

	if(sd->trade_partner)	// 取引を中断する
		trade_tradecancel(sd);

	if(sd->party_invite>0)	// パーティ勧誘を拒否する
		party_reply_invite(sd,sd->party_invite_account,0);

	if(sd->guild_invite>0)	// ギルド勧誘を拒否する
		guild_reply_invite(sd,sd->guild_invite,0);
	if(sd->guild_alliance>0)	// ギルド同盟勧誘を拒否する
		guild_reply_reqalliance(sd,sd->guild_alliance_account,0);

	party_send_logout(sd);	// パーティのログアウトメッセージ送信

	guild_send_memberinfoshort(sd,0);	// ギルドのログアウトメッセージ送信

	pc_cleareventtimer(sd);	// イベントタイマを破棄する

	if(sd->state.storage_flag)
		storage_guild_storage_quit(sd,0);
	else
		storage_storage_quit(sd);	// 倉庫を開いてるなら保存する

	skill_castcancel(&sd->bl,0);	// 詠唱を中断する
	skill_stop_dancing(&sd->bl,1);// ダンス/演奏中断

	if(sd->sc_data && sd->sc_data[SC_BERSERK].timer!=-1) //バーサーク中の終了はHPを100に
		sd->status.hp = 100;

	if(sd->sc_data && sd->sc_data[SC_WEDDING].timer!=-1) //結婚中はリログしても1時間は継続
		pc_setglobalreg(sd,"PC_WEDDING_TIME",sd->sc_data[SC_WEDDING].val2);
	else
		pc_setglobalreg(sd,"PC_WEDDING_TIME",0);

	skill_status_change_clear(&sd->bl,1);	// ステータス異常を解除する
	skill_clear_unitgroup(&sd->bl);	// スキルユニットグループの削除
	skill_cleartimerskill(&sd->bl);
	pc_stop_walking(sd,0);
	pc_stopattack(sd);
	pc_delinvincibletimer(sd);
	pc_delspiritball(sd,sd->spiritball,1);
	skill_gangsterparadise(sd,0);

	pc_calcstatus(sd,4);

	clif_clearchar_area(&sd->bl,2);

	if(sd->status.pet_id && sd->pd) {
		pet_lootitem_drop(sd->pd,sd);
		pet_remove_map(sd);
		if(sd->pet.intimate <= 0) {
			intif_delete_petdata(sd->status.pet_id);
			sd->status.pet_id = 0;
			sd->pd = NULL;
			sd->petDB = NULL;
		}
		else
			intif_save_petdata(sd->status.account_id,&sd->pet);
	}

	if(pc_isdead(sd))
		pc_setrestartvalue(sd,2);
	pc_makesavestatus(sd);
	//クローンスキルで覚えたスキルは消す
	for(i=0;i<MAX_SKILL;i++){
		if(sd->status.skill[i].flag == 13){
			sd->status.skill[i].id=0;
			sd->status.skill[i].lv=0;
			sd->status.skill[i].flag=0;
		}
	}
	chrif_save(sd);
	storage_storage_save(sd);
	storage_delete(sd->status.account_id);

	if( sd->npc_stackbuf && sd->npc_stackbuf != NULL)
		free( sd->npc_stackbuf );

	map_delblock(&sd->bl);

	numdb_erase(id_db,sd->bl.id);
	strdb_erase(nick_db,sd->status.name);
	numdb_erase(charid_db,sd->status.char_id);
//printf("map quit:%s\n",sd->status.name);

	return 0;
}

/*==========================================
 * id番号のPCを探す。居なければNULL
 *------------------------------------------
 */
struct map_session_data * map_id2sd(int id)
{
	struct block_list *bl;

	bl=numdb_search(id_db,id);
	if(bl && bl->type==BL_PC)
		return (struct map_session_data*)bl;
	return NULL;
}

/*==========================================
 * char_id番号の名前を探す
 *------------------------------------------
 */
char * map_charid2nick(int id)
{
	struct charid2nick *p=numdb_search(charid_db,id);
	if(p==NULL)
		return NULL;
	if(p->req_id!=0)
		return NULL;
	return p->nick;
}

/*==========================================
 * 名前がnickのPCを探す。居なければNULL
 *------------------------------------------
 */
struct map_session_data * map_nick2sd(char *nick)
{
	if(nick == NULL)
		return NULL;
	return strdb_search(nick_db,nick);
}

/*==========================================
 * id番号の物を探す
 * 一時objectの場合は配列を引くのみ
 *------------------------------------------
 */
struct block_list * map_id2bl(int id)
{
	struct block_list *bl;
	if(id<sizeof(object)/sizeof(object[0]))
		bl = object[id];
	else
		bl = numdb_search(id_db,id);

	return bl;
}

/*==========================================
 * id_db内の全てにfuncを実行
 *------------------------------------------
 */
int map_foreachiddb(int (*func)(void*,void*,va_list),...)
{
	va_list ap;

	va_start(ap,func);
	numdb_foreach(id_db,func,ap);
	va_end(ap);
	return 0;
}

/*==========================================
 * map.npcへ追加 (warp等の領域持ちのみ)
 *------------------------------------------
 */
int map_addnpc(int m,struct npc_data *nd)
{
	int i;
	if(m<0 || m>=map_num)
		return -1;
	for(i=0;i<map[m].npc_num && i<MAX_NPC_PER_MAP;i++)
		if(map[m].npc[i]==NULL)
			break;
	if(i==MAX_NPC_PER_MAP){
		if(battle_config.error_log)
			printf("too many NPCs in one map %s\n",map[m].name);
		return -1;
	}
	if(i==map[m].npc_num){
		map[m].npc_num++;
	}

	nullpo_retr(0, nd);

	map[m].npc[i]=nd;
	nd->n = i;
	numdb_insert(id_db,nd->bl.id,nd);

	return i;
}

/*==========================================
 * map名からmap番号へ変換
 *------------------------------------------
 */
int map_mapname2mapid(char *name)
{
	struct map_data *md;

	md=strdb_search(map_db,name);
	if(md==NULL || md->gat==NULL)
		return -1;
	return md->m;
}

/*==========================================
 * 他鯖map名からip,port変換
 *------------------------------------------
 */
int map_mapname2ipport(char *name,int *ip,int *port)
{
	struct map_data_other_server *mdos;

	mdos=strdb_search(map_db,name);
	if(mdos==NULL || mdos->gat)
		return -1;
	*ip=mdos->ip;
	*port=mdos->port;
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int map_check_dir(int s_dir,int t_dir)
{
	if(s_dir == t_dir)
		return 0;
	switch(s_dir) {
		case 0:
			if(t_dir == 7 || t_dir == 1 || t_dir == 0)
				return 0;
			break;
		case 1:
			if(t_dir == 0 || t_dir == 2 || t_dir == 1)
				return 0;
			break;
		case 2:
			if(t_dir == 1 || t_dir == 3 || t_dir == 2)
				return 0;
			break;
		case 3:
			if(t_dir == 2 || t_dir == 4 || t_dir == 3)
				return 0;
			break;
		case 4:
			if(t_dir == 3 || t_dir == 5 || t_dir == 4)
				return 0;
			break;
		case 5:
			if(t_dir == 4 || t_dir == 6 || t_dir == 5)
				return 0;
			break;
		case 6:
			if(t_dir == 5 || t_dir == 7 || t_dir == 6)
				return 0;
			break;
		case 7:
			if(t_dir == 6 || t_dir == 0 || t_dir == 7)
				return 0;
			break;
	}
	return 1;
}

/*==========================================
 * 彼我の方向を計算
 *------------------------------------------
 */
int map_calc_dir( struct block_list *src,int x,int y)
{
	int dir=0;
	int dx,dy;

	nullpo_retr(0, src);

	dx=x-src->x;
	dy=y-src->y;
	if( dx==0 && dy==0 ){	// 彼我の場所一致
		dir=0;	// 上
	}else if( dx>=0 && dy>=0 ){	// 方向的に右上
		dir=7;						// 右上
		if( dx*3-1<dy ) dir=0;		// 上
		if( dx>dy*3 ) dir=6;		// 右
	}else if( dx>=0 && dy<=0 ){	// 方向的に右下
		dir=5;						// 右下
		if( dx*3-1<-dy ) dir=4;		// 下
		if( dx>-dy*3 ) dir=6;		// 右
	}else if( dx<=0 && dy<=0 ){ // 方向的に左下
		dir=3;						// 左下
		if( dx*3+1>dy ) dir=4;		// 下
		if( dx<dy*3 ) dir=2;		// 左
	}else{						// 方向的に左上
		dir=1;						// 左上
		if( -dx*3-1<dy ) dir=0;		// 上
		if( -dx>dy*3 ) dir=2;		// 左
	}
	return dir;
}

// gat系
/*==========================================
 * (m,x,y)の状態を調べる
 *------------------------------------------
 */

/* gat用：map_getcell_gat,map_getcellp_gat
 * bitmap用: map_getcell_bitmap,map_getcellp_bitmap
 * 実際はどっちを使うかは立ち上がる時map_config_read()がmap_read_flagによって判断する
 */
int map_getcell_gat(int m,int x,int y,CELL_CHK cellchk)
{
	int j;
	if(x<0 || x>=map[m].xs-1 || y<0 || y>=map[m].ys-1)
	{
		if(cellchk==CELL_CHKNOPASS) return 1;
		return 0;
	}
	j=x+y*map[m].xs;

	//printf("m=%d,x=%d,y=%d,j=%d\n",m,x,y,j);
	switch(cellchk)
	{
		case CELL_CHKTOUCH: //0x80
			if(map[m].gat[j]&0x80) return 1;return 0;
		case CELL_CHKWATER: //3
			if(map[m].gat[j]==3) return 1;return 0;
		case CELL_CHKHIGH: //5
			if(map[m].gat[j]==5) return 1;return 0;
		case CELL_CHKPASS: //0,3,6
			if(map[m].gat[j]!=1&&map[m].gat[j]!=5) return 1;return 0;
		case CELL_CHKNOPASS://1,5
			if(map[m].gat[j]==1||map[m].gat[j]==5) return 1;return 0;
		case CELL_CHKTYPE:
			return map[m].gat[j];
		default: return 0;
	}	
	return 0;
}

int map_getcell_bitmap(int m,int x,int y,CELL_CHK cellchk)
{
	int i,j;
	if(x<0 || x>=map[m].xs-1 || y<0 || y>=map[m].ys-1)
	{
		if(cellchk==CELL_CHKNOPASS) return 1;
		return 0;
	}
	j=x+y*map[m].xs;
	switch(cellchk)
	{
		case CELL_CHKTOUCH: //MAX_CELL_TYPE+1
			if(*(map[m].gat_fileused[MAX_CELL_TYPE+1]+j/32)&(0x1<<(j%32))) return 1; return 0; 
		case CELL_CHKWATER://3
			if(map[m].gat_fileused[3]==NULL) return 0;
			if(*(map[m].gat_fileused[3]+j/32)&(0x1<<(j%32))) return 1; return 0; 
		case CELL_CHKHIGH://5
			if(map[m].gat_fileused[5]==NULL) return 0;
			if( *(map[m].gat_fileused[5]+j/32)&(0x1<<(j%32))) return 1; return 0; 
		case CELL_CHKPASS://0,3,6	
			if(*(map[m].gat_fileused[0]+j/32)&(0x1<<(j%32))) return 1;return 0;
		case CELL_CHKNOPASS://1,5
			if(*(map[m].gat_fileused[0]+j/32)&(0x1<<(j%32))) return 0;return 1;
		case CELL_CHKTYPE:
			for(i=MAX_CELL_TYPE;i>=0;i--)
			{
				if(map[m].gat_fileused[i]==NULL) continue;
				if(*(map[m].gat_fileused[i]+j/32)&(0x1<<(j%32))) 
					return i;
			}
			return 1;
		default: return 0;
	}
	return 0;
}

//

int map_getcellp_gat(struct map_data* m,int x,int y,CELL_CHK cellchk)
{
	int j;
	if(x<0 || x>=m->xs-1 || y<0 || y>=m->ys-1)
	{
		if(cellchk==CELL_CHKNOPASS) return 1;
		return 0;
	}
	j=x+y*m->xs;

	switch(cellchk)
	{
		case CELL_CHKTOUCH:
			if(m->gat[j]&0x80) return 1;return 0;
		case CELL_CHKWATER:
			if(m->gat[j]==3) return 1;return 0;
		case CELL_CHKHIGH:
			if(m->gat[j]==5) return 1;return 0;
		case CELL_CHKPASS:
			if(m->gat[j]!=1&&m->gat[j]!=5) return 1; return 0;
		case CELL_CHKNOPASS:
			if(m->gat[j]==1||m->gat[j]==5) return 1; return 0;
		case CELL_CHKTYPE:
			return m->gat[j];
		default: return 0;
	}	
	return 0;
}
int map_getcellp_bitmap(struct map_data* m,int x,int y,CELL_CHK cellchk)
{
	int i,j;
	if(x<0 || x>=m->xs-1 || y<0 || y>=m->ys-1)
	{
		if(cellchk==CELL_CHKNOPASS) return 1;
		return 0;
	}
	j=x+y*m->xs;
	switch(cellchk)
	{
		case CELL_CHKTOUCH:
			if((*(m->gat_fileused[MAX_CELL_TYPE+1]+j/32)&(0x1<<(j%32)))) return 1;return 0;
		case CELL_CHKWATER:
			if(m->gat_fileused[3]==NULL) return 0;
		if( *(m->gat_fileused[3]+j/32)&(0x1<<(j%32))) return 1;return 0;
		case CELL_CHKHIGH:
			if(m->gat_fileused[5]==NULL) return 0;
			if(*(m->gat_fileused[5]+j/32)&(0x1<<(j%32))) return 1; return 0;
		case CELL_CHKPASS:
			if( *(m->gat_fileused[0]+j/32)&(0x1<<(j%32))) return 1;return 0;
		case CELL_CHKNOPASS:
			if(*(m->gat_fileused[0]+j/32)&(0x1<<(j%32))) return 0;return 1;
		case CELL_CHKTYPE:
			for(i=MAX_CELL_TYPE;i>=0;i--)
			{
				if(m->gat_fileused[i]==NULL) continue;
				if((*(m->gat_fileused[i]+j/32)&(0x1<<(j%32))))
					return i;
			}
			return 1;
		default: return 0;
	}
	return 0;
}
/*==========================================
 * (m,x,y)の状態を設定する
 *------------------------------------------
 */
int map_setcell(int m,int x,int y,CELL_SET cellset)
{
	int i,j;
		
	if(x<0 || x>=map[m].xs || y<0 || y>=map[m].ys)
		return 0;
	j=x+y*map[m].xs;
	switch(cellset)
	{
	case CELL_SETTOUCH://MAX_CELL_TYPE+1(READ_FROM_BITMAP)か0x80(READ_FROM_GAT)
		if(map_read_flag==READ_FROM_GAT)
			return map[m].gat[j]|=0x80;
		i=MAX_CELL_TYPE+1;break;
	case CELL_SETWATER://3
		i=3;break;
	case CELL_SETPASS://0
		i=0;break;
	case CELL_SETNOPASS://gat_fileused[0](READ_FROM_BITMAP)か1(READ_FROM_GAT)
		if(map_read_flag==READ_FROM_BITMAP)
		{
			if( *(map[m].gat_fileused[0]+j/32)&(0x1<<(j%32)))
				*(map[m].gat_fileused[0]+j/32)^=0x1<<(j%32) ;
			return 1;	
		}
		else i=1;break;
	case CELL_SETHIGH://5
		i=5;break;
	case CELL_SETNOHIGH://5
		if(map_read_flag==READ_FROM_BITMAP)
		{
			if( *(map[m].gat_fileused[5]+j/32)&(0x1<<(j%32)))
				*(map[m].gat_fileused[5]+j/32)^=0x1<<(j%32) ;
			return 1;	
		}
		else i=5;break;
	default:
		return 0;
	}
	if(map_read_flag==READ_FROM_BITMAP)
	{
		if(map[m].gat_fileused[i]==NULL) 
			map[m].gat_fileused[i]=aCalloc((map[m].xs*map[m].ys+31)/32,sizeof(int));
		*(map[m].gat_fileused[i]+j/32)|=0x1<<(j%32) ;
	}
	else if(map_read_flag==READ_FROM_GAT)
		map[m].gat[j]=i;
	   
	return 1;
}
/*==========================================
 * 他鯖管理のマップをdbに追加
 *------------------------------------------
 */
int map_setipport(char *name,unsigned long ip,int port)
{
	struct map_data *md;
	struct map_data_other_server *mdos;

	md=strdb_search(map_db,name);
	if(md==NULL){ // not exist -> add new data
		mdos=(struct map_data_other_server *)aCalloc(1,sizeof(struct map_data_other_server));
		memcpy(mdos->name,name,24);
		mdos->gat  = NULL;
		mdos->ip   = ip;
		mdos->port = port;
		strdb_insert(map_db,mdos->name,mdos);
	} else {
		if(md->gat){ // local -> check data
			if(ip!=clif_getip() || port!=clif_getport()){
				printf("from char server : %s -> %08lx:%d\n",name,ip,port);
				return 1;
			}
		} else { // update
			mdos=(struct map_data_other_server *)md;
			mdos->ip   = ip;
			mdos->port = port;
		}
	}
	return 0;
}
/*==========================================
 * 他鯖管理のマップをdbから削除
 *------------------------------------------
 */
int map_eraseipport(char *name,unsigned long ip,int port)
{
	struct map_data *md;
	struct map_data_other_server *mdos;
//	unsigned char *p=(unsigned char *)&ip;

	md=strdb_search(map_db,name);
	if(md){
		if(md->gat) // local -> check data
			return 0;
		else{
			mdos=(struct map_data_other_server *)md;
			if(mdos->ip==ip && mdos->port == port){
				strdb_erase(map_db,name);
//				if(battle_config.etc_log)
//					printf("erase map %s %d.%d.%d.%d:%d\n",name,p[0],p[1],p[2],p[3],port);
			}
		}
	}
	return 0;
}

// 初期化周り
/*==========================================
 * 水場高さ設定
 *------------------------------------------
 */
static struct {
	char mapname[24];
	int waterheight;
} *waterlist=NULL;

#define NO_WATER 1000000

static int map_waterheight(char *mapname)
{
	if(waterlist){
		int i;
		for(i=0;waterlist[i].mapname[0] && i < MAX_MAP_PER_SERVER;i++)
			if(strcmp(waterlist[i].mapname,mapname)==0)
				return waterlist[i].waterheight;
	}
	return NO_WATER;
}

static void map_readwater(char *watertxt)
{
	char line[1024],w1[1024];
	FILE *fp;
	int n=0;

	fp=fopen(watertxt,"r");
	if(fp==NULL){
		printf("file not found: %s\n",watertxt);
		return;
	}
	if(waterlist==NULL)
		waterlist=aCalloc(MAX_MAP_PER_SERVER,sizeof(*waterlist));
	while(fgets(line,1020,fp) && n < MAX_MAP_PER_SERVER){
		int wh,count;
		if(line[0] == '/' && line[1] == '/')
			continue;
		if((count=sscanf(line,"%s%d",w1,&wh)) < 1){
			continue;
		}
		strncpy(waterlist[n].mapname,w1,24);
		if(count >= 2)
			waterlist[n].waterheight = wh;
		else
			waterlist[n].waterheight = 3;
		n++;
	}
	fclose(fp);
}
/*==========================================
*grfファイルからビットマップファイル生成する
*ファイル名デフォルトでdb/map.info
*map_athena.conf内のmap_bitmap_pathで指定できる
*===========================================*/
static int map_createbitmap(int m,char *fn,FILE* fp)
{
	unsigned char *gat;
	int x,y,xs,ys;
	struct gat_1cell {float high[4]; int type;} *p;
	int wh,i;
	size_t size;
	int *buff[MAX_CELL_TYPE];
	int currenttype;
	CELL_INFO cell_info;
	static long currentpos=sizeof(CELL_INFO)*MAX_MAP_PER_SERVER;
	static long currentpos_for_info=0;
	FILE *fp2=fp;

	strcpy(cell_info.fn,fn);
	for(i=0;i<MAX_CELL_TYPE;i++) buff[i]=NULL;
	// read & convert fn
	gat=grfio_read(fn);
	if(gat==NULL)
		return -1;

	printf("\rBitmap file  generating [%d/%d] %-20s  ",m,map_num,fn);
	fflush(stdout);

	cell_info.xs=xs=*(int*)(gat+6);
	cell_info.ys=ys=*(int*)(gat+10);

	cell_info.sizeinint=size=(xs*ys+31)/32;

	wh=map_waterheight(fn+5);

	for(y=0;y<ys;y++){
		p=(struct gat_1cell*)(gat+y*xs*20+14);
		for(x=0;x<xs;x++){
			i=x+y*xs;
			if(wh!=NO_WATER&&p->type==0)
				currenttype=(int)((p->high[0]>wh || p->high[1]>wh || p->high[2]>wh || p->high[3]>wh) ? 3 : 0);
			else 
				currenttype=p->type;
			if(currenttype!=1)
			{
				if(buff[currenttype]==NULL)
					buff[currenttype]=(int*)aCalloc(size,sizeof(int));
				if(currenttype==5)
				{
					if(buff[0]==NULL)
						buff[0]=(int*)aCalloc(size,sizeof(int));
					if( *(buff[0]+i/32)&(0x1<<(i%32)))
						*(buff[0]+i/32)^=0x1<<(i%32) ;
				}
				else
				{
					if(buff[0]==NULL)
						buff[0]=(int*)aCalloc(size,sizeof(int));
					*(buff[0]+i/32)|=0x1<<(i%32);
				}
				*(buff[currenttype]+i/32)|=0x1<<(i%32);
			}
			p++;
		}
	}
	free(gat);

	fseek(fp2,currentpos,SEEK_SET);
	for(i=0;i<MAX_CELL_TYPE;i++)
	{
		if(buff[i]==NULL)
		{
			cell_info.celltype[i]=1;
			cell_info.pos[i]=0;
			 continue;
		}
		cell_info.celltype[i]=i;
		cell_info.pos[i]=ftell(fp2);
		fwrite(buff[i],sizeof(int),size,fp2);
		
		free(buff[i]);
	}
	currentpos=ftell(fp2);

	fseek(fp,currentpos_for_info,SEEK_SET);
	fwrite(&cell_info,1,sizeof(CELL_INFO),fp);
	currentpos_for_info=ftell(fp);

	return 0;
}

/*====================================================
 * マップ1枚読み込み
 * もしmap_read_flagはREAD_FROM_BITMAPならこっちを使う
 * ===================================================*/
static int map_readmapfromfile(int m,char *fn,FILE* fp)
{
	int xs,ys;
	int size,j;
	CELL_INFO cell_info;

	printf("\rmap reading [%d/%d] %-20s",m,map_num,cell_info.fn);
	fflush(stdout);

	fseek(fp,sizeof(CELL_INFO)*m,SEEK_SET);
	fread(&cell_info,sizeof(CELL_INFO),1,fp);
	
	map[m].gat_fileused[MAX_CELL_TYPE+1]=aMalloc(sizeof(int)*cell_info.sizeinint);

	for(j=0;j<MAX_CELL_TYPE;j++) 
	{
		if(cell_info.celltype[j]==1)
		{
			map[m].gat_fileused[j]=NULL;
			continue;	
		}	
		map[m].gat_fileused[j]=aMalloc(sizeof(int)*cell_info.sizeinint);
		fseek(fp,cell_info.pos[j],SEEK_SET);
		fread(map[m].gat_fileused[j],sizeof(int),cell_info.sizeinint,fp);
	}

	map[m].gat = (unsigned char *)map[m].gat_fileused[0];
	map[m].xs=xs=cell_info.xs;
	map[m].ys=ys=cell_info.ys;
	map[m].m=m;
	memset(&map[m].flag,0,sizeof(map[m].flag));
	map[m].npc_num=0;
	map[m].users=0;
	map[m].bxs=(xs+BLOCK_SIZE-1)/BLOCK_SIZE;
	map[m].bys=(ys+BLOCK_SIZE-1)/BLOCK_SIZE;
	size = map[m].bxs * map[m].bys * sizeof(struct block_list*);
	map[m].block = (struct block_list **)aCalloc(1,size);
	map[m].block_mob = (struct block_list **)aCalloc(1,size);
	size = map[m].bxs*map[m].bys*sizeof(int);
	map[m].block_count = (int *)aCalloc(1,size);
	map[m].block_mob_count=(int *)aCalloc(1,size);
	strdb_insert(map_db,map[m].name,&map[m]);


	return 0;
}
/*==========================================
 * マップ1枚読み込み
 * もしmap_read_flagはREAD_FROM_GATならこっちを使う
 * ===================================================*/
static int map_readmap(int m,char *fn,FILE* fp)
{
	unsigned char *gat;
	int s;
	int x,y,xs,ys;
	struct gat_1cell {float high[4]; int type;} *p;
	int wh;
	size_t size;

	// read & convert fn
	gat=grfio_read(fn);
	if(gat==NULL)
		return -1;

	printf("\rmap reading [%d/%d] %-20s  ",m,map_num,fn);
	fflush(stdout);

	map[m].m=m;
	xs=map[m].xs=*(int*)(gat+6);
	ys=map[m].ys=*(int*)(gat+10);
	map[m].gat = (unsigned char *)aCalloc(s = map[m].xs * map[m].ys,sizeof(unsigned char));
	map[m].npc_num=0;
	map[m].users=0;
	memset(&map[m].flag,0,sizeof(map[m].flag));
	wh=map_waterheight(map[m].name);
	for(y=0;y<ys;y++){
		p=(struct gat_1cell*)(gat+y*xs*20+14);
		for(x=0;x<xs;x++){
			if(wh!=NO_WATER && p->type==0){
				// 水場判定
				map[m].gat[x+y*xs]=(p->high[0]>wh || p->high[1]>wh || p->high[2]>wh || p->high[3]>wh) ? 3 : 0;
			} else {
				map[m].gat[x+y*xs]=p->type;
			}
			p++;
		}
	}
	free(gat);

	map[m].bxs=(xs+BLOCK_SIZE-1)/BLOCK_SIZE;
	map[m].bys=(ys+BLOCK_SIZE-1)/BLOCK_SIZE;
	size = map[m].bxs * map[m].bys * sizeof(struct block_list*);
	map[m].block = (struct block_list **)aCalloc(1,size);
	map[m].block_mob = (struct block_list **)aCalloc(1,size);
	size = map[m].bxs*map[m].bys*sizeof(int);
	map[m].block_count = (int *)aCalloc(1,size);
	map[m].block_mob_count=(int *)aCalloc(1,size);
	strdb_insert(map_db,map[m].name,&map[m]);

//	printf("%s read done\n",fn);

	return 0;
}

/*==========================================
 * 全てのmapデータを読み込む
 *------------------------------------------
 */
int map_readallmap(void)
{
	int i;
	int (*map_read_func)(int,char*,FILE*)=NULL;
	char fn[256];
	FILE *fp=NULL;

	if(map_read_flag==READ_FROM_BITMAP)//ビットマップファイルから
	{
		puts("Read map from [bitmap] file.");
		fp=fopen(map_bitmap_filename,"rb");
		if(fp==NULL)//ビットマップファイルが存在しなかった
		{
			puts("Bitmap file not found,now creating.");
			fp=fopen(map_bitmap_filename,"wb");
			map_read_func=map_createbitmap;//作成する
		}
		else
			map_read_func=map_readmapfromfile;//見付かったらただちに読み出す
	}
	else //grfファイルから
	{
		puts("Read map from [grf] files.");
		map_read_func=map_readmap;
	}
	// 先に全部のマップの存在を確認
	for(i=0;i<map_num;i++){
		if(strstr(map[i].name,".gat")==NULL)
			continue;
		sprintf(fn,"data\\%s",map[i].name);
		grfio_size(fn);
	}
	for(i=0;i<map_num;i++){
		if(strstr(map[i].name,".gat")==NULL)
			continue;
		sprintf(fn,"data\\%s",map[i].name);
		map_read_func(i,fn,fp);

	}

	if(map_read_func==map_createbitmap) //ビットマップ作成終了
	{
		printf("\n%s\n","Generation completed.");
		printf("%s","Now reading");
		fclose(fp);
		fp=fopen(map_bitmap_filename,"rb");
		for(i=0;i<map_num;i++)
		{
			if(strstr(map[i].name,".gat")==NULL)
				continue;
			map_readmapfromfile(i,NULL,fp);//作成されたファイルから読み出す
		}
	}

	free(waterlist);
	printf("\rmap read done. (%d map) %24s\n",map_num,"");

	if(fp!=NULL)	fclose(fp);

	return 0;
}
/*==========================================
 * 読み込むmapを追加する
 *------------------------------------------
 */
int map_addmap(char *mapname)
{
	if( strcmpi(mapname,"clear")==0 ){
		map_num=0;
		return 0;
	}

	if(map_num>=MAX_MAP_PER_SERVER-1){
		printf("too many map\n");
		return 1;
	}
	memcpy(map[map_num].name,mapname,24);
	map_num++;
	return 0;
}
/*==========================================
 * 読み込むmapを削除する
 *------------------------------------------
 */
int map_delmap(char *mapname)
{
	int i;

	if( strcmpi(mapname,"all")==0 ){
		map_num=0;
		return 0;
	}

	for(i=0;i<map_num;i++){
		if(strcmp(map[i].name,mapname)==0){
			memmove(map+i,map+i+1,sizeof(map[0])*(map_num-i-1));
			map_num--;
		}
	}
	return 0;
}
/*==========================================
 * @whoのDB版
 *------------------------------------------
 */
 int map_who_sub(void *key,void *data,va_list ap)
{
	struct charid2nick *p;
	int fd;

	nullpo_retr(-1, ap);
	nullpo_retr(-1, data);
	nullpo_retr(-1, p=(struct charid2nick *)data);

//printf("who: %s %d %d\n",p->nick,(int)p->ip,p->port);

	fd=va_arg(ap,int);

	if( p->ip != 0 && 
		p->port != 0 &&
		!(battle_config.hide_GM_session && pc_numisGM(p->account_id))
	)
		clif_displaymessage(fd, p->nick);

	return 0;
}
int map_who(int fd){
	numdb_foreach( charid_db, map_who_sub, fd );
	return 0;
}
/*==========================================
 * 設定ファイルを読み込む
 *------------------------------------------
 */
int map_config_read(char *cfgName)
{
	int i;
	char line[1024],w1[1024],w2[1024];
	FILE *fp;
	struct hostent *h=NULL;

	fp=fopen(cfgName,"r");
	if(fp==NULL){
		printf("file not found: %s\n",cfgName);
		return 1;
	}
	while(fgets(line,1020,fp)){
		if(line[0] == '/' && line[1] == '/')
			continue;
		i=sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if(i!=2)
			continue;
		if(strcmpi(w1,"userid")==0){
			chrif_setuserid(w2);
		} else if(strcmpi(w1,"passwd")==0){
			chrif_setpasswd(w2);
		} else if(strcmpi(w1,"char_ip")==0){
			h = gethostbyname (w2);
			if(h != NULL) {
				printf("Character sever IP address : %s -> %d.%d.%d.%d\n",w2,(unsigned char)h->h_addr[0],(unsigned char)h->h_addr[1],(unsigned char)h->h_addr[2],(unsigned char)h->h_addr[3]);
				sprintf(w2,"%d.%d.%d.%d",(unsigned char)h->h_addr[0],(unsigned char)h->h_addr[1],(unsigned char)h->h_addr[2],(unsigned char)h->h_addr[3]);
			}
			chrif_setip(w2);
		} else if(strcmpi(w1,"char_port")==0){
			chrif_setport(atoi(w2));
		} else if(strcmpi(w1,"map_ip")==0){
			h = gethostbyname (w2);
			if(h != NULL) {
				printf("Map server IP address : %s -> %d.%d.%d.%d\n",w2,(unsigned char)h->h_addr[0],(unsigned char)h->h_addr[1],(unsigned char)h->h_addr[2],(unsigned char)h->h_addr[3]);
				sprintf(w2,"%d.%d.%d.%d",(unsigned char)h->h_addr[0],(unsigned char)h->h_addr[1],(unsigned char)h->h_addr[2],(unsigned char)h->h_addr[3]);
			}
			clif_setip(w2);
		} else if(strcmpi(w1,"map_port")==0){
			clif_setport(atoi(w2));
		} else if(strcmpi(w1,"water_height")==0){
			map_readwater(w2);
		} else if(strcmpi(w1,"gm_account_filename")==0){
			pc_set_gm_account_fname(w2);
		} else if(strcmpi(w1,"map")==0){
			map_addmap(w2);
		} else if(strcmpi(w1,"delmap")==0){
			map_delmap(w2);
		} else if(strcmpi(w1,"npc")==0){
			npc_addsrcfile(w2);
		} else if(strcmpi(w1,"delnpc")==0){
			npc_delsrcfile(w2);
		} else if(strcmpi(w1,"data_grf")==0){
			grfio_setdatafile(w2);
		} else if(strcmpi(w1,"sdata_grf")==0){
			grfio_setsdatafile(w2);
		} else if(strcmpi(w1,"adata_grf")==0){
			grfio_setadatafile(w2);
		} else if(strcmpi(w1,"packet_parse_time")==0){
			packet_parse_time=atoi(w2);
			if(packet_parse_time <= 0)
				packet_parse_time = 0;
		} else if(strcmpi(w1,"autosave_time")==0){
			autosave_interval=atoi(w2)*1000;
			if(autosave_interval <= 0)
				autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
		} else if(strcmpi(w1,"motd_txt")==0){
			strncpy(motd_txt,w2,256);
		} else if(strcmpi(w1,"help_txt")==0){
			strncpy(help_txt,w2,256);
		} else if(strcmpi(w1,"mapreg_txt")==0){
			strncpy(mapreg_txt,w2,256);
		}else if(strcmpi(w1,"read_map_from_bitmap")==0){
			map_read_flag=atoi(w2);
			if(map_read_flag==0) //grfファイル使用
			{
				map_getcell=map_getcell_gat;
				map_getcellp=map_getcellp_gat;
			}
			else if(map_read_flag==1)//ビットファイル使用
			{
				map_getcell=map_getcell_bitmap;
				map_getcellp=map_getcellp_bitmap;
			}
			else continue;//指定なしの場合はデフォルト方式(grfファイルから)使用
		}else if(strcmpi(w1,"map_bitmap_path")==0){
			strncpy(map_bitmap_filename,w2,255);
		}else if(strcmpi(w1,"import")==0){
			map_config_read(w2);
		}
	}
	fclose(fp);

	return 0;
}

/*==========================================
 * map鯖終了時処理
 *------------------------------------------
 */
static int id_db_final(void *key,void *data,va_list ap)
{
	return 0;
}
static int map_db_final(void *key,void *data,va_list ap)
{
//	char *name;

//	nullpo_retr(0, name=data);

//	free(name);

	return 0;
}
static int nick_db_final(void *key,void *data,va_list ap)
{
	char *nick;

	nullpo_retr(0, nick=data);

	free(nick);

	return 0;
}
static int charid_db_final(void *key,void *data,va_list ap)
{
	struct charid2nick *p;

	nullpo_retr(0, p=data);

	free(p);

	return 0;
}
void do_final(void)
{
	int i,j;

	chrif_mapactive(0); //マップサーバー停止中

	do_final_npc();
	do_final_script();
	do_final_itemdb();
	do_final_storage();
	do_final_guild();
	do_final_clif();
	do_final_chrif();
	do_final_pc();
	do_final_party();

	for(i=0;i<=map_num;i++){
		if(map[i].gat)
		{	
			if(map_read_flag==READ_FROM_GAT)
				free(map[i].gat);
			else if(map_read_flag==READ_FROM_BITMAP)
				for(j=0;j<=MAX_CELL_TYPE+1;j++) 
					if(map[i].gat_fileused[i])
					{
						free(map[i].gat_fileused[j]);	
						map[i].gat_fileused[j]=NULL;	
					}
			map[i].gat=NULL;//ビットマップファイル使う場合もちゃんとgat_fileused[0]に指してるので片付け
		}
		if(map[i].block) free(map[i].block);
		if(map[i].block_mob) free(map[i].block_mob);
		if(map[i].block_count) free(map[i].block_count);
		if(map[i].block_mob_count) free(map[i].block_mob_count);
	}

	if(map_db)
		strdb_final(map_db,map_db_final);
	if(nick_db)
		strdb_final(nick_db,nick_db_final);
	if(charid_db)
		numdb_final(charid_db,charid_db_final);
	if(id_db)
		numdb_final(id_db,id_db_final);
	exit_dbn();

	do_final_timer();
}

/*==========================================
 * map鯖初期化の大元
 *------------------------------------------
 */
int do_init(int argc,char *argv[])
{
	srand(gettick());

	if(map_config_read((argc<2)? MAP_CONF_NAME:argv[1]))
		exit(1);
	battle_config_read((argc>2)? argv[2]:BATTLE_CONF_FILENAME);
	atcommand_config_read((argc>3)? argv[3]:ATCOMMAND_CONF_FILENAME);
	script_config_read((argc>4)? argv[4]:SCRIPT_CONF_NAME);
	msg_config_read((argc>5)? argv[5]:MSG_CONF_NAME);

	atexit(do_final);

	id_db = numdb_init();
	map_db = strdb_init(16);
	nick_db = strdb_init(24);
	charid_db = numdb_init();

	grfio_init((argc>6)? argv[6]:GRF_PATH_FILENAME);
	map_readallmap();

	add_timer_func_list(map_clearflooritem_timer,"map_clearflooritem_timer");

	do_init_chrif();
	do_init_clif();
	do_init_itemdb();
	do_init_mob();	// npcの初期化時内でmob_spawnして、mob_dbを参照するのでinit_npcより先
	do_init_script();
	do_init_npc();
	do_init_pc();
	do_init_party();
	do_init_guild();
	do_init_storage();
	do_init_skill();
	do_init_pet();
	npc_event_do_oninit();	// npcのOnInitイベント実行

	return 0;
}
