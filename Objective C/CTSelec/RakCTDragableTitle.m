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

#import "RakCTDragableTitle.h"

@implementation RakCTDragableTitle

- (instancetype) initWithText : (NSRect) frame : (NSString *) text
{
	self = [super initWithText:frame :text];
	
	if(self != nil)
	{
		_dragResponder = [[RakDragResponder alloc] init];
		outOfArea = YES;
	}
	
	return self;
}

- (void) mouseDown:(NSEvent *)theEvent
{
	outOfArea = NO;
	
	//We check if it's worth the trouble to initialize
	if(self.isEmpty || self.currentID == UINT_MAX || _dragResponder == nil)
		outOfArea = YES;
	
	else
	{
		NSSize textSize = [self intrinsicContentSize];
		mousePosition = [self convertPoint:[theEvent locationInWindow] fromView:nil];
		
		if(mousePosition.x > textSize.width || mousePosition.y > textSize.height)
			outOfArea = YES;
	}
}

- (void) mouseDragged:(NSEvent *)theEvent
{
	if(outOfArea)
		return;
	
	//We get a copy of the project data
	PROJECT_DATA projectData = getElementByID(self.currentID);
	if(!projectData.isInitialized)
		return;
	
	//We initialize the pasteboard
	NSPasteboard * pBoard = [NSPasteboard pasteboardWithName:NSDragPboard];
	[RakDragResponder registerToPasteboard:pBoard];
	
	//We create the shared item
	RakDragItem * item = [[RakDragItem alloc] init];
	if(item == nil)
		return;
	
	//We initialize the item, then insert it in the pasteboard
	[item setDataProject:projectData isTome:self.isTome element:VALEUR_FIN_STRUCT];
	[pBoard setData:[item getData] forType:PROJECT_PASTEBOARD_TYPE];
	
	//We create the image for the dragging session
	NSImage * image = [_dragResponder initializeImageForItem : projectData : NSLocalizedString(self.isTome ? @"CT-ALL-VOLUMES" : @"CT-ALL-CHAPTERS", nil) : VALEUR_FIN_STRUCT];
	
	couldDL = self.isTome ? (projectData.nombreTomesInstalled < projectData.nombreTomes) : (projectData.nombreChapitreInstalled < projectData.nombreChapitre);
	[RakList propagateDragAndDropChangeState : YES : couldDL];
	
	[self dragImage:image at:NSMakePoint(mousePosition.x - image.size.width / 3, mousePosition.y + 3) offset:NSZeroSize event:theEvent pasteboard:pBoard source:self slideBack:YES];
	
	[RakList propagateDragAndDropChangeState : NO : couldDL];
}

- (BOOL) mouseDownCanMoveWindow
{
	return self.isEmpty;
}

#pragma - mark NSDraggingSource support

- (NSDragOperation)draggingSession:(NSDraggingSession *)session sourceOperationMaskForDraggingContext:(NSDraggingContext)context;
{
	return NSDragOperationCopy;
}

@end
