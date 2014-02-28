/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 *********************************************************************************************/

#include "sqlite3.h"

#define INITIAL_BUFFER_SIZE 1024
#define KEEP_UNUSED_TEAMS				//If droped, they won't be refreshed, nor their manga DB will be updated, so bad idea for now
//#define DELETE_UNLISTED_PROJECT

#define DB_CACHE_EXPIRENCY 5*60*1000	//5 minutes

unsigned long alreadyRefreshed;

/******		DBTools.c	  ******/
bool isRemoteRepoLineValid(char * data, int version);
bool parseRemoteRepoLine(char *data, TEAMS_DATA *previousData, int version, TEAMS_DATA *output);

uint defineBoundsTeamOnProjectDB(MANGAS_DATA * oldData, uint posBase, uint nbElem);
bool downloadedProjectListSeemsLegit(char *data, MANGAS_DATA reference);
uint getNumberLineReturn(char *input);
bool extractCurrentLine(char * input, uint * posInput, char * output, uint lengthOutput);
bool parseCurrentProjectLine(char * input, int version, MANGAS_DATA * output);
void parseDetailsBlock(char * input, MANGAS_DATA *data, char *teamName, uint lengthOfBlock);
bool isProjectListSorted(MANGAS_DATA* data, uint length);
void applyChangesProject(MANGAS_DATA * oldData, uint magnitudeOldData, MANGAS_DATA * newData, uint magnitudeNewData);

void resetUpdateDBCache();

/*Database*/
enum RDB_CODES {
	RDB_UPDATE_ID = 1,
	RDB_UPDATE_TEAM = 2
};

enum RDB_ISUPDATE {
	RDB_CTXSERIES	= 0x1,
	RDB_CTXCT		= 0x2,
	RDB_CTXLECTEUR	= 0x4,
	RDB_CTXMDL		= 0x8,
	RDB_CTXDEL		= 0x10,
	RDB_CTXFAVS		= 0x10,
	RDB_CTXSELMDL	= 0x40,
	RDB_UNUSED3		= 0x80
};

enum SORT_TYPES {
	SORT_NAME = 1,
	SORT_TEAM = 2,
	SORT_DEFAULT = SORT_NAME,
};

//========= Obfuscation	==========//

#define RDB_ID							1
#define RDB_team						2
#define RDB_mangaNameShort				3
#define RDB_isInstalled					4
#define RDB_mangaName					5
#define RDB_status						6
#define RDB_genre						7
#define RDB_pageInfos					8
#define RDB_firstChapter				9
#define RDB_lastChapter					10
#define RDB_nombreChapitreSpeciaux		11
#define RDB_nombreChapitre				12
#define RDB_chapitres					13
#define RDB_firstTome					14
#define RDB_nombreTomes					15
#define RDB_tomes						16
#define RDB_contentDownloadable			17
#define RDB_favoris						18

#define STRINGIZE2(s) "`"#s"`"
#define DBNAMETOID(s) STRINGIZE2(s)