/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                          **
 *********************************************************************************************/

/*Status*/
//This order + initial value is assumed not to change by RakImportQuery
enum
{
	STATUS_INVALID = 0,
	STATUS_OVER = 1,
	STATUS_CANCELED,
	STATUS_SUSPENDED,
	STATUS_WIP,
	STATUS_ANOUNCED,

	STATUS_MAX = STATUS_ANOUNCED
};

/*Repository*/
#define REPO_LANGUAGE_LENGTH 5
#define REPO_NAME_LENGTH 50
#define REPO_URL_LENGTH 256
#define REPO_WEBSITE_LENGTH 128

/*Database*/
#define LENGTH_DESCRIPTION 1024
#define LENGTH_PROJECT_NAME 51
#define LENGTH_AUTHORS		51
#define LENGTH_URL			256
#define NB_IMAGES			8
#define	LENGTH_CRC			9

enum
{
	THUMB_INDEX_SR = 0,
	THUMB_INDEX_SR2X = 1,
	THUMB_INDEX_HEAD = 2,
	THUMB_INDEX_HEAD2X = 3,
	THUMB_INDEX_CT = 4,
	THUMB_INDEX_CT2X = 5,
	THUMB_INDEX_DD = 6,
	THUMB_INDEX_DD2X = 7
};

#define LONGUEUR_COURT 10
#define LONGUEUR_SITE 100
#define LONGUEUR_TYPE_TEAM 5
#define LONGUEUR_URL 300
#define NUMBER_MAX_REPO_KILLSWITCHE 200
