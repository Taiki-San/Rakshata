/*********************************************************************************************
**  __________         __           .__            __                  ____     ________    **
**  \______   \_____  |  | __  _____|  |__ _____ _/  |______    ___  _/_   |    \_____  \   **
**   |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \  \/ /|   |     /  ____/   **
**   |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   / |   |    /       \   **
**   |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /   \_/  |___| /\ \_______ \  **
**          \/      \/     \/     \/     \/     \/          \/               \/         \/  **
**                                                                                          **
**    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
**                                                                                          **
*********************************************************************************************/

#include "main.h"

/** Ce fichier contient l'interface entre les admins et les utilisateurs**/

void updateSectionMessage(char messageVersion[5])
{
    if(!checkNetworkState(CONNEXION_OK))
        return;

    int i = 0, j = 0;
    char URL[200], bufferDL[500], bufferMessage[550];

    snprintf(URL, 200, "https://%s/message.php?OS=%s&version=%d&messageVersion=%s", MAIN_SERVER_URL[0], BUILD, CURRENTVERSION, messageVersion);

    crashTemp(bufferDL, 500);
    download_mem(URL, NULL, bufferDL, 500, 1);

    for(i = 0; bufferDL[i] == messageVersion[i] && i < 5 && bufferDL[i] != ' ' && bufferDL[i]; i++);

    if(!bufferDL[i] || (*messageVersion != '0' && bufferDL[i] == ' ' && i > 0))
        return;

    snprintf(bufferMessage, 550, "<%c>\n", SETTINGS_MESSAGE_SECTION_FLAG);
    for(j = strlen(bufferMessage), i = 0; i < 520 && bufferDL[i] != 0; bufferMessage[j++] = bufferDL[i++])
    {
        if(bufferDL[i] == '\r') //I assume the first char isn't a \r
        {
            i++;
            continue;
        }
    }
    bufferMessage[j++] = '\n';
    bufferMessage[j++] = '<';
    bufferMessage[j++] = '/';
    bufferMessage[j++] = SETTINGS_MESSAGE_SECTION_FLAG;
    bufferMessage[j++] = '>';
    bufferMessage[j++] = '\n';
    bufferMessage[j] = 0;

    if(bufferMessage[6] != '<' && bufferMessage[7] != '<' && bufferMessage[8] != '<')
    {
        if(*messageVersion == '0')
            addToPref(SETTINGS_MESSAGE_SECTION_FLAG, bufferMessage);
        else
            updatePrefs(SETTINGS_MESSAGE_SECTION_FLAG, bufferMessage);
    }
    else
        removeFromPref(SETTINGS_MESSAGE_SECTION_FLAG);
}

void checkSectionMessageUpdate()
{
    char *message = NULL;
    if((message = loadLargePrefs(SETTINGS_MESSAGE_SECTION_FLAG)) == NULL)
    {
        updateSectionMessage("0");
        return;
    }

    size_t sizeOfFile = strlen(message);
    if(sizeOfFile != 0 && sizeOfFile < 512)
    {
        int i;
        char version[5] = {0, 0, 0, 0, 0};
        for(i = 0; i < 5 && message[i] != ' ' && message[i]; i++)
            version[i] = message[i];
        updateSectionMessage(version);
    }
    else
        updateSectionMessage("0");
}
