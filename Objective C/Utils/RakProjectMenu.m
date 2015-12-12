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

@implementation RakProjectMenu

- (instancetype) initWithProject : (PROJECT_DATA) project
{
	self = [super init];
	
	if(self != nil)
		_project = project;
	
	return self;
}

- (void) configureCleanMenu : (BOOL) optionPressed toView : (NSView *) view
{
	_view = view;
	
	if(!_project.isInitialized)
		_view.menu = nil;
	
	NSMenu * menu = [[NSMenu alloc] initWithTitle:@"Menu"];
	if(menu == nil)
		return;

	[self configureMenu:menu withOption:optionPressed];
	
	[_view setMenu:menu];
}

- (void) configureMenu : (NSMenu *) menu
{
	[self configureMenu:menu withOption:[(RakAppDelegate *) NSApp.delegate window].optionPressed];
}

- (void) configureMenu : (NSMenu *) menu withOption : (BOOL) optionPressed
{
	if(menu.autoenablesItems)
		menu.autoenablesItems = NO;
	
	NSMenuItem * item = [[NSMenuItem alloc] initWithTitle:[NSString stringWithFormat:@"%@ - %@", getStringForWchar(_project.projectName), getStringForWchar(_project.authorName)] action:@selector(clicked) keyEquivalent:@""];
	if(item != nil)
	{
		if(_project.authorName[0] == 0)	//Unexpected, but we remove the author from the name then
			item.title = getStringForWchar(_project.projectName);
		
		item.enabled = NO;
		item.target = self;
		[menu addItem:item];
		[menu addItem:[NSMenuItem separatorItem]];
	}
	
	item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"PROJ-MENU-RESUME-READING", nil) action:@selector(resumeReading) keyEquivalent:@""];
	if(item != nil)
	{
		item.enabled = projectHaveValidSavedState(_project, getEmptyRecoverState());
		item.target = self;
		[menu addItem:item];
		[menu addItem:[NSMenuItem separatorItem]];
	}
	
	item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"PROJ-MENU-EDIT-SERIE", nil) action:@selector(editSerie) keyEquivalent:@""];
	if(item != nil)
	{
		item.enabled = _project.locale;
		item.target = self;
		[menu addItem:item];
	}
	
	item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"PROJ-MENU-DELETE-SERIE", nil) action:@selector(deleteSerie) keyEquivalent:@""];
	if(item != nil)
	{
		item.enabled = isInstalled(_project, NULL);
		item.target = self;
		[menu addItem:item];
	}
	
	//Easter egg
	if(optionPressed)
	{
		item = [[NSMenuItem alloc] initWithTitle:@"NUKE FROM ORBIT" action:@selector(clicked) keyEquivalent:@""];
		if(item != nil)
		{
			item.target = self;
			[menu addItem:[NSMenuItem separatorItem]];
			[menu addItem:item];
		}
	}
}

//Prevent the menu from appearing if clicking outside the active area

- (void) resumeReading
{
	[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_RESUME_READING object:@(_project.cacheDBID)];
}

- (void) editSerie
{
	[[[RakEditDetails alloc] initWithProject:_project] queryUser];
}

- (void) deleteSerie
{
	deleteProject(_project, INVALID_VALUE, false);

	if([_view respondsToSelector:@selector(deleteSerie)])
		[(id) _view deleteSerie];
}

//Easter egg
- (void) clicked
{
	NSAlert * alert = [[NSAlert alloc] init];
	
	if(alert != nil)
	{
		alert.alertStyle = NSInformationalAlertStyle;
		alert.messageText = @"I'm sorry Dave";
		alert.informativeText = @"I'm afraid I can't do that.";
		[alert addButtonWithTitle:@"Ok 😔"];
		[alert runModal];
	}
}

@end