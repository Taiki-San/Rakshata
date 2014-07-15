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
 ********************************************************************************************/

@implementation RakPageScrollView

- (id) init
{
	self = [super init];
	
	if(self != nil)
	{
		self.borderType =		NSNoBorder;
		self.scrollerStyle =	NSScrollerStyleOverlay;
		self.drawsBackground =	NO;
	}
	
	return self;
}

- (void) enforceScrollerPolicy
{
	self.hasVerticalScroller = self.pageTooHigh;
	self.verticalScroller.alphaValue =	0;
	self.hasHorizontalScroller = self.pageTooLarge;
	self.horizontalScroller.alphaValue = 0;
}

- (void) releaseData
{
	if(self.documentView != nil && [self.documentView class] == [NSImageView class])
		((NSImageView*) self.documentView).image = nil;
}

- (void) removeDocumentView
{
	id view = self.documentView;
	self.documentView = nil;
	[view release];
}

@end
