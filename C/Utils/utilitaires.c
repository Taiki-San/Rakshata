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
