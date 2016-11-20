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
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

@implementation RakTextComplete

#pragma mark - Completion is required

//By default, we complete with project names
- (NSArray<NSString *> *)textView:(NSTextView *)textView completions:(NSArray<NSString *> *)words forPartialWordRange:(NSRange)charRange indexOfSelectedItem:(NSInteger *)index
{
	if(!self.wantCompletion)
		return nil;
	
	else if(self.callbackOnCompletion != nil)
		return self.callbackOnCompletion();
	
	uint nbElem;
	const char * partialString = [textView.string UTF8String];
	SEARCH_SUGGESTION * output = getProjectNameWith(partialString, &nbElem, true);
	
	if(output == NULL || nbElem == 0)
	{
		free(output);
		return @[];
	}
	
	//We determine the begining of the last typed word (what the completion will replace)
	uint length = strlen(partialString);
	while(length > 0 && partialString[length - 1] != ' ')
		length -= 1;
	
	NSMutableArray * array = [NSMutableArray array];
	
	for(uint i = 0; i < nbElem; i++)
	{
		[array addObject:[NSString stringWithUTF8String:&output[i].string[length]]];
		free(output[i].string);
	}
	
	free(output);
	
	return array;
}

@end
