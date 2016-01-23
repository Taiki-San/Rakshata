/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

#include "JSONParser.h"

@interface RakAddRepoController()
{
	NSData * _fileContentCopy;
}

@end

@implementation RakAddRepoController

- (void) analyseFileContent : (NSData *) fileContent
{
	_fileContentCopy = fileContent;
	
	output = parserRakSourceFile(_fileContentCopy, &nbRoot);
	window = RakApp.window;
	
	ICONS_UPDATE * iconsData = NULL, * newIcons, * endIcons;
	for(uint i = 0; i < nbRoot; i++)
	{
		newIcons = enforceRepoExtra(output[i], true);
		
		if(newIcons != NULL)
		{
			if(iconsData == NULL)
				endIcons = iconsData = newIcons;
			else
				endIcons->next = newIcons;
			
			while(endIcons->next != NULL)
				endIcons = endIcons->next;
		}
	}
	
	if(window != nil)
		[self queryUser];
	
	updateProjectImages(iconsData);
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
		NSWindow * queryWindow = [[NSWindow alloc] initWithContentRect:(NSRect) {{200, 200}, mainView.bounds.size} styleMask:0 backing:NSBackingStoreBuffered defer:YES];
		if(queryWindow != nil)
		{
			queryWindow.backgroundColor = [RakColor clearColor];
			queryWindow.contentView = mainView;
			
			[window beginSheet:queryWindow completionHandler:^(NSModalResponse returnCode) {}];
			return;
		}
	}
	
	if(nbRoot > 0)
	{
		while (nbRoot-- > 0)
			freeSingleRootRepo(output[nbRoot]);
		free(output);
	}
}

@end
