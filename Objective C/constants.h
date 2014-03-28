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

/** Window behavior **/

#define SIZE_MIN_HEIGHT 600
#define SIZE_MIN_WIDTH 950
#define BORDER_WIDTH 4

#pragma mark	-	Reader

/** Reader itself **/

#define READER_BORDURE_VERT_PAGE 10

/** Constants used by pages **/

#define READER_PAGE_TOP_BORDER	78
#define READER_PAGE_BORDERS_HIGH (RD_CONTROLBAR_HEIGHT + RD_CONTROLBAR_POSY + READER_PAGE_TOP_BORDER + 10)

/** Bottom reader bar **/

#define RD_CONTROLBAR_HEIGHT		26
#define RD_CONTROLBAR_WIDHT_PERC	75
#define RD_CONTROLBAR_WIDHT_MIN		500
#define RD_CONTROLBAR_WIDHT_MAX		1000
#define RD_CONTROLBAR_POSY			20

/**	CT itself	**/

/** RakChapterView **/
#define CT_VIEW_READERMODE_LATERAL_BORDER 5			//	%
#define CT_VIEW_READERMORE_BOTTOMBAR_WIDTH	14

/**		Utils	**/

/*		RakBackButton		*/
#define RBB_BUTTON_HEIGHT	25
#define RBB_BUTTON_WIDTH	75						//	%
#define RBB_BUTTON_POSX		12.5f					//	%
#define RBB_TOP_BORDURE		10