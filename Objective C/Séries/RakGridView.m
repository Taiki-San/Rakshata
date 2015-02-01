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

#define BORDER 3

@implementation RakGridView

- (instancetype) initWithFrame:(NSRect)frameRect
{
	self = [self init];

	if(self != nil)
	{
		NSRect bounds = [self frameFromParent : frameRect];
		
		collection = [[RakCollectionView alloc] initWithFrame:bounds];
		if(collection == nil)
			return nil;
		
		scrollview = [[RakListScrollView alloc] initWithFrame : bounds];
		if(scrollview != nil)
		{
			scrollview.documentView = collection;
		}
	}
	
	return self;
}

#pragma mark - Access to view

- (RakListScrollView *) contentView		{	return scrollview;	}
- (void) setHidden : (BOOL) hidden		{	scrollview.hidden = hidden;	}
- (BOOL) isHidden 	{	return scrollview.isHidden;	}

#pragma mark - Resizing code

- (NSRect) frameFromParent : (NSRect) frame
{
	frame.origin.x = BORDER;
	frame.origin.y = BORDER;
	
	frame.size.width -= 2 * BORDER;
	frame.size.height -= 2 * BORDER;
	
	return frame;
}

- (void) setFrame : (NSRect) frameRect
{
	frameRect = [self frameFromParent:frameRect];
	
	[collection setFrame:frameRect];
	[scrollview setFrame:frameRect];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	frameRect = [self frameFromParent:frameRect];
	
	[collection resizeAnimation:frameRect];
	[scrollview.animator setFrame:frameRect];
}

@end
