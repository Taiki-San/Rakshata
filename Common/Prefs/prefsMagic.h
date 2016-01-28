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

//	Widths of tabs in percents by default
enum
{
	TAB_SERIE_ACTIVE =						78,
	TAB_SERIE_INACTIVE_CT =					20,
	TAB_SERIE_INACTIVE_LECTEUR =			30,
	TAB_SERIE_INACTIVE_LECTEUR_REDUCED =	3,
	TAB_SERIE_INACTIVE_DISTRACTION_FREE = 	TAB_SERIE_INACTIVE_LECTEUR,
	TAB_SERIE_FOOTER_HEIGHT =				30,
	TAB_SERIE_MDL_WIDTH =					27,
	
	TAB_CT_INACTIVE_SERIE =					20,
	TAB_CT_ACTIVE =							70,
	TAB_CT_INACTIVE_LECTEUR =				30,
	TAB_CT_INACTIVE_LECTEUR_REDUCED	=		3,
	TAB_CT_INACTIVE_DISTRACTION_FREE =		TAB_CT_INACTIVE_LECTEUR,
	TAB_CT_FOOTER_HEIGHT =					27,
	
	TAB_READER_INACTIVE_SERIE =				2,
	TAB_READER_INACTIVE_CT =				10,
	TAB_READER_ACTIVE =						94,
	TAB_READER_ACTIVE_PARTIAL =				82,
	TAB_READER_ACTIVE_DISTRACTION_FREE =	100,
	
	TAB_READER_FOOTER_HEIGHT =				27,
	
	TAB_MDL_WIDTH_READER =					33,
	TAB_MDL_WIDTH =							80,
	TAB_MDL_HEIGHT =						90,
	TAB_MDL_POSX =							10,
	TAB_MDL_POSY =							0,
};

//	State of tabs in the reader
enum : uint8_t
{
	STATE_READER_TAB_ALL_COLLAPSED = 	(1 << 0),
	STATE_READER_TAB_SERIE_FOCUS =		(1 << 1),
	STATE_READER_TAB_CT_FOCUS = 		(1 << 2),
	STATE_READER_TAB_MDL_FOCUS = 		(1 << 3),
	STATE_READER_TAB_DISTRACTION_FREE =	(1 << 4),
	STATE_READER_NONE_COLLAPSED =		(STATE_READER_TAB_SERIE_FOCUS | STATE_READER_TAB_CT_FOCUS | STATE_READER_TAB_MDL_FOCUS),
	
	STATE_READER_TAB_MASK = 0xFF,
	STATE_READER_TAB_DEFAULT = STATE_READER_TAB_SERIE_FOCUS
};

//Theme code
enum : uint
{
	THEME_CODE_ERROR = 0,
	THEME_CODE_DARK = 1,
	THEME_CODE_LIGHT = 2,
	THEME_CODE_CUSTOM = 3,
	
	MAX_THEME_ID = THEME_CODE_CUSTOM,
	THEME_CODE_DEFAULT = THEME_CODE_DARK
};

enum SCROLLER_STYLE {
	SCROLLER_STYLE_THIN,
	SCROLLER_STYLE_LARGE
};
