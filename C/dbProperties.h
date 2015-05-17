/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **		Source code and assets are property of Taiki, distribution is stricly forbidden		**
 **                                                                                          **
 *********************************************************************************************/

/*Status*/
enum
{
	STATUS_OVER = 1,
	STATUS_CANCELED,
	STATUS_SUSPENDED,
	STATUS_WIP,
	STATUS_ANOUNCED
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
#define	LENGTH_HASH			9
#define STATUS_MAX			5
#define TYPE_MAX			4

#define LONGUEUR_COURT 10
#define LONGUEUR_SITE 100
#define LONGUEUR_TYPE_TEAM 5
#define LONGUEUR_URL 300
#define NUMBER_MAX_REPO_KILLSWITCHE 200
