#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "mmo.h"

int storage_storageopen(struct map_session_data *sd);
int storage_storageadd(struct map_session_data *sd,int index,int amount);
int storage_storageget(struct map_session_data *sd,int index,int amount);
int storage_storageaddfromcart(struct map_session_data *sd,int index,int amount);
int storage_storagegettocart(struct map_session_data *sd,int index,int amount);
int storage_storageclose(struct map_session_data *sd);
int do_init_storage(void);
void do_final_storage(void);
int account2storage(int account_id);
int storage_storage_quit(struct map_session_data *sd);
int storage_storage_save(struct map_session_data *sd);

extern struct storage *storage;

#endif
