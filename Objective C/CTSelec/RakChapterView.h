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

@interface RakTextProjectName : RakMenuText

@end

@interface RakCTProjectImageView : NSImageView

- (id) initWithImageName : (char*) URLRepo : (NSString *) imageName : (NSRect) superViewFrame;
- (void) updateProject : (NSString *) imageName;

- (NSRect) getProjectImageSize : (NSRect) superViewFrame : (NSSize) imageSize;

- (void) resizeAnimation : (NSRect) frameRect;

@end

@interface RakCTContentTabView : NSView
{
	PROJECT_DATA data;
	RakCTCoreViewButtons * buttons;
	RakCTCoreContentView * tableViewControllerChapter;
	RakCTCoreContentView * tableViewControllerVolume;
}

- (id) initWithProject : (PROJECT_DATA) project : (bool) isTome : (NSRect) frame : (long [4]) context;
- (void) switchIsTome : (RakCTCoreViewButtons*) sender;
- (void) refreshCTData : (BOOL) checkIfRequired : (uint) ID;;

- (void) resizeAnimation : (NSRect) frameRect;

- (NSString *) getContextToGTFO;

- (void) gotClickedTransmitData : (bool) isTome : (uint) index;
- (void) selectElem : (uint) projectID : (BOOL) isTome : (int) element;
- (void) updateContext : (PROJECT_DATA) newData;

@end

@interface RakChapterView : RakTabContentTemplate
{
	RakTextProjectName *projectName;
	RakCTProjectImageView * projectImage;
	RakCTContentTabView * coreView;
}

- (id)initContent:(NSRect)frame : (PROJECT_DATA) project : (bool) isTome : (long [4]) context;

- (void) updateContext : (PROJECT_DATA) data;
- (void) refreshCT : (BOOL) checkIfRequired : (uint) ID;
- (void) selectElem : (uint) projectID : (BOOL) isTome : (int) element;

@end
