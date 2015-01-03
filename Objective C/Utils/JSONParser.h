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

id objectForKey(NSDictionary * dict, NSString * ID, NSString * fullName);

//Project parser

PROJECT_DATA parseBloc(NSDictionary * bloc);
PROJECT_DATA_EXTRA parseBlocExtra(NSDictionary * bloc);
void* parseProjectJSON(TEAMS_DATA* team, NSDictionary * remoteData, uint * nbElem, bool parseExtra);

#define JSON_PROJ_AUTHOR_URL	@"ROOT-URL"

#define JSON_PROJ_PROJECTS @"0"
#define JSON_PROJ_ID @"1"
#define JSON_PROJ_PROJECT_NAME @"2"
#define JSON_PROJ_CHAPTERS @"3"
#define JSON_PROJ_CHAP_DETAILS @"4"
#define JSON_PROJ_CHAP_FIRST @"5"
#define JSON_PROJ_CHAP_LAST @"6"
#define JSON_PROJ_CHAP_JUMP @"7"
#define JSON_PROJ_VOLUMES @"8"
#define JSON_PROJ_VOL_READING_NAME @"9"
#define JSON_PROJ_VOL_READING_ID @"10"
#define JSON_PROJ_VOL_INTERNAL_ID @"11"
#define JSON_PROJ_VOL_DESCRIPTION @"12"
#define JSON_PROJ_DESCRIPTION @"13"
#define JSON_PROJ_AUTHOR @"14"
#define JSON_PROJ_STATUS @"15"
#define JSON_PROJ_TYPE @"16"
#define JSON_PROJ_ASIAN_ORDER @"17"
#define JSON_PROJ_CATEGORY @"18"
#define JSON_PROJ_HASH_URL_LARGE @"19"
#define JSON_PROJ_HASH_URL_SMALL @"20"
#define JSON_PROJ_PRICE @"21"

//Repo parser

#define JSON_REPO_MIN_VERSION		@"0"
#define JSON_REPO_NAME				@"1"
#define JSON_REPO_TYPE				@"2"
#define JSON_REPO_URL				@"3"
#define JSON_REPO_DESCRIPTION		@"4"
#define JSON_REPO_LANGUAGE			@"5"
#define JSON_REPO_TRUSTED			@"6"
#define JSON_REPO_REPO_TREE			@"7"
#define JSON_REPO_SUB_ID			@"8"
#define JSON_REPO_SUB_WEBSITE		@"9"
#define JSON_REPO_SUB_IMAGE			@"10"
#define JSON_REPO_SUB_RETINA_IMAGE	@"11"
#define JSON_REPO_SUB_IMAGE_HASH	@"12"
#define JSON_REPO_SUB_MATURE		@"13"
