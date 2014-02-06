/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
**                                                                                          **
*********************************************************************************************/

#include "moduleDL.h"

extern volatile bool quit;
extern int pageCourante;
extern int nbElemTotal;
extern int **status;
extern int **statusCache;

/*Check externalized to readibility*/

bool MDLisClicOnAValidX(int x, bool twoColumns)
{
    if(x >= MDL_ICON_POS && x <= MDL_ICON_POS + MDL_ICON_SIZE) //Première colonne
        return true;
    if(twoColumns && x >= MDL_ESPACE_INTERCOLONNE + MDL_ICON_POS&& x <= MDL_ESPACE_INTERCOLONNE + MDL_ICON_POS + MDL_ICON_SIZE) //Seconde colonne
        return true;
    return false;
}

int MDLisClicOnAValidY(int y, int nombreElement)
{
    y -= MDL_HAUTEUR_DEBUT_CATALOGUE - (MDL_ICON_SIZE / 2 - MDL_LARGEUR_FONT / 2);

    if(y < 0) //Trop haut
        return -1;

    if(y % MDL_INTERLIGNE > MDL_ICON_SIZE) //Sous l'icone
        return -1;

    int ligne = y / MDL_INTERLIGNE;

    if(ligne >= MDL_NOMBRE_ELEMENT_COLONNE || ligne >= nombreElement) //Sous les icones
        return -1;

    return ligne;
}

void MDLDealWithClicsOnIcons(DATA_LOADED ***todoList, int ligne, bool isFirstNonDL, bool isLastNonDL)
{
    int valIcon = *statusCache[ligne]; //Caching
    char trad[SIZE_TRAD_ID_16][TRAD_LENGTH];
    loadTrad(trad, 16);

    switch(valIcon)
    {
        case MDL_CODE_DL:
        {
            char titre[2*TRAD_LENGTH+5], contenu[500];
            snprintf(titre, 2*TRAD_LENGTH+5, "%s %s", trad[6], trad[8]);
            snprintf(contenu, 500, "%s %s %s %s", trad[11], trad[(*todoList)[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne]->subFolder?13:12], trad[14], trad[16]);
            UI_Alert(titre, contenu);
            break;
        }
        case MDL_CODE_DL_OVER:
        {
            char titre[2*TRAD_LENGTH+5], contenu[500];
            snprintf(titre, 2*TRAD_LENGTH+5, "%s %s", trad[7], trad[9]);
            snprintf(contenu, 500, "%s %s %s %s", trad[10], trad[(*todoList)[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne]->subFolder?13:12], trad[15], trad[16]);
            UI_Alert(titre, contenu);
            break;
        }
        case MDL_CODE_INSTALL:
        {
            char titre[2*TRAD_LENGTH+5], contenu[500];
            snprintf(titre, 2*TRAD_LENGTH+5, "%s %s", trad[7], trad[8]);
            snprintf(contenu, 500, "%s %s %s %s", trad[10], trad[(*todoList)[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne]->subFolder?13:12], trad[14], trad[16]);
            UI_Alert(titre, contenu);
            break;
        }
        case MDL_CODE_DEFAULT:
        {
            void *buffer;
            int ret_value = 0, pos = pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne, i;

            if(*status[pos] == MDL_CODE_DEFAULT)
            {
                switch(ret_value)
                {
                    case 1: //premier
                    {
                        for(i = 0; i < nbElemTotal && *status[i] != MDL_CODE_DEFAULT; i++);
                        if(pos != i)
                        {
                            buffer = (*todoList)[pos];
                            memmove(&(*todoList)[i+1], &(*todoList)[i], (pos-i)*sizeof(DATA_LOADED*));
                            (*todoList)[i] = buffer;
                            buffer = status[pos];
                            memmove(&status[i+1], &status[i], (pos-i)*sizeof(int*));
                            status[i] = buffer;
                            buffer = statusCache[pos];
                            memmove(&statusCache[i+1], &statusCache[i], (pos-i)*sizeof(int*));
                            statusCache[i] = buffer;
                        }
                        break;
                    }
                    case 2: //+1
                    case 4: //-1
                    {
                        if(pos > 0 && *status[pos-1] == MDL_CODE_DEFAULT)
                        {
                            buffer = (*todoList)[pos+ret_value-3];
                            (*todoList)[pos+ret_value-3] = (*todoList)[pos];
                            (*todoList)[pos] = buffer;
                            buffer = status[pos+ret_value-3];
                            status[pos+ret_value-3] = status[pos];
                            status[pos] = buffer;
                            buffer = statusCache[pos+ret_value-3];
                            statusCache[pos+ret_value-3] = statusCache[pos];
                            statusCache[pos] = buffer;
                        }
                        break;
                    }
                    case 3: //delete
                    {
                        if(*status[pos] == MDL_CODE_DEFAULT)
                        {
                            free((*todoList)[pos]);
                            free(status[pos]);
                            free(statusCache[pos]);
                            if(pos < nbElemTotal)
                            {
                                memmove(&(*todoList)[pos], &(*todoList)[pos+1], (nbElemTotal-ligne-1)*sizeof(DATA_LOADED*));
                                memmove(&status[pos], &status[pos+1], (nbElemTotal-ligne-1)*sizeof(int*));
                                memmove(&statusCache[pos], &statusCache[pos+1], (nbElemTotal-ligne-1)*sizeof(int*));
                            }
                            nbElemTotal--;
                        }
                        break;
                    }
                    case 5: //dernier
                    {
                        for(i = nbElemTotal-1; i >= 0 && *status[i] != MDL_CODE_DEFAULT; i--);
                        if(pos != i)
                        {
                            buffer = (*todoList)[pos];
                            memmove(&(*todoList)[pos], &(*todoList)[pos+1], (i-pos)*sizeof(DATA_LOADED*));
                            (*todoList)[i] = buffer;
                            buffer = status[pos];
                            memmove(&status[pos], &status[pos+1], (i-pos)*sizeof(int*));
                            status[i] = buffer;
                            buffer = statusCache[pos];
                            memmove(&statusCache[pos], &statusCache[pos+1], (i-pos)*sizeof(int*));
                            statusCache[i] = buffer;
                        }
                        break;
                    }
                }
            }
            break;
        }
        case MDL_CODE_INSTALL_OVER:
        {
            //This is hacky: we will kill the other thread by sending an appropriate event
            //Then, we'll restart it after created a laststate.dat file
            //First, we'll ask for confirmation

            int ret_value = 0;
            char contenu[500];

            snprintf(contenu, 500, "%s %s %s %s %s", trad[1], (*todoList)[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne]->datas->mangaName, trad[2], (*todoList)[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne]->datas->team->teamLong, trad[3]);
            changeTo(contenu, '_', ' ');
            //trad[0] = titrem 4 et 5 boutons

            if(ret_value == 1)
            {
                //We got the confirmation \o/ let's kill the reader
                FILE* inject = fopenR("data/laststate.dat", "w+");
                if(inject != NULL)
                {
                    if((*todoList)[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne]->subFolder)
                        fprintf(inject, "%s T %d", (*todoList)[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne]->datas->mangaName, (*todoList)[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne]->partOfTome);
                    else
                        fprintf(inject, "%s C %d", (*todoList)[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne]->datas->mangaName, (*todoList)[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne]->chapitre);
                    fclose(inject);

                    inject = fopen("data/externalLaunch", "w+");
                    if(inject != NULL)
                        fclose(inject);

                    createNewThread(mainRakshata, NULL);
                }
                else
                {
                    logR("Failed at write data needed to inject the chapter to the reader");
                }
            }
            break;
        }
        case MDL_CODE_ERROR_DL:
        case MDL_CODE_ERROR_INSTALL:
        {
            char contenu[2*TRAD_LENGTH+2];
            snprintf(contenu, 0x400, "%s %s", trad[18], trad[valIcon == MDL_CODE_ERROR_DL ? 19: 20]);
            unescapeLineReturn(contenu);
            UI_Alert(trad[17], contenu);
            break;
        }
        case MDL_CODE_INTERNAL_ERROR:
        {
            unescapeLineReturn(trad[21]);
            UI_Alert(trad[17], trad[21]);
            break;
        }
        case MDL_CODE_WAITING_LOGIN:
        {
            unescapeLineReturn(trad[30]);
            UI_Alert(trad[29], trad[30]);
            break;
        }
        case MDL_CODE_WAITING_PAY:
        {
            unescapeLineReturn(trad[32]);
            UI_Alert(trad[31], trad[32]);
            break;
        }
        case MDL_CODE_UNPAID:
        {
            char buffer[2*TRAD_LENGTH];
            snprintf(buffer, 2*TRAD_LENGTH, trad[34], trad[(*todoList)[pageCourante * MDL_NOMBRE_ELEMENT_COLONNE + ligne]->subFolder?13:12]);
            UI_Alert(trad[33], buffer);
            break;
        }
    }
}

