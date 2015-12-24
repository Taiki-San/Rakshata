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
#define NSEqualRects	CGRectEqualToRect

#define NSSize		CGSize
#define NSZeroSize	CGSizeZero
#define NSMakeSize	CGSizeMake

#define NSPoint		CGPoint
#define NSZeroPoint	CGPointZero
#define NSPointSize	CGPointMake
#define NSPointInRect(a, b) CGRectContainsPoint(b, a)

#define NSOffState		0
#define NSOnState		1
#define NSMixedState	2

#define RakApplication	UIApplication
#define RakRealApp	[RakApplication sharedApplication]
#define RakApp		((RakAppiOSDelegate *) [RakRealApp delegate])

#define RakOutlineList		NSObject
#define RakMDLList			NSObject
#define RakTabForegroundView NSObject
#define RakMDLListView		NSObject

#import "RakOutlineListItem.h"

@interface Series : NSObject
@property BOOL forceNextFrameUpdate;
- (void) removeFromSuperview;
- (NSString *) byebye;
@end

@interface CTSelec : NSObject
@property BOOL forceNextFrameUpdate;
- (void) removeFromSuperview;
- (NSString *) byebye;
- (void) ownFocus;

@end

@interface Reader : NSObject
- (void) removeFromSuperview;
- (NSString *) byebye;
@end

@interface PDFPage : NSObject
{
	CGPDFPageRef _page;
}

- (instancetype) initWithPage : (CGPDFPageRef) page;

@end

@interface PDFDocument : NSObject
{
	CGPDFDocumentRef internalDocument;
}

- (instancetype) initWithData : (NSData *) data;

- (NSUInteger) pageCount;

- (PDFPage *) pageAtIndex : (NSUInteger) page;

@end

@interface NSObject (Comparaison)

- (BOOL)isNotEqualTo:(id)object;

@end

@interface NSBundle (MacCompatibility)

- (RakImage *)imageForResource:(NSString *)name;

@end

@interface RakColor (MacCompatibility)

+ (RakColor *) colorWithDeviceWhite:(CGFloat)white alpha:(CGFloat)alpha;
+ (RakColor *) colorWithSRGBRed:(CGFloat)red green:(CGFloat)green blue:(CGFloat)blue alpha:(CGFloat)alpha;

@end

@interface RakView (Redirection)

- (void) setFrameOrigin : (NSPoint) origin;
- (void) setNeedsDisplay : (BOOL) value;

@end

enum
{
	NSModalResponseStop                 = (-1000), // Also used as the default response for sheets
	NSModalResponseAbort                = (-1001),
	NSModalResponseContinue             = (-1002),
};
