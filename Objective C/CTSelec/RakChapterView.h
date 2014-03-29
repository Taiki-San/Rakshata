/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 ********************************************************************************************/

@interface RakTextProjectName : RakText

- (NSRect) getProjectNameSize : (NSRect) superViewSize;

@end

@interface RakCTProjectImageView : NSImageView

- (id) initWithImageName : (NSString *) imageName : (NSRect) superViewFrame;
- (NSRect) getProjectImageSize : (NSRect) superViewFrame : (NSSize) imageSize;

@end

@interface RakChapterView : NSView
{
	uint mainThread;
	RakTextProjectName *projectName;
	RakCTProjectImageView * projectImage;
}

- (NSColor*) getBackgroundColor;

@end
