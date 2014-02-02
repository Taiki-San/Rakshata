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

//	Widths of tabs in percents by default
#define TAB_SERIE_ACTIVE					78
#define TAB_SERIE_INACTIVE_CT				20
#define TAB_SERIE_INACTIVE_LECTEUR			15
#define TAB_SERIE_INACTIVE_LECTEUR_REDUCED	3
#define TAB_SERIE_INACTIVE_DISTRACTION_FREE	0
#define TAB_SERIE_FOOTER_HEIGHT				25
#define TAB_SERIE_MDL_WIDTH					25

#define TAB_CT_INACTIVE_SERIE				20
#define TAB_CT_ACTIVE						70
#define TAB_CT_INACTIVE_LECTEUR				15
#define TAB_CT_INACTIVE_LECTEUR_REDUCED		3
#define TAB_CT_INACTIVE_DISTRACTION_FREE	0
#define TAB_CT_FOOTER_HEIGHT				25

#define TAB_READER_INACTIVE_SERIE			2
#define TAB_READER_INACTIVE_CT				10
#define TAB_READER_ACTIVE					94
#define TAB_READER_ACTIVE_PARTIAL			82
#define TAB_READER_ACTIVE_DISTRACTION_FREE	100
#define TAB_READER_FOOTER_HEIGHT			25

#define TAB_MDL_WIDTH						80
#define TAB_MDL_HEIGHT						90
#define TAB_MDL_POSX						10
#define TAB_MDL_POSY						0

//	State of tabs in the reader
#define STATE_READER_TAB_DEFAULT			STATE_READER_TAB_CT_FOCUS

#define STATE_READER_NONE_COLLAPSED			(STATE_READER_TAB_SERIE_FOCUS | STATE_READER_TAB_CT_FOCUS | STATE_READER_TAB_MDL_FOCUS)

#define STATE_READER_TAB_ALL_COLLAPSED		0x1
#define STATE_READER_TAB_SERIE_FOCUS		0x2
#define STATE_READER_TAB_CT_FOCUS			0x4
#define STATE_READER_TAB_MDL_FOCUS			0x8
#define STATE_READER_TAB_DISTRACTION_FREE	0x10


#define STATE_READER_TAB_MASK				0xFF
