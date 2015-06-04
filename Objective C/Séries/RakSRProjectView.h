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

enum
{
	SUGGESTION_REASON_TAG = 1,
	SUGGESTION_REASON_AUTHOR
};

@interface RakSRProjectView : NSView
{
	PROJECT_DATA _project;
	NSRect _workingArea;
	
	BOOL _animationRequested;
	
	NSImageView * thumbnail;
	RakText * projectName, * projectAuthor;
}

@property (readonly) uint elementID;
@property (readonly) NSRect workingArea;

- (instancetype) initWithProject : (PROJECT_DATA) project;

- (void) initContent;
- (void) updateProject : (PROJECT_DATA) project;

- (RakText *) getTextElement : (NSString *) string : (NSColor *) color : (byte) fontCode : (CGFloat) fontSize;

- (NSColor *) getTextColor;
- (NSColor *) getTagTextColor;
- (NSColor *) backgroundColor;
- (void) reloadColors;

- (NSSize) defaultWorkingSize;

- (NSSize) thumbSize;
- (NSPoint) originOfThumb : (NSRect) frameRect;
- (NSPoint) originOfName : (NSRect) frameRect : (NSPoint) thumbOrigin;
- (NSPoint) originOfAuthor : (NSRect) frameRect : (NSPoint) nameOrigin;

- (NSPoint) resizeContent : (NSSize) newSize : (BOOL) animated;

@end
