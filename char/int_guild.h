#ifndef _INT_GUILD_H_
#define _INT_GUILD_H_

int inter_guild_init();
int inter_guild_save();
int inter_guild_parse_frommap(int fd);

extern char guild_txt[1024];
extern char castle_txt[1024];

#endif
