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
	WIDTH = 80,
	HEIGHT = 50
};

@implementation RakImageDropArea

- (instancetype) initWithContentString : (NSString *) string
{
	self = [self initWithFrame:NSMakeRect(0, 0, WIDTH, HEIGHT)];

	if(self != nil)
	{
		self.wantsLayer = YES;
		self.layer.cornerRadius = 5;
		self.layer.backgroundColor = [Prefs getSystemColor:COLOR_BACKGROUND_DROP_AREA :self].CGColor;

		RakText * content = [[RakText alloc] initWithText:string :[self textColor]];
		if(content != nil)
		{
			content.alignment = NSTextAlignmentCenter;

			[content setFrameOrigin : NSCenterPoint(_bounds, content.bounds)];
			[self addSubview:content];
		}
	}

	return self;
}

- (void) dealloc
{
	[Prefs deRegisterForChanges:self];
}

- (NSColor *) textColor
{
	return [Prefs getSystemColor:COLOR_CLICKABLE_TEXT :nil];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];

	self.layer.backgroundColor = [Prefs getSystemColor:COLOR_BACKGROUND_DROP_AREA :nil].CGColor;
}

@end
