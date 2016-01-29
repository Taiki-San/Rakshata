/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

void logR(const char *error)
{
#ifdef __APPLE__
	sendToLog(error);
#else
    FILE* logFile = fopen("log", "a+");
    if(logFile != NULL)
    {
        if(error != NULL)
        {
            fputs(error, logFile);
            if(error[strlen(error)-1] != '\n')
                fputc('\n', logFile);
        }
        fclose(logFile);
    }
#endif
}

int libcurlErrorCode(CURLcode code)
{
    bool noLog = false;
    int ret_value = CODE_RETOUR_DL_CLOSE;
    char log_message[100];
    switch(code)
    {
        case CURLE_FAILED_INIT :
        {
            strncpy(log_message, "Initialization failed", 100);
            break;
        }

        case CURLE_URL_MALFORMAT:
        {
            strncpy(log_message, "URL is malformated", 100);
            break;
        }
        case CURLE_COULDNT_RESOLVE_PROXY:
        {
            strncpy(log_message, "Failed at resolve the proxy", 100);
            break;
        }
        case CURLE_COULDNT_RESOLVE_HOST:
        case CURLE_COULDNT_CONNECT:
        {
            ret_value = CODE_FAILED_AT_RESOLVE;
            noLog = true;
            break;
        }
        case CURLE_PARTIAL_FILE:
        {
            strncpy(log_message, "Partial file", 100);
            ret_value = CODE_RETOUR_PARTIAL;
            break;
        }
        case CURLE_OUT_OF_MEMORY:
        {
			strncpy(log_message, "Everything is screwed up...", 100);
            ret_value = CODE_RETOUR_INTERNAL_FAIL;
            break;
        }
        case CURLE_OPERATION_TIMEDOUT:
        {
            strncpy(log_message, "Request timed out", 100);
            ret_value = CODE_RETOUR_INTERNAL_FAIL;
            break;
        }
        case CURLE_ABORTED_BY_CALLBACK:
		case CURLE_WRITE_ERROR:
        {
            ret_value = CODE_RETOUR_DL_CLOSE;
            noLog = true;
            break;
        }
		case CURLE_SSL_CONNECT_ERROR:
        case CURLE_SSL_CACERT_BADFILE:
        {
            strncpy(log_message, "SSL error", 100);
            ret_value = CODE_RETOUR_INTERNAL_FAIL;
            break;
        }
        case CURLE_SSL_CACERT:
        {
            noLog = true;
            break;
        }

        default:
        {
            snprintf(log_message, 100, "Unknown libcURL error: %d", code);
            break;
        }
    }
    if(!noLog)
        logR(log_message);
    return ret_value;
}

void memoryError(size_t size)
{
    char temp[0x100];
#ifndef __APPLE__
    snprintf(temp, 0x100, "Failed at allocate memory for : %d bytes\n", size);
#else
    snprintf(temp, 0x100, "Failed at allocate memory for : %ld bytes\n", size);
#endif
    logR(temp);
}

