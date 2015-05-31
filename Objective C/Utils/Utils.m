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

@implementation RakFlippedView

- (BOOL) isFlipped
{
	return YES;
}

@end

NSString * getStringForWchar(const charType * string)
{
	if(string == NULL)
		return @"";
	
	uint length = wstrlen(string);
	NSString * output;
	
	do
	{
		output = [[NSString alloc] initWithData:[NSData dataWithBytes:string length:length * sizeof(charType)] encoding:NSUTF32LittleEndianStringEncoding];
		length--;
		
	} while(output == nil && length > 0);
	
	if(output == nil)
		output = @"";
	
	return output;
}

int compareStrings(const void* a, uint lengthA, const void* b, uint lengthB, int compareEncoding)
{
	NSString * stringA = compareEncoding == COMPARE_UTF8 ? [[NSString alloc] initWithBytes:a length:lengthA encoding:NSUTF8StringEncoding] : getStringForWchar(a);
	NSString * stringB = compareEncoding == COMPARE_UTF8 ? [[NSString alloc] initWithBytes:b length:lengthB encoding:NSUTF8StringEncoding] : getStringForWchar(b);
	
	return [stringA localizedCompare:stringB];
}

//Flush NSBundle cache

// First, we declare the function. Making it weak-linked
// ensures the preference pane won't crash if the function
// is removed from in a future version of Mac OS X.
extern void _CFBundleFlushBundleCaches(CFBundleRef bundle)
__attribute__((weak_import));

void flushBundleCache(NSBundle *bundle)
{
	// Before calling the function, we need to check if it exists
	// since it was weak-linked.
	if (_CFBundleFlushBundleCaches != NULL)
	{
		CFBundleRef cfBundle = CFBundleCreate(nil, (__bridge CFURLRef)[bundle bundleURL]);

		_CFBundleFlushBundleCaches(cfBundle);
		CFRelease(cfBundle);
	}
}