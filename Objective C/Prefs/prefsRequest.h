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
enum PREFS_REQUEST {
	PREFS_GET_MAIN_THREAD,
	
	PREFS_GET_TAB_SERIE_WIDTH,
	PREFS_GET_TAB_CT_WIDTH,
	PREFS_GET_TAB_READER_WIDTH,
	
	PREFS_GET_TAB_SERIE_POSX,	//Les trois élements de chaques blocs doivent se suivre, en revanche, les deux blocs peuvent être séparés
	PREFS_GET_TAB_CT_POSX,
	PREFS_GET_TAB_READER_POSX,
	PREFS_GET_SERIE_FOOTER_HEIGHT,
	PREFS_GET_CT_FOOTER_HEIGHT,
	PREFS_GET_READER_FOOTER_HEIGHT,
	
	PREFS_GET_MDL_POS_X,
	PREFS_GET_MDL_POS_Y,
	PREFS_GET_MDL_HEIGHT,
	PREFS_GET_MDL_WIDTH,
	PREFS_GET_MDL_FRAME,
	
	PREFS_GET_IS_READER_MT,
	PREFS_GET_READER_TABS_STATE
};
