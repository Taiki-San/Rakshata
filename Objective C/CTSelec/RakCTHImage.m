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

#define DEFAULT_TITLE_HEIGHT (CT_READERMODE_WIDTH_PROJECT_NAME + 6)

@implementation RakCTHImage

- (id) initWithProject : (NSRect) parentFrame : (PROJECT_DATA) data
{
	if(data.repo == NULL)
		return nil;
	
	self = [self initWithFrame : parentFrame];
	
	if(self != nil)
	{
		if(![self loadProject : data])
			return nil;
		
		[self setImageScaling:NSImageScaleProportionallyUpOrDown];
	}
	
	return self;
}

- (BOOL) mouseDownCanMoveWindow
{
	return YES;
}

- (BOOL) loadProject : (PROJECT_DATA) data
{
	char * teamPath = getPathForRepo(data.repo);
	
	if(teamPath == NULL)
		return NO;
	
	NSImage * image = nil;

	NSBundle * bundle = [NSBundle bundleWithPath: [NSString stringWithFormat:@"imageCache/%s/", teamPath]];
	if(bundle != nil)
		image = [bundle imageForResource:[NSString stringWithFormat:@"%d_"PROJ_IMG_SUFFIX_HEAD, data.projectID]];
	
	if(image == nil)
	{
		if(defaultImage == nil)
			defaultImage = [NSImage imageNamed:@"project_large"];
		
		image = defaultImage;
	}
	
	self.image = image;
	free(teamPath);
	
	return YES;
}

- (RakCTHImageGradient *) gradientView
{
	if(gradient == nil)
	{
		gradient = [[RakCTHImageGradient alloc] initWithFrame:self.frame];
		if(gradient != nil)
		{
			gradient.gradientMaxWidth = 200;
			gradient.gradientWidth = 0.2;
			gradient.angle = 180;
			[gradient initGradient];
		}
	}
	return gradient;
}

- (void) dealloc
{
	self.image = nil;
	[gradient removeFromSuperview];
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame : frameRect];
	[gradient setFrame : frameRect];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[self.animator setFrame : frameRect];
	[gradient.animator setFrame : frameRect];
}

@end

@implementation RakCTHImageGradient

- (NSColor *) startColor
{
	return [Prefs getSystemColor : GET_COLOR_BACKGROUD_COREVIEW : nil];
}

- (NSColor *) endColor : (NSColor *) startColor
{
	return [startColor colorWithAlphaComponent:0];
}

- (void) drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];
	
	NSRect frame = [super grandientBounds];
	frame.origin.y = frame.size.height - DEFAULT_TITLE_HEIGHT;
	frame.size.height = DEFAULT_TITLE_HEIGHT;

	[_titleGradient drawInRect : frame angle : self.angle];
}

#pragma mark - UI utilities

- (NSRect) grandientBounds
{
	NSRect frame = [super grandientBounds];
	
	frame.size.height -= DEFAULT_TITLE_HEIGHT;
	
	return frame;
}

- (void) updateGradient
{
	NSColor * startColor = [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS : nil];
	_titleGradient = [[NSGradient alloc] initWithStartingColor : startColor endingColor : [self endColor: startColor]];
	
	[super updateGradient];
}

@end
