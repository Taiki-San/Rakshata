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

@interface RakCTHImageGradient : RakGradientView
{
	NSGradient * _titleGradient;
}

@end

@interface RakCTHImage : NSImageView
{
	RakCTHImageGradient * gradient;
	RakImage * defaultImage;
	
	PROJECT_DATA _cachedProject;
}

- (instancetype) initWithProject : (NSRect) parentFrame : (PROJECT_DATA) data;
- (BOOL) loadProject : (PROJECT_DATA) data;

- (RakCTHImageGradient *) gradientView;

- (void) resizeAnimation : (NSRect) frameRect;

@end
