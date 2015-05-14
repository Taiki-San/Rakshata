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

/*Codes servant à identifier les requêtes*/

enum PREFS_REQUEST
{
	PREFS_GET_MAIN_THREAD,
	
	PREFS_GET_TAB_SERIE_WIDTH,
	PREFS_GET_TAB_CT_WIDTH,
	
	PREFS_GET_TAB_CT_POSX,
	PREFS_GET_TAB_READER_POSX,
	
	PREFS_GET_CT_FOOTER_HEIGHT,
	PREFS_GET_READER_FOOTER_HEIGHT,
	
	PREFS_GET_TAB_SERIE_FRAME,
	PREFS_GET_TAB_CT_FRAME,
	PREFS_GET_TAB_READER_FRAME,
	PREFS_GET_MDL_FRAME,
	
	PREFS_GET_IS_READER_MT,
	PREFS_GET_READER_TABS_STATE,
	
	PREFS_GET_SCROLLER_STYLE,
	
	PREFS_GET_INVALID
};

enum PREFS_PROVIDER
{
	PREFS_SET_OWNMAINTAB,
	PREFS_SET_READER_TABS_STATE,
	PREFS_SET_READER_DISTRACTION_FREE,
	PREFS_SET_READER_TABS_STATE_FROM_CALLER
};

#define TAB_READER				(1 << 0)
#define TAB_READER_DF			(0x0010 & TAB_READER)
#define TAB_CT					(1 << 1)
#define TAB_MDL					(1 << 2)
#define TAB_SERIES				(1 << 3)
#define TAB_MASK				0xFFFF
