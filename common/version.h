#ifndef _VERSION_H_
#define _VERSION_H_

#define ATHENA_MAJOR_VERSION	2	// Major Version
#define ATHENA_MINOR_VERSION	1	// Minor Version
#define ATHENA_REVISION			1	// Revision

#define ATHENA_RELEASE_FLAG		1	// 1=Develop,0=Stable
#define ATHENA_OFFICIAL_FLAG	1	// 1=Mod,0=Official

#define ATHENA_SERVER_LOGIN		1	// login server
#define ATHENA_SERVER_CHAR		2	// char server
#define ATHENA_SERVER_INTER		4	// inter server
#define ATHENA_SERVER_MAP		8	// map server

// ATHENA_MOD_VERSIONはパッチ番?です。
// これは無?に変えなくても気が向いたら変える程度の扱いで。
// （?回アップ?ードの度に変更するのも面倒と思われるし、そもそも
// 　この?目を参照する人がいるかどうかで疑問だから。）
// その程度の扱いなので、サーバーに問い?わせる側も、?くまで目安程度の扱いで
// ?んまり信用しないこと。
// 鯖snapshotの?や、大きな変更が?った場?は設定してほしいです。
// C言語の仕様上、最?に0を付けると8進?になるので間違えないで下さい。
#define ATHENA_MOD_VERSION		800	// mod version (patch No.)

#endif
