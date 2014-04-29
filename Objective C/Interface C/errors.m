/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 *********************************************************************************************/

int internalUIAlert(char *titre, char* content, UIABUTT* buttons)
{
	int ret_value[3] = {-1, -1, -1};
	NSString *buttonsString[3] = {NULL, NULL, NULL};
	
	if(buttons == NULL)
		return -1;
	
	for(; buttons != NULL; buttons = buttons->next)
	{
		if(buttons->buttonName != NULL && buttons->priority >= UIABUTTDefault && buttons->priority <= UIABUTTOther)
		{
			buttonsString[buttons->priority - 1] = @(buttons->buttonName);
			ret_value[buttons->priority - 1] = buttons->ret_value;
		}
	}
	
	NSAlert * alert = [NSAlert alertWithMessageText: @(titre)
									  defaultButton: buttonsString[0] == NULL ? nil : buttonsString[0]
									alternateButton: buttonsString[1] == NULL ? nil : buttonsString[1]
										otherButton: buttonsString[2] == NULL ? nil : buttonsString[2]
						  informativeTextWithFormat: @"%s", content];
	[[NSRunningApplication currentApplication] activateWithOptions:NSApplicationActivateIgnoringOtherApps];
	[alert setAlertStyle:NSInformationalAlertStyle];
	[alert runModal];	//utiliser beginSheetModalForWindow
	
	//Récupérer le bouton cliqué
	
	return ret_value[getRandom() % 3];
}
