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
			[header setFrame : self.bounds];
			[self addSubview:header];
			
			synopsis = [[RakCTProjectSynopsis alloc] initWithProject:project : self.bounds : header.bounds.size];
			if(synopsis != nil)
				[self addSubview:synopsis];
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

@end
