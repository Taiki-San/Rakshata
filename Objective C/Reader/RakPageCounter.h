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

@interface RakPageCounterTextFieldCell : NSTextFieldCell

@end

@interface RakPageCounterPopoverController : NSViewController
{
	IBOutlet NSTextField *mainLabel;
	IBOutlet NSTextField *textField;
	IBOutlet NSView * gotoButtonContainer;
	
	IBOutlet NSPopover * popover;
	
	NSView * _anchor;
	
	uint _maxPage;
}

- (void) launchPopover : (NSView *) anchor : (uint) curPage : (uint) maxPage;

@end

@interface RakPageCounter : RakText
{
	Reader * _target;
	
	uint currentPage;
	uint pageMax;
	
	IBOutlet RakPageCounterPopoverController * controller;
}

- (id)init: (NSView*) superView : (CGFloat) posX : (uint) currentPageArg : (uint) pageMaxArg : (Reader *) target;
- (void) updateContext;

- (void) updateSize : (CGFloat) heightSuperView : (CGFloat) posX;

- (NSColor *) getColorBackground;
- (NSColor *) getFontColor;

- (void) updatePage : (uint) newCurrentPage : (uint) newPageMax;
- (void) transmitPageJump : (uint) newPage;

@end
