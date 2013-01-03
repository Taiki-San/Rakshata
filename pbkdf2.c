/*********************************************************************************************
**      __________         __           .__            __                ____     ____      **
**      \______   \_____  |  | __  _____|  |__ _____ _/  |______    /\  /_   |   /_   |     **
**       |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \/   |   |    |   |     **
**       |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_ /\   |   |    |   |     **
**       |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  / \/   |___| /\ |___|     **
**              \/      \/     \/     \/     \/     \/          \/             \/           **
**                                                                                          **
**   Licence propriétaire, code source confidentiel, distribution formellement interdite    **
**                                                                                          **
*********************************************************************************************/

#define ENOMEM 1
#define EINVAL 2

#define MAX(a,b)    ((a>b)?a:b)

#include "main.h"

static void F(uint32_t prf_hlen, const uint8_t *pw, uint32_t pwlen, const uint8_t *salt, uint32_t saltlen, uint32_t count, uint32_t i, uint8_t *buffer, uint8_t *u)
{
    uint8_t     ival[4];
    uint32_t    j,k;

    ival[0] = (i >> 24) & 0xff;
    ival[1] = (i >> 16) & 0xff;
    ival[2] = (i >> 8) & 0xff;
    ival[3] = i & 0xff;

    memcpy(buffer,salt,saltlen);
    memcpy(buffer+saltlen,ival,4);
    sha256_salted(pw,pwlen,buffer,saltlen+4,buffer);

    memcpy(u,buffer,prf_hlen);

    for(j=2; j<=count; j++){
        sha256_salted(pw,pwlen,buffer,prf_hlen,buffer);
        for(k=0; k<prf_hlen; k++){
            u[k] ^= buffer[k];
        }
    }

    return;
}


int I2pbkdf2(uint32_t prf_hlen, const uint8_t *pw, uint32_t pwlen, const uint8_t *salt, uint32_t saltlen, uint32_t count, uint32_t dklen, uint8_t *dk_ret)
{
    int         rc = 0;
    uint32_t    l,r;
    uint32_t    i;
    uint8_t     *tmpbuff=NULL; /* Intermediate memspace for F */
    uint8_t     *outbuff=NULL; /* Results of F iteration */
    uint8_t     *out;

#if 0
    if((dklen / prf_hlen) > 0xffffffff){
        rc = EINVAL;
        goto end;
    }
#endif

    if( !(tmpbuff = calloc(MAX(saltlen,prf_hlen)+4,sizeof(uint8_t))) ||
            !(outbuff = calloc(prf_hlen,sizeof(uint8_t)))){
        rc = ENOMEM;
        goto end;
    }

    l = dklen / prf_hlen;
    r = dklen % prf_hlen;


    for(i=0; i<l; i++){
        out = dk_ret + (i * prf_hlen);
        F(prf_hlen,pw,pwlen,salt,saltlen,count,i+1,tmpbuff,out);
    }

    if(r){
        F(prf_hlen,pw,pwlen,salt,saltlen,count,l+1,tmpbuff,outbuff);
        out = dk_ret + (l * prf_hlen);
        memcpy(out,outbuff,r);
    }

end:
    if(tmpbuff) free(tmpbuff);
    if(outbuff) free(outbuff);

    if(rc){
        errno = rc;
        return -1;
    }

    return 0;
}

