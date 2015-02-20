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

enum
{
	TYPE_SEPARATOR_WIDTH = 3,
	TYPE_SEPARATOR_BORDER = 1,
	SYNOPSIS_SEPARATOR_BORDER = 4,
	SYNOPSIS_SEPARATOR_WIDTH = 1,
	OFFSET_SYNOPSIS = 7
};

@implementation RakSRDetails

- (instancetype) initWithFrame : (NSRect) frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		_project = getEmtpyProject();
		
		thumb = [[NSImageView alloc] init];
		if(thumb != nil)
			[self addSubview:thumb];
		else
			return nil;
		
		infos = [[RakText alloc] initWithText:@"Data" :[self getTextColor]];
		if(infos != nil)
		{
			[infos setAlignment:NSCenterTextAlignment];
			[self addSubview:infos];
		}
		else
			return nil;
		
		type = [[RakText alloc] initWithText:@"Data" :[self getTagColor]];
		if(type != nil)
			[self addSubview:type];
		else
			return nil;
		
		tag = [[RakText alloc] initWithText:@"Data" :[self getTagColor]];
		if(tag != nil)
			[self addSubview:tag];
		else
			return nil;
		
		synopsis = [[RakText alloc] init];
		if(synopsis != nil)
		{
			synopsis.fixedWidth = _bounds.size.width - 2 * OFFSET_SYNOPSIS;
			[synopsis setFrameOrigin : NSMakePoint(OFFSET_SYNOPSIS, 0)];
			
			[synopsis setAlignment:NSJustifiedTextAlignment];
			[synopsis.cell setWraps:YES];
			
			[synopsis setTextColor : [self getSynopsisColor]];

			[self addSubview:synopsis];
		}
		else
			return nil;
		
		[Prefs getCurrentTheme:self];
	}
	
	return self;
}

- (BOOL) isFlipped	{	return YES;	}

- (NSString *) getString : (PROJECT_DATA) project
{
	NSString * output = [NSString stringWithFormat:@"%@\n", getStringForWchar(project.authorName)], *current;
	
	if(project.nombreTomes && project.nombreChapitre)
	{
		if(project.nombreChapitre > 1)
		{
			if(project.nombreTomes > 1)
				current = [NSString localizedStringWithFormat:NSLocalizedString(@"PROJ-DETAILS-%zu-CHAPTERS-AND-%zu-VOLUMES", nil), project.nombreTomes, project.nombreChapitre];
			else
				current = [NSString localizedStringWithFormat:NSLocalizedString(@"PROJ-DETAILS-%zu-CHAPTERS-AND-%zu-VOLUME", nil), project.nombreTomes, project.nombreChapitre];
		}
		else
		{
			if(project.nombreTomes > 1)
				current = [NSString localizedStringWithFormat:NSLocalizedString(@"PROJ-DETAILS-%zu-CHAPTER-AND-%zu-VOLUMES", nil), project.nombreTomes, project.nombreChapitre];
			else
				current = [NSString localizedStringWithFormat:NSLocalizedString(@"PROJ-DETAILS-%zu-CHAPTER-AND-%zu-VOLUME", nil), project.nombreTomes, project.nombreChapitre];
		}
		
	}
	else if(project.nombreTomes)
	{
		current = [NSString localizedStringWithFormat:NSLocalizedString(project.nombreTomes > 1 ? @"PROJ-DETAILS-%zu-VOLUMES" : @"PROJ-DETAILS-%zu-VOLUME", nil), project.nombreTomes];
	}
	else if(project.nombreChapitre)
	{
		current = [NSString localizedStringWithFormat:NSLocalizedString(project.nombreChapitre > 1 ? @"PROJ-DETAILS-%zu-CHAPTERS" : @"PROJ-DETAILS-%zu-CHAPTER", nil), project.nombreChapitre];
	}
	
	output = [output stringByAppendingString:current];

	BOOL DRM = getRandom() & 1;
	
	if(project.isPaid)
		current = NSLocalizedString(DRM ? @"PROJ-DETAILS-PAID-DRM" : @"PROJ-DETAILS-PAID-NO-DRM" , nil);
	else
		current = NSLocalizedString(DRM ? @"PROJ-DETAILS-FREE-DRM" : @"PROJ-DETAILS-FREE-NO-DRM", nil);
	
	return [output stringByAppendingString:current];
}

- (PROJECT_DATA) project
{
	return _project;
}

- (void) setProject:(PROJECT_DATA)project
{
	if(!project.isInitialized)
	{
		NSLog(@"Invalid project D:");
		return;
	}
	
	NSImage * image = loadImage(project, PROJ_IMG_SUFFIX_SRGRID);
	if(image != nil)
	{
		thumb.image = image;
		[thumb setFrameSize:image.size];
		
		infos.stringValue = [self getString:project];
		[infos sizeToFit];
		
		type.stringValue = getStringForWchar(getTagForCode(project.type));		[type sizeToFit];
		tag.stringValue = getStringForWchar(getTagForCode(project.tag));		[tag sizeToFit];
		
		synopsis.stringValue = getStringForWchar(project.description);			[synopsis sizeToFit];
		
		_project = project;
		
		[self setFrame:_frame];
	}
	else
	{
		NSLog(@"Invalid image D:");
	}
}

#pragma mark - Sizing

- (void) setFrame:(NSRect)frameRect
{
	[self updateFrame:frameRect :NO];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[self updateFrame:frameRect :YES];
}

- (void) updateFrame : (NSRect) newFrame : (BOOL) animated
{
	if(animated)
		[self.animator setFrame:newFrame];
	else
		[super setFrame:newFrame];
	
	if(!_project.isInitialized)
		return;
	
	NSPoint oldOrigin;
	newFrame.origin = NSZeroPoint;
	
	if(newFrame.size.width != _bounds.size.width)
	{
		infos.fixedWidth = newFrame.size.width;
		synopsis.fixedWidth = newFrame.size.width - 2 * SYNOPSIS_MAIN_TEXT_BORDER;
	}
	
	if(animated)
	{
		[thumb.animator setFrameOrigin:	(oldOrigin = [self thumbOrigin:newFrame])];
		[infos.animator setFrameOrigin:	(oldOrigin = [self infoOrigin:newFrame :oldOrigin])];
		[type.animator setFrameOrigin:	(oldOrigin = [self typeOrigin:newFrame :oldOrigin])];
		[tag.animator setFrameOrigin:	(oldOrigin = [self tagOrigin:newFrame :oldOrigin])];
		[synopsis.animator setFrame:	[self synopsisFrame:newFrame :oldOrigin]];
	}
	else
	{
		[thumb setFrameOrigin:	(oldOrigin = [self thumbOrigin:newFrame])];
		[infos setFrameOrigin:	(oldOrigin = [self infoOrigin:newFrame :oldOrigin])];
		[type setFrameOrigin:	(oldOrigin = [self typeOrigin:newFrame :oldOrigin])];
		[tag setFrameOrigin:	(oldOrigin = [self tagOrigin:newFrame :oldOrigin])];
		[synopsis setFrame:		[self synopsisFrame:newFrame :oldOrigin]];
	}
}

- (NSPoint) thumbOrigin : (NSRect) frame
{
	return NSMakePoint(frame.size.width / 2 - thumb.bounds.size.width / 2, 8);
}

- (NSPoint) infoOrigin : (NSRect) frame : (NSPoint) previousPoint
{
	return NSMakePoint(frame.size.width / 2 - infos.bounds.size.width / 2, previousPoint.y + thumb.bounds.size.height);
}

- (NSPoint) typeOrigin : (NSRect) frame : (NSPoint) previousPoint
{
	const CGFloat fullWidth = type.bounds.size.width + TYPE_SEPARATOR_WIDTH + TYPE_SEPARATOR_BORDER + tag.bounds.size.width;
	
	return NSMakePoint(frame.size.width / 2 - fullWidth / 2, previousPoint.y + infos.bounds.size.height);
}

- (NSPoint) tagOrigin : (NSRect) frame : (NSPoint) previousPoint
{
	previousPoint.x += type.bounds.size.width + TYPE_SEPARATOR_WIDTH + TYPE_SEPARATOR_BORDER;
	return previousPoint;
}

- (NSRect) synopsisFrame : (NSRect) frame : (NSPoint) previousPoint
{
	frame.size.width -= 2 * OFFSET_SYNOPSIS;
	frame.origin.x = OFFSET_SYNOPSIS;
	frame.origin.y = previousPoint.y + MAX(type.bounds.size.height, tag.bounds.size.height) + 2 * SYNOPSIS_SEPARATOR_BORDER + SYNOPSIS_SEPARATOR_WIDTH;
	
	frame.size.height -= frame.origin.y;
	
	return frame;
}

#pragma mark - Drawing

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if ([object class] != [Prefs class])
		return;
	
	infos.textColor = [self getTextColor];
	type.textColor = [self getTagColor];
	tag.textColor = [self getTagColor];
	synopsis.textColor = [self getSynopsisColor];
	
	[self setNeedsDisplay:YES];
}

- (NSColor *) getTextColor
{
	return [Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT :nil];
}

- (NSColor *) getTagColor
{
	return [Prefs getSystemColor:GET_COLOR_TAGITEM_FONT :nil];
}

- (NSColor *) getSynopsisColor
{
	return [Prefs getSystemColor:GET_COLOR_ACTIVE : nil];
}

- (NSColor *) interTagColor
{
	return [self getTextColor];
}

- (NSColor *) borderColor
{
	return [self getTextColor];
}

- (void) drawRect : (NSRect) dirtyRect
{
	NSRect frame = type.frame;
	
	//Intertag separator
	frame.origin.x += frame.size.width;
	frame.origin.y += ceil(frame.size.height / 2) + 1;
	frame.size.width = TYPE_SEPARATOR_WIDTH;
	frame.size.height = 1;
	
	[[self interTagColor] setFill];
	NSRectFill(frame);
	
	//Description separator
	if(_project.description[0])
	{
		frame.origin.x = 20;
		frame.size.width = _bounds.size.width - 2 * frame.origin.x;
		
		frame.origin.y = NSMaxY(tag.frame) + SYNOPSIS_SEPARATOR_BORDER;
		frame.size.height = SYNOPSIS_SEPARATOR_WIDTH;
		
		[[self borderColor] setFill];
		NSRectFill(frame);
	}
}

@end
