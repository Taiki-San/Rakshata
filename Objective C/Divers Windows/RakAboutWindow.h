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

@class RakAboutWindow;

#import "RakAboutTools.h"

@interface RakAboutWindow : NSObject <NSWindowDelegate>
{
	RakWindow * window;
	
	NSImageView * icon;
	RakAboutText * projectName;
	RakText * version;
	
	RakText * devTitle, * designTitle;
	RakAboutText * taikiName, * blag, * planchette, *FOSS;
	
	RakText * copyright;
}

- (void) createWindow;

- (void) respondTo : (RakAboutText *) sender;

@end
