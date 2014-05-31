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

@implementation RakResPath

+ (NSImage *) craftResNameFromContext: (NSString*) baseName : (BOOL) highlighted : (BOOL) available : (uint) themeID
{
	NSString * pathPortionHighlighted, *pathPortionAvailable;
	
	if(highlighted)
		pathPortionHighlighted = @"_focus";
	else
		pathPortionHighlighted = @"";
	
	if(!available)
		pathPortionAvailable = @"_unavailable";
	else
		pathPortionAvailable = @"";

	if(themeID == 1 || themeID == 2)
	{
		NSString * fullFileName = [NSString stringWithFormat:@"theme_%d_%@%@%@", themeID, baseName, pathPortionHighlighted, pathPortionAvailable];
		return [[NSBundle mainBundle] imageForResource:fullFileName];
	}
	else
	{
		NSLog(@"Unsupported, for now");
		return nil;
	}
}

@end
