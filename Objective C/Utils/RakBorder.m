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

@implementation RakBorder

- (void) setColor : (NSColor *) color
{
	self.layer.backgroundColor	= color.CGColor;
	self.layer.borderColor		= color.CGColor;
}

-  (id) initWithFrame:(NSRect)frameRect : (CGFloat) borderWidth : (CGFloat) angleRounds : (NSColor *) color
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		[self setWantsLayer:YES];
		self.layer.masksToBounds	= YES;
		self.layer.borderWidth      = borderWidth;
		self.layer.cornerRadius		= angleRounds;
		self.layer.backgroundColor	= color.CGColor;
		self.layer.borderColor		= color.CGColor;
	}
	
	return self;
}

@end
