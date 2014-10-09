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
 ********************************************************************************************/

#include "RakCTContentTabView.h"

@interface RakChapterView : RakTabContentTemplate
{
	//CT mode only
	RakCTHeader * header;
	RakCTProjectSynopsis * synopsis;
	
	//Reader mode only
	RakMenuText * projectName;
	RakCTProjectImageView * projectImage;

	RakCTContentTabView * coreView;
}

- (id)initContent:(NSRect)frame : (PROJECT_DATA) project : (bool) isTome : (long [4]) context;

- (void) updateContext : (PROJECT_DATA) data;
- (BOOL) refreshCT : (BOOL) checkIfRequired : (uint) ID;
- (void) selectElem : (uint) projectID : (BOOL) isTome : (int) element;

@end
