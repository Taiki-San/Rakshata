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

@implementation RakPopoverWrapper

- (id) init : (NSView *) contentView
{
	self = [super init];
	
	if(self != nil)
	{
		self.anchor = nil;
		self.direction = INPopoverArrowDirectionLeft;

		NSViewController * controller = [[[NSViewController alloc] init] autorelease];
		controller.view = contentView;
		_popover = [[INPopoverController alloc] initWithContentViewController:controller];
		_popover.closesWhenApplicationBecomesInactive = YES;
	}
	
	return self;
}

- (void) togglePopover
{
	if(_popover.popoverIsVisible)
		[_popover closePopover:nil];
	else
		[_popover presentPopoverFromRect:[self.anchor bounds] inView:self.anchor preferredArrowDirection:self.direction anchorsToPositionView:YES];
}

- (void) additionalConfiguration : (id) target : (SEL) selector
{
	[target performSelector:selector withObject:_popover];
}

- (void)setDelegate:(id <INPopoverControllerDelegate>) delegate
{
	[_popover setDelegate:delegate];
}

- (void) clearMemory
{
	_popover.contentViewController.view = nil;
	_popover.contentViewController = nil;
}

- (void) dealloc
{
	[_popover release];
	[super dealloc];
}

@end
