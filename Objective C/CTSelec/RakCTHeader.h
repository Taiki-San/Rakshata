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

#import "RakCTHeaderImage.h"
#import "RakCTProjectSynopsis.h"

@interface RakCTHeader : NSView
{
	RakCTHeaderImage * header;
	RakCTProjectSynopsis * synopsis;
	
	//Background thingy
	NSColor * _backgroundColor;
	NSColor * _synopsisTitleBackground;
	CGFloat _synopsisTitleHeight;
}

- (instancetype) initWithData : (NSRect) frameRect : (PROJECT_DATA) project;
- (void) updateProject : (PROJECT_DATA) project;
- (void) updateProjectDiff : (PROJECT_DATA) oldData : (PROJECT_DATA) newData;

- (NSRect) frameByParent : (NSRect) parentFrame;

- (void) resizeAnimation : (NSRect) frame;

@end
