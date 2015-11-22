/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**		Source code and assets are property of Taiki, distribution is stricly forbidden		**
**                                                                                          **
*********************************************************************************************/

void checkIfCharToEscapeFromPOST(char * input, uint length, char * output)
{
	//Only forbiden caracter so far
	uint posInput, posOutput;
	
	for(posInput = posOutput = 0; posInput < length; posInput++)
	{
		if(input[posInput] == '&')
		{
			output[posOutput++] = '%';
			output[posOutput++] = '2';
			output[posOutput++] = '6';
		}
		else
			output[posOutput++] = input[posInput];
	}
}

IMG_DATA* readFile(char * path)
{
	if(path == NULL)
		return NULL;
	
	IMG_DATA *output = calloc(1, sizeof(IMG_DATA));
	if(output != NULL)
	{
		FILE* input = fopen(path, "r");
		size_t length = getFileSize(path);

		if(input != NULL && length)
		{
			output->data = calloc(length, sizeof(char));

			if(output->data != NULL)
			{
				rewind(input);
				output->length = fread(output->data, sizeof(char), length, input);
			}
			else
			{
				free(output);
				output = NULL;
			}
			
			fclose(input);
		}
		else
		{
			free(output);
			output = NULL;
			
			if(input != NULL)
				fclose(input);
		}
	}
	return output;
}

inline bool isDownloadValid(char *input)
{
	if(input == NULL)
		return false;
	
    byte i = 0;
    for(; input[i] && (input[i] <= ' ' || input[i] > '~') && i < 10; i++);
    return (i < 10 && input[i] != '<' && input[i]);
}

bool haveSuffixCaseInsensitive(const char *input, const char * stringToFind)
{
	if(input == NULL || *input == 0 || stringToFind == NULL || *stringToFind == 0)
		return false;
	
	char c;
	uint cursor;

	//The *input != 0 is performed at the end to prevent an extra test in the case two string start the same way
	//We already checked *stringToFind != 0
	while(1)
	{
		//Start with the same letter
		if((c = *input) == *stringToFind)
		{
			//Ignoring the first letter, are the two string identical from then on?
			for(cursor = 1; stringToFind[cursor] && tolower(input[cursor]) == stringToFind[cursor]; ++cursor);
			
			//We are leaving either because the character are not equal (so this is false)
			//	either because we are at the end of stringToFind, and thus this equality indicate
			//	if we are at the end of input, and thus if this was a suffix
			if(input[cursor] == stringToFind[cursor])
				return true;
		}
		//Reached the end
		else if(c == 0)
			return false;

		++input;
	}
}
