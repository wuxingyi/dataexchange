#include "dataexchange.hpp"
#include "base58.hpp"
#include "utils.hpp"

using namespace std;

#include <stdint.h>
#include <string.h> // CBC mode, for memset

/*****************************************************************************/
/* Defines:                                                                  */
/*****************************************************************************/
// The number of columns comprising a state in AES. This is a constant in AES. Value=4
#define Nb 4

#define Nk 8
#define Nr 14

// jcallan@github points out that declaring Multiply as a function 
// reduces code size considerably with the Keil ARM compiler.
// See this link for more information: https://github.com/kokke/tiny-AES-C/pull/3
#ifndef MULTIPLY_AS_A_FUNCTION
  #define MULTIPLY_AS_A_FUNCTION 0
#endif

#define AES_BLOCKLEN 16 //Block length in bytes AES is 128b block only
#define AES_KEYLEN 32
#define AES_keyExpSize 240

struct AES_ctx
{
  uint8_t RoundKey[AES_keyExpSize];
  uint8_t Iv[AES_BLOCKLEN];
};




/*****************************************************************************/
/* Private variables:                                                        */
/*****************************************************************************/
// state - array holding the intermediate results during decryption.
typedef uint8_t state_t[4][4];



// The lookup-tables are marked const so they can be placed in read-only storage instead of RAM
// The numbers below can be computed dynamically trading ROM for RAM - 
// This can be useful in (embedded) bootloader applications, where ROM is often limited.
static const uint8_t sbox[256] = {
  //0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F
  0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
  0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
  0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
  0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
  0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
  0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
  0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
  0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
  0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
  0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
  0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
  0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
  0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
  0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
  0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
  0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 };

static const uint8_t rsbox[256] = {
  0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
  0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
  0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
  0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
  0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
  0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
  0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
  0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
  0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
  0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
  0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
  0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
  0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
  0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
  0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
  0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d };

// The round constant word array, Rcon[i], contains the values given by 
// x to the power (i-1) being powers of x (x is denoted as {02}) in the field GF(2^8)
static const uint8_t Rcon[11] = {
  0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36 };

/*
 * Jordan Goulder points out in PR #12 (https://github.com/kokke/tiny-AES-C/pull/12),
 * that you can remove most of the elements in the Rcon array, because they are unused.
 *
 * From Wikipedia's article on the Rijndael key schedule @ https://en.wikipedia.org/wiki/Rijndael_key_schedule#Rcon
 * 
 * "Only the first some of these constants are actually used – up to rcon[10] for AES-128 (as 11 round keys are needed), 
 *  up to rcon[8] for AES-192, up to rcon[7] for AES-256. rcon[0] is not used in AES algorithm."
 */


/*****************************************************************************/
/* Private functions:                                                        */
/*****************************************************************************/
/*
static uint8_t getSBoxValue(uint8_t num)
{
  return sbox[num];
}
*/
#define getSBoxValue(num) (sbox[(num)])
/*
static uint8_t getSBoxInvert(uint8_t num)
{
  return rsbox[num];
}
*/
#define getSBoxInvert(num) (rsbox[(num)])

// This function produces Nb(Nr+1) round keys. The round keys are used in each round to decrypt the states. 
static void KeyExpansion(uint8_t* RoundKey, const uint8_t* Key)
{
  unsigned i, j, k;
  uint8_t tempa[4]; // Used for the column/row operations
  
  // The first round key is the key itself.
  for (i = 0; i < Nk; ++i)
  {
    RoundKey[(i * 4) + 0] = Key[(i * 4) + 0];
    RoundKey[(i * 4) + 1] = Key[(i * 4) + 1];
    RoundKey[(i * 4) + 2] = Key[(i * 4) + 2];
    RoundKey[(i * 4) + 3] = Key[(i * 4) + 3];
  }

  // All other round keys are found from the previous round keys.
  for (i = Nk; i < Nb * (Nr + 1); ++i)
  {
    {
      k = (i - 1) * 4;
      tempa[0]=RoundKey[k + 0];
      tempa[1]=RoundKey[k + 1];
      tempa[2]=RoundKey[k + 2];
      tempa[3]=RoundKey[k + 3];

    }

    if (i % Nk == 0)
    {
      // This function shifts the 4 bytes in a word to the left once.
      // [a0,a1,a2,a3] becomes [a1,a2,a3,a0]

      // Function RotWord()
      {
        k = tempa[0];
        tempa[0] = tempa[1];
        tempa[1] = tempa[2];
        tempa[2] = tempa[3];
        tempa[3] = k;
      }

      // SubWord() is a function that takes a four-byte input word and 
      // applies the S-box to each of the four bytes to produce an output word.

      // Function Subword()
      {
        tempa[0] = getSBoxValue(tempa[0]);
        tempa[1] = getSBoxValue(tempa[1]);
        tempa[2] = getSBoxValue(tempa[2]);
        tempa[3] = getSBoxValue(tempa[3]);
      }

      tempa[0] = tempa[0] ^ Rcon[i/Nk];
    }
    if (i % Nk == 4)
    {
      // Function Subword()
      {
        tempa[0] = getSBoxValue(tempa[0]);
        tempa[1] = getSBoxValue(tempa[1]);
        tempa[2] = getSBoxValue(tempa[2]);
        tempa[3] = getSBoxValue(tempa[3]);
      }
    }
    j = i * 4; k=(i - Nk) * 4;
    RoundKey[j + 0] = RoundKey[k + 0] ^ tempa[0];
    RoundKey[j + 1] = RoundKey[k + 1] ^ tempa[1];
    RoundKey[j + 2] = RoundKey[k + 2] ^ tempa[2];
    RoundKey[j + 3] = RoundKey[k + 3] ^ tempa[3];
  }
}

void AES_init_ctx(struct AES_ctx* ctx, const uint8_t* key)
{
  KeyExpansion(ctx->RoundKey, key);
}
void initiv(struct AES_ctx* ctx, const uint8_t* key, const uint8_t* iv)
{
  KeyExpansion(ctx->RoundKey, key);
  memcpy (ctx->Iv, iv, AES_BLOCKLEN);
}
void setit(struct AES_ctx* ctx, const uint8_t* iv)
{
  memcpy (ctx->Iv, iv, AES_BLOCKLEN);
}

// This function adds the round key to state.
// The round key is added to the state by an XOR function.
static void AddRoundKey(uint8_t round,state_t* state,uint8_t* RoundKey)
{
  uint8_t i,j;
  for (i = 0; i < 4; ++i)
  {
    for (j = 0; j < 4; ++j)
    {
      (*state)[i][j] ^= RoundKey[(round * Nb * 4) + (i * Nb) + j];
    }
  }
}

// The SubBytes Function Substitutes the values in the
// state matrix with values in an S-box.
static void SubBytes(state_t* state)
{
  uint8_t i, j;
  for (i = 0; i < 4; ++i)
  {
    for (j = 0; j < 4; ++j)
    {
      (*state)[j][i] = getSBoxValue((*state)[j][i]);
    }
  }
}

// The ShiftRows() function shifts the rows in the state to the left.
// Each row is shifted with different offset.
// Offset = Row number. So the first row is not shifted.
static void ShiftRows(state_t* state)
{
  uint8_t temp;

  // Rotate first row 1 columns to left  
  temp           = (*state)[0][1];
  (*state)[0][1] = (*state)[1][1];
  (*state)[1][1] = (*state)[2][1];
  (*state)[2][1] = (*state)[3][1];
  (*state)[3][1] = temp;

  // Rotate second row 2 columns to left  
  temp           = (*state)[0][2];
  (*state)[0][2] = (*state)[2][2];
  (*state)[2][2] = temp;

  temp           = (*state)[1][2];
  (*state)[1][2] = (*state)[3][2];
  (*state)[3][2] = temp;

  // Rotate third row 3 columns to left
  temp           = (*state)[0][3];
  (*state)[0][3] = (*state)[3][3];
  (*state)[3][3] = (*state)[2][3];
  (*state)[2][3] = (*state)[1][3];
  (*state)[1][3] = temp;
}

static uint8_t xtime(uint8_t x)
{
  return ((x<<1) ^ (((x>>7) & 1) * 0x1b));
}

// MixColumns function mixes the columns of the state matrix
static void MixColumns(state_t* state)
{
  uint8_t i;
  uint8_t Tmp, Tm, t;
  for (i = 0; i < 4; ++i)
  {  
    t   = (*state)[i][0];
    Tmp = (*state)[i][0] ^ (*state)[i][1] ^ (*state)[i][2] ^ (*state)[i][3] ;
    Tm  = (*state)[i][0] ^ (*state)[i][1] ; Tm = xtime(Tm);  (*state)[i][0] ^= Tm ^ Tmp ;
    Tm  = (*state)[i][1] ^ (*state)[i][2] ; Tm = xtime(Tm);  (*state)[i][1] ^= Tm ^ Tmp ;
    Tm  = (*state)[i][2] ^ (*state)[i][3] ; Tm = xtime(Tm);  (*state)[i][2] ^= Tm ^ Tmp ;
    Tm  = (*state)[i][3] ^ t ;              Tm = xtime(Tm);  (*state)[i][3] ^= Tm ^ Tmp ;
  }
}

// Multiply is used to multiply numbers in the field GF(2^8)
// Note: The last call to xtime() is unneeded, but often ends up generating a smaller binary
//       The compiler seems to be able to vectorize the operation better this way.
//       See https://github.com/kokke/tiny-AES-c/pull/34
#if MULTIPLY_AS_A_FUNCTION
static uint8_t Multiply(uint8_t x, uint8_t y)
{
  return (((y & 1) * x) ^
       ((y>>1 & 1) * xtime(x)) ^
       ((y>>2 & 1) * xtime(xtime(x))) ^
       ((y>>3 & 1) * xtime(xtime(xtime(x)))) ^
       ((y>>4 & 1) * xtime(xtime(xtime(xtime(x)))))); /* this last call to xtime() can be omitted */
  }
#else
#define Multiply(x, y)                                \
      (  ((y & 1) * x) ^                              \
      ((y>>1 & 1) * xtime(x)) ^                       \
      ((y>>2 & 1) * xtime(xtime(x))) ^                \
      ((y>>3 & 1) * xtime(xtime(xtime(x)))) ^         \
      ((y>>4 & 1) * xtime(xtime(xtime(xtime(x))))))   \

#endif

// MixColumns function mixes the columns of the state matrix.
// The method used to multiply may be difficult to understand for the inexperienced.
// Please use the references to gain more information.
static void InvMixColumns(state_t* state)
{
  int i;
  uint8_t a, b, c, d;
  for (i = 0; i < 4; ++i)
  { 
    a = (*state)[i][0];
    b = (*state)[i][1];
    c = (*state)[i][2];
    d = (*state)[i][3];

    (*state)[i][0] = Multiply(a, 0x0e) ^ Multiply(b, 0x0b) ^ Multiply(c, 0x0d) ^ Multiply(d, 0x09);
    (*state)[i][1] = Multiply(a, 0x09) ^ Multiply(b, 0x0e) ^ Multiply(c, 0x0b) ^ Multiply(d, 0x0d);
    (*state)[i][2] = Multiply(a, 0x0d) ^ Multiply(b, 0x09) ^ Multiply(c, 0x0e) ^ Multiply(d, 0x0b);
    (*state)[i][3] = Multiply(a, 0x0b) ^ Multiply(b, 0x0d) ^ Multiply(c, 0x09) ^ Multiply(d, 0x0e);
  }
}


// The SubBytes Function Substitutes the values in the
// state matrix with values in an S-box.
static void InvSubBytes(state_t* state)
{
  uint8_t i, j;
  for (i = 0; i < 4; ++i)
  {
    for (j = 0; j < 4; ++j)
    {
      (*state)[j][i] = getSBoxInvert((*state)[j][i]);
    }
  }
}

static void InvShiftRows(state_t* state)
{
  uint8_t temp;

  // Rotate first row 1 columns to right  
  temp = (*state)[3][1];
  (*state)[3][1] = (*state)[2][1];
  (*state)[2][1] = (*state)[1][1];
  (*state)[1][1] = (*state)[0][1];
  (*state)[0][1] = temp;

  // Rotate second row 2 columns to right 
  temp = (*state)[0][2];
  (*state)[0][2] = (*state)[2][2];
  (*state)[2][2] = temp;

  temp = (*state)[1][2];
  (*state)[1][2] = (*state)[3][2];
  (*state)[3][2] = temp;

  // Rotate third row 3 columns to right
  temp = (*state)[0][3];
  (*state)[0][3] = (*state)[1][3];
  (*state)[1][3] = (*state)[2][3];
  (*state)[2][3] = (*state)[3][3];
  (*state)[3][3] = temp;
}


// Cipher is the main function that encrypts the PlainText.
static void Cipher(state_t* state, uint8_t* RoundKey)
{
  uint8_t round = 0;

  // Add the First round key to the state before starting the rounds.
  AddRoundKey(0, state, RoundKey); 
  
  // There will be Nr rounds.
  // The first Nr-1 rounds are identical.
  // These Nr-1 rounds are executed in the loop below.
  for (round = 1; round < Nr; ++round)
  {
    SubBytes(state);
    ShiftRows(state);
    MixColumns(state);
    AddRoundKey(round, state, RoundKey);
  }
  
  // The last round is given below.
  // The MixColumns function is not here in the last round.
  SubBytes(state);
  ShiftRows(state);
  AddRoundKey(Nr, state, RoundKey);
}

static void InvCipher(state_t* state,uint8_t* RoundKey)
{
  uint8_t round = 0;

  // Add the First round key to the state before starting the rounds.
  AddRoundKey(Nr, state, RoundKey); 

  // There will be Nr rounds.
  // The first Nr-1 rounds are identical.
  // These Nr-1 rounds are executed in the loop below.
  for (round = (Nr - 1); round > 0; --round)
  {
    InvShiftRows(state);
    InvSubBytes(state);
    AddRoundKey(round, state, RoundKey);
    InvMixColumns(state);
  }
  
  // The last round is given below.
  // The MixColumns function is not here in the last round.
  InvShiftRows(state);
  InvSubBytes(state);
  AddRoundKey(0, state, RoundKey);
}


static void XorWithIv(uint8_t* buf, uint8_t* Iv)
{
  uint8_t i;
  for (i = 0; i < AES_BLOCKLEN; ++i) // The block in AES is always 128bit no matter the key size
  {
    buf[i] ^= Iv[i];
  }
}

static void cbcencrypt(struct AES_ctx *ctx,uint8_t* buf, uint32_t length)
{
  uintptr_t i;
  uint8_t *Iv = ctx->Iv;
  for (i = 0; i < length; i += AES_BLOCKLEN)
  {
    XorWithIv(buf, Iv);
    Cipher((state_t*)buf, ctx->RoundKey);
    Iv = buf;
    buf += AES_BLOCKLEN;
    //printf("Step %d - %d", i/16, i);
  }
  /* store Iv in ctx for next call */
  memcpy(ctx->Iv, Iv, AES_BLOCKLEN);
}

static void cbcdecrypt(struct AES_ctx* ctx, uint8_t* buf,  uint32_t length)
{
  uintptr_t i;
  uint8_t storeNextIv[AES_BLOCKLEN];
  for (i = 0; i < length; i += AES_BLOCKLEN)
  {
    memcpy(storeNextIv, buf, AES_BLOCKLEN);
    InvCipher((state_t*)buf, ctx->RoundKey);
    XorWithIv(buf, ctx->Iv);
    memcpy(ctx->Iv, storeNextIv, AES_BLOCKLEN);
    buf += AES_BLOCKLEN;
  }
}



//remove an data market, can only be made by the market owner.
//we can only remove a suspended market with enough suspend time to make the inflight deals finished.
//removed market will not be removed from the market table because this table will not be to big.
//we kept the removed market alive because it can show some statistics about data trading.
void dataexchange::removemarket(account_name owner, uint64_t marketid){
    require_auth(owner);

    auto iter = _markets.find(marketid);
    eosio_assert(iter != _markets.end(), "market not have been created yet");
    eosio_assert(iter->mowner == owner , "have no permission to this market");
    eosio_assert(iter->issuspended == true, "only suspended market can be removed");
    eosio_assert(iter->isremoved != true, "this market has already been removed");
    eosio_assert(time_point_sec(now()) > iter->minremovaltime, 
                 "market should have enought suspend time before removal");

    eosio_assert(iter->mstats.inflightdeals_nr == 0, "market can't be removed because there are some inflight deals");

    marketordertable orders(_self, owner);
    auto orderiter = orders.begin();
    auto removedorders = 0;
    while(true) {
        if (orderiter != orders.end()) {
            eosio_assert(orderiter->ostats.o_inflightdeals_nr == 0, "order can't be removed because there are some inflight deals");
            removedorders++;
            orders.erase(orderiter++);
        } else {
            break;
        }
    }

    _markets.modify( iter, 0, [&]( auto& mkt) {
        mkt.isremoved = true;
        mkt.mstats.suspendedorders_nr -= removedorders;
        mkt.mstats.totalopenorders_nr -= removedorders;
    });
}

//create an new data market, only the contract owner can create a market.
//a datasource can have ONLY ONE living data market, but can have lots of removed markets.
void dataexchange::createmarket(account_name owner, uint64_t type, string desp){
    require_auth(_self);

    eosio_assert(desp.length() < 30, "market description should be less than 30 characters");
    eosio_assert(cancreate(owner) == true, "an account can only create only one market now");
    eosio_assert((type > typestart && type < typeend), "out of market type");

    uint64_t newid = 0;
    if (_availableid.exists()) {
        auto iditem = _availableid.get();
        newid = ++iditem.availmarketid;
        _availableid.set(iditem, _self);
    } else {
        _availableid.set(availableid(), _self);
    }

    _markets.emplace( _self, [&]( auto& row) {
        row.marketid = newid;
        row.mowner = owner;
        row.mtype = type;
        row.mdesp = desp;
        row.minremovaltime = time_point_sec(0);
        row.issuspended = false;
    });
}

//suspend an market so no more orders can be made, this abi is for market management concern.
void dataexchange::suspendmkt(account_name owner, uint64_t marketid){
    //only the market owner can suspend a market
    require_auth(owner);

    auto iter = _markets.find(marketid);
    eosio_assert(iter != _markets.end(), "market not have been created yet");
    eosio_assert(iter->mowner == owner , "have no permission to this market");
    eosio_assert(iter->issuspended == false, "market should be work now");

    marketordertable orders(_self, owner);
    auto suspendedorders = 0;
    auto orderiter = orders.begin();
    while(true) {
        if (orderiter != orders.end()) {
            suspendedorders++;
            orders.modify( orderiter, 0, [&]( auto& order) {
                order.issuspended = true;
            });
            orderiter++;
        } else {
            break;
        }
    }
    _markets.modify( iter, 0, [&]( auto& mkt) {
        mkt.issuspended = true;
        mkt.mstats.suspendedorders_nr += suspendedorders;
        mkt.minremovaltime = time_point_sec(now() + market_min_suspendtoremoveal_interval);
    });
}

//resume a suspended data market.
void dataexchange::resumemkt(account_name owner, uint64_t marketid){
    //only the market owner can resume a market
    require_auth(owner);

    auto iter = _markets.find(marketid);
    eosio_assert(iter != _markets.end(), "market not have been created yet");
    eosio_assert(iter->mowner == owner , "have no permission to this market");
    eosio_assert(iter->issuspended == true, "market should be suspened");

    marketordertable orders(_self, owner);
    auto orderiter = orders.begin();
    auto resumedorders = 0;
    while(true) {
        if (orderiter != orders.end()) {
            resumedorders++;
            orders.modify( orderiter, 0, [&]( auto& order) {
                order.issuspended = true;
            });
            orderiter++;
        } else {
            break;
        }
    }
    _markets.modify( iter, 0, [&]( auto& mkt) {
        mkt.issuspended = false;
        mkt.mstats.suspendedorders_nr -= resumedorders;
        mkt.minremovaltime = time_point_sec(0);
    });
}

// create an order in a market, deals can be made under this order.
void dataexchange::createorder(account_name orderowner, uint64_t ordertype, uint64_t marketid, asset& price) {
    require_auth(orderowner);

    eosio_assert( price.is_valid(), "invalid price" );
    eosio_assert( price.amount > 0, "price must be positive price" );
    eosio_assert( ordertype < ordertype_end, "bad ordertype" );

    auto miter = _markets.find(marketid);
    eosio_assert(miter != _markets.end(), "market not have been created yet");
    eosio_assert(miter->isremoved != true, "market has already been removed");
    eosio_assert(miter->mowner != orderowner, "please don't trade on your own market");
    eosio_assert(miter->issuspended != true, "market has already suspened, can't create orders");

    marketordertable orders(_self, miter->mowner); 
    eosio_assert( hasorder_byorderowner(miter->mowner, orderowner) != true, 
                  "one user can only create a single order");

    auto iditem = _availableid.get();
    auto newid = ++iditem.availorderid;
    _availableid.set(availableid(iditem.availmarketid, newid, iditem.availdealid), _self);

    // we can only put it to the contract owner scope
    orders.emplace(orderowner, [&]( auto& order) {
        order.orderid = newid;
        order.orderowner = orderowner;
        order.order_type = ordertype;
        order.marketid = marketid;
        order.price = price;
    });

    //reg seller to accounts table 
    auto itr = _accounts.find(orderowner);
    if( itr == _accounts.end() ) {
        itr = _accounts.emplace(_self, [&](auto& acnt){
           acnt.owner = orderowner;
        });
    }

    _markets.modify( miter, 0, [&]( auto& mkt) {
        mkt.mstats.totalopenorders_nr++;
    });
}

//suspend an order so buyers can not make deals in this order.
void dataexchange::suspendorder(account_name orderowner, account_name marketowner, uint64_t orderid) {
    require_auth(orderowner);

    marketordertable orders(_self, marketowner);
    auto iter = orders.find(orderid);

    eosio_assert(iter != orders.end() , "no such order");
    eosio_assert(iter->orderowner == orderowner, "order doesn't belong to you");
    eosio_assert(iter->issuspended == false, "order should be work");
    orders.modify( iter, 0, [&]( auto& order) {
        order.issuspended = true;
    });

    auto miter = _markets.find(iter->marketid);
    _markets.modify( miter, 0, [&]( auto& mkt) {
        mkt.mstats.suspendedorders_nr++;
    });
}

//resume an suspended order.
void dataexchange::resumeorder(account_name orderowner, account_name marketowner, uint64_t orderid) {
    require_auth(orderowner);

    marketordertable orders(_self, marketowner);
    auto iter = orders.find(orderid);

    eosio_assert(iter != orders.end() , "no such order");
    eosio_assert(iter->orderowner == orderowner, "order doesn't belong to you");
    eosio_assert(iter->issuspended == true, "order should be suspened");
    orders.modify( iter, 0, [&]( auto& order) {
        order.issuspended = false;
    });
    auto miter = _markets.find(iter->marketid);
    _markets.modify( miter, 0, [&]( auto& mkt) {
        mkt.mstats.suspendedorders_nr--;
    });
}

//remove from data source so no new deals can be made, but inflight deals still can be finished.
void dataexchange::removeorder(account_name orderowner, account_name marketowner, uint64_t orderid) {
    require_auth(orderowner);

    marketordertable orders(_self, marketowner);
    auto iter = orders.find(orderid);

    eosio_assert(iter != orders.end() , "no such order");
    eosio_assert(iter->orderowner == orderowner, "order doesn't belong to you");

    //if there is inflight deals, please erase it before removing a order
    eosio_assert(iter->ostats.o_inflightdeals_nr == 0, "please erase or cancel deal before removing an order");

    auto miter = _markets.find(iter->marketid);
    _markets.modify( miter, 0, [&]( auto& mkt) {
        mkt.mstats.totalopenorders_nr--;
        if (iter->issuspended) {
            mkt.mstats.suspendedorders_nr--;
        }
    });

    orders.erase(iter);
}

//cancel an inflight deal both sides
void dataexchange::canceldeal(account_name canceler, account_name owner, uint64_t dealid) {
    require_auth(canceler);

    auto dealiter = _deals.find(dealid);
    eosio_assert(dealiter != _deals.end() , "no such deal");
    eosio_assert(dealiter->dealstate == dealstate_waitingslicehash || dealiter->dealstate == dealstate_waitingauthorize || 
                 dealiter->dealstate == dealstate_expired, 
                 "deal state is not correct for this operation");
    eosio_assert(dealiter->maker == canceler || dealiter->taker == canceler, "only maker or taker can cancel a deal");

    auto mktiter = _markets.find(dealiter->marketid);
    eosio_assert(mktiter != _markets.end(), "no such market");

    _deals.erase(dealiter);
    account_name buyer, seller;
    if (dealiter->ordertype == ordertype_ask) {
        buyer = dealiter->taker;
        seller = dealiter->maker;
    } else if (dealiter->ordertype == ordertype_bid) {
        buyer = dealiter->maker;
        seller = dealiter->taker;
    }

    // refund buyer's tokens
    auto buyeritr = _accounts.find(buyer);
    eosio_assert(buyeritr != _accounts.end() , "buyer should have have account");
    _accounts.modify( buyeritr, 0, [&]( auto& acnt ) {
        acnt.asset_balance += dealiter->price;
        acnt.inflightbuy_deals--;
    });

    auto selleritr = _accounts.find(seller);
    eosio_assert(selleritr != _accounts.end() , "seller should have have account");
    _accounts.modify( selleritr , 0, [&]( auto& acnt ) {
        acnt.inflightsell_deals--;
    });


    _markets.modify( mktiter, 0, [&]( auto& mkt) {
        mkt.mstats.inflightdeals_nr--;
    });

    marketordertable orders(_self, dealiter->marketowner);
    auto iter = orders.find(dealiter->orderid);
    eosio_assert(iter != orders.end() , "no such order");

    orders.modify( iter, 0, [&]( auto& order) {
        order.ostats.o_inflightdeals_nr--;
    });
}

//erase deal from ledger to free the memory usage.
void dataexchange::erasedeal(uint64_t dealid) {
    auto dealiter = _deals.find(dealid);
    eosio_assert(dealiter != _deals.end() , "no such deal");
    auto state = dealiter->dealstate;
    if (dealiter->dealstate != dealstate_finished && dealiter->expiretime < time_point_sec(now())) {
        state = dealstate_expired;
    }

    eosio_assert(dealiter->dealstate == dealstate_finished || state == dealstate_expired,
                 "deal state is not dealstate_finished、dealstate_expired or dealstate_wrongsecret");

    account_name buyer, seller;
    if (dealiter->ordertype == ordertype_ask) {
        buyer = dealiter->taker;
        seller = dealiter->maker;
    } else if (dealiter->ordertype == ordertype_bid) {
        buyer = dealiter->maker;
        seller = dealiter->taker;
    }

    if (state == dealstate_expired) {
        auto buyeritr = _accounts.find(buyer);
        _accounts.modify( buyeritr, 0, [&]( auto& acnt ) {
            acnt.expired_deals++;
            acnt.inflightbuy_deals--;
        });
        auto selleritr = _accounts.find(seller);
        _accounts.modify( selleritr, 0, [&]( auto& acnt ) {
            acnt.expired_deals++;
            acnt.inflightsell_deals--;
        });
    }

    if (dealiter->dealstate != dealstate_finished) {
        auto mktiter = _markets.find(dealiter->marketid);
        eosio_assert(mktiter != _markets.end(), "no such market");
        _markets.modify( mktiter, 0, [&]( auto& mkt) {
            mkt.mstats.inflightdeals_nr--;
        });
    }

    if (dealiter->dealstate != dealstate_finished) {
        marketordertable orders(_self, dealiter->marketowner);
        auto iter = orders.find(dealiter->orderid);
        eosio_assert(iter != orders.end() , "no such order");

        orders.modify( iter, 0, [&]( auto& order) {
            order.ostats.o_inflightdeals_nr--;
        });
    }

    _deals.erase(dealiter);
}

//owner is the market owner, market owner must be provided because all orders are stored in market owner's scope.
//see code: marketordertable orders(_self, marketowner);
//taker is the one who try to make a deal by taking an existing order.
void dataexchange::makedeal(account_name taker, account_name marketowner, uint64_t orderid) {
    require_auth(taker);

    marketordertable orders(_self, marketowner);
    auto iter = orders.find(orderid);

    eosio_assert(iter != orders.end() , "no such order");
    eosio_assert(iter->issuspended != true , "can not make deals because the order is suspended");
    auto mktiter = _markets.find(iter->marketid);
    eosio_assert(mktiter != _markets.end(), "no such market");

    uint64_t otype = iter->order_type;
    eosio_assert( otype < ordertype_end, "bad ordertype" );

    account_name buyer, seller;
    if (otype == ordertype_ask) {
        buyer = taker;
        seller = iter->orderowner;
    } else if (otype == ordertype_bid) {
        buyer = iter->orderowner;
        seller = taker;
    }

    //if order type is ask, then the taker is a buyer
    auto buyeriter = _accounts.find(buyer);
    if( buyeriter == _accounts.end() ) {
        _accounts.emplace(_self, [&](auto& acnt){
            acnt.owner = buyer;
        });
    }

    _accounts.modify( buyeriter, 0, [&]( auto& acnt ) {
        eosio_assert(acnt.asset_balance >= iter->price , "buyer should have enough token");

        //deduct token from the buyer's account
        acnt.asset_balance -= iter->price;
        acnt.inflightbuy_deals++;
    });

    auto selleritr = _accounts.find(seller);
    //reg seller to accounts table 
    if( selleritr == _accounts.end() ) {
        _accounts.emplace(_self, [&](auto& acnt){
            acnt.owner = seller;
            acnt.inflightsell_deals++; 
        });
    } else {
        _accounts.modify( selleritr, 0, [&]( auto& acnt ) {
            acnt.inflightsell_deals++;
        });
    }

    auto iditem = _availableid.get();
    auto newid = ++iditem.availdealid;
    _availableid.set(availableid(iditem.availmarketid, iditem.availorderid, newid), _self);

    //use self scope to make it simple for memory reclaiming.
    _deals.emplace(_self, [&](auto& deal) { 
        deal.dealid = newid;
        deal.marketowner = marketowner;
        deal.orderid = orderid;
        deal.marketid = iter->marketid;
        deal.source_datahash = "";
        deal.dealstate = dealstate_waitingauthorize;
        deal.ordertype = otype;
        deal.maker = iter->orderowner;
        deal.taker = taker;
        deal.price = iter->price;
        deal.expiretime = time_point_sec(now() + deal_expire_interval);
    });

    _markets.modify( mktiter, 0, [&]( auto& mkt) {
        mkt.mstats.inflightdeals_nr++;
    });
    orders.modify( iter, 0, [&]( auto& order) {
        order.ostats.o_inflightdeals_nr++;
    });
}

//maker authorize a deal, all deals are stored in contract owner's scope.
void dataexchange::authorize(account_name maker, uint64_t dealid) {
    require_auth(maker);

    auto dealiter = _deals.find(dealid);
    eosio_assert(dealiter != _deals.end() , "no such deal");
    eosio_assert(dealiter->dealstate == dealstate_waitingauthorize, "deal state is not dealstate_waitingauthorize");
    eosio_assert(dealiter->maker == maker, "this deal doesnot belong to you");
    eosio_assert(dealiter->expiretime > time_point_sec(now()), "this deal has been expired");
    _deals.modify( dealiter, 0, [&]( auto& deal) {
        //after seller's authorization, let's go to trading parameters negotiation.
        deal.dealstate = dealstate_waitingnegotiation;
    });
}

//datahash is generated using the buyers public key encrypted user's data.
//uploadhash is called by datasource(aka market owner).
void dataexchange::uploadhash(account_name sender, uint64_t dealid, string datahash) {
    require_auth(sender);

    auto dealiter = _deals.find(dealid);
    eosio_assert(dealiter != _deals.end() , "no such deal");
    eosio_assert(dealiter->dealstate == dealstate_waitingslicehash, "deal state is not dealstate_waitinghash");
    eosio_assert(dealiter->expiretime > time_point_sec(now()), "this deal has been expired");

    //(TODO) p2p deal can also do this
    eosio_assert(sender == dealiter->marketowner, "only datasource can uploadhash");

    auto mktiter = _markets.find(dealiter->marketid);
    eosio_assert(mktiter != _markets.end(), "no such market");

    _deals.modify( dealiter, 0, [&]( auto& deal) {
        deal.source_datahash = datahash;
        deal.dealstate = dealstate_waitingslicehashcomfirm;
    });
}

//a buyer comfirm the ipfs hash is valid
void dataexchange::confirmhash(account_name buyer, uint64_t dealid) {
    require_auth(buyer);

    auto dealiter = _deals.find(dealid);
    eosio_assert(dealiter != _deals.end() , "no such deal");
    auto state = dealiter->dealstate;
    eosio_assert(state == dealstate_waitingslicehashcomfirm, "deal state is not waiting dealstate_waitingslicehashcomfirm");
    eosio_assert(dealiter->expiretime > time_point_sec(now()), "deal is already expired");

    account_name _buyer;
    if (dealiter->ordertype == ordertype_ask) {
        _buyer = dealiter->taker;
    } else if (dealiter->ordertype == ordertype_bid) {
        _buyer = dealiter->maker;
    }

    eosio_assert(_buyer == buyer, "buyer is not correct");
    _deals.modify( dealiter, 0, [&]( auto& deal) {
        deal.dealstate = dealstate_waitingsecret;
    });
}

//deposit token to contract, all token will transfer to contract owner.
void dataexchange::deposit(account_name from, asset& quantity ) {
    require_auth( from);
   
    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must deposit positive quantity" );

    auto itr = _accounts.find(from);
    if( itr == _accounts.end() ) {
        itr = _accounts.emplace(_self, [&](auto& acnt){
            acnt.owner = from;
        });
    }

    _accounts.modify( itr, 0, [&]( auto& acnt ) {
        acnt.asset_balance += quantity;
    });

    //make sure contract xingyitoken have been deployed to blockchain to make it runnable
    //xingyitoken is our own token, its symbol is SYS
    action(
        permission_level{ from, N(active) },
        N(xingyitoken), N(transfer),
        std::make_tuple(from, _self, quantity, std::string("deposit token"))
    ).send();
}

//withdraw token from contract owner, token equals to quantity will transfer to owner.
void dataexchange::withdraw(account_name owner, asset& quantity ) {
    require_auth( owner );

    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must withdraw positive quantity" );

    auto itr = _accounts.find( owner );
    eosio_assert(itr != _accounts.end(), "account has no fund, can't withdraw");

    _accounts.modify( itr, 0, [&]( auto& acnt ) {
        eosio_assert( acnt.asset_balance >= quantity, "insufficient balance" );
        acnt.asset_balance -= quantity;
    });

    //make sure contract xingyitoken have been deployed to blockchain to make it runnable
    //xingyitoken is our own token, its symblo is SYS
    action(
        permission_level{ _self, N(active) },
        N(xingyitoken), N(transfer),
        std::make_tuple(_self, owner, quantity, std::string("withdraw token"))
    ).send();

    // erase account when no more fund to free memory 
    if( itr->asset_balance.amount == 0 && itr->pkey.length() == 0 && 
        itr->finished_deals == 0 && itr->inflightbuy_deals == 0 && itr->inflightsell_deals == 0) {
       _accounts.erase(itr);
    }
}

//register public key to ledger, the data source can encrypt data by this public key.
void dataexchange::regpkey(account_name owner, string pkey) {
    require_auth( owner );

    pkey.erase(pkey.begin(), find_if(pkey.begin(), pkey.end(), [](int ch) {
        return !isspace(ch);
    }));
    pkey.erase(find_if(pkey.rbegin(), pkey.rend(), [](int ch) {
        return !isspace(ch);
    }).base(), pkey.end());

    eosio_assert(pkey.length() == 53, "Length of public key should be 53");
    string pubkey_prefix("EOS");
    auto result = mismatch(pubkey_prefix.begin(), pubkey_prefix.end(), pkey.begin());
    eosio_assert(result.first == pubkey_prefix.end(), "Public key should be prefix with EOS");

    auto base58substr = pkey.substr(pubkey_prefix.length());
    vector<unsigned char> vch;
    //(fixme)decode_base58 can be very time-consuming, must remove it in the future.
    eosio_assert(decode_base58(base58substr, vch), "Decode public failed");
    eosio_assert(vch.size() == 37, "Invalid public key: invalid base58 length");

    array<unsigned char,33> pubkey_data;
    copy_n(vch.begin(), 33, pubkey_data.begin());

    checksum160 check_pubkey;
    ripemd160(reinterpret_cast<char *>(pubkey_data.data()), 33, &check_pubkey);
    eosio_assert(memcmp(&check_pubkey.hash, &vch.end()[-4], 4) == 0, "Invalid public key: invalid checksum");

    auto itr = _accounts.find( owner );
    if( itr == _accounts.end() ) {
        itr = _accounts.emplace(_self, [&](auto& acnt){
            acnt.owner = owner;
        });
    }

    _accounts.modify( itr, 0, [&]( auto& acnt ) {
        acnt.pkey = pkey;
    });
}

//deregister public key, aka remove from ledger.
void dataexchange::deregpkey(account_name owner) {
    require_auth( owner );

    auto itr = _accounts.find( owner );
    eosio_assert(itr != _accounts.end(), "account not registered yet");

    //reducer uncessary account erasal
    if (itr->asset_balance.amount > 0 || itr->finished_deals > 0 || itr->inflightbuy_deals > 0 || itr->inflightsell_deals > 0) {
        _accounts.modify( itr, 0, [&]( auto& acnt ) {
            acnt.pkey = "";
        });
    } else {
        _accounts.erase(itr);
    }
}

//negotiate trading parameters for data delivery
void dataexchange::negotiate(account_name buyer, uint64_t dealid, uint64_t slices, uint64_t expiration, uint64_t sliceprice, uint64_t collateral){
    require_auth(buyer);

    auto dealiter = _deals.find(dealid);
    eosio_assert(dealiter != _deals.end() , "no such deal");
    auto state = dealiter->dealstate;
    eosio_assert(state == dealstate_waitingnegotiation, "deal state is not waiting dealstate_waitingnegotiation");
    eosio_assert(dealiter->expiretime > time_point_sec(now()), "deal is already expired");
    account_name dealbuyer = (dealiter->ordertype == ordertype_ask) ? dealiter->taker : dealiter->maker;
    eosio_assert(dealbuyer == buyer, "negotiate should only raised by buyer");

    _deals.modify( dealiter, 0, [&]( auto& deal ) {
        deal.dealstate = dealstate_waitingnegotiationcomfirm;
        deal.dp.slices = slices;
        deal.dp.collateral = collateral;
        deal.dp.expiration = expiration;
    });
}

void dataexchange::renegotiate(uint64_t dealid, uint64_t slices, uint64_t expiraiton, uint64_t seliceprice, uint64_t collateral){

}

// in ordinary case datasource is the one to deliver data
void dataexchange::confirmparam(account_name datasource, uint64_t dealid){
    require_auth(datasource);

    auto dealiter = _deals.find(dealid);
    eosio_assert(dealiter != _deals.end() , "no such deal");
    auto state = dealiter->dealstate;
    eosio_assert(state == dealstate_waitingnegotiationcomfirm, "deal state is not waiting dealstate_waitingnegotiationcomfirm");
    eosio_assert(dealiter->expiretime > time_point_sec(now()), "deal is already expired");
    eosio_assert(dealiter->marketowner == datasource, "datasource is not correct");

    _deals.modify( dealiter, 0, [&]( auto& deal ) {
        deal.dealstate = dealstate_waitingslicehash;
        deal.deliverdslices = 0;
    });
}

//a buyer request for the slice password
void dataexchange::askforsecret(account_name buyer, uint64_t dealid){
    require_auth(buyer);

    auto dealiter = _deals.find(dealid);
    eosio_assert(dealiter != _deals.end() , "no such deal");
    auto state = dealiter->dealstate;
    eosio_assert(state == dealstate_waitingnegotiation, "deal state is not waiting dealstate_waitingnegotiation");
    eosio_assert(dealiter->expiretime > time_point_sec(now()), "deal is already expired");
    account_name dealbuyer = (dealiter->ordertype == ordertype_ask) ? dealiter->taker : dealiter->maker;
    eosio_assert(dealbuyer == buyer, "negotiate should only raised by buyer");

    _deals.modify( dealiter, 0, [&]( auto& deal ) {
        deal.dealstate = dealstate_waitingsecret;
    });
}

void dataexchange::showsecret(account_name datasource, uint64_t dealid, string password) {
    require_auth(datasource);

    auto dealiter = _deals.find(dealid);
    eosio_assert(dealiter != _deals.end() , "no such deal");
    auto state = dealiter->dealstate;
    eosio_assert(state == dealstate_waitingnegotiationcomfirm, "deal state is not waiting dealstate_waitingnegotiationcomfirm");
    eosio_assert(dealiter->expiretime > time_point_sec(now()), "deal is already expired");
    eosio_assert(dealiter->marketowner == datasource, "datasource is not correct");

   auto mktiter = _markets.find(dealiter->marketid);
   eosio_assert(mktiter != _markets.end(), "no such market");

   //this abi should only run by the market owner
   require_auth(mktiter->mowner);
   eosio_assert(mktiter != _markets.end(), "no such market");

   auto otype = dealiter->ordertype;
   account_name buyer, seller;
   if (otype == ordertype_ask) {
       buyer = dealiter->taker;
       seller = dealiter->maker;
   } else if (otype == ordertype_bid) {
       buyer = dealiter->maker;
       seller = dealiter->taker;
   }
   auto selleriter = _accounts.find(seller);
   if( selleriter == _accounts.end() ) {
       selleriter = _accounts.emplace(_self, [&](auto& acnt){
           acnt.owner = seller;
       });
   }

    _deals.modify( dealiter, 0, [&]( auto& deal ) {
        deal.dealstate = dealstate_waitingslicehash;
        deal.deliverdslices++;
    });


    auto sellertoken = asset(uint64_t(0.9 * dealiter->price.amount));
    auto sourcetoken = asset(uint64_t(0.1 * dealiter->price.amount));

    // add token to seller's account

    _accounts.modify( selleriter, 0, [&]( auto& acnt ) {
        acnt.asset_balance += sellertoken;
        acnt.finished_deals++;
        acnt.inflightsell_deals--;
    });

    // add token to data source account
    auto sourceitr = _accounts.find(dealiter->marketowner);
    if( sourceitr == _accounts.end() ) {
        sourceitr = _accounts.emplace(_self, [&](auto& acnt){
            acnt.owner = dealiter->marketowner;
        });
    }
    _accounts.modify( sourceitr, 0, [&]( auto& acnt ) {
        acnt.asset_balance += sourcetoken;
    });    

    // modify buyers finished order data
    auto buyeriter = _accounts.find(buyer);
    eosio_assert(buyeriter != _accounts.end(), "buyer should have account");
    _accounts.modify( buyeriter, 0, [&]( auto& acnt ) {
        acnt.inflightbuy_deals--;
        acnt.finished_deals++;
    });

    _markets.modify( mktiter, 0, [&]( auto& mkt) {
        mkt.mstats.inflightdeals_nr--;
        mkt.mstats.finisheddeals_nr++;
        mkt.mstats.tradingincome_nr += sourcetoken;
        mkt.mstats.tradingvolume_nr += dealiter->price;
    });

    marketordertable orders(_self, dealiter->marketowner);
    auto iter = orders.find(dealiter->orderid);
    eosio_assert(iter!= orders.end() , "no such order");
    orders.modify( iter, 0, [&]( auto& order) {
        order.ostats.o_inflightdeals_nr--;
        order.ostats.o_finisheddeals_nr++;
        order.ostats.o_finishedvolume_nr += dealiter->price;
    });    
}

void dataexchange::arbitrate(account_name datasource, uint64_t dealid, string encrypteddata) {

}

//this abi will generate an dealid
void dataexchange::directdeal(account_name buyer, account_name seller, asset &price, string data_spec){

}

void dataexchange::higherprice(account_name seller, uint64_t dealid, asset &price) {
    require_auth(seller);
    return;
}
void dataexchange::directredeal(account_name buyer, uint64_t dealid, asset &price, string data_spec) {
}

void dataexchange::directhash(account_name buyer, account_name seller, asset &price){

}
void dataexchange::directack(account_name buyer, uint64_t dealid){

}
void dataexchange::directsecret(uint64_t marketid, uint64_t dealid, string secret){
}

void dataexchange::aesencrypt(){
    uint8_t key[] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
                      0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };
    uint8_t out[] = { 0xf5, 0x8c, 0x4c, 0x04, 0xd6, 0xe5, 0xf1, 0xba, 0x77, 0x9e, 0xab, 0xfb, 0x5f, 0x7b, 0xfb, 0xd6,
                      0x9c, 0xfc, 0x4e, 0x96, 0x7e, 0xdb, 0x80, 0x8d, 0x67, 0x9f, 0x77, 0x7b, 0xc6, 0x70, 0x2c, 0x7d,
                      0x39, 0xf2, 0x33, 0x69, 0xa9, 0xd9, 0xba, 0xcf, 0xa5, 0x30, 0xe2, 0x63, 0x04, 0x23, 0x14, 0x61,
                      0xb2, 0xeb, 0x05, 0xe2, 0xc3, 0x9b, 0xe9, 0xfc, 0xda, 0x6c, 0x19, 0x07, 0x8c, 0x6a, 0x9d, 0x1b };
    uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
    uint8_t in[]  = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
                      0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
                      0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
                      0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10 };
    struct AES_ctx ctx;

    initiv(&ctx, key, iv);
    cbcencrypt(&ctx, in, 64);
    eosio_assert(0 == memcmp((char*) out, (char*) in, 64), "failed to encrypt");
    print("aes encrypt success");
}

void dataexchange::aesdecrypt(){
    uint8_t key[] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
                      0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };
    uint8_t in[]  = { 0xf5, 0x8c, 0x4c, 0x04, 0xd6, 0xe5, 0xf1, 0xba, 0x77, 0x9e, 0xab, 0xfb, 0x5f, 0x7b, 0xfb, 0xd6,
                      0x9c, 0xfc, 0x4e, 0x96, 0x7e, 0xdb, 0x80, 0x8d, 0x67, 0x9f, 0x77, 0x7b, 0xc6, 0x70, 0x2c, 0x7d,
                      0x39, 0xf2, 0x33, 0x69, 0xa9, 0xd9, 0xba, 0xcf, 0xa5, 0x30, 0xe2, 0x63, 0x04, 0x23, 0x14, 0x61,
                      0xb2, 0xeb, 0x05, 0xe2, 0xc3, 0x9b, 0xe9, 0xfc, 0xda, 0x6c, 0x19, 0x07, 0x8c, 0x6a, 0x9d, 0x1b };
    uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
    uint8_t out[] = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
                      0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
                      0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
                      0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10 };
//  uint8_t buffer[64];
    struct AES_ctx ctx;

    initiv(&ctx, key, iv);
    cbcdecrypt(&ctx, in, 64);
    eosio_assert(0 == memcmp((char*) out, (char*) in, 64), "failed to decrypt");
    print("aes decrypt success");
}

