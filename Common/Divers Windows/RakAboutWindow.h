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

@interface RakAboutWindow : RakCustomWindow
{
	RakAboutIcon * icon;
	RakClickableText * projectName;
	RakText * version;
	
	RakText * devTitle, * designTitle;
	RakClickableText * taikiName, * blag, * planchette, *FOSS;
	
	uint easterCount;
	RakText * mainEaster;
	RakClickableText * easterLink;
	
	RakText * copyright;
}

@property BOOL haveEaster;

- (void) respondTo : (RakClickableText *) sender;
- (void) clicIcon;

@end
