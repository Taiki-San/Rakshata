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

@implementation RakCTHeader

- (instancetype) initWithData : (NSRect) frameRect : (PROJECT_DATA) project
{
	header = [[RakCTHeaderImage alloc] initWithData : frameRect : project];

	if(header != nil)
	{
		self = [self initWithFrame:[self frameByParent:frameRect]];
		
		if(self != nil)
		{
			_backgroundColor = [Prefs getSystemColor : GET_COLOR_BACKGROUD_CT_READERMODE : self];
			_synopsisTitleBackground = [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS : nil];

			[header setFrame : self.bounds];
			[self addSubview:header];
			
			synopsis = [[RakCTProjectSynopsis alloc] initWithProject:project : self.bounds : header.bounds.size];
			if(synopsis != nil)
			{
				_synopsisTitleHeight = [synopsis titleHeight];	//For now, this height is stable
				[self addSubview:synopsis];
			}
		}
	}
	
	return self;
}

- (void) updateProject : (PROJECT_DATA) project
{
	[header updateHeaderProject:project];
	
	if(synopsis != nil)
		[synopsis updateProject : project];
	else
	{
		synopsis = [[RakCTProjectSynopsis alloc] initWithProject : project : self.bounds : header.bounds.size];
		if(synopsis != nil)
			[self addSubview:synopsis];
	}
}

#pragma mark - Resizing

- (NSRect) frameByParent : (NSRect) parentFrame
{
	NSRect output;
	
	output.size.width = parentFrame.size.width;
	output.size.height = [header sizeByParent : parentFrame].height;
	
	output.origin.x = 0;
	output.origin.y = parentFrame.size.height - output.size.height;
	
	return output;
}

- (void) setFrame:(NSRect) frameRect
{
	[super setFrame:[self frameByParent:frameRect]];
	
	[header setFrame : frameRect];
	[synopsis setFrame : frameRect : header.bounds.size];
}

- (void) resizeAnimation : (NSRect) frame
{
	frame = [self frameByParent : frame];

	[self.animator setFrame:frame];

	NSSize headerSize = [header frameByParent : frame].size;
	
	[header resizeAnimation:frame];
	[synopsis resizeAnimation : frame : headerSize];
}

#pragma mark - Drawing

- (void) drawRect:(NSRect)dirtyRect
{
	if(_synopsisTitleHeight)
	{
		[_synopsisTitleBackground setFill];
		NSRectFill(NSMakeRect(0, dirtyRect.size.height - _synopsisTitleHeight, dirtyRect.size.width, _synopsisTitleHeight));
		dirtyRect.size.height -= _synopsisTitleHeight;
	}

	[_backgroundColor setFill];
	NSRectFill(dirtyRect);
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	_backgroundColor = [Prefs getSystemColor : GET_COLOR_BACKGROUD_CT_READERMODE : nil];
	_synopsisTitleBackground = [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS : nil];
}

@end
