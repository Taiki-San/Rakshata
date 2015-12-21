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

@implementation PDFDocument

- (instancetype) initWithData:(NSData *)data
{
	if(data == nil)
		return nil;
	
	self = [self init];
	if(self != nil)
	{
		CFDataRef myPDFData = (CFDataRef) data;
		CGDataProviderRef provider = CGDataProviderCreateWithCFData(myPDFData);
		if(provider != nil)
		{
			internalDocument = CGPDFDocumentCreateWithProvider(provider);
			CFRelease(provider);
		}
	}

	return self;
}

- (NSUInteger) pageCount
{
	return internalDocument == NULL ? 0 : CGPDFDocumentGetNumberOfPages(internalDocument);
}

- (void) dealloc
{
	CGPDFDocumentRelease(internalDocument);
}

@end

@implementation PDFPage
@end

@implementation NSObject(Comparaison)

- (BOOL)isNotEqualTo:(id)object
{
	return object != nil && ![self isEqualTo : object];
}

@end