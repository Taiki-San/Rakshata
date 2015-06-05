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

#ifndef ENOMEM
	#define ENOMEM 1
#endif
#ifndef EINVAL
	#define EINVAL 2
#endif

#define MAX(a,b)    ((a>b)?a:b)

static void F(uint32_t prf_hlen, const uint8_t *pw, size_t pwlen, const uint8_t *salt, size_t saltlen, uint32_t count, size_t i, uint8_t *buffer, uint8_t *u)
{
    uint32_t k;
    memcpy(buffer, salt, saltlen);
    sha256_salted(pw,pwlen,buffer,saltlen,buffer);

    memcpy(u,buffer,prf_hlen);

    for(i=1; i<count; i++)
    {
        sha256_salted(pw,pwlen,buffer,prf_hlen,buffer);
        for(k=0; k<prf_hlen; k++)
            u[k] ^= buffer[k];
    }
}

int internal_pbkdf2(uint32_t prf_hlen, const uint8_t *input, size_t inputLength, const uint8_t *salt, size_t saltLength, uint32_t iteneration, size_t lengthOutput, uint8_t *output)
{
    size_t    l,r;
    uint32_t    i;
    uint8_t     *tmpbuff = NULL; /* Intermediate memspace for F */
    uint8_t     *outbuff = NULL; /* Results of F iteration */
    uint8_t     *out;

	tmpbuff = calloc(MAX(saltLength, prf_hlen) + 4, sizeof(uint8_t));
	outbuff = calloc(prf_hlen,sizeof(uint8_t));
	
    if(tmpbuff != NULL && outbuff != NULL)
    {
        l = lengthOutput / prf_hlen;
        r = lengthOutput % prf_hlen;


        for(i=0; i<l; i++)
		{
            out = output + (i * prf_hlen);
            F(prf_hlen, input, inputLength, salt, saltLength, iteneration, i+1, tmpbuff, out);
        }

        if(r){
            F(prf_hlen, input, inputLength, salt, saltLength, iteneration, l+1, tmpbuff, outbuff);
            out = output + (l * prf_hlen);
            memcpy(out, outbuff, r);
        }
    }

    free(tmpbuff);
    free(outbuff);
    return 0;
}

void pbkdf2(uint8_t input[], uint8_t salt[], uint8_t output[])
{
    uint32_t inputLength = 0, saltLength = 0;

    for(inputLength = 0; input[inputLength]; inputLength++);
    for(saltLength = 0; salt[saltLength]; saltLength++);

    internal_pbkdf2(SHA256_DIGEST_LENGTH, input, inputLength, salt, saltLength, 2048, PBKDF2_OUTPUT_LENGTH, output);
}
