//
//  RakTextComplete.m
//  Rakshata
//
//  Created by Taiki on 20/11/2016.
//  Copyright © 2016 Taiki. All rights reserved.
//

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
