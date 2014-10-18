/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **		Source code and assets are property of Taiki, distribution is stricly forbidden		**
 **                                                                                         **
 *********************************************************************************************/

PROJECT_DATA parseBloc(NSDictionary * bloc);
PROJECT_DATA_EXTRA parseBlocExtra(NSDictionary * bloc);
void* parseJSON(TEAMS_DATA* team, NSDictionary * remoteData, uint * nbElem, bool parseExtra);

id objectForKey(NSDictionary * dict, NSString * ID, NSString * fullName);

#define JSON_RP_AUTHOR_NAME @"ROOT-NAME"
#define JSON_RP_AUTHOR_URL	@"ROOT-URL"

#define JSON_RP_PROJECTS @"0"
#define JSON_RP_ID @"1"
#define JSON_RP_PROJECT_NAME @"2"
#define JSON_RP_CHAPTERS @"3"
#define JSON_RP_CHAP_DETAILS @"4"
#define JSON_RP_CHAP_FIRST @"5"
#define JSON_RP_CHAP_LAST @"6"
#define JSON_RP_CHAP_JUMP @"7"
#define JSON_RP_VOLUMES @"8"
#define JSON_RP_VOL_READING_NAME @"9"
#define JSON_RP_VOL_READING_ID @"10"
#define JSON_RP_VOL_INTERNAL_ID @"11"
#define JSON_RP_VOL_DESCRIPTION @"12"
#define JSON_RP_DESCRIPTION @"13"
#define JSON_RP_AUTHOR @"14"
#define JSON_RP_STATUS @"15"
#define JSON_RP_TYPE @"16"
#define JSON_RP_ASIAN_ORDER @"17"
#define JSON_RP_CATEGORY @"18"
#define JSON_RP_HASH_URL_LARGE @"19"
#define JSON_RP_HASH_URL_SMALL @"20"
#define JSON_RP_PRICE @"21"
