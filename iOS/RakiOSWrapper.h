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

#define IOS_DIRTY_HACK

#define RakView		UIView
#define RakImage	UIImage
#define RakColor	UIColor

#define NSRect		CGRect
#define NSZeroRect	CGRectZero
#define NSMakeRect	CGRectMake

#define NSSize		CGSize
#define NSZeroSize	CGSizeZero
#define NSMakeSize	CGSizeMake

#define NSPoint		CGPoint
#define NSZeroPoint	CGPointZero
#define NSPointSize	CGPointMake

#define NSOffState		0
#define NSOnState		1
#define NSMixedState	2

#define RakRealApp	[UIApplication sharedApplication]
#define RakApp		[RakRealApp delegate]

#define RakOutlineList		NSObject
#define MDL					NSObject
#define RakMDLList			NSObject
#define RakTabForegroundView NSObject
#define RakMDLListView		NSObject

#import "RakOutlineListItem.h"
#import "RakMDLController.h"


@interface PDFDocument : NSObject
{
	CGPDFDocumentRef internalDocument;
}

- (instancetype) initWithData : (NSData *) data;

- (NSUInteger) pageCount;

@end

@interface PDFPage : NSObject

@end

@interface NSObject (Comparaison)

- (BOOL)isNotEqualTo:(id)object;

@end