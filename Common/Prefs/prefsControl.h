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
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
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

enum DIRECT_QUERY_REQUEST
{
	QUERY_SERIE = 0,	//Unused
	QUERY_CT = 1,
	QUERY_READER = 2,	//Unused
	QUERY_MDL = 3		//Unused
};

enum QUERY_SUBREQUEST {
	QUERY_GET_WIDTH,
	QUERY_GET_HEIGHT,	//Unused
	QUERY_GET_POSX,		//Unused
	QUERY_GET_POSY		//Unused
};

enum COLOR_REQUEST
{
	COLOR_GRID_FOCUS_BACKGROUND = COLOR_KIT_MAX,
	COLOR_SR_PLACEHOLDER_TEXT,
	
	COLOR_FILTER_FOREGROUND,
	
	COLOR_TAGITEM_BORDER,
	COLOR_TAGITEM_BACKGROUND,
	COLOR_TAGITEM_FONT,
	
	COLOR_SEARCHBAR_BACKGROUND,
	COLOR_SEARCHBAR_BACKGROUND_EXTRA,
	COLOR_SEARCHBAR_PLACEHOLDER_TEXT,
	COLOR_SEARCHBAR_SELECTION_BACKGROUND,
	COLOR_SEARCHBAR_SELECTION_TEXT,
	COLOR_SEARCHBAR_BORDER,
	COLOR_SEARCHSUG_SEPARATOR,
	COLOR_SEARCHTAB_BACKGROUND,
	COLOR_SEARCHTAB_PEAK_BACKGROUND,
	COLOR_SEARCHTAB_BORDER_BAR,
	COLOR_SEARCHTAB_BORDER_COLLAPSED,
	COLOR_SEARCHTAB_BORDER_DEPLOYED,
	
	COLOR_CTHEADER_GRADIENT_START,
	COLOR_CTHEADER_GRADIENT_END,
	COLOR_CTHEADER_FONT,
	COLOR_CTLIST_BACKGROUND,
	
	COLOR_READER_BAR,
	COLOR_READER_BAR_FRONT,
	COLOR_READER_BAR_PAGE_COUNTER,
	COLOR_READER_BACKGROUND_INTAB,
	COLOR_PDF_BACKGROUND,
	
	COLOR_PROGRESSLINE_SLOT,
	COLOR_PROGRESSLINE_PROGRESS,
	
	COLOR_PREFS_HEADER_BACKGROUND,
	COLOR_PREFS_HEADER_BORDER,
	COLOR_PREFS_BUTTON_FOCUS,
	
	COLOR_REPO_LIST_BACKGROUND,
	COLOR_REPO_LIST_ITEM_BACKGROUND,
	COLOR_REPO_TEXT_PLACEHOLDER,
	COLOR_ADD_REPO_BACKGROUND,
	
	COLOR_DROP_AREA_BACKGROUND,
	COLOR_EXPORT_BACKGROUND,
	COLOR_IMPORT_LIST_BACKGROUND
};

enum FONT_REQUEST {
	FONT_TAGS = FONT_KIT_MAX,
	FONT_SR_TITLE,
	FONT_ABOUT,
	FONT_PREFS_TITLE,
	FONT_AUTHOR_ITALIC
};

enum KVO_REQUEST {
	KVO_MAIN_THREAD = KVO_KIT_MAX,
	KVO_PDF_BACKGRND,
	KVO_MAGNIFICATION,
	KVO_DIROVERRIDE
};
