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

/*Codes servant à identifier les requêtes*/

enum REQUEST_CODE_CONVERTER
{
	CONVERT_CODE_POSX,
	CONVERT_CODE_POSY,
	CONVERT_CODE_HEIGHT,
	CONVERT_CODE_WIDTH,
	CONVERT_CODE_FRAME
};

enum PREFS_REQUEST {
	PREFS_GET_MAIN_THREAD,
	
	PREFS_GET_TAB_SERIE_WIDTH,
	PREFS_GET_TAB_CT_WIDTH,
	PREFS_GET_TAB_READER_WIDTH,
	PREFS_GET_MDL_WIDTH,
	
	PREFS_GET_TAB_SERIE_HEIGHT,
	PREFS_GET_TAB_CT_HEIGHT,
	PREFS_GET_TAB_READER_HEIGHT,
	PREFS_GET_MDL_HEIGHT,
	
	PREFS_GET_TAB_SERIE_POSX,
	PREFS_GET_TAB_CT_POSX,
	PREFS_GET_TAB_READER_POSX,
	PREFS_GET_MDL_POSX,
	
	PREFS_GET_TAB_SERIE_POSY,
	PREFS_GET_TAB_CT_POSY,
	PREFS_GET_TAB_READER_POSY,
	PREFS_GET_MDL_POSY,
	
	PREFS_GET_SERIE_FOOTER_HEIGHT,
	PREFS_GET_CT_FOOTER_HEIGHT,
	PREFS_GET_READER_FOOTER_HEIGHT,
	
	PREFS_GET_TAB_SERIE_FRAME,
	PREFS_GET_TAB_CT_FRAME,
	PREFS_GET_TAB_READER_FRAME,
	PREFS_GET_MDL_FRAME,
	
	PREFS_GET_IS_READER_MT,
	PREFS_GET_READER_TABS_STATE
};
