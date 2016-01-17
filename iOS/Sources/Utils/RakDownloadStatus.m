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

@interface RakDownloadStatus()
{
	double percentage;
	UIColor * borderColor;
}

@end

@implementation RakDownloadStatus

- (instancetype) init
{
	self = [super init];
	if(self != nil)
	{
		[self setImage:[UIImage imageNamed:@"dl_status"] forState:UIControlStateNormal];
		[self sizeToFit];
		
		borderColor = [Prefs getSystemColor:COLOR_PROGRESSLINE_PROGRESS];
		[Prefs registerForChange:self forType:KVO_THEME];
	}
	
	return self;
}

- (void) setPercentage : (double) _percentage
{
	percentage = _percentage;
	[self setNeedsDisplay];
}

#pragma mark - Drawing

- (void) setupPath
{
	CGContextRef ctx = UIGraphicsGetCurrentContext();
	const CGSize size = self.frame.size;
	const CGFloat x = size.width / 2, radius = x - 2, y = size.height / 2;
	
	// Progress Arc:
	CGContextBeginPath(ctx);
	CGContextAddArc(ctx, x, y, radius, -M_PI_2, -M_PI_2 + 2 * M_PI * percentage / 100, 0);
	CGContextSetLineWidth(ctx, 3);
	CGContextSetStrokeColorWithColor(ctx, borderColor.CGColor);
	CGContextStrokePath(ctx);
}

- (void)drawRect:(CGRect)rect
{
	[super drawRect:rect];
	
	if(percentage != 0)
		[self setupPath];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] == [Prefs class])
	{
		borderColor = [Prefs getSystemColor:COLOR_PROGRESSLINE_PROGRESS];
		[self setNeedsDisplay];
	}
}

@end
