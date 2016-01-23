/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

@interface RakCTRowCell()
{
	uint cacheID;
	uint selection;
	BOOL isTome;
	
	UILabel * mainText;
	UIButton * buyButton;
	UIImageView * unread, * startDL, * justDL;
	RakDownloadStatus * DLStatus;
	
	UIColor * buttonBlue;
	
	NSString * priceString;
	BOOL tapRegistered, MDLUpdateNotified;
}

@end

@implementation RakCTRowCell

- (void) awakeFromNib
{
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(percentageUpdateNotification:) name:NOTIFICATION_MDL_PERCENTAGE_UPDATE object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(MDLStatusChangedNotification:) name:NOTIFICATION_MDL_STATUS_UPDATE object:nil];
}

- (void) dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void) setSelected : (BOOL)selected animated:(BOOL)animated
{
	if(_isReadable)
		[super setSelected:selected animated:animated];
}

- (void) setFrame:(CGRect)frame
{
	CGSize oldSize = self.frame.size;
	[super setFrame:frame];
	
	if(!CGSizeEqualToSize(oldSize, frame.size))
		[self updateOrigins];
}

- (void) setBounds:(CGRect)bounds
{
	CGSize oldSize = self.bounds.size;
	[super setBounds:bounds];

	if(!CGSizeEqualToSize(oldSize, bounds.size))
		[self updateOrigins];
}

- (void) updateData : (PROJECT_DATA) project withSelection : (uint) posInList isTome : (BOOL) _isTome
{
	if(!project.isInitialized)
		return;
	
	cacheID = project.cacheDBID;
	isTome = _isTome;

	NSString * string;
	uint price;
	
	if(isTome)
	{
		META_TOME metadata = project.volumesFull[posInList];
		selection = metadata.ID;
		string = getStringForVolumeFull(metadata);
		price = metadata.price;
	}
	else
	{
		selection = project.chaptersFull[posInList];
		string = getStringForChapter(selection);
		price = project.chaptersPrix == NULL ? 0 : project.chaptersPrix[posInList];
	}
	
	[self processDataUpdateForProject:project withName:string withPriceString:(price != 0 ? getStringForPrice(price) : nil)];
}

- (void) processDataUpdateForProject : (PROJECT_DATA) project withName : (NSString *) selectionName withPriceString : (NSString *) _priceString
{
	_isReadable = checkReadable(project, isTome, selection);
	
	BOOL DLStatusWasHidden = DLStatus.isHidden;
	
	//Slight overhead, but we reset all the view to hidden = YES
	if(DLStatus != nil)			DLStatus.hidden = YES;
	if(startDL != nil)			startDL.hidden = YES;
	if(unread != nil)			unread.hidden = YES;
	if(buyButton != nil)		buyButton.hidden = YES;
	if(justDL != nil)			justDL.hidden = YES;
	priceString = _priceString;
	
	//Unread bullet
	if(_isReadable && !checkAlreadyRead(project, isTome, selection))
	{
		if(unread == nil)
		{
			unread = [[UIImageView alloc] initWithImage: [UIImage imageNamed:@"unread"]];
			if(unread != nil)
				[self.contentView addSubview:unread];
		}
		else
			unread.hidden = NO;
	}
	
	//Main text
	if(mainText != nil)
	{
		mainText.text = selectionName;
		[mainText sizeToFit];
	}
	else
	{
		mainText = [[UILabel alloc] init];
		
		mainText.text = selectionName;
		[mainText sizeToFit];
		
		[self.contentView addSubview:mainText];
	}
	
	//Item at the extreme right
	BOOL wentInMDL = checkIfElementAlreadyInMDL(project, isTome, selection);
	if(!_isReadable)
	{
		self.accessoryType = UITableViewCellAccessoryNone;
		
		//If the item is being processed in the MDL, we have to show the proper image
		if(wentInMDL)
		{
			if(DLStatus == nil)
			{
				DLStatus = [[RakDownloadStatus alloc] init];
				if(DLStatus != nil)
					[self.contentView addSubview:DLStatus];
			}
			else
			{
				DLStatus.hidden = NO;
				
				if(!MDLUpdateNotified || DLStatusWasHidden)
					[DLStatus setPercentage:0.0];
				else
					[DLStatus setPercentage:100.0];
			}

		}
		//Then, if the item could be bought
		else if(_priceString != nil)
		{
			if(buyButton == nil)
			{
				buyButton = [UIButton buttonWithType:UIButtonTypeSystem];
				if(buyButton != nil)
				{
					[buyButton addTarget:self action:@selector(tapRegistered) forControlEvents:UIControlEventTouchUpInside];
					[buyButton addTarget:self action:@selector(updateBackgroundColor) forControlEvents:UIControlEventValueChanged];
					[buyButton addTarget:self action:@selector(discardTap) forControlEvents:UIControlEventTouchUpOutside | UIControlEventTouchCancel];
					
					buttonBlue = [[buyButton titleColorForState:UIControlStateNormal] copy];
					
					buyButton.layer.borderWidth = 1;
					buyButton.layer.cornerRadius = 5;
					buyButton.layer.borderColor = buttonBlue.CGColor;

					[self.contentView addSubview:buyButton];
					
					[buyButton setTitle:priceString forState:UIControlStateNormal];
					[buyButton sizeToFit];
					[self updateButtonFrame];
				}
			}
			else
			{
				buyButton.hidden = NO;
				[self discardTap];
			}
		}
		//Otherwise, it is free, easy scenario
		else
		{
			if(startDL == nil)
			{
				startDL = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"availableToDL"]];
				if(startDL != nil)
					[self.contentView addSubview:startDL];
			}
			else
				startDL.hidden = NO;
		}
	}
	else
	{
		self.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
		
		if(wentInMDL)
		{
			if(justDL == nil)
			{
				justDL = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"valid"]];
				if(justDL != nil)
					[self.contentView addSubview:justDL];
			}
			else
				justDL.hidden = NO;
		}
	}
	
	[self updateOrigins];
}

- (void) updateOrigins
{
	CGSize cellSize = self.bounds.size, statusSize;
	CGFloat baseX = 25;

	if(unread != nil && !unread.isHidden)
	{
		[unread setFrameOrigin:CGPointMake(CELL_IMAGE_OFFSET, cellSize.height / 2 - CELL_BULLET_WIDTH / 2)];
		baseX += CELL_IMAGE_OFFSET;
	}
	
	[mainText setFrameOrigin:CGPointMake(baseX, cellSize.height / 2 - mainText.bounds.size.height / 2)];

	if(DLStatus != nil && !DLStatus.isHidden)
	{
		statusSize = DLStatus.bounds.size;
		[DLStatus setFrameOrigin:CGPointMake(cellSize.width - statusSize.width - 20, cellSize.height / 2 - statusSize.height / 2)];
	}
	
	if(buyButton != nil && !buyButton.isHidden)
	{
		[self updateButtonOrigin];
	}
	
	if(startDL != nil && !startDL.isHidden)
	{
		statusSize = startDL.bounds.size;
		[startDL setFrameOrigin:CGPointMake(cellSize.width - statusSize.width - 20, cellSize.height / 2 - statusSize.height / 2)];
	}

	if(justDL != nil && !justDL.isHidden)
	{
		statusSize = justDL.bounds.size;
		[justDL setFrameOrigin:CGPointMake(cellSize.width - statusSize.width - 30, cellSize.height / 2 - statusSize.height / 2)];
	}
}

#pragma mark - Buttons

- (void) tapRegistered
{
	if(tapRegistered)
	{
		addElementWithIDToMDL(cacheID, isTome, selection, false);
		[self discardTap];
	}
	else
	{
		tapRegistered = YES;
		[self updateButtonWithText:@"MONEY!" color:[UIColor colorWithRed:68/255.0 green:219/255.0 blue:94/255.0 alpha:1]];
	}
}

- (void) updateBackgroundColor
{
	[buyButton setBackgroundColor:[UIColor whiteColor]];
}

- (void) discardTap
{
	tapRegistered = NO;
	[self updateButtonWithText:priceString color:buttonBlue];
}

- (void) updateButtonWithText : (NSString *) text color : (UIColor *) color
{
	[CATransaction begin];
	[CATransaction setAnimationDuration:0.3];
	
	[buyButton setTitle:text forState:UIControlStateNormal];
	[buyButton setTitleColor:color forState:UIControlStateNormal];
	buyButton.layer.borderColor = color.CGColor;
	[buyButton sizeToFit];
	
	[CATransaction setCompletionBlock:^{
		[self updateButtonFrame];
		[self updateButtonOrigin];
	}];
	
	[CATransaction commit];
}

- (void) updateButtonFrame
{
	CGSize buttonSize = buyButton.bounds.size;
	[buyButton setFrame:CGRectMake(0, 0, buttonSize.width + 20, buttonSize.height)];
}

- (void) updateButtonOrigin
{
	CGSize cellSize = self.frame.size, statusSize = buyButton.bounds.size;
	[buyButton setFrameOrigin:CGPointMake(cellSize.width - statusSize.width - 20, cellSize.height / 2 - statusSize.height / 2)];
}

#pragma mark - Notification

- (void) MDLStatusChangedNotification : (NSNotification *) notification
{
	if ([notification.object isEqualToString:[NSString stringWithFormat:@"%d-%d-%d", cacheID, isTome, selection]])
	{
		if(![NSThread isMainThread])
		{
			dispatch_async(dispatch_get_main_queue(), ^{
				[self MDLStatusChangedNotification:notification];
			});
		}
		else
		{
			PROJECT_DATA project = getProjectByID(cacheID);
			
			MDLUpdateNotified = YES;
			[self processDataUpdateForProject:project withName:mainText.text withPriceString:priceString];
			MDLUpdateNotified = NO;
			
			releaseCTData(project);
		}
	}
}

- (void) percentageUpdateNotification : (NSNotification *) notification
{
	if ([notification.object isEqualToString:[NSString stringWithFormat:@"%d-%d-%d", cacheID, isTome, selection]])
	{
		if(![NSThread isMainThread])
		{
			dispatch_async(dispatch_get_main_queue(), ^{
				[self percentageUpdateNotification:notification];
			});
		}
		else
		{
			[DLStatus setPercentage:[[notification.userInfo objectForKey:@"percentage"] doubleValue]];
		}
	}
}

- (BOOL) canSelect
{
	return _isReadable || (buyButton == nil || buyButton.hidden);
}

@end
