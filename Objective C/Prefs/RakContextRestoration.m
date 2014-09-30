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

@implementation RakContextRestoration

+ (void) saveContext : (NSString *) contextSerie : (NSString *) contextCT : (NSString *) contextReader : (NSString *) contextMDL
{
	FILE * output = fopen("context.dat.new", "w+");
	
	if(output == NULL)
		return;
	
	NSString * chain[] = {contextSerie, contextCT, contextReader, contextMDL};
	const char * intermediaryBuffer;
	char* stringOutput;
	size_t length;
	
	for(byte i = 0; i < 4; i++)
	{
		if(chain[i] == nil)
			chain[i] = STATE_EMPTY;
		
		intermediaryBuffer = [chain[i] UTF8String];
		length = strlen(intermediaryBuffer);
		
		stringOutput = malloc(length * 2 + 1);
		
		if (stringOutput == NULL)
		{
			memoryError(length + 1);
			fclose(output);
			remove("context.dat.new");
			return;
		}
		
		decToHex((const unsigned char*) intermediaryBuffer, length, stringOutput);
		
		stringOutput[length*2] = 0;
		
		fputs(stringOutput, output);
		
		if(i < 3)
			fputc(' ', output);
	}
	
	fclose(output);
	remove("context.dat");
	rename("context.dat.new", "context.dat");
}

+ (NSArray *) newContext
{
	NSString * fileContent = [NSString stringWithContentsOfFile:@"context.dat" encoding:NSASCIIStringEncoding error:NULL];
	
	if(fileContent == nil)
		return nil;
	
	NSArray *componentsWithSpaces = [fileContent componentsSeparatedByCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
	NSArray *data = [componentsWithSpaces filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"length > 0"]];
	
	char count;
	if([data count] >= 4)
		count = 4;
	else
		count = [data count];
	
	NSString * buffer;
	const char *bufferC;
	NSString *output[4] = {@"", @"", @"", @""};
	
	for (char pos = 0; pos < count; pos++)
	{
		buffer = [data objectAtIndex:pos];
		
		if([buffer length] & 1)	//Impaire, il faut ajouter un espace
		{
			NSLog(@"[Warning]: weird data received, will try to deal with it...");
			
			NSMutableString * recovery = [NSMutableString stringWithString:buffer];
			[recovery insertString:@"0" atIndex:0];
			buffer = [recovery copy];
		}
		
		bufferC = [buffer cStringUsingEncoding : NSASCIIStringEncoding];

		unsigned char decodedString[strlen(bufferC) / 2 + 1];
		
		hexToDec(bufferC, decodedString);
		
		decodedString[sizeof(decodedString) - 1] = 0;
		
		output[pos] = [NSString stringWithFormat:@"%s", decodedString];
	}
	
	return [[NSArray alloc] initWithObjects:output count:4];
}

@end
