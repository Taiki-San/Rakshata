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

@implementation RakSRSearchBar

- (instancetype) initWithFrame : (NSRect) frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		NSSearchFieldCell * cell = self.cell;
		
		_button = [[RakButtonCell alloc] initWithPage : @"loupe" : RB_STATE_STANDARD];
		if(_button != nil)
		{
			[_button setBordered:NO];
			[_button setHighlightAllowed:NO];
			[cell setSearchButtonCell:_button];
		}
		
		self.wantsLayer = YES;
		self.layer.backgroundColor = [Prefs getSystemColor:GET_COLOR_BACKGROUD_BACK_BUTTONS:self].CGColor;

	}
	
	return self;
}

- (void) resizeAnimation : (NSRect) frame
{
	[self.animator setFrame:frame];
}

@end
