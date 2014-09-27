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

#define CT_HEADER_MIN_HEIGHT 200

@class RakCTHeader;

#import "RakCTHImage.h"
#import "RakCTHContainer.h"

@interface RakCTHeader : NSView
{
	uint projectCacheID;
	PROJECT_DATA _data;
	
	//Elements
	RakCTHImage * _background;
	RakCTHContainer * _container;
	
}

- (id) initWithData : (NSRect) frame : (PROJECT_DATA) project;
- (BOOL) updateHeaderProject : (PROJECT_DATA) project;

@end
