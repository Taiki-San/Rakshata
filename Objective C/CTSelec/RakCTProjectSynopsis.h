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

@interface RakCTProjectSynopsis : NSView
{
	RakMenuText * _title;
	
	RakListScrollView * _scrollview;
	RakText * _synopsis;
	RakText * _placeholder;
	
	BOOL placeholderString;
}

- (instancetype) initWithProject : (PROJECT_DATA) project : (NSRect) frame : (NSSize) headerSize;
- (void) updateProject : (PROJECT_DATA) newProject;

- (void) setFrame : (NSRect) frameRect : (NSSize) headerSize;
- (void) resizeAnimation : (NSRect) frameRect : (NSSize) headerSize;

@end
