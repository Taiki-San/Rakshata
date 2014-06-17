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

typedef struct project_data_for_drag_drop
{
	MANGAS_DATA data;
	bool isTome;
	int selection;
} INTERNAL_D_AND_D;

@implementation RakDragItem

- (void) setDataProject : (MANGAS_DATA) project isTome : (BOOL) isTome element : (int) element
{
	self.project = project;
	self.isTome = isTome;
	self.selection = element;
}

- (id) initWithData : (NSData *) data
{
	if(data == nil || [data length] != sizeof(INTERNAL_D_AND_D))
		return nil;
	
	self = [super init];
	
	if(self != nil)
	{
		INTERNAL_D_AND_D structure;
		[data getBytes:&structure length:sizeof(INTERNAL_D_AND_D)];
		
		self.project = structure.data;
		self.isTome = structure.isTome;
		self.selection = structure.selection;
	}
	
	return self;
}

- (NSData *) getData
{
	INTERNAL_D_AND_D structure;
	
	structure.data = self.project;
	structure.isTome = self.isTome;
	structure.selection = self.selection;
	
	return [NSData dataWithBytes:&structure length:sizeof(structure)];
}

- (BOOL) defineIsTomePriority : (MANGAS_DATA*) project  alreadyRefreshed : (BOOL) refreshed
{
	if(project == NULL)
		return NO;
	
	if(!refreshed)
	{
		getUpdatedCTList(project, true);
		getUpdatedCTList(project, false);
	}
	
	if(project->chapitresFull == NULL && project->tomesFull != NULL)
		return YES;
	else if(project->chapitresFull != NULL && project->tomesFull == NULL)
		return NO;
	else if(project->chapitresFull == NULL && project->tomesFull != NULL)
		return YES;

	if(project->tomesInstalled == NULL || project->nombreTomesInstalled < project->nombreTomes)
		return YES;
	
	return NO;
}

@end
