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

@implementation RakCTHImage

- (id) initWithProject : (NSRect) parentFrame : (PROJECT_DATA) data
{
	if(data.team == NULL)
		return nil;
	
	self = [self initWithFrame : parentFrame];
	
	if(self != nil)
	{
		if(![self loadProject : data])
		{
			[self release];
			return nil;
		}
		
		[self setImageScaling:NSImageScaleProportionallyUpOrDown];
	}
	
	return self;
}

- (BOOL) loadProject : (PROJECT_DATA) data
{
	char * teamPath = getPathForTeam(data.team->URLRepo);
	
	if(teamPath == NULL)
		return NO;
	
	NSImage * image = nil;

	NSBundle * bundle = [NSBundle bundleWithPath: [NSString stringWithFormat:@"imageCache/%s/", teamPath]];
	if(bundle != nil)
		image = [[bundle imageForResource:[NSString stringWithFormat:@"%d_"PROJ_IMG_SUFFIX_HEAD, data.projectID]] autorelease];
	
	if(image == nil)
	{
		if(defaultImage == nil)
			defaultImage = [[NSImage imageNamed:@"project_large"] retain];
		
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
	[defaultImage release];
	
	[gradient removeFromSuperview];
	[gradient release];
	
	[super dealloc];
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame : frameRect];
	[gradient setFrame : frameRect];
}

@end

@implementation RakCTHImageGradient

- (NSColor *) startColor
{
	return [[Prefs getSystemColor : GET_COLOR_BACKGROUND_TABS : nil] retain];
}

- (NSColor *) endColor : (NSColor *) startColor
{
	return [[startColor colorWithAlphaComponent:0] retain];
}

@end
