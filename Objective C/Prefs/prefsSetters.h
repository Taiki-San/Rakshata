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

enum
{
	PREFS_SET_OWNMAINTAB,
	PREFS_SET_READER_TABS_STATE,
	PREFS_SET_READER_DISTRACTION_FREE,
	PREFS_SET_READER_TABS_STATE_FROM_CALLER
};

#define TAB_READER				0x0001
#define TAB_READER_DF			0x0011
#define TAB_CT					0x0002
#define TAB_MDL					0x0004
#define TAB_SERIES				0x0008
#define TAB_MASK				0xFFFF
