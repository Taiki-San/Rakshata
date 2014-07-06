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

@implementation RakDragView

- (void) setupContent : (PROJECT_DATA) projectData : (NSString *) selectionNameString
{
	if(projectImage != nil)
	{
		NSBundle * bundle = [NSBundle bundleWithPath: [NSString stringWithFormat:@"imageCache/%s/", projectData.team->URLRepo]];
		if(bundle != nil)
		{
			projectImage.image = [bundle imageForResource:[NSString stringWithFormat:@"%d_DD", projectData.projectID]];
		}
		
		if(projectImage.image == nil)
		{
			projectImage.image = [RakResPath craftResNameFromContext:@"defaultDragImage" :NO :YES : 1];
		}
	}
	
	if (projectName != nil)
	{
		[projectName setStringValue:[[NSString alloc] initWithData:[NSData dataWithBytes:projectData.projectName length:sizeof(projectData.projectName)] encoding:NSUTF32LittleEndianStringEncoding]];
		[projectName setTextColor:[Prefs getSystemColor:GET_COLOR_ACTIVE]];
	}
	
	if(selectionNameString != nil && selectionName != nil)
	{
		[selectionName setStringValue:selectionNameString];
		[selectionName setTextColor:[Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT]];
	}
	else
	{
		[selectionName setHidden:YES];
		if(separationLine != nil)	[separationLine setHidden:YES];
		
		NSRect frame = [projectName frame];
		[projectName removeConstraints:[projectName constraints]];
		
		frame.origin.y = self.frame.size.height / 2 - frame.size.height / 2;
		[projectName setFrameOrigin:frame.origin];
	}
	
	if(separationLine != nil)
	{
		[separationLine setBorderColor:[Prefs getSystemColor:GET_COLOR_SURVOL]];
	}
}

#define DRAGVIEW_BORDURE_VERT 7
#define DRAGVIEW_BORDURE_HORIZ 7

- (NSImage *) initImage
{
	NSRect frame = NSMakeRect(0, 0, self.frame.size.width + 2 * DRAGVIEW_BORDURE_VERT, self.frame.size.height + 2 * DRAGVIEW_BORDURE_HORIZ);
	NSView * view = [[[NSView alloc] initWithFrame:frame] autorelease];
	
	if(view == nil)
		return nil;
	
	[view setWantsLayer:YES];
	[view.layer setBackgroundColor:[Prefs getSystemColor:GET_COLOR_BACKGROUND_READER_INTAB].CGColor];
	[view.layer setCornerRadius:5];
	
	[view addSubview:self];
	[self setFrameOrigin:NSMakePoint(DRAGVIEW_BORDURE_HORIZ, DRAGVIEW_BORDURE_VERT)];
	
	NSBitmapImageRep *bir = [view bitmapImageRepForCachingDisplayInRect:[view bounds]];
	[bir setSize:view.bounds.size];
	[view cacheDisplayInRect:[view bounds] toBitmapImageRep:bir];
	NSImage * output = [[NSImage alloc] initWithSize : view.bounds.size];
	[output addRepresentation : bir];
	
	[self removeFromSuperview];
	
	return output;
}

@end
