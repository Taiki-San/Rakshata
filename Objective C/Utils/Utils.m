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

NSNumber * getNumberForString(NSString * string)
{
	NSNumberFormatter * formatter = [[NSNumberFormatter alloc] init];
	formatter.numberStyle = NSNumberFormatterDecimalStyle;

	return [formatter numberFromString:string];
}

NSString * getRepoName(REPO_DATA * repo)
{
	if(isLocalRepo(repo))
		return NSLocalizedString(@"LOCAL-REPO", nil);

	return getStringForWchar(repo->name);
}

int compareStrings(const void* a, uint lengthA, const void* b, uint lengthB, int compareEncoding)
{
	NSString * stringA = compareEncoding == COMPARE_UTF8 ? [[NSString alloc] initWithBytes:a length:lengthA encoding:NSUTF8StringEncoding] : getStringForWchar(a);
	NSString * stringB = compareEncoding == COMPARE_UTF8 ? [[NSString alloc] initWithBytes:b length:lengthB encoding:NSUTF8StringEncoding] : getStringForWchar(b);
	
	return [stringA localizedCompare:stringB];
}

//The retina version pixelSize = 2 x size
void exportImageToPath(NSImage * image, NSSize size, NSSize pixelSize,  NSString * outputPath)
{
	NSRect pixelInRect = (NSRect) {{0, 0}, pixelSize};
	NSBitmapImageRep *workingRep = [[NSBitmapImageRep alloc] initWithCGImage:[image CGImageForProposedRect:&(pixelInRect) context:nil hints:nil]];

	//Resize the image if needed
	if(!NSEqualSizes(size, NSZeroSize) && !NSEqualSizes(NSMakeSize(workingRep.pixelsWide, workingRep.pixelsHigh), pixelSize))
	{
		//Create a representation of a given size in pixel
		workingRep = [[NSBitmapImageRep alloc]
								 initWithBitmapDataPlanes:NULL
								 pixelsWide:pixelSize.width
								 pixelsHigh:pixelSize.height
								 bitsPerSample:8
								 samplesPerPixel:4
								 hasAlpha:YES
								 isPlanar:NO
								 colorSpaceName:NSCalibratedRGBColorSpace
								 bytesPerRow:0
								 bitsPerPixel:0];
		//Set its size in point
		[workingRep setSize:size];

		//Draw the image in the representation
		[NSGraphicsContext saveGraphicsState];
		[NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep:workingRep]];
		[image drawInRect:(NSRect) {{0, 0}, size} fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
		[NSGraphicsContext restoreGraphicsState];
	}

	//Export the image
	NSData * data = [workingRep representationUsingType:NSPNGFileType properties:nil];

	[data writeToFile:outputPath atomically:YES];
	if(!checkFileExist([outputPath UTF8String]))
	{
		//Create the path to the directory if needed
		createPath([[outputPath substringToIndex:([outputPath rangeOfString:@"/" options: NSBackwardsSearch].location + 1)] UTF8String]);
		[data writeToFile:outputPath atomically:YES];
	}
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
	if(_CFBundleFlushBundleCaches != NULL)
	{
		CFBundleRef cfBundle = CFBundleCreate(nil, (__bridge CFURLRef)[bundle bundleURL]);

		_CFBundleFlushBundleCaches(cfBundle);
		CFRelease(cfBundle);
	}
}