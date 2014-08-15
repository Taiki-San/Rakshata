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

#pragma mark - Preference size utilities

NSRect prefsPercToFrame(NSRect percentage, NSSize superview)
{
	percentage.origin.x *= superview.width / 100.0f;
	percentage.origin.y *= superview.height / 100.0f;
	percentage.size.width *= superview.width / 100.0f;
	percentage.size.height *= superview.height / 100.0f;

	return percentage;
}

CGFloat percToSize(CGFloat percentage, CGFloat superview, CGFloat max)
{
	percentage *= superview / 100.0f;
	
	if(max != -1 && percentage > max)
		percentage = max;
	
	return percentage;
}