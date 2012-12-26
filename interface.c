/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

#include "main.h"

/** Ce fichier contient l'interface entre les admins et les utilisateurs**/

void updateSectionMessage(char messageVersion[5])
{
    if(NETWORK_ACCESS != CONNEXION_OK)
        return;
    int i = 0, j = 0;
    char URL[200], bufferDL[500], bufferMessage[500];

    sprintf(URL, "http://rsp.%s/message.php?OS=%s&version=%d&messageVersion=%s", MAIN_SERVER_URL[0], BUILD, CURRENTVERSION, messageVersion);
    setupBufferDL(bufferDL, 5, 10, 10, 1);

    download(URL, bufferDL, 0);

    for(i = 0; bufferDL[i] == messageVersion[i] && i < 5 && bufferDL[i] != ' ' && bufferDL[i]; i++);

    if(*messageVersion != '0' && bufferDL[i] != ' ' && i > 0)
        return;

    for(i = j = 0; i < 500 && bufferDL[i] != 0; bufferMessage[j++] = bufferDL[i++])
    {
        if(bufferDL[i] == '\r') //I assume the first char isn't a \r
        {
            i++;
            continue;
        }
    }
    bufferMessage[j] = 0;

    if(bufferMessage[0] != '<' && bufferMessage[1] != '<' && bufferMessage[2] != '<')
        AESEncrypt(MESSAGE_PASSWORD, bufferMessage, "data/section.msg", INPUT_IN_MEMORY);

    else
    {
        FILE *file = fopenR("data/section.msg", "w+");
        fclose(file);
    }
}

void checkSectionMessageUpdate()
{
    FILE *checkFile = fopenR("data/section.msg", "r");

    if(checkFile == NULL)
    {
        updateSectionMessage("0");
        return;
    }

    fseek(checkFile, 0, SEEK_END);

    size_t sizeOfFile = ftell(checkFile);
    fclose(checkFile);

    if(sizeOfFile != 0 && sizeOfFile < 512)
    {
        char *bufferSection = malloc(sizeOfFile);
        if(bufferSection != NULL)
        {
            int i;
            char version[5] = {0, 0, 0, 0, 0};
            AESDecrypt(MESSAGE_PASSWORD, "data/section.msg", bufferSection, OUTPUT_IN_MEMORY);
            for(i = 0; i < 5 && bufferSection[i] != ' ' && bufferSection[i]; i++)
                version[i] = bufferSection[i];
            updateSectionMessage(version);
        }
        else
            logR("Not enough memory\n");
    }
    else
        updateSectionMessage("0");
}
