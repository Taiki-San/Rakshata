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

@class RakCTHeaderImage;

#import "RakCTHImage.h"
#import "RakCTHTableController.h"
#import "RakCTHContainer.h"

@interface RakCTHeaderImage : NSView
{
	PROJECT_DATA _data;
	
	//Elements
	RakCTHImage * _background;
	RakCTHContainer * _container;
}

- (instancetype) initWithData : (NSRect) frame : (PROJECT_DATA) project;
- (void) updateHeaderProject : (PROJECT_DATA) project;
- (BOOL) updateHeaderProjectInternal : (PROJECT_DATA) project;

- (void) resizeAnimation : (NSRect) frameRect;
- (NSRect) frameByParent : (NSRect) parentFrame;
- (NSSize) sizeByParent : (NSRect) parentFrame;

@end
