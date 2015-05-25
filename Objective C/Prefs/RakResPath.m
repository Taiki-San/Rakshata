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

+ (NSImage *) getImageFromTheme : (NSString *) baseName : (uint) themeID
{
	return [self craftResNameFromContext:baseName :NO :YES :themeID];
}

+ (NSImage *) craftResNameFromContext: (NSString*) baseName : (BOOL) highlighted : (BOOL) available : (uint) themeID
{
	NSString * pathPortionHighlighted, *pathPortionAvailable, *fullFileName;
	
	if(highlighted)
		pathPortionHighlighted = @"_active";
	else
		pathPortionHighlighted = @"";
	
	if(!available)
		pathPortionAvailable = @"_template";
	else
		pathPortionAvailable = @"";
	
	fullFileName = [NSString stringWithFormat:@"%@%@%@", baseName, pathPortionHighlighted, pathPortionAvailable];
	
	NSBundle *bundle = nil;
	
	if(themeID == 1 || themeID == 2)
	{
		NSString *path = [[NSBundle mainBundle] pathForResource:fullFileName ofType:@"png" inDirectory:[NSString stringWithFormat:@"theme %d", themeID]];
		NSRange range = [path rangeOfString:@"/" options: NSBackwardsSearch];
		
		bundle = [NSBundle bundleWithPath:[path substringToIndex:range.location+1]];
	}
	else
	{
		bundle = [NSBundle bundleWithPath:@"theme"];
	}
	
	if(bundle == nil)
		return nil;
	
	return [bundle imageForResource:fullFileName];
}

@end
