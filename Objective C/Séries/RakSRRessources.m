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

@implementation RakSRHeaderText

- (NSRect) getMenuFrame : (NSRect) superviewSize
{
	superviewSize.size.height = CT_READERMODE_WIDTH_PROJECT_NAME;
	
	return superviewSize;
}

@end

@implementation RakTableRowView

- (void) drawBackgroundInRect:(NSRect)dirtyRect
{
	
}

@end

@implementation RakSRSubMenu

- (CGFloat) getFontSize
{
	return [NSFont systemFontSize];
}

- (CGFloat) getTextHeight
{
	return 21;
}

- (NSColor *) getBackgroundColor
{
	return nil;
}

@end

NSImage * loadImage(const PROJECT_DATA project, const char * suffix)
{
	if(!project.isInitialized || suffix == NULL)
		return nil;
	
	char * teamPath = getPathForRepo(project.repo);
	
	if(teamPath == NULL)
		return nil;
	
	NSImage * image = nil;
	
	NSBundle * bundle = [NSBundle bundleWithPath: [NSString stringWithFormat:@"imageCache/%s/", teamPath]];
	if(bundle != nil)
		image = [bundle imageForResource:[NSString stringWithFormat:@"%d_%s", project.projectID, suffix]];
	
	if(image == nil)
		image = [NSImage imageNamed:@"defaultSRImage"];
	
	return image;
}