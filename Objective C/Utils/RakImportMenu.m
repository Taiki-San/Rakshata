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

@implementation RakImportMenu

- (instancetype) initWithResponder : (id) responder
{
	self = [super init];
	
	if(self != nil)
		_responder = responder;
	
	return self;
}

- (void) configureMenu : (NSView *) receiver
{
	NSMenu * menu = [[NSMenu alloc] initWithTitle:@"Menu"];
	if(menu == nil || receiver == nil)
		return;
	
	menu.autoenablesItems = NO;
	
	NSMenuItem * item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"IMPORT-MENU-MOVE-OUT", nil) action:@selector(moveToIndependentNode) keyEquivalent:@""];
	if(item != nil)
	{
		if(![_responder respondsToSelector:@selector(canMoveToIndependentNode)] || ![_responder respondsToSelector:@selector(moveToIndependentNode)])
			item.enabled = NO;
		
		else
			item.enabled = [_responder canMoveToIndependentNode];
		
		item.target = _responder;
		[menu addItem:item];
		[menu addItem:[NSMenuItem separatorItem]];
	}
	
	item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"IMPORT-MENU-NUKE", nil) action:@selector(removeItem) keyEquivalent:@""];
	if(item != nil)
	{
		item.enabled = [_responder respondsToSelector:@selector(removeItem)];
		item.target = _responder;
		[menu addItem:item];
	}

	CONFIGURE_APPEARANCE_DARK(receiver);
	[receiver setMenu:menu];
}

@end
