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

#define EXPORT_GENERIC_NAME "Archive"

@implementation RakExportController

+ (NSString *) craftArchiveNameFromPasteboard : (NSPasteboard *) pasteboard
{
	if(pasteboard == nil)
		return nil;

	RakDragItem * item = [[RakDragItem alloc] initWithData: [pasteboard dataForType:PROJECT_PASTEBOARD_TYPE]];
	if(item == nil)
		return nil;

	return [@"/" stringByAppendingString:[self craftArchiveName:item.project isTome:item.isTome selection:item.selection]];
}

+ (NSString *) craftArchiveName : (PROJECT_DATA) project isTome : (BOOL) isTome selection : (int) selection
{
	NSString * projectPath = [getStringForWchar(project.projectName) stringByTrimmingCharactersInSet:[NSCharacterSet characterSetWithCharactersInString:@"/:"]], * outputPath = nil;

	if(projectPath != nil && [projectPath length] > 0)
	{
		if(selection != INVALID_SIGNED_VALUE)
		{
			if(isTome)
			{
				uint position = getPosForID(project, false, selection);
				if(position == INVALID_VALUE)
					return nil;

				META_TOME volumeData = project.tomesFull[position];

				if(volumeData.readingName[0] != 0)
					outputPath = [NSString stringWithFormat:@"%@ - %@ ", projectPath, getStringForWchar(volumeData.readingName)];
				else
					outputPath = [NSString stringWithFormat:NSLocalizedString(@"VOLUME-%d", nil), volumeData.readingID];
			}

			else
				outputPath = [NSString stringWithFormat:@"%@ - %@ ", projectPath, [NSString stringWithFormat:NSLocalizedString(@"CHAPTER-%d", nil), selection]];
		}
		else
			outputPath = projectPath;
	}
	else
		outputPath = @EXPORT_GENERIC_NAME;

	return [outputPath stringByAppendingString:@"."ARCHIVE_FILE_EXT];
}

+ (void) createArchiveFromPasteboard : (NSPasteboard *) pasteboard toPath : (NSString *) path
{

}

+ (void) addProject : (PROJECT_DATA) project item : (uint) position isTome : (BOOL) isTome toArchive : (zipFile *) file
{

}

#pragma mark - Utils

+ (BOOL) onlyHaveOneProject : (PROJECT_DATA *) project nbEntries : (uint) nbEntries
{
	for(uint i = 1; i < nbEntries; i++)
	{
		if(project[i - 1].cacheDBID != project[i].cacheDBID)
			return NO;
	}

	return YES;
}

@end
