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
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

typedef struct //project_data_for_drag_drop
{
	PROJECT_DATA data;
	uint selection;
	BOOL isTome;
	BOOL fullProject;
	BOOL canDL;
	uint price;
} INTERNAL_D_AND_D;

@implementation RakDragItem

- (void) setDataProject : (PROJECT_DATA) project fullProject : (BOOL) fullProject isTome : (BOOL) isTome element : (uint) element
{
	self.project = project;
	self.isTome = isTome;
	self.fullProject = fullProject;
	self.selection = element;
	
	canDL = [[self class] canDL:project isTome:isTome element:element];
}

- (instancetype) initWithData : (NSData *) data
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
		self.fullProject = structure.fullProject;
		self.selection = structure.selection;
		self.price = structure.price;
		canDL = structure.canDL;
	}
	
	return self;
}

- (NSData *) getData
{
	INTERNAL_D_AND_D structure;
	
	structure.data = self.project;
	structure.isTome = self.isTome;
	structure.fullProject = self.fullProject;
	structure.selection = self.selection;
	structure.price = self.price;
	structure.canDL = canDL;
	
	return [NSData dataWithBytes:&structure length:sizeof(structure)];
}

- (BOOL) isFavorite
{
	return self.project.favoris;
}

+ (BOOL) isFavorite : (NSPasteboard*) pasteboard
{
	if(pasteboard != nil)
	{
		RakDragItem * item = [[RakDragItem alloc] initWithData: [pasteboard dataForType:PROJECT_PASTEBOARD_TYPE]];
		if(item != nil)
		{
			return [item isFavorite];
		}
	}
	return YES;
}

- (BOOL) canDL
{
	return canDL;
}

- (BOOL) canDLRefreshed
{
	return (canDL = [[self class] canDL:_project isTome:_isTome element:_selection]);
}

+ (BOOL) canDL : (NSPasteboard*) pasteboard
{
	if(pasteboard != nil)
	{
		RakDragItem * item = [[RakDragItem alloc] initWithData: [pasteboard dataForType:PROJECT_PASTEBOARD_TYPE]];
		if(item != nil)
		{
			return [item canDL];
		}
	}
	return YES;
}

+ (BOOL) canDL : (PROJECT_DATA) project isTome : (BOOL) isTome element : (uint) element
{
	if(element == INVALID_VALUE)
	{
		if(isTome)
			return (project.nbVolumes != INVALID_VALUE && project.nbVolumes != project.nbVolumesInstalled);
		
		return (project.nbChapter != INVALID_VALUE && project.nbChapter != project.nbChapterInstalled);
	}
	
	return !checkReadable(project, isTome, (uint) element);
}

+ (BOOL) defineIsTomePriority : (PROJECT_DATA*) project  alreadyRefreshed : (BOOL) refreshed
{
	if(project == NULL)
		return NO;
	
	if(!refreshed)
	{
		getUpdatedCTList(project, true);
		getUpdatedCTList(project, false);
	}
	
	if(project->chaptersFull == NULL && project->volumesFull != NULL)
		return YES;
	else if(project->chaptersFull != NULL && project->volumesFull == NULL)
		return NO;
	else if(project->chaptersFull == NULL && project->volumesFull != NULL)
		return YES;
	
	if(project->volumesInstalled == NULL || project->nbVolumesInstalled < project->nbVolumes)
		return YES;
	
	return NO;
}

@end
