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

#ifndef uint32_t
	#include <stdint.h>
#endif

#define KEYLENGTH(keybits) ((keybits)/8)
#define RKLENGTH(keybits)  ((keybits)/8+28)
#define NROUNDS(keybits)   ((keybits)/32+6)

#define KEYBITS 256
#define CRYPTO_BUFFER_SIZE 16

#ifndef _WIN32
    typedef uint32_t DWORD;
#endif
typedef uint32_t u4byte;
typedef unsigned char BYTE;

/*****************************************************
**                                                  **
**                      Rijndael                    **
**                                                  **
*****************************************************/

typedef uint32_t RK_KEY;

int rijndaelSetupEncrypt(RK_KEY *rk, const unsigned char *key, int keybits);
int rijndaelSetupDecrypt(RK_KEY *rk, const unsigned char *key, int keybits);
void rijndaelEncrypt(const RK_KEY *rk, int nrounds, const rawData plaintext[16], rawData ciphertext[16]);
void rijndaelDecrypt(const RK_KEY *rk, int nrounds, const rawData ciphertext[16], rawData plaintext[16]);

/*****************************************************
**                                                  **
**                      Serpent                     **
**                                                  **
*****************************************************/

typedef struct {
	uint32_t	key[140];
} SERPENT_STATIC_DATA;

void Serpent_set_key(SERPENT_STATIC_DATA *l_key,const uint32_t *in_key, const uint32_t key_len);
void Serpent_encrypt(SERPENT_STATIC_DATA *l_key,const uint32_t *in_blk, uint32_t *out_blk);
void Serpent_decrypt(SERPENT_STATIC_DATA *l_key,const uint32_t *in_blk, uint32_t *out_blk);


/*****************************************************
**                                                  **
**                      Twofish                     **
**                                                  **
*****************************************************/

typedef struct
{
	u4byte l_key[40];
	u4byte s_key[4];
	u4byte mk_tab[4 * 256];
	u4byte k_len;

} TwofishInstance;

//Truecrypt don't use the return value, so I guess it's fine to discard it
u4byte * TwofishSetKey(TwofishInstance *instance, const u4byte in_key[], const u4byte key_len);
void TwofishEncrypt(TwofishInstance *instance, const u4byte in_blk[4], u4byte out_blk[]);
void TwofishDecrypt(TwofishInstance *instance, const u4byte in_blk[4], u4byte out_blk[4]);


/*****************************************************
**                                                  **
**                      PBKDF2                      **
**                                                  **
*****************************************************/

int internal_pbkdf2(uint32_t prf_hlen, const uint8_t *input, size_t inputLength, const uint8_t *salt, size_t saltLength, uint32_t iteneration, size_t lengthOutput, uint8_t *output);
void pbkdf2(uint8_t input[], uint8_t salt[], uint8_t output[]);


/*****************************************************
**                                                  **
**                      SHA-256                     **
**                                                  **
*****************************************************/

int sha256(unsigned char* input, void* output);
int sha256_legacy(char input[], char output[2*SHA256_DIGEST_LENGTH+1]);
void sha256_salted(const uint8_t *input, size_t inputLen, const uint8_t *salt, size_t saltlen, uint8_t *output);


/*****************************************************
**                                                  **
**                    Whirlpool                     **
**                                                  **
*****************************************************/

void whirlpool(const unsigned char *input, size_t inputLength, char *output, bool outHex);
