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

@implementation RakAuthController

- (void) launch
{
	RakAppDelegate * core = [NSApp delegate];
	
	if([core class] != [RakAppDelegate class])
	{
		[self release];
		return;
	}
	
	self.view.wantsLayer = YES;
	self.view.layer.backgroundColor = [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS :self].CGColor;
	self.view.layer.cornerRadius = 4;
	
	foreground = [[RakForegroundView alloc] init : [core getContentView] : self.view];
	
	RakButton * inactiveConfirm = [RakButton allocWithText:@"Valider" :NSZeroRect];
	[inactiveConfirm sizeToFit];
	[inactiveConfirm setEnabled:NO];
	
	[inactiveConfirm setFrameOrigin: NSMakePoint(container.frame.size.width / 2 - inactiveConfirm.frame.size.width / 2, 15)];
	[container addSubview:inactiveConfirm];
	
	[foreground switchState];
}

@end
