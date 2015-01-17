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

//We don't register for color changes as the displayed image is a bitmap of what we're crafting, and the lifespan of the real object is really short

@implementation RakDragView

- (void) setupContent : (PROJECT_DATA) projectData : (NSString *) selectionNameString
{
	_originalWidth = self.bounds.size.width;
	
	if(projectImage != nil)
	{
		char * encodedHash = getPathForRepo(projectData.repo->URL);
		
		if(encodedHash != NULL)
		{
			NSBundle * bundle = [NSBundle bundleWithPath: [NSString stringWithFormat:@"imageCache/%s/", encodedHash]];
			if(bundle != nil)
			{
				projectImage.image = [bundle imageForResource:[NSString stringWithFormat:@"%d_"PROJ_IMG_SUFFIX_DD, projectData.projectID]];
			}
			
			if(projectImage.image == nil)
			{
				projectImage.image = [RakResPath craftResNameFromContext:@"defaultDragImage" :NO :YES : 1];
			}
			free(encodedHash);
		}
	}
	
	if (projectName != nil)
	{
		[projectName setStringValue:getStringForWchar(projectData.projectName)];
		[projectName setTextColor:[Prefs getSystemColor:GET_COLOR_ACTIVE:nil]];
		[self optimizeWidth : projectName];
	}
	
	if(selectionNameString != nil && selectionName != nil)
	{
		[selectionName setStringValue:selectionNameString];
		[selectionName setTextColor:[Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT:nil]];
		[self optimizeWidth : selectionName];
	}
	else
	{
		_noContent = YES;
	}
	
	if(separationLine != nil)
		[separationLine setBorderColor:[Prefs getSystemColor:GET_COLOR_SURVOL:nil]];
}

#define SPACE_AT_THE_RIGHT 5

- (void) optimizeWidth : (NSTextField *) element
{
	NSRect oldFrame = element.frame;
	
	[element sizeToFit];
	
	NSRect newFrame = element.bounds;
	newFrame.origin = oldFrame.origin;
	
	//We recenter on the y axis
	if(newFrame.size.height != oldFrame.size.height)
	{
		newFrame.origin.y += (newFrame.size.height - oldFrame.size.height) / 2;
		[element setFrameOrigin:newFrame.origin];
	}

	//We check if we need to increse the full width
	if(NSMaxX(newFrame) + SPACE_AT_THE_RIGHT > self.bounds.size.width)
	{
		NSSize size = self.bounds.size;
		size.width = newFrame.origin.x + newFrame.size.width + SPACE_AT_THE_RIGHT;
		[self setFrameSize:size];
	}
}

#define PRICE_BORDER 20

//Our goal is to be centered to the right, but with at least PRICE_BORDER between the content and us.
//The one exception is when there is no content, when we are centered to the base position of the content
//To achieve that, we set our origin to the minimal margin, then ask for an optimization.
//If width didn't changed, that mean we weren't far enough to the right
- (void) addPrice : (RakText *) price
{
	if(selectionName == nil)
		return;

	NSRect contentFrame = selectionName.frame;
	if(!_noContent)
		contentFrame.origin.x = NSMaxX(contentFrame) + PRICE_BORDER;

	[price setFrameOrigin: contentFrame.origin];
	[self addSubview:price];

	CGFloat preoptimizationWidth = self.bounds.size.width;
	[self optimizeWidth:price];
	
	if(!_noContent && preoptimizationWidth == self.bounds.size.width)
	{
		contentFrame.origin.x = preoptimizationWidth - SPACE_AT_THE_RIGHT - price.bounds.size.width;
		[price setFrameOrigin: contentFrame.origin];
	}
	
	_noContent = NO;
}

- (void) finalPostProcessing
{
	if(_noContent)
	{
		[selectionName removeFromSuperview];
		selectionName = nil;

		if(separationLine != nil)
		{
			[separationLine removeFromSuperview];
			separationLine = nil;
		}
		
		NSRect frame = projectName.frame;
		frame.origin.y = self.frame.size.height / 2 - frame.size.height / 2;
		[projectName setFrameOrigin:frame.origin];
	}
	else if(separationLine != nil && self.bounds.size.width > _originalWidth)
	{
		NSRect frame = separationLine.frame;
		frame.size.width = self.bounds.size.width - frame.origin.x - 20;
		[separationLine setFrame : frame];
	}
}

#define DRAGVIEW_BORDURE_VERT 7
#define DRAGVIEW_BORDURE_HORIZ 7

- (NSImage *) createImage
{
	NSRect frame = NSMakeRect(0, 0, self.frame.size.width + 2 * DRAGVIEW_BORDURE_VERT, self.frame.size.height + 2 * DRAGVIEW_BORDURE_HORIZ);
	NSView * view = [[NSView alloc] initWithFrame:frame];
	
	if(view == nil)
		return nil;
	
	[view setWantsLayer:YES];
	[view.layer setBackgroundColor:[Prefs getSystemColor:GET_COLOR_BACKGROUND_DRAG_AND_DROP:nil].CGColor];
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
