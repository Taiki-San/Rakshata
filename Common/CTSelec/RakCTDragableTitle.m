/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
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
		return [super mouseDragged:theEvent];
	
	//We get a copy of the project data
	PROJECT_DATA projectData = getProjectByID(self.currentID);
	if(!projectData.isInitialized)
		return;
	
	//We initialize the pasteboard
	NSPasteboard * pBoard = [NSPasteboard pasteboardWithName:NSDragPboard];
	[RakDragResponder registerToPasteboard:pBoard];

	//No file drop if nothing installed
	if(ACCESS_DATA(self.isTome, projectData.nbChapterInstalled, projectData.nbVolumesInstalled) != 0)
		[RakDragResponder patchPasteboardForFiledrop:pBoard forType:ARCHIVE_FILE_EXT];
	
	//We create the shared item
	RakDragItem * item = [[RakDragItem alloc] init];
	if(item == nil)
		return;
	
	//We initialize the item, then insert it in the pasteboard
	[item setDataProject:projectData fullProject:NO isTome:self.isTome element:INVALID_VALUE];
	[pBoard setData:[item getData] forType:PROJECT_PASTEBOARD_TYPE];
	
	//We create the image for the dragging session
	RakImage * image = [_dragResponder initializeImageForItem : projectData : NSLocalizedString(self.isTome ? @"CT-ALL-VOLUMES" : @"CT-ALL-CHAPTERS", nil) : INVALID_VALUE];
	
	couldDL = self.isTome ? (projectData.nbVolumesInstalled < projectData.nbVolumes) : (projectData.nbChapterInstalled < projectData.nbChapter);
	[RakList propagateDragAndDropChangeState : YES : couldDL];
	
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	
	[self dragImage:image at:NSMakePoint(mousePosition.x - image.size.width / 3, mousePosition.y + 3) offset:NSZeroSize event:theEvent pasteboard:pBoard source:self slideBack:YES];
	
#pragma clang diagnostic pop
	
	[RakList propagateDragAndDropChangeState : NO : couldDL];
}

- (BOOL) mouseDownCanMoveWindow
{
	return self.isEmpty;
}

#pragma mark - NSDraggingSource support

- (void) draggingSession:(NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint
{
	currentSession = session;
}

- (void) draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation
{
	currentSession = nil;
}

- (NSDragOperation)draggingSession:(NSDraggingSession *)session sourceOperationMaskForDraggingContext:(NSDraggingContext)context;
{
	return NSDragOperationCopy;
}

- (NSArray<NSString *> *)namesOfPromisedFilesDroppedAtDestination:(NSURL *)dropDestination
{
	[RakExportController createArchiveFromPasteboard:[currentSession draggingPasteboard] toPath:nil withURL:dropDestination];
	return nil;
}

@end
