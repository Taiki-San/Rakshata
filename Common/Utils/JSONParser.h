/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

id objectForKey(NSDictionary * dict, NSString * ID, NSString * fullName, Class expectedClass);

//Project parser
META_TOME * getVolumes(NSArray* volumeBloc, uint * nbElem, BOOL paidContent, BOOL remoteData);
NSArray * recoverVolumeBloc(META_TOME * volume, uint length, BOOL paidContent);
void * parseChapterStructure(NSArray * chapterBloc, uint * nbElem, BOOL isChapter, BOOL paidContent, uint ** chaptersPrice);
PROJECT_DATA parseBloc(NSDictionary * bloc);
PROJECT_DATA_EXTRA parseBlocExtra(NSDictionary * bloc);
void* parseProjectJSON(REPO_DATA* repo, NSDictionary * remoteData, uint * nbElem, bool parseExtra);

bool convertTagMask(NSArray * input, TAG ** tagData, uint32_t * nbTags, uint32_t * mainTag);
NSArray * reverseTag(PROJECT_DATA project);

//Repo parser
NSDictionary * linearizeRootRepo(ROOT_REPO_DATA * root);

//Rak parser
//This function will perform a lot of network task, and thus should _really_ be run in a thread
ROOT_REPO_DATA ** parserRakSourceFile(NSData * fileContent, uint * nbElem);

#define JSON_PROJ_AUTHOR_ID	@"ROOT-ID"
#define ARE_CLASSES_DIFFERENT(a, b) (![a isKindOfClass:b])

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
#define JSON_PROJ_VOL_ISRESERVEDTOVOL @"12"
#define JSON_PROJ_DESCRIPTION @"13"
#define JSON_PROJ_AUTHOR @"14"
#define JSON_PROJ_STATUS @"15"
#define JSON_PROJ_TAG_DATA @"16"
#define JSON_PROJ_RIGHT_TO_LEFT @"17"
#define JSON_PROJ_TAG_CATEGORY @"18"
#define JSON_PROJ_PRICE @"19"
#define JSON_PROJ_DRM @"4.2"
#define JSON_PROJ_CHAP_REMOTE @"20"
#define JSON_PROJ_CHAP_LOCAL @"21"
#define JSON_PROJ_VOL_REMOTE @"22"
#define JSON_PROJ_VOL_LOCAL @"23"
#define JSON_PROJ_ISLOCAL @"24"

#define JSON_PROJ_URL_SRGRID @"20"
#define JSON_PROJ_HASH_SRGRID @"21"
#define JSON_PROJ_URL_SRGRID_2X @"22"
#define JSON_PROJ_HASH_SRGRID_2X @"23"
#define JSON_PROJ_URL_HEAD @"24"
#define JSON_PROJ_HASH_HEAD @"25"
#define JSON_PROJ_URL_HEAD_2X @"26"
#define JSON_PROJ_HASH_HEAD_2X @"27"
#define JSON_PROJ_URL_CT @"28"
#define JSON_PROJ_HASH_CT @"29"
#define JSON_PROJ_URL_CT_2X @"30"
#define JSON_PROJ_HASH_CT_2X @"31"
#define JSON_PROJ_URL_DD @"32"
#define JSON_PROJ_HASH_DD @"33"
#define JSON_PROJ_URL_DD_2X @"34"
#define JSON_PROJ_HASH_DD_2X @"35"

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
#define JSON_REPO_SUB_RETINA_HASH	@"13"
#define JSON_REPO_SUB_MATURE		@"14"
#define JSON_REPO_ID				@"15"
#define JSON_REPO_SUB_ACTIVE		@"16"
#define JSON_REPO_SIGNATURE			@"17"
#define JSON_REPO_EXPIRENCY			@"18"

//.Raksource file parser
#define JSON_RAK_MIN_VERSION	@"0"
#define JSON_RAK_PAYLOAD		@"1"
#define JSON_RAK_TYPE			@"2"
#define JSON_RAK_URL			@"3"
#define JSON_RAK_PRESELECTION	@"4"

//Tags file parser
#define JSON_TAG_VERSION	@"0"
#define JSON_TAG_TAGS 		@"1"
#define JSON_TAG_CATEGORY 	@"2"
#define JSON_TAG_ID			@"3"
#define JSON_TAG_NAME		@"4"
#define JSON_TAG_MASTER		@"5"

//.Rak file parser
#define ARCHIVE_VERSION 1

#define METADATA_FILE "catalog.json"

#define RAK_STRING_VERSION				@"version"
#define RAK_STRING_METADATA				@"metadata"
#define RAK_STRING_METADATA_PROJECT		@"projects"
#define RAK_STRING_METADATA_PROJECTID	@"projectID"
#define RAK_STRING_METADATA_PROJECTNAME	@"projectName"
#define RAK_STRING_METADATA_REPOTYPE	@"repoType"
#define RAK_STRING_METADATA_REPOURL		@"repoURL"
#define RAK_STRING_METADATA_REPO_PROJID	@"repoProjectID"
#define RAK_STRING_METADATA_DESCRIPTION	@"description"
#define RAK_STRING_METADATA_AUTHOR		@"author"
#define RAK_STRING_METADATA_STATUS		@"status"
#define RAK_STRING_METADATA_CATEGORY	@"category"
#define RAK_STRING_METADATA_TAG_DATA	@"tagData"
#define RAK_STRING_METADATA_RIGHT2LEFT	@"rightToLeft"
#define RAK_STRING_METADATA_IMG_GRID	@"imgGrid"
#define RAK_STRING_METADATA_IMG_GRID_2X	@"imgGrid@2x"
#define RAK_STRING_METADATA_IMG_CT		@"imgCT"
#define RAK_STRING_METADATA_IMG_CT_2X	@"imgCT@2x"
#define RAK_STRING_METADATA_IMG_DD		@"imgDD"
#define RAK_STRING_METADATA_IMG_DD_2X	@"imgDD@2x"

#define RAK_STRING_CONTENT				@"content"
#define RAK_STRING_CONTENT_DIRECTORY	@"dirName"
#define RAK_STRING_CONTENT_PROJECT		@"projectID"
#define RAK_STRING_CONTENT_ISTOME		@"isTome"
#define RAK_STRING_CONTENT_VOL_DETAILS	@"detail"
#define RAK_STRING_CONTENT_ID			@"ID"