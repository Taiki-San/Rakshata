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

void firstLaunch();

/*****************************************
 **										**
 **				  ERRORS				**
 **										**
 *****************************************/

enum BUTTONS_CODES {
	UIABUTTDefault		= 1,
	UIABUTTAlternate	= 2,
	UIABUTTOther		= 3
};

typedef struct UIABUTT UIABUTT;
struct UIABUTT
{
	char *buttonName;
	char priority;
	int ret_value;
	UIABUTT * next;
};

int internalUIAlert(char *titre, char* content, UIABUTT* buttons);


/*****************************************
 **										**
 **				  PREFS					**
 **										**
 *****************************************/

uint32_t getMainThread();
void sendToLog(char * string);

/*****************************************
 **										**
 **				 DB update				**
 **										**
 *****************************************/

void notifyFullUpdate();
void notifyUpdateRepo(REPO_DATA repo);
void notifyUpdateProject(PROJECT_DATA project);

/*****************************************
 **										**
 **			Restrictions update			**
 **										**
 *****************************************/

void notifyRestrictionChanged();

/*****************************************
 **										**
 **				  Series				**
 **										**
 *****************************************/

void updateRecentSeries();

/*****************************************
 **										**
 **				    MDL					**
 **										**
 *****************************************/

bool checkIfElementAlreadyInMDL(PROJECT_DATA data, bool isTome, int element);
void addElementToMDL(PROJECT_DATA data, bool isTome, int element, bool partOfBatch);