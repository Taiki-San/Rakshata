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

- (void) setColor : (RakColor *) color
{
	self.backgroundColor		= color;
	self.layer.borderColor		= color.CGColor;
}

-  (id) initWithFrame:(NSRect)frameRect : (CGFloat) borderWidth : (CGFloat) angleRounds : (RakColor *) color
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		self.backgroundColor		= color;
		self.layer.borderColor		= color.CGColor;
		self.layer.masksToBounds	= YES;
		self.layer.borderWidth      = borderWidth;
		self.layer.cornerRadius		= angleRounds;
	}
	
	return self;
}

- (BOOL) isOpaque
{
	return YES;
}

- (BOOL) mouseDownCanMoveWindow
{
	return YES;
}

@end
