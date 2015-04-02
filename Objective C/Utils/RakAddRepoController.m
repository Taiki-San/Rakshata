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

#include "JSONParser.h"

@implementation RakAddRepoController

- (void) analyseFileContent : (NSString *) fileContent
{
	output = parserRakFile(fileContent, &nbRoot);
	window = ((RakAppDelegate *) [NSApp delegate]).window;
	
	if(window != nil)
		[self queryUser];
}

- (void) queryUser
{
	if(![NSThread isMainThread])
	{
		[self performSelectorOnMainThread:@selector(queryUser) withObject:nil waitUntilDone:NO];
		return;
	}
	
	RakPrefsRepoAddView * mainView = [[RakPrefsRepoAddView alloc] initWithRoot:output nbElem:nbRoot];
	
	if(mainView != nil)
	{
		NSWindow * queryWindow = [[NSWindow alloc] initWithContentRect:NSMakeRect(200, 200, mainView.bounds.size.width, mainView.bounds.size.height) styleMask:0 backing:NSBackingStoreBuffered defer:YES];
		if(queryWindow != nil)
		{
			queryWindow.contentView = mainView;
			
			[window beginSheet:queryWindow completionHandler:^(NSModalResponse returnCode) {}];
			return;
		}
	}
	
	while (nbRoot-- > 0)
		freeSingleRootRepo(output[nbRoot]);
	free(output);
}

@end
