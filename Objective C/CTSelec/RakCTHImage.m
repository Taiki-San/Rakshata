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

@implementation RakCTHImage

- (id) initWithProject : (NSRect) parentFrame : (PROJECT_DATA) data
{
	if(data.team == NULL)
		return nil;
	
	self = [self initWithFrame : parentFrame];
	
	if(self != nil)
	{
		char * teamPath = getPathForTeam(data.team->URLRepo);
		
		if(teamPath == NULL)
		{
			[self release];
			return nil;
		}
		
		NSBundle * bundle = [NSBundle bundleWithPath: [NSString stringWithFormat:@"imageCache/%s/", teamPath]];
		if(bundle != nil)
		{
			self.image = [[bundle imageForResource:[NSString stringWithFormat:@"%d_"PROJ_IMG_SUFFIX_HEAD, data.projectID]] autorelease];
		}
	}
	
	return self;
}

@end
