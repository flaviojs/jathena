// $Id: itemdb.c,v 1.8 2003/06/29 05:56:44 lemit Exp $
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db.h"
#include "grfio.h"
#include "map.h"
#include "battle.h"
#include "itemdb.h"
#include "script.h"
#include "pc.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

#define MAX_RANDITEM	2000

// ** ITEMDB_OVERRIDE_NAME_VERBOSE **
//   定義すると、itemdb.txtとgrfで名前が異なる場合、表示します.
//#define ITEMDB_OVERRIDE_NAME_VERBOSE	1

static struct dbt* item_db;

static struct random_item_data blue_box[MAX_RANDITEM],violet_box[MAX_RANDITEM],card_album[MAX_RANDITEM],gift_box[MAX_RANDITEM],scroll[MAX_RANDITEM];
static int blue_box_count=0,violet_box_count=0,card_album_count=0,gift_box_count=0,scroll_count=0;
static int blue_box_default=0,violet_box_default=0,card_album_default=0,gift_box_default=0,scroll_default=0;

/*==========================================
 * 名前で検索用
 *------------------------------------------
 */
int itemdb_searchname_sub(void *key,void *data,va_list ap)
{
	struct item_data *item=(struct item_data *)data,**dst;
	char *str;
	str=va_arg(ap,char *);
	dst=va_arg(ap,struct item_data **);
	if(strcmpi(item->name,str)==0 || strcmp(item->jname,str)==0)
		*dst=item;
	return 0;
}
/*==========================================
 * 名前で検索
 *------------------------------------------
 */
struct item_data* itemdb_searchname(const char *str)
{
	struct item_data *item=NULL;
	numdb_foreach(item_db,itemdb_searchname_sub,str,&item);
	return item;
}

/*==========================================
 * 箱系アイテム検索
 *------------------------------------------
 */
int itemdb_searchrandomid(int flags)
{
	int nameid=0,i,index,count;
	struct random_item_data *list=NULL;

	struct {
		int nameid,count;
		struct random_item_data *list;
	} data[] ={
		{ 0,0,NULL },
		{ blue_box_default	,blue_box_count		,blue_box	 },
		{ violet_box_default,violet_box_count	,violet_box	 },
		{ card_album_default,card_album_count	,card_album	 },
		{ gift_box_default	,gift_box_count		,gift_box	 },
		{ scroll_default	,scroll_count		,scroll		 },
	};

	if(flags>=1 && flags<=5){
		nameid=data[flags].nameid;
		count=data[flags].count;
		list=data[flags].list;

		if(count > 0) {
			for(i=0;i<MAX_RANDITEM;i++) {
				index = rand()%count;
				if(	rand()%1000000 < list[index].per) {
					nameid = list[index].nameid;
					break;
				}
			}
		}
	}
//	printf("get %d\n",nameid);
	return nameid;
}

/*==========================================
 * DBの存在確認
 *------------------------------------------
 */
struct item_data* itemdb_exists(int nameid)
{
	return numdb_search(item_db,nameid);
}
/*==========================================
 * DBの検索
 *------------------------------------------
 */
struct item_data* itemdb_search(int nameid)
{
	struct item_data *id;

	id=numdb_search(item_db,nameid);
	if(id) return id;

	id=malloc(sizeof(struct item_data));
	if(id==NULL){
		printf("out of memory : itemdb_search\n");
		exit(1);
	}
	memset(id,0,sizeof(struct item_data));
	numdb_insert(item_db,nameid,id);

	id->nameid=nameid;
	id->value_buy=10;
	id->value_sell=id->value_buy/2;
	id->weight=10;
	id->sex=2;
	id->elv=0;
	id->class=0xffffffff;
	id->flag.available=0;
	id->flag.value_notdc=0;  //一応・・・
	id->flag.value_notoc=0;
	id->view_id=0;

	if(nameid>500 && nameid<600)
		id->type=0;   //heal item
	else if(nameid>600 && nameid<700)
		id->type=2;   //use item
	else if((nameid>700 && nameid<1100) ||
			(nameid>7000 && nameid<8000))
		id->type=3;   //correction
	else if(nameid>=1750 && nameid<1771)
		id->type=10;  //arrow
	else if(nameid>1100 && nameid<2000)
		id->type=4;   //weapon
	else if((nameid>2100 && nameid<3000) ||
			(nameid>5000 && nameid<6000))
		id->type=5;   //armor
	else if(nameid>4000 && nameid<5000)
		id->type=6;   //card
	else if(nameid>9000 && nameid<10000)
		id->type=7;   //egg
	else if(nameid>10000)
		id->type=8;   //petequip

	return id;
}

/*==========================================
 *
 *------------------------------------------
 */
int itemdb_isequip(int nameid)
{
	int type=itemdb_type(nameid);
	if(type==0 || type==2 || type==3 || type==6 || type==10)
		return 0;
	return 1;
}
/*==========================================
 *
 *------------------------------------------
 */
int itemdb_isequip2(struct item_data *data)
{
	if(data) {
		int type=data->type;
		if(type==0 || type==2 || type==3 || type==6 || type==10)
			return 0;
		else
			return 1;
	}
	return 0;
}

//
// 初期化
//
/*==========================================
 *
 *------------------------------------------
 */
static int itemdb_read_itemslottable(void)
{
	char *buf,*p;
	int s;

	buf=grfio_read("data\\itemslottable.txt");
	if(buf==NULL)
		return -1;
	s=grfio_size("data\\itemslottable.txt");
	buf[s]=0;
	for(p=buf;p-buf<s;){
		int nameid,equip;
		sscanf(p,"%d#%d#",&nameid,&equip);
		itemdb_search(nameid)->equip=equip;
		p=strchr(p,10);
		if(!p) break;
		p++;
		p=strchr(p,10);
		if(!p) break;
		p++;
	}
	free(buf);

	return 0;
}

/*==========================================
 * アイテムデータベースの読み込み
 *------------------------------------------
 */
static int itemdb_readdb(void)
{
	FILE *fp;
	char line[1024];
	int ln=0;
	int nameid,j;
	char *str[32],*p,*np;
	struct item_data *id;

	fp=fopen("db/item_db.txt","r");
	if(fp==NULL){
		printf("can't read db/item_db.txt\n");
		exit(1);
	}
	while(fgets(line,1020,fp)){
		if(line[0]=='/' && line[1]=='/')
			continue;
		memset(str,0,sizeof(str));
		for(j=0,np=p=line;j<17 && p;j++){
			str[j]=p;
			p=strchr(p,',');
			if(p){ *p++=0; np=p; }
		}
		if(str[0]==NULL)
			continue;
		
		nameid=atoi(str[0]);
		if(nameid<=0 || nameid>=20000)
			continue;
		ln++;

		//ID,Name,Jname,Type,Price,Sell,Weight,ATK,DEF,Range,Slot,Job,Gender,Loc,wLV,eLV,View
		id=itemdb_search(nameid);
		memcpy(id->name,str[1],24);
		memcpy(id->jname,str[2],24);
		id->type=atoi(str[3]);
		// buy≠sell*2 は item_value_db.txt で指定してください。
		if (atoi(str[5])) {		// sell値を優先とする
			id->value_buy=atoi(str[5])*2;
			id->value_sell=atoi(str[5]);
		} else {
			id->value_buy=atoi(str[4]);
			id->value_sell=atoi(str[4])/2;
		}
		id->weight=atoi(str[6]);
		id->atk=atoi(str[7]);
		id->def=atoi(str[8]);
		id->range=atoi(str[9]);
		id->slot=atoi(str[10]);
		id->class=atoi(str[11]);
		id->sex=atoi(str[12]);
		if(id->equip != atoi(str[13])){
			//printf("%d : equip point %d -> %d\n",nameid,id->equip,atoi(str[13]));
			id->equip=atoi(str[13]);
		}
		id->wlv=atoi(str[14]);
		id->elv=atoi(str[15]);
		id->look=atoi(str[16]);
		id->flag.available=1;
		id->flag.value_notdc=0;
		id->flag.value_notoc=0;
		id->view_id=0;

		id->use_script=NULL;
		id->equip_script=NULL;

		if((p=strchr(np,'{'))==NULL)
			continue;
		id->use_script = parse_script(p,0);
		if((p=strchr(p+1,'{'))==NULL)
			continue;
		id->equip_script = parse_script(p,0);
	}
	fclose(fp);
	printf("read db/item_db.txt done (count=%d)\n",ln);
	return 0;
}

/*==========================================
 * アイテム価格テーブルのオーバーライド
 *------------------------------------------
 */
static int itemdb_read_itemvaluedb(void)
{
	FILE *fp;
	char line[1024];
	int ln=0;
	int nameid,j;
	char *str[32],*p;
	struct item_data *id;
	
	if( (fp=fopen("db/item_value_db.txt","r"))==NULL){
		printf("can't read db/item_value_db.txt\n");
		return -1;
	}

	while(fgets(line,1020,fp)){
		if(line[0]=='/' && line[1]=='/')
			continue;
		memset(str,0,sizeof(str));
		for(j=0,p=line;j<7 && p;j++){
			str[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(str[0]==NULL)
			continue;

		nameid=atoi(str[0]);
		if(nameid<=0 || nameid>=20000)
			continue;

		if( !(id=itemdb_exists(nameid)) )
			continue;

		ln++;
		// それぞれ記述した個所のみオーバーライト
		if(str[3]!=NULL && *str[3]){
			id->value_buy=atoi(str[3]);
		}
		if(str[4]!=NULL && *str[4]){
			id->value_sell=atoi(str[4]);
		}
		if(str[5]!=NULL && *str[5]){
			id->flag.value_notdc=(atoi(str[5])==0)? 0:1;
		}
		if(str[6]!=NULL && *str[6]){
			id->flag.value_notoc=(atoi(str[6])==0)? 0:1;
		}
	}
	fclose(fp);
	printf("read db/item_value_db.txt done (count=%d)\n",ln);
	return 0;
}

/*==========================================
 * 装備可能職業テーブルのオーバーライド
 *------------------------------------------
 */
static int itemdb_read_classequipdb(void)
{
	FILE *fp;
	char line[1024];
	int ln=0;
	int nameid,i,j;
	char *str[32],*p;
	struct item_data *id;
	
	if( (fp=fopen("db/class_equip_db.txt","r"))==NULL ){
		printf("can't read db/class_equip_db.txt\n");
		return -1;
	}
	while(fgets(line,1020,fp)){
		if(line[0]=='/' && line[1]=='/')
			continue;
		memset(str,0,sizeof(str));
		for(j=0,p=line;j<MAX_PC_CLASS+4 && p;j++){
			str[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(str[0]==NULL)
			continue;

		nameid=atoi(str[0]);
		if(nameid<=0 || nameid>=20000)
			continue;

		ln++;

		//ID,Name,class1,class2,class3, ...... ,class22
		if( !(id=itemdb_exists(nameid)) )
			continue;
			
		id->class = 0;
		if(str[2] && str[2][0]) {
			j = atoi(str[2]);
			id->sex = j;
		}
		if(str[3] && str[3][0]) {
			j = atoi(str[3]);
			id->elv = j;
		}
		for(i=4;i<MAX_PC_CLASS+4;i++) {
			if(str[i]!=NULL && str[i][0]) {
				j = atoi(str[i]);
				if(j == 99) {
					for(j=0;j<MAX_PC_CLASS;j++)
						id->class |= 1<<j;
					break;
				}
				else if(j == 9999)
					break;
				else if(j >= 0 && j < MAX_PC_CLASS) {
					id->class |= 1<<j;
					if(j == 7)
						id->class |= 1<<13;
					if(j == 14)
						id->class |= 1<<21;
				}
			}
		}
	}
	fclose(fp);
	printf("read db/class_equip_db.txt done (count=%d)\n",ln);
	return 0;
}

/*==========================================
 * ランダムアイテム出現データの読み込み
 *------------------------------------------
 */
static int itemdb_read_randomitem()
{
	FILE *fp;
	char line[1024];
	int ln=0;
	int nameid,i,j;
	char *str[10],*p;
	
	const struct {
		char filename[64];
		struct random_item_data *pdata;
		int *pcount,*pdefault;
	} data[] = {
		{"db/item_bluebox.txt",		blue_box,	&blue_box_count, &blue_box_default		},
		{"db/item_violetbox.txt",	violet_box,	&violet_box_count, &violet_box_default	},
		{"db/item_cardalbum.txt",	card_album,	&card_album_count, &card_album_default	},
		{"db/item_giftbox.txt",	gift_box,	&gift_box_count, &gift_box_default	},
		{"db/item_scroll.txt",	scroll,	&scroll_count, &scroll_default	},
	};

	for(i=0;i<sizeof(data)/sizeof(data[0]);i++){
		struct random_item_data *pd=data[i].pdata;
		int *pc=data[i].pcount;
		int *pdefault=data[i].pdefault;
		char *fn=data[i].filename;

		*pdefault = 0;
		if( (fp=fopen(fn,"r"))==NULL ){
			printf("can't read %s\n",fn);
			continue;
		}

		while(fgets(line,1020,fp)){
			if(line[0]=='/' && line[1]=='/')
				continue;
			memset(str,0,sizeof(str));
			for(j=0,p=line;j<3 && p;j++){
				str[j]=p;
				p=strchr(p,',');
				if(p) *p++=0;
			}

			if(str[0]==NULL)
				continue;

			nameid=atoi(str[0]);
			if(nameid<0 || nameid>=20000)
				continue;
			if(nameid == 0) {
				if(str[2])
					*pdefault = atoi(str[2]);
				continue;
			}

			if(str[2]){
				pd[ *pc   ].nameid = nameid;
				pd[(*pc)++].per = atoi(str[2]);
			}

			if(ln >= MAX_RANDITEM)
				break;
			ln++;
		}
		fclose(fp);
		printf("read %s done (count=%d)\n",fn,*pc);
	}

	return 0;
}
/*==========================================
 * アイテム使用可能フラグのオーバーライド
 *------------------------------------------
 */
static int itemdb_read_itemavail(void)
{
	FILE *fp;
	char line[1024];
	int ln=0;
	int nameid,j,k;
	char *str[10],*p;
	
	if( (fp=fopen("db/item_avail.txt","r"))==NULL ){
		printf("can't read db/item_avail.txt\n");
		return -1;
	}
	
	while(fgets(line,1020,fp)){
		struct item_data *id;
		if(line[0]=='/' && line[1]=='/')
			continue;
		memset(str,0,sizeof(str));
		for(j=0,p=line;j<2 && p;j++){
			str[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}

		if(str[0]==NULL)
			continue;

		nameid=atoi(str[0]);
		if(nameid<0 || nameid>=20000 || !(id=itemdb_exists(nameid)) )
			continue;
		k=atoi(str[1]);
		if(k > 0) {
			id->flag.available = 1;
			id->view_id = k;
		}
		else
			id->flag.available = 0;
		ln++;
	}
	fclose(fp);
	printf("read db/item_avail.txt done (count=%d)\n",ln);
	return 0;
}

/*==========================================
 * アイテムの名前テーブルを読み込む
 *------------------------------------------
 */
static int itemdb_read_itemnametable(void)
{
	char *buf,*p;
	int s;

	buf=grfio_reads("data\\idnum2itemdisplaynametable.txt",&s);

	if(buf==NULL)
		return -1;

	buf[s]=0;
	for(p=buf;p-buf<s;){
		int nameid;
		char buf2[64];

		if(	sscanf(p,"%d#%[^#]#",&nameid,buf2)==2 ){

			if( itemdb_exists(nameid) &&
				strncmp(itemdb_search(nameid)->jname,buf2,24)!=0 )
#ifdef ITEMDB_OVERRIDE_NAME_VERBOSE
				printf("[override] %d %s => %s\n",nameid
					,itemdb_search(nameid)->jname,buf2);
#endif
			memcpy(itemdb_search(nameid)->jname,buf2,24);
		}
		
		p=strchr(p,10);
		if(!p) break;
		p++;
	}
	free(buf);
	printf("read data\\idnum2itemdisplaynametable.txt done.\n");

	return 0;
}


/*==========================================
 *
 *------------------------------------------
 */
static int itemdb_final(void *key,void *data,va_list ap)
{
	struct item_data *id;

	id=data;
	if(id->use_script)
		free(id->use_script);
	if(id->equip_script)
		free(id->equip_script);
	free(id);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
void do_final_itemdb(void)
{
	if(item_db){
		numdb_final(item_db,itemdb_final);
		item_db=NULL;
	}
}

/*
static FILE *dfp;
static int itemdebug(void *key,void *data,va_list ap){
//	struct item_data *id=(struct item_data *)data;
	fprintf(dfp,"%6d",(int)key);
	return 0;
}
void itemdebugtxt()
{
	dfp=fopen("itemdebug.txt","wt");
	numdb_foreach(item_db,itemdebug);
	fclose(dfp);
}
*/

/*==========================================
 *
 *------------------------------------------
 */
int do_init_itemdb(void)
{
	item_db = numdb_init();

	itemdb_read_itemslottable();
	itemdb_readdb();
	itemdb_read_classequipdb();
	itemdb_read_itemvaluedb();
	itemdb_read_randomitem();
	itemdb_read_itemavail();
	if(battle_config.item_name_override_grffile)
		itemdb_read_itemnametable();

	return 0;
}
