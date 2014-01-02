/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
 **                                                                                          **
 *********************************************************************************************/

#define DEV_VERSION

@interface Prefs : NSObject

+ (void) initCache;
+ (void) rebuildCache;
+ (void) clearCache;
+ (void *) getPref : (int) requestID;
+ (bool) setPref : (uint) requestID : (uint64) value;

@end

/*Codes servant à identifier les requêtes*/
#define PREFS_GET_TAB_SERIE_WIDTH	1
#define PREFS_GET_TAB_CT_WIDTH		2
#define PREFS_GET_TAB_READER_WIDTH	3
#define PREFS_GET_TAB_SERIE_POSX	4	//Les trois élements de chaques blocs doivent se suivre, en revanche, les deux blocs peuvent être séparés
#define PREFS_GET_TAB_CT_POSX		5
#define PREFS_GET_TAB_READER_POSX	6

#define PREFS_SET_OWNMAINTAB		1
#define PREFS_SET_READER_TABS_STATE	2


/*Divers constantes utilisées un peu partout mais renvoyés par Prefs*/

//	Widths of tabs in percents
#define TAB_SERIE_ACTIVE					78
#define TAB_SERIE_INACTIVE_CT				20
#define TAB_SERIE_INACTIVE_LECTEUR			15
#define TAB_SERIE_INACTIVE_LECTEUR_REDUCED	3
#define TAB_SERIE_INACTIVE_DISTRACTION_FREE	0

#define TAB_CT_INACTIVE_SERIE				20
#define TAB_CT_ACTIVE						70
#define TAB_CT_INACTIVE_LECTEUR				15
#define TAB_CT_INACTIVE_LECTEUR_REDUCED		3
#define TAB_CT_INACTIVE_DISTRACTION_FREE	0

#define TAB_READER_INACTIVE_SERIE			2
#define TAB_READER_INACTIVE_CT				10
#define TAB_READER_ACTIVE					70
#define TAB_READER_ACTIVE_PARTIAL			82
#define TAB_READER_ACTIVE_FULL				94
#define TAB_READER_ACTIVE_DISTRACTION_FREE	100

//	State of tabs in the reader
#define STATE_READER_TAB_DEFAULT			STATE_READER_TAB_SERIE_COLLAPSED

#define STATE_READER_NONE_COLLAPSED			0x0
#define STATE_READER_TAB_SERIE_COLLAPSED	0x1
#define STATE_READER_TAB_CT_COLLAPSED		0x2
#define STATE_READER_TAB_ALL_COLLAPSED		(STATE_READER_TAB_SERIE_COLLAPSED | STATE_READER_TAB_CT_COLLAPSED)
#define STATE_READER_TAB_DISTRACTION_FREE	0x4
#define STATE_READER_TAB_MASK				(STATE_READER_TAB_SERIE_COLLAPSED | STATE_READER_TAB_CT_COLLAPSED)
