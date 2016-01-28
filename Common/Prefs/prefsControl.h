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
	
	PREFS_GET_FAVORITE_AUTODL,
	PREFS_GET_ACTIVE_PREFS_PANEL,

	PREFS_GET_DIROVERRIDE,
	PREFS_GET_SAVE_MAGNIFICATION,
	PREFS_GET_HAVE_PDF_BACKGROUND,
	
	PREFS_GET_SUGGEST_FROM_LAST_READ,

	PREFS_GET_INVALID
};

enum PREFS_PROVIDER
{
	PREFS_SET_OWNMAINTAB,
	PREFS_SET_FAVORITE_AUTODL,
	PREFS_SET_READER_TABS_STATE,
	PREFS_SET_READER_DISTRACTION_FREE,
	PREFS_SET_READER_TABS_STATE_FROM_CALLER,
	PREFS_SET_ACTIVE_PREFS_PANEL,
	
	PREFS_SET_DIROVERRIDE,
	PREFS_SET_SAVE_MAGNIFICATION,
	PREFS_SET_HAVE_PDF_BACKGROUND
};

enum TAB_CODE
{
	TAB_UNKNOWN = 	0,
	TAB_READER = 	(1 << 0),
	TAB_READER_DF = (0x0010 & TAB_READER),
	TAB_CT = 		(1 << 1),
	TAB_MDL = 		(1 << 2),
	TAB_SERIES = 	(1 << 3),
	TAB_MASK = 		0xFFFF,
	TAB_DEFAULT = TAB_SERIES
};
