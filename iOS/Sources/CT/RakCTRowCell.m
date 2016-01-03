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

@interface RakCTRowCell()
{
	uint cacheID;
	uint selection;
	BOOL isTome;
	
	UILabel * mainText;
	UIImageView * unread;
}

@end

@implementation RakCTRowCell

- (void) setSelected : (BOOL)selected animated:(BOOL)animated
{
    [super setSelected:selected animated:animated];

    // Configure the view for the selected state
}

- (void) updateData : (PROJECT_DATA) project withSelection : (uint) posInList isTome : (BOOL) _isTome
{
	if(!project.isInitialized)
		return;
	
	cacheID = project.cacheDBID;
	isTome = _isTome;

	NSString * string;
	CGSize cellSize = self.bounds.size;
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
	
	_isReadable = checkReadable(project, isTome, selection);
	
	CGFloat baseX = CELL_IMAGE_OFFSET;

	//Unread bullet
	if(_isReadable && checkAlreadyRead(project, isTome, selection))
	{
		if(unread == nil)
		{
			unread = [[UIImageView alloc] initWithImage: [UIImage imageNamed:@"unread"]];
			if(unread != nil)
			{
				[unread setFrameOrigin:CGPointMake(CELL_BULLET_OFFSET, cellSize.height / 2 - CELL_BULLET_WIDTH / 2)];
				[self.contentView addSubview:unread];
			}
		}
		else
			unread.hidden = NO;
		
		baseX += CELL_BULLET_FULL_WIDTH;
	}
	else if(unread != nil)
		unread.hidden = YES;
	
	//Main text
	if(mainText != nil)
	{
		mainText.text = string;
		[mainText sizeToFit];
	}
	else
	{
		mainText = [[UILabel alloc] init];
		
		mainText.text = string;
		[mainText sizeToFit];
		
		[mainText setFrameOrigin:CGPointMake(baseX, cellSize.height / 2 - mainText.bounds.size.height / 2)];
		[self.contentView addSubview:mainText];
	}
	
	//Item at the extreme right
	if(!_isReadable)
	{
		self.accessoryType = UITableViewCellAccessoryNone;
		
		//If the item is being processed in the MDL, we have to show the proper image
		if(checkIfElementAlreadyInMDL(project, isTome, selection))
		{
			
		}
		//Then, if the item could be bought
		else if(price != 0)
		{
			
		}
		//Otherwise, it is free, easy scenario
		else
		{
			
		}
	}
	else
		self.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
}

@end
