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

@implementation RakImportBaseController

- (BOOL) noValidFileFoundForDir : (const char *) dirname butFoundInFiles : (BOOL) foundDirInFiles shouldRedirectTo : (NSString **) redirection
{
#ifdef EXTENSIVE_LOGGING
	NSLog(@"[WARNING]: was tasked with processing a CT but couldn't find a file starting with %s (cound find dirname? %d)", dirname == NULL ? "(no name)" : dirname, foundDirInFiles);
#endif
	
	//If the directory existed in the dir, or we were already using a wildcard, no need to enlarge the search
	if(foundDirInFiles || dirname == NULL)
	{
		*redirection = nil;
		return NO;
	}
	
#ifdef EXTENSIVE_LOGGING
	NSLog(@"Removing the first dir in the dirname");
#endif
	
	//We remove the first path component of the archive
	
	NSString * newDirName;
	uint i = 0, lengthExpected = strlen(dirname);
	
	for(; i < lengthExpected && dirname[i] != 0 && dirname[i] != '/'; ++i);
	
	if(i == lengthExpected || dirname[i] == 0)
		newDirName = @"";
	else
	{
		for(; i < lengthExpected && dirname[i] == '/'; ++i);
		if(i == lengthExpected || dirname[i] == 0)
			newDirName = @"";
		else
			newDirName = [NSString stringWithUTF8String:&dirname[i]];
	}
	
	*redirection = newDirName;
	
	return YES;
}

@end