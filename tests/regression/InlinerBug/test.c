 #include <stdio.h>
 #include <assert.h>

 #include "private.h"
 #include <stdio.h>


 #define SASR(x, by) ((x) >> (by))



typedef struct gsm_state * gsm;
typedef short gsm_signal;
typedef unsigned char gsm_byte;
typedef gsm_byte gsm_frame[33];
extern gsm gsm_create (void);
extern void gsm_destroy (gsm);

extern int gsm_print (FILE *, gsm, gsm_byte *);
extern int gsm_option (gsm, int, int *);

extern void gsm_encode (gsm, gsm_signal *, gsm_byte *);
extern int gsm_decode (gsm, gsm_byte *, gsm_signal *);

extern int gsm_explode (gsm, gsm_byte *, gsm_signal *);
extern void gsm_implode (gsm, gsm_signal *, gsm_byte *);




word gsm_add (word a, word b)
{
 longword sum = (longword)a + (longword)b;
 return ((sum) < MIN_WORD ? MIN_WORD : (sum) > MAX_WORD ? MAX_WORD: (sum));
}

word gsm_sub (word a, word b)
{
 longword diff = (longword)a - (longword)b;
 return ((diff) < MIN_WORD ? MIN_WORD : (diff) > MAX_WORD ? MAX_WORD: (diff));
}

word gsm_mult (word a, word b)
{
 if (a == MIN_WORD && b == MIN_WORD) return MAX_WORD;
 else return SASR( (longword)a * (longword)b, 15 );
}

word gsm_mult_r (word a, word b)
{
 if (b == MIN_WORD && a == MIN_WORD) return MAX_WORD;
 else {
  longword prod = (longword)a * (longword)b + 16384;
  prod >>= 15;
  return prod & 0xFFFF;
 }
}

word gsm_abs (word a)
{
 return a < 0 ? (a == MIN_WORD ? MAX_WORD : -a) : a;
}

longword gsm_L_mult (word a, word b)
{
 assert( a != MIN_WORD || b != MIN_WORD );
 return ((longword)a * (longword)b) << 1;
}

longword gsm_L_add (longword a, longword b)
{
 if (a < 0) {
  if (b >= 0) return a + b;
  else {
   ulongword A = (ulongword)-(a + 1) + (ulongword)-(b + 1);
   return A >= MAX_LONGWORD ? MIN_LONGWORD :-(longword)A-2;
  }
 }
 else if (b <= 0) return a + b;
 else {
  ulongword A = (ulongword)a + (ulongword)b;
  return A > MAX_LONGWORD ? MAX_LONGWORD : A;
 }
}

longword gsm_L_sub (longword a, longword b)
{
 if (a >= 0) {
  if (b >= 0) return a - b;
  else {


   ulongword A = (ulongword)a + -(b + 1);
   return A >= MAX_LONGWORD ? MAX_LONGWORD : (A + 1);
  }
 }
 else if (b <= 0) return a - b;
 else {


  ulongword A = (ulongword)-(a + 1) + b;
  return A >= MAX_LONGWORD ? MIN_LONGWORD : -(longword)A - 1;
 }
}

static unsigned char bitoff[ 256 ] = {
  8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

word gsm_norm (longword a)
{
 assert(a != 0);

 if (a < 0) {
  if (a <= -1073741824) return 0;
  a = ~a;
 }

 return a & 0xffff0000
  ? ( a & 0xff000000
    ? -1 + bitoff[ 0xFF & (a >> 24) ]
    : 7 + bitoff[ 0xFF & (a >> 16) ] )
  : ( a & 0xff00
    ? 15 + bitoff[ 0xFF & (a >> 8) ]
    : 23 + bitoff[ 0xFF & a ] );
}

longword gsm_L_asl (longword a, int n)
{
 if (n >= 32) return 0;
 if (n <= -32) return -(a < 0);
 if (n < 0) return gsm_L_asr(a, -n);
 return a << n;
}

word gsm_asl (word a, int n)
{
 if (n >= 16) return 0;
 if (n <= -16) return -(a < 0);
 if (n < 0) return gsm_asr(a, -n);
 return a << n;
}

longword gsm_L_asr (longword a, int n)
{
 if (n >= 32) return -(a < 0);
 if (n <= -32) return 0;
 if (n < 0) return a << -n;




  if (a >= 0) return a >> n;
  else return -(longword)( -(ulongword)a >> n );

}

word gsm_asr (word a, int n)
{
 if (n >= 16) return -(a < 0);
 if (n <= -16) return 0;
 if (n < 0) return a << -n;




  if (a >= 0) return a >> n;
  else return -(word)( -(uword)a >> n );

}
word gsm_div (word num, word denum)
{
 longword L_num = num;
 longword L_denum = denum;
 word div = 0;
 int k = 15;
 assert(num >= 0 && denum >= num);
 if (num == 0)
     return 0;

 while (k--) {
  div <<= 1;
  L_num <<= 1;

  if (L_num >= L_denum) {
   L_num -= L_denum;
   div++;
  }
 }

 return div;
}











 #include <stdlib.h>





 #include "private.h"
 #include <stdio.h>






typedef struct gsm_state * gsm;
typedef short gsm_signal;
typedef unsigned char gsm_byte;
typedef gsm_byte gsm_frame[33];
extern gsm gsm_create (void);
extern void gsm_destroy (gsm);

extern int gsm_print (FILE *, gsm, gsm_byte *);
extern int gsm_option (gsm, int, int *);

extern void gsm_encode (gsm, gsm_signal *, gsm_byte *);
extern int gsm_decode (gsm, gsm_byte *, gsm_signal *);

extern int gsm_explode (gsm, gsm_byte *, gsm_signal *);
extern void gsm_implode (gsm, gsm_signal *, gsm_byte *);





void Gsm_Coder (struct gsm_state * S, word * s, word * LARc, word * Nc, word * bc, word * Mc, word * xmaxc, word * xMc)
{
 int k;
 word * dp = S->dp0 + 120;
 word * dpp = dp;

 static word e [50] = {0};

 word so[160];

 Gsm_Preprocess (S, s, so);
 Gsm_LPC_Analysis (S, so, LARc);
 Gsm_Short_Term_Analysis_Filter (S, LARc, so);

 for (k = 0; k <= 3; k++, xMc += 13) {

  Gsm_Long_Term_Predictor ( S,
      so+k*40,
      dp,
     e + 5,
     dpp,
      Nc++,
      bc++);

  Gsm_RPE_Encoding ( S,
     e + 5,
       xmaxc++, Mc++, xMc );





  { register int i;
    register longword ltmp;
    for (i = 0; i <= 39; i++)
   dp[ i ] = GSM_ADD( e[5 + i], dpp[i] );
  }
  dp += 40;
  dpp += 40;

 }
 (void)memcpy( (char *)S->dp0, (char *)(S->dp0 + 160),
  120 * sizeof(*S->dp0) );
}








 #include "private.h"








 #include <stdio.h>

void gsm_debug_words (char * name, int from, int to, word * ptr)




{
 int nprinted = 0;

 fprintf( stderr, "%s [%d .. %d]: ", name, from, to );
 while (from <= to) {
  fprintf(stderr, "%d ", ptr[ from ] );
  from++;
  if (nprinted++ >= 7) {
   nprinted = 0;
   if (from < to) putc('\n', stderr);
  }
 }
 putc('\n', stderr);
}

void gsm_debug_longwords (char * name, int from, int to, longword * ptr)




{
 int nprinted = 0;

 fprintf( stderr, "%s [%d .. %d]: ", name, from, to );
 while (from <= to) {

  fprintf(stderr, "%d ", ptr[ from ] );
  from++;
  if (nprinted++ >= 7) {
   nprinted = 0;
   if (from < to) putc('\n', stderr);
  }
 }
 putc('\n', stderr);
}

void gsm_debug_longword (char * name, longword value)


{
 fprintf(stderr, "%s: %d\n", name, (long)value );
}

void gsm_debug_word (char * name, word value)


{
 fprintf(stderr, "%s: %d\n", name, (long)value);
}








 #include <stdio.h>

 #include "private.h"
 #include <stdio.h>






typedef struct gsm_state * gsm;
typedef short gsm_signal;
typedef unsigned char gsm_byte;
typedef gsm_byte gsm_frame[33];
extern gsm gsm_create (void);
extern void gsm_destroy (gsm);

extern int gsm_print (FILE *, gsm, gsm_byte *);
extern int gsm_option (gsm, int, int *);

extern void gsm_encode (gsm, gsm_signal *, gsm_byte *);
extern int gsm_decode (gsm, gsm_byte *, gsm_signal *);

extern int gsm_explode (gsm, gsm_byte *, gsm_signal *);
extern void gsm_implode (gsm, gsm_signal *, gsm_byte *);





static void Postprocessing (struct gsm_state * S, register word * s)


{
 register int k;
 register word msr = S->msr;
 register longword ltmp;
 register word tmp;

 for (k = 160; k--; s++) {
  tmp = GSM_MULT_R( msr, 28180 );
  msr = GSM_ADD(*s, tmp);
  *s = GSM_ADD(msr, msr) & 0xFFF8;
 }
 S->msr = msr;
}

void Gsm_Decoder (struct gsm_state * S, word * LARcr, word * Ncr, word * bcr, word * Mcr, word * xmaxcr, word * xMcr, word * s)
{
 int j, k;
 word erp[40], wt[160];
 word * drp = S->dp0 + 120;

 for (j=0; j <= 3; j++, xmaxcr++, bcr++, Ncr++, Mcr++, xMcr += 13) {

  Gsm_RPE_Decoding( S, *xmaxcr, *Mcr, xMcr, erp );
  Gsm_Long_Term_Synthesis_Filtering( S, *Ncr, *bcr, erp, drp );

  for (k = 0; k <= 39; k++) wt[ j * 40 + k ] = drp[ k ];
 }

 Gsm_Short_Term_Synthesis_Filter( S, LARcr, wt, s );
 Postprocessing(S, s);
}






static char ident[] = "$Header: /home/mguthaus/.cvsroot/mibench/telecomm/gsm/src/gsm_create.c,v 1.1.1.1 2000/11/06 19:54:25 mguthaus Exp $";




 #include <string.h>






 #include <stdlib.h>








 #include <stdio.h>

 #include <stdio.h>






typedef struct gsm_state * gsm;
typedef short gsm_signal;
typedef unsigned char gsm_byte;
typedef gsm_byte gsm_frame[33];
extern gsm gsm_create (void);
extern void gsm_destroy (gsm);

extern int gsm_print (FILE *, gsm, gsm_byte *);
extern int gsm_option (gsm, int, int *);

extern void gsm_encode (gsm, gsm_signal *, gsm_byte *);
extern int gsm_decode (gsm, gsm_byte *, gsm_signal *);

extern int gsm_explode (gsm, gsm_byte *, gsm_signal *);
extern void gsm_implode (gsm, gsm_signal *, gsm_byte *);
 #include "private.h"

gsm gsm_create (void)
{
 gsm r;
 r = (gsm)malloc(sizeof(struct gsm_state));
 if (!r) return r;

 memset((char *)r, 0, sizeof(*r));
 r->nrp = 40;

 return r;
}








 #include "private.h"

 #include <stdio.h>






typedef struct gsm_state * gsm;
typedef short gsm_signal;
typedef unsigned char gsm_byte;
typedef gsm_byte gsm_frame[33];
extern gsm gsm_create (void);
extern void gsm_destroy (gsm);

extern int gsm_print (FILE *, gsm, gsm_byte *);
extern int gsm_option (gsm, int, int *);

extern void gsm_encode (gsm, gsm_signal *, gsm_byte *);
extern int gsm_decode (gsm, gsm_byte *, gsm_signal *);

extern int gsm_explode (gsm, gsm_byte *, gsm_signal *);
extern void gsm_implode (gsm, gsm_signal *, gsm_byte *);

int gsm_decode (gsm s, gsm_byte * c, gsm_signal * target)
{
 word LARc[8], Nc[4], Mc[4], bc[4], xmaxc[4], xmc[13*4];



 if (((*c >> 4) & 0x0F) != 0xD) return -1;

 LARc[0] = (*c++ & 0xF) << 2;
 LARc[0] |= (*c >> 6) & 0x3;
 LARc[1] = *c++ & 0x3F;
 LARc[2] = (*c >> 3) & 0x1F;
 LARc[3] = (*c++ & 0x7) << 2;
 LARc[3] |= (*c >> 6) & 0x3;
 LARc[4] = (*c >> 2) & 0xF;
 LARc[5] = (*c++ & 0x3) << 2;
 LARc[5] |= (*c >> 6) & 0x3;
 LARc[6] = (*c >> 3) & 0x7;
 LARc[7] = *c++ & 0x7;
 Nc[0] = (*c >> 1) & 0x7F;
 bc[0] = (*c++ & 0x1) << 1;
 bc[0] |= (*c >> 7) & 0x1;
 Mc[0] = (*c >> 5) & 0x3;
 xmaxc[0] = (*c++ & 0x1F) << 1;
 xmaxc[0] |= (*c >> 7) & 0x1;
 xmc[0] = (*c >> 4) & 0x7;
 xmc[1] = (*c >> 1) & 0x7;
 xmc[2] = (*c++ & 0x1) << 2;
 xmc[2] |= (*c >> 6) & 0x3;
 xmc[3] = (*c >> 3) & 0x7;
 xmc[4] = *c++ & 0x7;
 xmc[5] = (*c >> 5) & 0x7;
 xmc[6] = (*c >> 2) & 0x7;
 xmc[7] = (*c++ & 0x3) << 1;
 xmc[7] |= (*c >> 7) & 0x1;
 xmc[8] = (*c >> 4) & 0x7;
 xmc[9] = (*c >> 1) & 0x7;
 xmc[10] = (*c++ & 0x1) << 2;
 xmc[10] |= (*c >> 6) & 0x3;
 xmc[11] = (*c >> 3) & 0x7;
 xmc[12] = *c++ & 0x7;
 Nc[1] = (*c >> 1) & 0x7F;
 bc[1] = (*c++ & 0x1) << 1;
 bc[1] |= (*c >> 7) & 0x1;
 Mc[1] = (*c >> 5) & 0x3;
 xmaxc[1] = (*c++ & 0x1F) << 1;
 xmaxc[1] |= (*c >> 7) & 0x1;
 xmc[13] = (*c >> 4) & 0x7;
 xmc[14] = (*c >> 1) & 0x7;
 xmc[15] = (*c++ & 0x1) << 2;
 xmc[15] |= (*c >> 6) & 0x3;
 xmc[16] = (*c >> 3) & 0x7;
 xmc[17] = *c++ & 0x7;
 xmc[18] = (*c >> 5) & 0x7;
 xmc[19] = (*c >> 2) & 0x7;
 xmc[20] = (*c++ & 0x3) << 1;
 xmc[20] |= (*c >> 7) & 0x1;
 xmc[21] = (*c >> 4) & 0x7;
 xmc[22] = (*c >> 1) & 0x7;
 xmc[23] = (*c++ & 0x1) << 2;
 xmc[23] |= (*c >> 6) & 0x3;
 xmc[24] = (*c >> 3) & 0x7;
 xmc[25] = *c++ & 0x7;
 Nc[2] = (*c >> 1) & 0x7F;
 bc[2] = (*c++ & 0x1) << 1;
 bc[2] |= (*c >> 7) & 0x1;
 Mc[2] = (*c >> 5) & 0x3;
 xmaxc[2] = (*c++ & 0x1F) << 1;
 xmaxc[2] |= (*c >> 7) & 0x1;
 xmc[26] = (*c >> 4) & 0x7;
 xmc[27] = (*c >> 1) & 0x7;
 xmc[28] = (*c++ & 0x1) << 2;
 xmc[28] |= (*c >> 6) & 0x3;
 xmc[29] = (*c >> 3) & 0x7;
 xmc[30] = *c++ & 0x7;
 xmc[31] = (*c >> 5) & 0x7;
 xmc[32] = (*c >> 2) & 0x7;
 xmc[33] = (*c++ & 0x3) << 1;
 xmc[33] |= (*c >> 7) & 0x1;
 xmc[34] = (*c >> 4) & 0x7;
 xmc[35] = (*c >> 1) & 0x7;
 xmc[36] = (*c++ & 0x1) << 2;
 xmc[36] |= (*c >> 6) & 0x3;
 xmc[37] = (*c >> 3) & 0x7;
 xmc[38] = *c++ & 0x7;
 Nc[3] = (*c >> 1) & 0x7F;
 bc[3] = (*c++ & 0x1) << 1;
 bc[3] |= (*c >> 7) & 0x1;
 Mc[3] = (*c >> 5) & 0x3;
 xmaxc[3] = (*c++ & 0x1F) << 1;
 xmaxc[3] |= (*c >> 7) & 0x1;
 xmc[39] = (*c >> 4) & 0x7;
 xmc[40] = (*c >> 1) & 0x7;
 xmc[41] = (*c++ & 0x1) << 2;
 xmc[41] |= (*c >> 6) & 0x3;
 xmc[42] = (*c >> 3) & 0x7;
 xmc[43] = *c++ & 0x7;
 xmc[44] = (*c >> 5) & 0x7;
 xmc[45] = (*c >> 2) & 0x7;
 xmc[46] = (*c++ & 0x3) << 1;
 xmc[46] |= (*c >> 7) & 0x1;
 xmc[47] = (*c >> 4) & 0x7;
 xmc[48] = (*c >> 1) & 0x7;
 xmc[49] = (*c++ & 0x1) << 2;
 xmc[49] |= (*c >> 6) & 0x3;
 xmc[50] = (*c >> 3) & 0x7;
 xmc[51] = *c & 0x7;

 Gsm_Decoder(s, LARc, Nc, bc, Mc, xmaxc, xmc, target);

 return 0;
}








 #include <stdio.h>






typedef struct gsm_state * gsm;
typedef short gsm_signal;
typedef unsigned char gsm_byte;
typedef gsm_byte gsm_frame[33];
extern gsm gsm_create (void);
extern void gsm_destroy (gsm);

extern int gsm_print (FILE *, gsm, gsm_byte *);
extern int gsm_option (gsm, int, int *);

extern void gsm_encode (gsm, gsm_signal *, gsm_byte *);
extern int gsm_decode (gsm, gsm_byte *, gsm_signal *);

extern int gsm_explode (gsm, gsm_byte *, gsm_signal *);
extern void gsm_implode (gsm, gsm_signal *, gsm_byte *);


 #include <stdlib.h>








void gsm_destroy (gsm S)
{
 if (S) free((char *)S);
}








 #include "private.h"
 #include <stdio.h>






typedef struct gsm_state * gsm;
typedef short gsm_signal;
typedef unsigned char gsm_byte;
typedef gsm_byte gsm_frame[33];
extern gsm gsm_create (void);
extern void gsm_destroy (gsm);

extern int gsm_print (FILE *, gsm, gsm_byte *);
extern int gsm_option (gsm, int, int *);

extern void gsm_encode (gsm, gsm_signal *, gsm_byte *);
extern int gsm_decode (gsm, gsm_byte *, gsm_signal *);

extern int gsm_explode (gsm, gsm_byte *, gsm_signal *);
extern void gsm_implode (gsm, gsm_signal *, gsm_byte *);

void gsm_encode (gsm s, gsm_signal * source, gsm_byte * c)
{
 word LARc[8], Nc[4], Mc[4], bc[4], xmaxc[4], xmc[13*4];

 Gsm_Coder(s, source, LARc, Nc, bc, Mc, xmaxc, xmc);
 *c++ = ((0xD & 0xF) << 4)
        | ((LARc[0] >> 2) & 0xF);
 *c++ = ((LARc[0] & 0x3) << 6)
        | (LARc[1] & 0x3F);
 *c++ = ((LARc[2] & 0x1F) << 3)
        | ((LARc[3] >> 2) & 0x7);
 *c++ = ((LARc[3] & 0x3) << 6)
        | ((LARc[4] & 0xF) << 2)
        | ((LARc[5] >> 2) & 0x3);
 *c++ = ((LARc[5] & 0x3) << 6)
        | ((LARc[6] & 0x7) << 3)
        | (LARc[7] & 0x7);
 *c++ = ((Nc[0] & 0x7F) << 1)
        | ((bc[0] >> 1) & 0x1);
 *c++ = ((bc[0] & 0x1) << 7)
        | ((Mc[0] & 0x3) << 5)
        | ((xmaxc[0] >> 1) & 0x1F);
 *c++ = ((xmaxc[0] & 0x1) << 7)
        | ((xmc[0] & 0x7) << 4)
        | ((xmc[1] & 0x7) << 1)
        | ((xmc[2] >> 2) & 0x1);
 *c++ = ((xmc[2] & 0x3) << 6)
        | ((xmc[3] & 0x7) << 3)
        | (xmc[4] & 0x7);
 *c++ = ((xmc[5] & 0x7) << 5)
        | ((xmc[6] & 0x7) << 2)
        | ((xmc[7] >> 1) & 0x3);
 *c++ = ((xmc[7] & 0x1) << 7)
        | ((xmc[8] & 0x7) << 4)
        | ((xmc[9] & 0x7) << 1)
        | ((xmc[10] >> 2) & 0x1);
 *c++ = ((xmc[10] & 0x3) << 6)
        | ((xmc[11] & 0x7) << 3)
        | (xmc[12] & 0x7);
 *c++ = ((Nc[1] & 0x7F) << 1)
        | ((bc[1] >> 1) & 0x1);
 *c++ = ((bc[1] & 0x1) << 7)
        | ((Mc[1] & 0x3) << 5)
        | ((xmaxc[1] >> 1) & 0x1F);
 *c++ = ((xmaxc[1] & 0x1) << 7)
        | ((xmc[13] & 0x7) << 4)
        | ((xmc[14] & 0x7) << 1)
        | ((xmc[15] >> 2) & 0x1);
 *c++ = ((xmc[15] & 0x3) << 6)
        | ((xmc[16] & 0x7) << 3)
        | (xmc[17] & 0x7);
 *c++ = ((xmc[18] & 0x7) << 5)
        | ((xmc[19] & 0x7) << 2)
        | ((xmc[20] >> 1) & 0x3);
 *c++ = ((xmc[20] & 0x1) << 7)
        | ((xmc[21] & 0x7) << 4)
        | ((xmc[22] & 0x7) << 1)
        | ((xmc[23] >> 2) & 0x1);
 *c++ = ((xmc[23] & 0x3) << 6)
        | ((xmc[24] & 0x7) << 3)
        | (xmc[25] & 0x7);
 *c++ = ((Nc[2] & 0x7F) << 1)
        | ((bc[2] >> 1) & 0x1);
 *c++ = ((bc[2] & 0x1) << 7)
        | ((Mc[2] & 0x3) << 5)
        | ((xmaxc[2] >> 1) & 0x1F);
 *c++ = ((xmaxc[2] & 0x1) << 7)
        | ((xmc[26] & 0x7) << 4)
        | ((xmc[27] & 0x7) << 1)
        | ((xmc[28] >> 2) & 0x1);
 *c++ = ((xmc[28] & 0x3) << 6)
        | ((xmc[29] & 0x7) << 3)
        | (xmc[30] & 0x7);
 *c++ = ((xmc[31] & 0x7) << 5)
        | ((xmc[32] & 0x7) << 2)
        | ((xmc[33] >> 1) & 0x3);
 *c++ = ((xmc[33] & 0x1) << 7)
        | ((xmc[34] & 0x7) << 4)
        | ((xmc[35] & 0x7) << 1)
        | ((xmc[36] >> 2) & 0x1);
 *c++ = ((xmc[36] & 0x3) << 6)
        | ((xmc[37] & 0x7) << 3)
        | (xmc[38] & 0x7);
 *c++ = ((Nc[3] & 0x7F) << 1)
        | ((bc[3] >> 1) & 0x1);
 *c++ = ((bc[3] & 0x1) << 7)
        | ((Mc[3] & 0x3) << 5)
        | ((xmaxc[3] >> 1) & 0x1F);
 *c++ = ((xmaxc[3] & 0x1) << 7)
        | ((xmc[39] & 0x7) << 4)
        | ((xmc[40] & 0x7) << 1)
        | ((xmc[41] >> 2) & 0x1);
 *c++ = ((xmc[41] & 0x3) << 6)
        | ((xmc[42] & 0x7) << 3)
        | (xmc[43] & 0x7);
 *c++ = ((xmc[44] & 0x7) << 5)
        | ((xmc[45] & 0x7) << 2)
        | ((xmc[46] >> 1) & 0x3);
 *c++ = ((xmc[46] & 0x1) << 7)
        | ((xmc[47] & 0x7) << 4)
        | ((xmc[48] & 0x7) << 1)
        | ((xmc[49] >> 2) & 0x1);
 *c++ = ((xmc[49] & 0x3) << 6)
        | ((xmc[50] & 0x7) << 3)
        | (xmc[51] & 0x7);
}








 #include "private.h"
 #include <stdio.h>






typedef struct gsm_state * gsm;
typedef short gsm_signal;
typedef unsigned char gsm_byte;
typedef gsm_byte gsm_frame[33];
extern gsm gsm_create (void);
extern void gsm_destroy (gsm);

extern int gsm_print (FILE *, gsm, gsm_byte *);
extern int gsm_option (gsm, int, int *);

extern void gsm_encode (gsm, gsm_signal *, gsm_byte *);
extern int gsm_decode (gsm, gsm_byte *, gsm_signal *);

extern int gsm_explode (gsm, gsm_byte *, gsm_signal *);
extern void gsm_implode (gsm, gsm_signal *, gsm_byte *);

int gsm_explode (gsm s, gsm_byte * c, gsm_signal * target)
{


 if (((*c >> 4) & 0x0F) != 0xD) return -1;



 target[0] = (*c++ & 0xF) << 2;
 target[0] |= (*c >> 6) & 0x3;
 target[1] = *c++ & 0x3F;
 target[2] = (*c >> 3) & 0x1F;
 target[3] = (*c++ & 0x7) << 2;
 target[3] |= (*c >> 6) & 0x3;
 target[4] = (*c >> 2) & 0xF;
 target[5] = (*c++ & 0x3) << 2;
 target[5] |= (*c >> 6) & 0x3;
 target[6] = (*c >> 3) & 0x7;
 target[7] = *c++ & 0x7;



 (target + 8)[0] = (*c >> 1) & 0x7F;



 (target + 9)[0] = (*c++ & 0x1) << 1;
 (target + 9)[0] |= (*c >> 7) & 0x1;



 (target + 10)[0] = (*c >> 5) & 0x3;



 (target + 11)[0] = (*c++ & 0x1F) << 1;
 (target + 11)[0] |= (*c >> 7) & 0x1;



 (target + 12)[0] = (*c >> 4) & 0x7;
 (target + 12)[1] = (*c >> 1) & 0x7;
 (target + 12)[2] = (*c++ & 0x1) << 2;
 (target + 12)[2] |= (*c >> 6) & 0x3;
 (target + 12)[3] = (*c >> 3) & 0x7;
 (target + 12)[4] = *c++ & 0x7;
 (target + 12)[5] = (*c >> 5) & 0x7;
 (target + 12)[6] = (*c >> 2) & 0x7;
 (target + 12)[7] = (*c++ & 0x3) << 1;
 (target + 12)[7] |= (*c >> 7) & 0x1;
 (target + 12)[8] = (*c >> 4) & 0x7;
 (target + 12)[9] = (*c >> 1) & 0x7;
 (target + 12)[10] = (*c++ & 0x1) << 2;
 (target + 12)[10] |= (*c >> 6) & 0x3;
 (target + 12)[11] = (*c >> 3) & 0x7;
 (target + 12)[12] = *c++ & 0x7;




 (target + 25 - 1)[1] = (*c >> 1) & 0x7F;




 (target + 26 - 1)[1] = (*c++ & 0x1) << 1;
 (target + 26 - 1)[1] |= (*c >> 7) & 0x1;




 (target + 27 - 1)[1] = (*c >> 5) & 0x3;




 (target + 28 - 1)[1] = (*c++ & 0x1F) << 1;
 (target + 28 - 1)[1] |= (*c >> 7) & 0x1;




 (target + 29 - 13)[13] = (*c >> 4) & 0x7;
 (target + 29 - 13)[14] = (*c >> 1) & 0x7;
 (target + 29 - 13)[15] = (*c++ & 0x1) << 2;
 (target + 29 - 13)[15] |= (*c >> 6) & 0x3;
 (target + 29 - 13)[16] = (*c >> 3) & 0x7;
 (target + 29 - 13)[17] = *c++ & 0x7;
 (target + 29 - 13)[18] = (*c >> 5) & 0x7;
 (target + 29 - 13)[19] = (*c >> 2) & 0x7;
 (target + 29 - 13)[20] = (*c++ & 0x3) << 1;
 (target + 29 - 13)[20] |= (*c >> 7) & 0x1;
 (target + 29 - 13)[21] = (*c >> 4) & 0x7;
 (target + 29 - 13)[22] = (*c >> 1) & 0x7;
 (target + 29 - 13)[23] = (*c++ & 0x1) << 2;
 (target + 29 - 13)[23] |= (*c >> 6) & 0x3;
 (target + 29 - 13)[24] = (*c >> 3) & 0x7;
 (target + 29 - 13)[25] = *c++ & 0x7;




 (target + 42 - 2)[2] = (*c >> 1) & 0x7F;




 (target + 43 - 2)[2] = (*c++ & 0x1) << 1;
 (target + 43 - 2)[2] |= (*c >> 7) & 0x1;




 (target + 44 - 2)[2] = (*c >> 5) & 0x3;




 (target + 45 - 2)[2] = (*c++ & 0x1F) << 1;
 (target + 45 - 2)[2] |= (*c >> 7) & 0x1;




 (target + 46 - 26)[26] = (*c >> 4) & 0x7;
 (target + 46 - 26)[27] = (*c >> 1) & 0x7;
 (target + 46 - 26)[28] = (*c++ & 0x1) << 2;
 (target + 46 - 26)[28] |= (*c >> 6) & 0x3;
 (target + 46 - 26)[29] = (*c >> 3) & 0x7;
 (target + 46 - 26)[30] = *c++ & 0x7;
 (target + 46 - 26)[31] = (*c >> 5) & 0x7;
 (target + 46 - 26)[32] = (*c >> 2) & 0x7;
 (target + 46 - 26)[33] = (*c++ & 0x3) << 1;
 (target + 46 - 26)[33] |= (*c >> 7) & 0x1;
 (target + 46 - 26)[34] = (*c >> 4) & 0x7;
 (target + 46 - 26)[35] = (*c >> 1) & 0x7;
 (target + 46 - 26)[36] = (*c++ & 0x1) << 2;
 (target + 46 - 26)[36] |= (*c >> 6) & 0x3;
 (target + 46 - 26)[37] = (*c >> 3) & 0x7;
 (target + 46 - 26)[38] = *c++ & 0x7;




 (target + 59 - 3)[3] = (*c >> 1) & 0x7F;




 (target + 60 - 3)[3] = (*c++ & 0x1) << 1;
 (target + 60 - 3)[3] |= (*c >> 7) & 0x1;




 (target + 61 - 3)[3] = (*c >> 5) & 0x3;




 (target + 62 - 3)[3] = (*c++ & 0x1F) << 1;
 (target + 62 - 3)[3] |= (*c >> 7) & 0x1;




 (target + 63 - 39)[39] = (*c >> 4) & 0x7;
 (target + 63 - 39)[40] = (*c >> 1) & 0x7;
 (target + 63 - 39)[41] = (*c++ & 0x1) << 2;
 (target + 63 - 39)[41] |= (*c >> 6) & 0x3;
 (target + 63 - 39)[42] = (*c >> 3) & 0x7;
 (target + 63 - 39)[43] = *c++ & 0x7;
 (target + 63 - 39)[44] = (*c >> 5) & 0x7;
 (target + 63 - 39)[45] = (*c >> 2) & 0x7;
 (target + 63 - 39)[46] = (*c++ & 0x3) << 1;
 (target + 63 - 39)[46] |= (*c >> 7) & 0x1;
 (target + 63 - 39)[47] = (*c >> 4) & 0x7;
 (target + 63 - 39)[48] = (*c >> 1) & 0x7;
 (target + 63 - 39)[49] = (*c++ & 0x1) << 2;
 (target + 63 - 39)[49] |= (*c >> 6) & 0x3;
 (target + 63 - 39)[50] = (*c >> 3) & 0x7;
 (target + 63 - 39)[51] = *c & 0x7;

 return 0;
}








 #include "private.h"

 #include <stdio.h>






typedef struct gsm_state * gsm;
typedef short gsm_signal;
typedef unsigned char gsm_byte;
typedef gsm_byte gsm_frame[33];
extern gsm gsm_create (void);
extern void gsm_destroy (gsm);

extern int gsm_print (FILE *, gsm, gsm_byte *);
extern int gsm_option (gsm, int, int *);

extern void gsm_encode (gsm, gsm_signal *, gsm_byte *);
extern int gsm_decode (gsm, gsm_byte *, gsm_signal *);

extern int gsm_explode (gsm, gsm_byte *, gsm_signal *);
extern void gsm_implode (gsm, gsm_signal *, gsm_byte *);

void gsm_implode (gsm s, gsm_signal * source, gsm_byte * c)
{
 *c++ = ((0xD & 0xF) << 4)
        | ((source[0] >> 2) & 0xF);
 *c++ = ((source[0] & 0x3) << 6)
        | (source[1] & 0x3F);
 *c++ = ((source[2] & 0x1F) << 3)
        | ((source[3] >> 2) & 0x7);
 *c++ = ((source[3] & 0x3) << 6)
        | ((source[4] & 0xF) << 2)
        | ((source[5] >> 2) & 0x3);
 *c++ = ((source[5] & 0x3) << 6)
        | ((source[6] & 0x7) << 3)
        | (source[7] & 0x7);



 *c++ = (((source + 8)[0] & 0x7F) << 1)



        | (((source + 9)[0] >> 1) & 0x1);
 *c++ = (((source + 9)[0] & 0x1) << 7)



        | (((source + 10)[0] & 0x3) << 5)



        | (((source + 11)[0] >> 1) & 0x1F);
 *c++ = (((source + 11)[0] & 0x1) << 7)



        | (((source + 12)[0] & 0x7) << 4)
        | (((source + 12)[1] & 0x7) << 1)
        | (((source + 12)[2] >> 2) & 0x1);
 *c++ = (((source + 12)[2] & 0x3) << 6)
        | (((source + 12)[3] & 0x7) << 3)
        | ((source + 12)[4] & 0x7);
 *c++ = (((source + 12)[5] & 0x7) << 5)
        | (((source + 12)[6] & 0x7) << 2)
        | (((source + 12)[7] >> 1) & 0x3);
 *c++ = (((source + 12)[7] & 0x1) << 7)
        | (((source + 12)[8] & 0x7) << 4)
        | (((source + 12)[9] & 0x7) << 1)
        | (((source + 12)[10] >> 2) & 0x1);
 *c++ = (((source + 12)[10] & 0x3) << 6)
        | (((source + 12)[11] & 0x7) << 3)
        | ((source + 12)[12] & 0x7);




 *c++ = (((source + 25 - 1)[1] & 0x7F) << 1)




        | (((source + 26 - 1)[1] >> 1) & 0x1);
 *c++ = (((source + 26 - 1)[1] & 0x1) << 7)




        | (((source + 27 - 1)[1] & 0x3) << 5)




        | (((source + 28 - 1)[1] >> 1) & 0x1F);
 *c++ = (((source + 28 - 1)[1] & 0x1) << 7)




        | (((source + 29 - 13)[13] & 0x7) << 4)
        | (((source + 29 - 13)[14] & 0x7) << 1)
        | (((source + 29 - 13)[15] >> 2) & 0x1);
 *c++ = (((source + 29 - 13)[15] & 0x3) << 6)
        | (((source + 29 - 13)[16] & 0x7) << 3)
        | ((source + 29 - 13)[17] & 0x7);
 *c++ = (((source + 29 - 13)[18] & 0x7) << 5)
        | (((source + 29 - 13)[19] & 0x7) << 2)
        | (((source + 29 - 13)[20] >> 1) & 0x3);
 *c++ = (((source + 29 - 13)[20] & 0x1) << 7)
        | (((source + 29 - 13)[21] & 0x7) << 4)
        | (((source + 29 - 13)[22] & 0x7) << 1)
        | (((source + 29 - 13)[23] >> 2) & 0x1);
 *c++ = (((source + 29 - 13)[23] & 0x3) << 6)
        | (((source + 29 - 13)[24] & 0x7) << 3)
        | ((source + 29 - 13)[25] & 0x7);




 *c++ = (((source + 42 - 2)[2] & 0x7F) << 1)




        | (((source + 43 - 2)[2] >> 1) & 0x1);
 *c++ = (((source + 43 - 2)[2] & 0x1) << 7)




        | (((source + 44 - 2)[2] & 0x3) << 5)




        | (((source + 45 - 2)[2] >> 1) & 0x1F);
 *c++ = (((source + 45 - 2)[2] & 0x1) << 7)




        | (((source + 46 - 26)[26] & 0x7) << 4)
        | (((source + 46 - 26)[27] & 0x7) << 1)
        | (((source + 46 - 26)[28] >> 2) & 0x1);
 *c++ = (((source + 46 - 26)[28] & 0x3) << 6)
        | (((source + 46 - 26)[29] & 0x7) << 3)
        | ((source + 46 - 26)[30] & 0x7);
 *c++ = (((source + 46 - 26)[31] & 0x7) << 5)
        | (((source + 46 - 26)[32] & 0x7) << 2)
        | (((source + 46 - 26)[33] >> 1) & 0x3);
 *c++ = (((source + 46 - 26)[33] & 0x1) << 7)
        | (((source + 46 - 26)[34] & 0x7) << 4)
        | (((source + 46 - 26)[35] & 0x7) << 1)
        | (((source + 46 - 26)[36] >> 2) & 0x1);
 *c++ = (((source + 46 - 26)[36] & 0x3) << 6)
        | (((source + 46 - 26)[37] & 0x7) << 3)
        | ((source + 46 - 26)[38] & 0x7);




 *c++ = (((source + 59 - 3)[3] & 0x7F) << 1)




        | (((source + 60 - 3)[3] >> 1) & 0x1);
 *c++ = (((source + 60 - 3)[3] & 0x1) << 7)




        | (((source + 61 - 3)[3] & 0x3) << 5)




        | (((source + 62 - 3)[3] >> 1) & 0x1F);
 *c++ = (((source + 62 - 3)[3] & 0x1) << 7)




        | (((source + 63 - 39)[39] & 0x7) << 4)
        | (((source + 63 - 39)[40] & 0x7) << 1)
        | (((source + 63 - 39)[41] >> 2) & 0x1);
 *c++ = (((source + 63 - 39)[41] & 0x3) << 6)
        | (((source + 63 - 39)[42] & 0x7) << 3)
        | ((source + 63 - 39)[43] & 0x7);
 *c++ = (((source + 63 - 39)[44] & 0x7) << 5)
        | (((source + 63 - 39)[45] & 0x7) << 2)
        | (((source + 63 - 39)[46] >> 1) & 0x3);
 *c++ = (((source + 63 - 39)[46] & 0x1) << 7)
        | (((source + 63 - 39)[47] & 0x7) << 4)
        | (((source + 63 - 39)[48] & 0x7) << 1)
        | (((source + 63 - 39)[49] >> 2) & 0x1);
 *c++ = (((source + 63 - 39)[49] & 0x3) << 6)
        | (((source + 63 - 39)[50] & 0x7) << 3)
        | ((source + 63 - 39)[51] & 0x7);
}








 #include "private.h"

 #include <stdio.h>






typedef struct gsm_state * gsm;
typedef short gsm_signal;
typedef unsigned char gsm_byte;
typedef gsm_byte gsm_frame[33];
extern gsm gsm_create (void);
extern void gsm_destroy (gsm);

extern int gsm_print (FILE *, gsm, gsm_byte *);
extern int gsm_option (gsm, int, int *);

extern void gsm_encode (gsm, gsm_signal *, gsm_byte *);
extern int gsm_decode (gsm, gsm_byte *, gsm_signal *);

extern int gsm_explode (gsm, gsm_byte *, gsm_signal *);
extern void gsm_implode (gsm, gsm_signal *, gsm_byte *);

int gsm_option (gsm r, int opt, int * val)
{
 int result = -1;

 switch (opt) {
 case 1:

  result = r->verbose;
  if (val) r->verbose = *val;

  break;

 case 2:





  break;

 default:
  break;
 }
 return result;
}








 #include <stdio.h>

 #include "private.h"

 #include <stdio.h>






typedef struct gsm_state * gsm;
typedef short gsm_signal;
typedef unsigned char gsm_byte;
typedef gsm_byte gsm_frame[33];
extern gsm gsm_create (void);
extern void gsm_destroy (gsm);

extern int gsm_print (FILE *, gsm, gsm_byte *);
extern int gsm_option (gsm, int, int *);

extern void gsm_encode (gsm, gsm_signal *, gsm_byte *);
extern int gsm_decode (gsm, gsm_byte *, gsm_signal *);

extern int gsm_explode (gsm, gsm_byte *, gsm_signal *);
extern void gsm_implode (gsm, gsm_signal *, gsm_byte *);

int gsm_print (FILE * f, gsm s, gsm_byte * c)
{
 word LARc[8], Nc[4], Mc[4], bc[4], xmaxc[4], xmc[13*4];



 if (((*c >> 4) & 0x0F) != 0xD) return -1;

 LARc[0] = (*c++ & 0xF) << 2;
 LARc[0] |= (*c >> 6) & 0x3;
 LARc[1] = *c++ & 0x3F;
 LARc[2] = (*c >> 3) & 0x1F;
 LARc[3] = (*c++ & 0x7) << 2;
 LARc[3] |= (*c >> 6) & 0x3;
 LARc[4] = (*c >> 2) & 0xF;
 LARc[5] = (*c++ & 0x3) << 2;
 LARc[5] |= (*c >> 6) & 0x3;
 LARc[6] = (*c >> 3) & 0x7;
 LARc[7] = *c++ & 0x7;


 Nc[0] = (*c >> 1) & 0x7F;
 bc[0] = (*c++ & 0x1) << 1;
 bc[0] |= (*c >> 7) & 0x1;
 Mc[0] = (*c >> 5) & 0x3;
 xmaxc[0] = (*c++ & 0x1F) << 1;
 xmaxc[0] |= (*c >> 7) & 0x1;
 xmc[0] = (*c >> 4) & 0x7;
 xmc[1] = (*c >> 1) & 0x7;
 xmc[2] = (*c++ & 0x1) << 2;
 xmc[2] |= (*c >> 6) & 0x3;
 xmc[3] = (*c >> 3) & 0x7;
 xmc[4] = *c++ & 0x7;
 xmc[5] = (*c >> 5) & 0x7;
 xmc[6] = (*c >> 2) & 0x7;
 xmc[7] = (*c++ & 0x3) << 1;
 xmc[7] |= (*c >> 7) & 0x1;
 xmc[8] = (*c >> 4) & 0x7;
 xmc[9] = (*c >> 1) & 0x7;
 xmc[10] = (*c++ & 0x1) << 2;
 xmc[10] |= (*c >> 6) & 0x3;
 xmc[11] = (*c >> 3) & 0x7;
 xmc[12] = *c++ & 0x7;

 Nc[1] = (*c >> 1) & 0x7F;
 bc[1] = (*c++ & 0x1) << 1;
 bc[1] |= (*c >> 7) & 0x1;
 Mc[1] = (*c >> 5) & 0x3;
 xmaxc[1] = (*c++ & 0x1F) << 1;
 xmaxc[1] |= (*c >> 7) & 0x1;
 xmc[13] = (*c >> 4) & 0x7;
 xmc[14] = (*c >> 1) & 0x7;
 xmc[15] = (*c++ & 0x1) << 2;
 xmc[15] |= (*c >> 6) & 0x3;
 xmc[16] = (*c >> 3) & 0x7;
 xmc[17] = *c++ & 0x7;
 xmc[18] = (*c >> 5) & 0x7;
 xmc[19] = (*c >> 2) & 0x7;
 xmc[20] = (*c++ & 0x3) << 1;
 xmc[20] |= (*c >> 7) & 0x1;
 xmc[21] = (*c >> 4) & 0x7;
 xmc[22] = (*c >> 1) & 0x7;
 xmc[23] = (*c++ & 0x1) << 2;
 xmc[23] |= (*c >> 6) & 0x3;
 xmc[24] = (*c >> 3) & 0x7;
 xmc[25] = *c++ & 0x7;


 Nc[2] = (*c >> 1) & 0x7F;
 bc[2] = (*c++ & 0x1) << 1;
 bc[2] |= (*c >> 7) & 0x1;
 Mc[2] = (*c >> 5) & 0x3;
 xmaxc[2] = (*c++ & 0x1F) << 1;
 xmaxc[2] |= (*c >> 7) & 0x1;
 xmc[26] = (*c >> 4) & 0x7;
 xmc[27] = (*c >> 1) & 0x7;
 xmc[28] = (*c++ & 0x1) << 2;
 xmc[28] |= (*c >> 6) & 0x3;
 xmc[29] = (*c >> 3) & 0x7;
 xmc[30] = *c++ & 0x7;
 xmc[31] = (*c >> 5) & 0x7;
 xmc[32] = (*c >> 2) & 0x7;
 xmc[33] = (*c++ & 0x3) << 1;
 xmc[33] |= (*c >> 7) & 0x1;
 xmc[34] = (*c >> 4) & 0x7;
 xmc[35] = (*c >> 1) & 0x7;
 xmc[36] = (*c++ & 0x1) << 2;
 xmc[36] |= (*c >> 6) & 0x3;
 xmc[37] = (*c >> 3) & 0x7;
 xmc[38] = *c++ & 0x7;

 Nc[3] = (*c >> 1) & 0x7F;
 bc[3] = (*c++ & 0x1) << 1;
 bc[3] |= (*c >> 7) & 0x1;
 Mc[3] = (*c >> 5) & 0x3;
 xmaxc[3] = (*c++ & 0x1F) << 1;
 xmaxc[3] |= (*c >> 7) & 0x1;

 xmc[39] = (*c >> 4) & 0x7;
 xmc[40] = (*c >> 1) & 0x7;
 xmc[41] = (*c++ & 0x1) << 2;
 xmc[41] |= (*c >> 6) & 0x3;
 xmc[42] = (*c >> 3) & 0x7;
 xmc[43] = *c++ & 0x7;
 xmc[44] = (*c >> 5) & 0x7;
 xmc[45] = (*c >> 2) & 0x7;
 xmc[46] = (*c++ & 0x3) << 1;
 xmc[46] |= (*c >> 7) & 0x1;
 xmc[47] = (*c >> 4) & 0x7;
 xmc[48] = (*c >> 1) & 0x7;
 xmc[49] = (*c++ & 0x1) << 2;
 xmc[49] |= (*c >> 6) & 0x3;
 xmc[50] = (*c >> 3) & 0x7;
 xmc[51] = *c & 0x7;

 fprintf(f,
       "LARc:\t%2.2d  %2.2d  %2.2d  %2.2d  %2.2d  %2.2d  %2.2d  %2.2d\n",
        LARc[0],LARc[1],LARc[2],LARc[3],LARc[4],LARc[5],LARc[6],LARc[7]);

 fprintf(f, "#1: 	Nc %4.4d    bc %d    Mc %d    xmaxc %d\n",
  Nc[0], bc[0], Mc[0], xmaxc[0]);
 fprintf(f,
"\t%.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d\n",
  xmc[0],xmc[1],xmc[2],xmc[3],xmc[4],xmc[5],xmc[6],
  xmc[7],xmc[8],xmc[9],xmc[10],xmc[11],xmc[12] );

 fprintf(f, "#2: 	Nc %4.4d    bc %d    Mc %d    xmaxc %d\n",
  Nc[1], bc[1], Mc[1], xmaxc[1]);
 fprintf(f,
"\t%.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d\n",
  xmc[13+0],xmc[13+1],xmc[13+2],xmc[13+3],xmc[13+4],xmc[13+5],
  xmc[13+6], xmc[13+7],xmc[13+8],xmc[13+9],xmc[13+10],xmc[13+11],
  xmc[13+12] );

 fprintf(f, "#3: 	Nc %4.4d    bc %d    Mc %d    xmaxc %d\n",
  Nc[2], bc[2], Mc[2], xmaxc[2]);
 fprintf(f,
"\t%.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d\n",
  xmc[26+0],xmc[26+1],xmc[26+2],xmc[26+3],xmc[26+4],xmc[26+5],
  xmc[26+6], xmc[26+7],xmc[26+8],xmc[26+9],xmc[26+10],xmc[26+11],
  xmc[26+12] );

 fprintf(f, "#4: 	Nc %4.4d    bc %d    Mc %d    xmaxc %d\n",
  Nc[3], bc[3], Mc[3], xmaxc[3]);
 fprintf(f,
"\t%.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d\n",
  xmc[39+0],xmc[39+1],xmc[39+2],xmc[39+3],xmc[39+4],xmc[39+5],
  xmc[39+6], xmc[39+7],xmc[39+8],xmc[39+9],xmc[39+10],xmc[39+11],
  xmc[39+12] );

 return 0;
}








 #include <stdio.h>
 #include <assert.h>

 #include "private.h"

 #include <stdio.h>






typedef struct gsm_state * gsm;
typedef short gsm_signal;
typedef unsigned char gsm_byte;
typedef gsm_byte gsm_frame[33];
extern gsm gsm_create (void);
extern void gsm_destroy (gsm);

extern int gsm_print (FILE *, gsm, gsm_byte *);
extern int gsm_option (gsm, int, int *);

extern void gsm_encode (gsm, gsm_signal *, gsm_byte *);
extern int gsm_decode (gsm, gsm_byte *, gsm_signal *);

extern int gsm_explode (gsm, gsm_byte *, gsm_signal *);
extern void gsm_implode (gsm, gsm_signal *, gsm_byte *);
static void Calculation_of_the_LTP_parameters (register word * d, register word * dp, word * bc_out, word * Nc_out)





{
 register int k, lambda;
 word Nc, bc;
 word wt[40];

 longword L_max, L_power;
 word R, S, dmax, scal;
 register word temp;



 dmax = 0;

 for (k = 0; k <= 39; k++) {
  temp = d[k];
  temp = GSM_ABS( temp );
  if (temp > dmax) dmax = temp;
 }

 temp = 0;
 if (dmax == 0) scal = 0;
 else {
  assert(dmax > 0);
  temp = gsm_norm( (longword)dmax << 16 );
 }

 if (temp > 6) scal = 0;
 else scal = 6 - temp;

 assert(scal >= 0);




 for (k = 0; k <= 39; k++) wt[k] = SASR( d[k], scal );



 L_max = 0;
 Nc = 40;

 for (lambda = 40; lambda <= 120; lambda++) {
  register longword L_result;

  L_result = (wt[0] * dp[0 - lambda]) ; L_result += (wt[1] * dp[1 - lambda]) ;
  L_result += (wt[2] * dp[2 - lambda]) ; L_result += (wt[3] * dp[3 - lambda]) ;
  L_result += (wt[4] * dp[4 - lambda]) ; L_result += (wt[5] * dp[5 - lambda]) ;
  L_result += (wt[6] * dp[6 - lambda]) ; L_result += (wt[7] * dp[7 - lambda]) ;
  L_result += (wt[8] * dp[8 - lambda]) ; L_result += (wt[9] * dp[9 - lambda]) ;
  L_result += (wt[10] * dp[10 - lambda]) ; L_result += (wt[11] * dp[11 - lambda]) ;
  L_result += (wt[12] * dp[12 - lambda]) ; L_result += (wt[13] * dp[13 - lambda]) ;
  L_result += (wt[14] * dp[14 - lambda]) ; L_result += (wt[15] * dp[15 - lambda]) ;
  L_result += (wt[16] * dp[16 - lambda]) ; L_result += (wt[17] * dp[17 - lambda]) ;
  L_result += (wt[18] * dp[18 - lambda]) ; L_result += (wt[19] * dp[19 - lambda]) ;
  L_result += (wt[20] * dp[20 - lambda]) ; L_result += (wt[21] * dp[21 - lambda]) ;
  L_result += (wt[22] * dp[22 - lambda]) ; L_result += (wt[23] * dp[23 - lambda]) ;
  L_result += (wt[24] * dp[24 - lambda]) ; L_result += (wt[25] * dp[25 - lambda]) ;
  L_result += (wt[26] * dp[26 - lambda]) ; L_result += (wt[27] * dp[27 - lambda]) ;
  L_result += (wt[28] * dp[28 - lambda]) ; L_result += (wt[29] * dp[29 - lambda]) ;
  L_result += (wt[30] * dp[30 - lambda]) ; L_result += (wt[31] * dp[31 - lambda]) ;
  L_result += (wt[32] * dp[32 - lambda]) ; L_result += (wt[33] * dp[33 - lambda]) ;
  L_result += (wt[34] * dp[34 - lambda]) ; L_result += (wt[35] * dp[35 - lambda]) ;
  L_result += (wt[36] * dp[36 - lambda]) ; L_result += (wt[37] * dp[37 - lambda]) ;
  L_result += (wt[38] * dp[38 - lambda]) ; L_result += (wt[39] * dp[39 - lambda]) ;

  if (L_result > L_max) {

   Nc = lambda;
   L_max = L_result;
  }
 }

 *Nc_out = Nc;

 L_max <<= 1;



 assert(scal <= 100 && scal >= -100);
 L_max = L_max >> (6 - scal);

 assert( Nc <= 120 && Nc >= 40);




 L_power = 0;
 for (k = 0; k <= 39; k++) {

  register longword L_temp;

  L_temp = SASR( dp[k - Nc], 3 );
  L_power += L_temp * L_temp;
 }
 L_power <<= 1;




 if (L_max <= 0) {
  *bc_out = 0;
  return;
 }
 if (L_max >= L_power) {
  *bc_out = 3;
  return;
 }

 temp = gsm_norm( L_power );

 R = SASR( L_max << temp, 16 );
 S = SASR( L_power << temp, 16 );







 for (bc = 0; bc <= 2; bc++) if (R <= gsm_mult(S, gsm_DLB[bc])) break;
 *bc_out = bc;
}
static void Long_term_analysis_filtering (word bc, word Nc, register word * dp, register word * d, register word * dpp, register word * e)
{
 register int k;
 register longword ltmp;
 switch (bc) {
 case 0: for (k = 0; k <= 39; k++) { dpp[k] = GSM_MULT_R( 3277, dp[k - Nc]); e[k] = GSM_SUB( d[k], dpp[k] ); }; break;
 case 1: for (k = 0; k <= 39; k++) { dpp[k] = GSM_MULT_R( 11469, dp[k - Nc]); e[k] = GSM_SUB( d[k], dpp[k] ); }; break;
 case 2: for (k = 0; k <= 39; k++) { dpp[k] = GSM_MULT_R( 21299, dp[k - Nc]); e[k] = GSM_SUB( d[k], dpp[k] ); }; break;
 case 3: for (k = 0; k <= 39; k++) { dpp[k] = GSM_MULT_R( 32767, dp[k - Nc]); e[k] = GSM_SUB( d[k], dpp[k] ); }; break;
 }
}

void Gsm_Long_Term_Predictor (struct gsm_state * S, word * d, word * dp, word * e, word * dpp, word * Nc, word * bc)
{
 assert( d ); assert( dp ); assert( e );
 assert( dpp); assert( Nc ); assert( bc );






  Calculation_of_the_LTP_parameters( d, dp, bc, Nc );

 Long_term_analysis_filtering( *bc, *Nc, dp, d, dpp, e );
}


void Gsm_Long_Term_Synthesis_Filtering (struct gsm_state * S, word Ncr, word bcr, register word * erp, register word * drp)
{
 register longword ltmp;
 register int k;
 word brp, drpp, Nr;



 Nr = Ncr < 40 || Ncr > 120 ? S->nrp : Ncr;
 S->nrp = Nr;
 assert(Nr >= 40 && Nr <= 120);



 brp = gsm_QLB[ bcr ];




 assert(brp != MIN_WORD);

 for (k = 0; k <= 39; k++) {
  drpp = GSM_MULT_R( brp, drp[ k - Nr ] );
  drp[k] = GSM_ADD( erp[k], drpp );
 }






 for (k = 0; k <= 119; k++) drp[ -120 + k ] = drp[ -80 + k ];
}








 #include <stdio.h>
 #include <assert.h>

 #include "private.h"

 #include <stdio.h>






typedef struct gsm_state * gsm;
typedef short gsm_signal;
typedef unsigned char gsm_byte;
typedef gsm_byte gsm_frame[33];
extern gsm gsm_create (void);
extern void gsm_destroy (gsm);

extern int gsm_print (FILE *, gsm, gsm_byte *);
extern int gsm_option (gsm, int, int *);

extern void gsm_encode (gsm, gsm_signal *, gsm_byte *);
extern int gsm_decode (gsm, gsm_byte *, gsm_signal *);

extern int gsm_explode (gsm, gsm_byte *, gsm_signal *);
extern void gsm_implode (gsm, gsm_signal *, gsm_byte *);
static void Autocorrelation (word * s, longword * L_ACF)






{
 register int k, i;

 word temp, smax, scalauto;
 smax = 0;
 for (k = 0; k <= 159; k++) {
  temp = GSM_ABS( s[k] );
  if (temp > smax) smax = temp;
 }



 if (smax == 0) scalauto = 0;
 else {
  assert(smax > 0);
  scalauto = 4 - gsm_norm( (longword)smax << 16 );
 }




 if (scalauto > 0) {
  switch (scalauto) {
  case 1: for (k = 0; k <= 159; k++) s[k] = GSM_MULT_R( s[k], 16384 >> (1 -1) ); break;
  case 2: for (k = 0; k <= 159; k++) s[k] = GSM_MULT_R( s[k], 16384 >> (2 -1) ); break;
  case 3: for (k = 0; k <= 159; k++) s[k] = GSM_MULT_R( s[k], 16384 >> (3 -1) ); break;
  case 4: for (k = 0; k <= 159; k++) s[k] = GSM_MULT_R( s[k], 16384 >> (4 -1) ); break;
  }

 }






 {






  word * sp = s;
  word sl = *sp;







 for (k = 9; k--; L_ACF[k] = 0) ;

 L_ACF[0] += ((longword)sl * sp[ -(0) ]);;
 sl = *++sp;
 L_ACF[0] += ((longword)sl * sp[ -(0) ]);; L_ACF[1] += ((longword)sl * sp[ -(1) ]);;
 sl = *++sp;
 L_ACF[0] += ((longword)sl * sp[ -(0) ]);; L_ACF[1] += ((longword)sl * sp[ -(1) ]);; L_ACF[2] += ((longword)sl * sp[ -(2) ]);;
 sl = *++sp;
 L_ACF[0] += ((longword)sl * sp[ -(0) ]);; L_ACF[1] += ((longword)sl * sp[ -(1) ]);; L_ACF[2] += ((longword)sl * sp[ -(2) ]);; L_ACF[3] += ((longword)sl * sp[ -(3) ]);;
 sl = *++sp;
 L_ACF[0] += ((longword)sl * sp[ -(0) ]);; L_ACF[1] += ((longword)sl * sp[ -(1) ]);; L_ACF[2] += ((longword)sl * sp[ -(2) ]);; L_ACF[3] += ((longword)sl * sp[ -(3) ]);; L_ACF[4] += ((longword)sl * sp[ -(4) ]);;
 sl = *++sp;
 L_ACF[0] += ((longword)sl * sp[ -(0) ]);; L_ACF[1] += ((longword)sl * sp[ -(1) ]);; L_ACF[2] += ((longword)sl * sp[ -(2) ]);; L_ACF[3] += ((longword)sl * sp[ -(3) ]);; L_ACF[4] += ((longword)sl * sp[ -(4) ]);; L_ACF[5] += ((longword)sl * sp[ -(5) ]);;
 sl = *++sp;
 L_ACF[0] += ((longword)sl * sp[ -(0) ]);; L_ACF[1] += ((longword)sl * sp[ -(1) ]);; L_ACF[2] += ((longword)sl * sp[ -(2) ]);; L_ACF[3] += ((longword)sl * sp[ -(3) ]);; L_ACF[4] += ((longword)sl * sp[ -(4) ]);; L_ACF[5] += ((longword)sl * sp[ -(5) ]);; L_ACF[6] += ((longword)sl * sp[ -(6) ]);;
 sl = *++sp;
 L_ACF[0] += ((longword)sl * sp[ -(0) ]);; L_ACF[1] += ((longword)sl * sp[ -(1) ]);; L_ACF[2] += ((longword)sl * sp[ -(2) ]);; L_ACF[3] += ((longword)sl * sp[ -(3) ]);; L_ACF[4] += ((longword)sl * sp[ -(4) ]);; L_ACF[5] += ((longword)sl * sp[ -(5) ]);; L_ACF[6] += ((longword)sl * sp[ -(6) ]);; L_ACF[7] += ((longword)sl * sp[ -(7) ]);;

 for (i = 8; i <= 159; i++) {

  sl = *++sp;

  L_ACF[0] += ((longword)sl * sp[ -(0) ]);;
  L_ACF[1] += ((longword)sl * sp[ -(1) ]);; L_ACF[2] += ((longword)sl * sp[ -(2) ]);; L_ACF[3] += ((longword)sl * sp[ -(3) ]);; L_ACF[4] += ((longword)sl * sp[ -(4) ]);;
  L_ACF[5] += ((longword)sl * sp[ -(5) ]);; L_ACF[6] += ((longword)sl * sp[ -(6) ]);; L_ACF[7] += ((longword)sl * sp[ -(7) ]);; L_ACF[8] += ((longword)sl * sp[ -(8) ]);;
 }

 for (k = 9; k--; L_ACF[k] <<= 1) ;

 }


 if (scalauto > 0) {
  assert(scalauto <= 4);
  for (k = 160; k--; *s++ <<= scalauto) ;
 }
}
static void Reflection_coefficients (longword * L_ACF, register word * r)



{
 register int i, m, n;
 register word temp;
 register longword ltmp;
 word ACF[9];
 word P[ 9];
 word K[ 9];




 if (L_ACF[0] == 0) {
  for (i = 8; i--; *r++ = 0) ;
  return;
 }

 assert( L_ACF[0] != 0 );
 temp = gsm_norm( L_ACF[0] );

 assert(temp >= 0 && temp < 32);


 for (i = 0; i <= 8; i++) ACF[i] = SASR( L_ACF[i] << temp, 16 );




 for (i = 1; i <= 7; i++) K[ i ] = ACF[ i ];
 for (i = 0; i <= 8; i++) P[ i ] = ACF[ i ];



 for (n = 1; n <= 8; n++, r++) {

  temp = P[1];
  temp = GSM_ABS(temp);
  if (P[0] < temp) {
   for (i = n; i <= 8; i++) *r++ = 0;
   return;
  }

  *r = gsm_div( temp, P[0] );

  assert(*r >= 0);
  if (P[1] > 0) *r = -*r;
  assert (*r != MIN_WORD);
  if (n == 8) return;



  temp = GSM_MULT_R( P[1], *r );
  P[0] = GSM_ADD( P[0], temp );

  for (m = 1; m <= 8 - n; m++) {
   temp = GSM_MULT_R( K[ m ], *r );
   P[m] = GSM_ADD( P[ m+1 ], temp );

   temp = GSM_MULT_R( P[ m+1 ], *r );
   K[m] = GSM_ADD( K[ m ], temp );
  }
 }
}



static void Transformation_to_Log_Area_Ratios (register word * r)
{
 register word temp;
 register int i;




 for (i = 1; i <= 8; i++, r++) {

  temp = *r;
  temp = GSM_ABS(temp);
  assert(temp >= 0);

  if (temp < 22118) {
   temp >>= 1;
  } else if (temp < 31130) {
   assert( temp >= 11059 );
   temp -= 11059;
  } else {
   assert( temp >= 26112 );
   temp -= 26112;
   temp <<= 2;
  }

  *r = *r < 0 ? -temp : temp;
  assert( *r != MIN_WORD );
 }
}



static void Quantization_and_coding (register word * LAR)


{
 register word temp;
 longword ltmp;
 temp = GSM_MULT( 20480, *LAR ); temp = GSM_ADD( temp, 0 ); temp = GSM_ADD( temp, 256 ); temp = SASR( temp, 9 ); *LAR = temp>31 ? 31 - -32 : (temp<-32 ? 0 : temp - -32); LAR++;;
 temp = GSM_MULT( 20480, *LAR ); temp = GSM_ADD( temp, 0 ); temp = GSM_ADD( temp, 256 ); temp = SASR( temp, 9 ); *LAR = temp>31 ? 31 - -32 : (temp<-32 ? 0 : temp - -32); LAR++;;
 temp = GSM_MULT( 20480, *LAR ); temp = GSM_ADD( temp, 2048 ); temp = GSM_ADD( temp, 256 ); temp = SASR( temp, 9 ); *LAR = temp>15 ? 15 - -16 : (temp<-16 ? 0 : temp - -16); LAR++;;
 temp = GSM_MULT( 20480, *LAR ); temp = GSM_ADD( temp, -2560 ); temp = GSM_ADD( temp, 256 ); temp = SASR( temp, 9 ); *LAR = temp>15 ? 15 - -16 : (temp<-16 ? 0 : temp - -16); LAR++;;

 temp = GSM_MULT( 13964, *LAR ); temp = GSM_ADD( temp, 94 ); temp = GSM_ADD( temp, 256 ); temp = SASR( temp, 9 ); *LAR = temp>7 ? 7 - -8 : (temp<-8 ? 0 : temp - -8); LAR++;;
 temp = GSM_MULT( 15360, *LAR ); temp = GSM_ADD( temp, -1792 ); temp = GSM_ADD( temp, 256 ); temp = SASR( temp, 9 ); *LAR = temp>7 ? 7 - -8 : (temp<-8 ? 0 : temp - -8); LAR++;;
 temp = GSM_MULT( 8534, *LAR ); temp = GSM_ADD( temp, -341 ); temp = GSM_ADD( temp, 256 ); temp = SASR( temp, 9 ); *LAR = temp>3 ? 3 - -4 : (temp<-4 ? 0 : temp - -4); LAR++;;
 temp = GSM_MULT( 9036, *LAR ); temp = GSM_ADD( temp, -1144 ); temp = GSM_ADD( temp, 256 ); temp = SASR( temp, 9 ); *LAR = temp>3 ? 3 - -4 : (temp<-4 ? 0 : temp - -4); LAR++;;


}

void Gsm_LPC_Analysis (struct gsm_state *S, word * s, word * LARc)



{
 longword L_ACF[9];





 Autocorrelation (s, L_ACF );
 Reflection_coefficients (L_ACF, LARc );
 Transformation_to_Log_Area_Ratios (LARc);
 Quantization_and_coding (LARc);
}








 #include <stdio.h>
 #include <assert.h>

 #include "private.h"

 #include <stdio.h>






typedef struct gsm_state * gsm;
typedef short gsm_signal;
typedef unsigned char gsm_byte;
typedef gsm_byte gsm_frame[33];
extern gsm gsm_create (void);
extern void gsm_destroy (gsm);

extern int gsm_print (FILE *, gsm, gsm_byte *);
extern int gsm_option (gsm, int, int *);

extern void gsm_encode (gsm, gsm_signal *, gsm_byte *);
extern int gsm_decode (gsm, gsm_byte *, gsm_signal *);

extern int gsm_explode (gsm, gsm_byte *, gsm_signal *);
extern void gsm_implode (gsm, gsm_signal *, gsm_byte *);
void Gsm_Preprocess (struct gsm_state * S, word * s, word * so)



{

 word z1 = S->z1;
 longword L_z2 = S->L_z2;
 word mp = S->mp;

 word s1;
 longword L_s2;

 longword L_temp;

 word msp, lsp;
 word SO;

 longword ltmp;
 ulongword utmp;

 register int k = 160;

 while (k--) {



  SO = SASR( *s, 3 ) << 2;
  s++;

  assert (SO >= -0x4000);
  assert (SO <= 0x3FFC);
  s1 = SO - z1;
  z1 = SO;

  assert(s1 != MIN_WORD);



  L_s2 = s1;
  L_s2 <<= 15;




  msp = SASR( L_z2, 15 );
  lsp = L_z2-((longword)msp<<15);

  L_s2 += GSM_MULT_R( lsp, 32735 );
  L_temp = (longword)msp * 32735;
  L_z2 = GSM_L_ADD( L_temp, L_s2 );



  L_temp = GSM_L_ADD( L_z2, 16384 );




  msp = GSM_MULT_R( mp, -28180 );
  mp = SASR( L_temp, 15 );
  *so++ = GSM_ADD( mp, msp );
 }

 S->z1 = z1;
 S->L_z2 = L_z2;
 S->mp = mp;
}








 #include <stdio.h>
 #include <assert.h>

 #include "private.h"

 #include <stdio.h>






typedef struct gsm_state * gsm;
typedef short gsm_signal;
typedef unsigned char gsm_byte;
typedef gsm_byte gsm_frame[33];
extern gsm gsm_create (void);
extern void gsm_destroy (gsm);

extern int gsm_print (FILE *, gsm, gsm_byte *);
extern int gsm_option (gsm, int, int *);

extern void gsm_encode (gsm, gsm_signal *, gsm_byte *);
extern int gsm_decode (gsm, gsm_byte *, gsm_signal *);

extern int gsm_explode (gsm, gsm_byte *, gsm_signal *);
extern void gsm_implode (gsm, gsm_signal *, gsm_byte *);






static void Weighting_filter (register word * e, word * x)
{


 register longword L_result;
 register int k ;
 e -= 5;



 for (k = 0; k <= 39; k++) {

  L_result = 8192 >> 1;
  L_result +=
    (e[ k + 0 ] * (longword)-134)
  + (e[ k + 1 ] * (longword)-374)

  + (e[ k + 3 ] * (longword)2054)
  + (e[ k + 4 ] * (longword)5741)
  + (e[ k + 5 ] * (longword)8192)
  + (e[ k + 6 ] * (longword)5741)
  + (e[ k + 7 ] * (longword)2054)

  + (e[ k + 9 ] * (longword)-374)
  + (e[ k + 10 ] * (longword)-134)
  ;
  L_result = SASR( L_result, 13 );
  x[k] = ( L_result < MIN_WORD ? MIN_WORD
   : (L_result > MAX_WORD ? MAX_WORD : L_result ));
 }
}



static void RPE_grid_selection (word * x, word * xM, word * Mc_out)
{

 register int i;
 register longword L_result, L_temp;
 longword EM;
 word Mc;

 longword L_common_0_3;

 EM = 0;
 Mc = 0;
 L_result = 0;
 L_temp = SASR( x[0 + 3 * 1], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[0 + 3 * 2], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[0 + 3 * 3], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[0 + 3 * 4], 2 ); L_result += L_temp * L_temp;;
 L_temp = SASR( x[0 + 3 * 5], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[0 + 3 * 6], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[0 + 3 * 7], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[0 + 3 * 8], 2 ); L_result += L_temp * L_temp;;
 L_temp = SASR( x[0 + 3 * 9], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[0 + 3 * 10], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[0 + 3 * 11], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[0 + 3 * 12], 2 ); L_result += L_temp * L_temp;;
 L_common_0_3 = L_result;



 L_temp = SASR( x[0 + 3 * 0], 2 ); L_result += L_temp * L_temp;;
 L_result <<= 1;
 EM = L_result;



 L_result = 0;
 L_temp = SASR( x[1 + 3 * 0], 2 ); L_result += L_temp * L_temp;;
 L_temp = SASR( x[1 + 3 * 1], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[1 + 3 * 2], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[1 + 3 * 3], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[1 + 3 * 4], 2 ); L_result += L_temp * L_temp;;
 L_temp = SASR( x[1 + 3 * 5], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[1 + 3 * 6], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[1 + 3 * 7], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[1 + 3 * 8], 2 ); L_result += L_temp * L_temp;;
 L_temp = SASR( x[1 + 3 * 9], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[1 + 3 * 10], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[1 + 3 * 11], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[1 + 3 * 12], 2 ); L_result += L_temp * L_temp;;
 L_result <<= 1;
 if (L_result > EM) {
  Mc = 1;
   EM = L_result;
 }



 L_result = 0;
 L_temp = SASR( x[2 + 3 * 0], 2 ); L_result += L_temp * L_temp;;
 L_temp = SASR( x[2 + 3 * 1], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[2 + 3 * 2], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[2 + 3 * 3], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[2 + 3 * 4], 2 ); L_result += L_temp * L_temp;;
 L_temp = SASR( x[2 + 3 * 5], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[2 + 3 * 6], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[2 + 3 * 7], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[2 + 3 * 8], 2 ); L_result += L_temp * L_temp;;
 L_temp = SASR( x[2 + 3 * 9], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[2 + 3 * 10], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[2 + 3 * 11], 2 ); L_result += L_temp * L_temp;; L_temp = SASR( x[2 + 3 * 12], 2 ); L_result += L_temp * L_temp;;
 L_result <<= 1;
 if (L_result > EM) {
  Mc = 2;
   EM = L_result;
 }



 L_result = L_common_0_3;
 L_temp = SASR( x[3 + 3 * 12], 2 ); L_result += L_temp * L_temp;;
 L_result <<= 1;
 if (L_result > EM) {
  Mc = 3;
   EM = L_result;
 }






 for (i = 0; i <= 12; i ++) xM[i] = x[Mc + 3*i];
 *Mc_out = Mc;
}



static void APCM_quantization_xmaxc_to_exp_mant (word xmaxc, word * exp_out, word * mant_out)



{
 word exp, mant;




 exp = 0;
 if (xmaxc > 15) exp = SASR(xmaxc, 3) - 1;
 mant = xmaxc - (exp << 3);

 if (mant == 0) {
  exp = -4;
  mant = 7;
 }
 else {
  while (mant <= 7) {
   mant = mant << 1 | 1;
   exp--;
  }
  mant -= 8;
 }

 assert( exp >= -4 && exp <= 6 );
 assert( mant >= 0 && mant <= 7 );

 *exp_out = exp;
 *mant_out = mant;
}

static void APCM_quantization (word * xM, word * xMc, word * mant_out, word * exp_out, word * xmaxc_out)







{
 int i, itest;

 word xmax, xmaxc, temp, temp1, temp2;
 word exp, mant;





 xmax = 0;
 for (i = 0; i <= 12; i++) {
  temp = xM[i];
  temp = GSM_ABS(temp);
  if (temp > xmax) xmax = temp;
 }




 exp = 0;
 temp = SASR( xmax, 9 );
 itest = 0;

 for (i = 0; i <= 5; i++) {

  itest |= (temp <= 0);
  temp = SASR( temp, 1 );

  assert(exp <= 5);
  if (itest == 0) exp++;
 }

 assert(exp <= 6 && exp >= 0);
 temp = exp + 5;

 assert(temp <= 11 && temp >= 0);
 xmaxc = gsm_add( SASR(xmax, temp), exp << 3 );





 APCM_quantization_xmaxc_to_exp_mant( xmaxc, &exp, &mant );
 assert( exp <= 4096 && exp >= -4096);
 assert( mant >= 0 && mant <= 7 );

 temp1 = 6 - exp;
 temp2 = gsm_NRFAC[ mant ];

 for (i = 0; i <= 12; i++) {

  assert(temp1 >= 0 && temp1 < 16);

  temp = xM[i] << temp1;
  temp = GSM_MULT( temp, temp2 );
  temp = SASR(temp, 12);
  xMc[i] = temp + 4;
 }




 *mant_out = mant;
 *exp_out = exp;
 *xmaxc_out = xmaxc;
}



static void APCM_inverse_quantization (register word * xMc, word mant, word exp, register word * xMp)
{
 int i;
 word temp, temp1, temp2, temp3;
 longword ltmp;

 assert( mant >= 0 && mant <= 7 );

 temp1 = gsm_FAC[ mant ];
 temp2 = gsm_sub( 6, exp );
 temp3 = gsm_asl( 1, gsm_sub( temp2, 1 ));

 for (i = 13; i--;) {

  assert( *xMc <= 7 && *xMc >= 0 );


  temp = (*xMc++ << 1) - 7;
  assert( temp <= 7 && temp >= -7 );

  temp <<= 12;
  temp = GSM_MULT_R( temp1, temp );
  temp = GSM_ADD( temp, temp3 );
  *xMp++ = gsm_asr( temp, temp2 );
 }
}



static void RPE_grid_positioning (word Mc, register word * xMp, register word * ep)
{
 int i = 13;

 assert(0 <= Mc && Mc <= 3);

        switch (Mc) {
                case 3: *ep++ = 0;
                case 2: do {
                                *ep++ = 0;
                case 1: *ep++ = 0;
                case 0: *ep++ = *xMp++;
                         } while (--i);
        }
        while (++Mc < 4) *ep++ = 0;
}
void Gsm_RPE_Encoding (struct gsm_state * S, word * e, word * xmaxc, word * Mc, word * xMc)







{
 word x[40];
 word xM[13], xMp[13];
 word mant, exp;

 Weighting_filter(e, x);
 RPE_grid_selection(x, xM, Mc);

 APCM_quantization( xM, xMc, &mant, &exp, xmaxc);
 APCM_inverse_quantization( xMc, mant, exp, xMp);

 RPE_grid_positioning( *Mc, xMp, e );

}

void Gsm_RPE_Decoding (struct gsm_state * S, word xmaxcr, word Mcr, word * xMcr, word * erp)







{
 word exp, mant;
 word xMp[ 13 ];

 APCM_quantization_xmaxc_to_exp_mant( xmaxcr, &exp, &mant );
 APCM_inverse_quantization( xMcr, mant, exp, xMp );
 RPE_grid_positioning( Mcr, xMp, erp );

}








 #include <stdio.h>
 #include <assert.h>

 #include "private.h"

 #include <stdio.h>






typedef struct gsm_state * gsm;
typedef short gsm_signal;
typedef unsigned char gsm_byte;
typedef gsm_byte gsm_frame[33];
extern gsm gsm_create (void);
extern void gsm_destroy (gsm);

extern int gsm_print (FILE *, gsm, gsm_byte *);
extern int gsm_option (gsm, int, int *);

extern void gsm_encode (gsm, gsm_signal *, gsm_byte *);
extern int gsm_decode (gsm, gsm_byte *, gsm_signal *);

extern int gsm_explode (gsm, gsm_byte *, gsm_signal *);
extern void gsm_implode (gsm, gsm_signal *, gsm_byte *);







static void Decoding_of_the_coded_Log_Area_Ratios (word * LARc, word * LARpp)


{
 register word temp1 ;
 register long ltmp;
 temp1 = GSM_ADD( *LARc++, -32 ) << 10; temp1 = GSM_SUB( temp1, 0 << 1 ); temp1 = GSM_MULT_R( 13107, temp1 ); *LARpp++ = GSM_ADD( temp1, temp1 );;
 temp1 = GSM_ADD( *LARc++, -32 ) << 10; temp1 = GSM_SUB( temp1, 0 << 1 ); temp1 = GSM_MULT_R( 13107, temp1 ); *LARpp++ = GSM_ADD( temp1, temp1 );;
 temp1 = GSM_ADD( *LARc++, -16 ) << 10; temp1 = GSM_SUB( temp1, 2048 << 1 ); temp1 = GSM_MULT_R( 13107, temp1 ); *LARpp++ = GSM_ADD( temp1, temp1 );;
 temp1 = GSM_ADD( *LARc++, -16 ) << 10; temp1 = GSM_SUB( temp1, -2560 << 1 ); temp1 = GSM_MULT_R( 13107, temp1 ); *LARpp++ = GSM_ADD( temp1, temp1 );;

 temp1 = GSM_ADD( *LARc++, -8 ) << 10; temp1 = GSM_SUB( temp1, 94 << 1 ); temp1 = GSM_MULT_R( 19223, temp1 ); *LARpp++ = GSM_ADD( temp1, temp1 );;
 temp1 = GSM_ADD( *LARc++, -8 ) << 10; temp1 = GSM_SUB( temp1, -1792 << 1 ); temp1 = GSM_MULT_R( 17476, temp1 ); *LARpp++ = GSM_ADD( temp1, temp1 );;
 temp1 = GSM_ADD( *LARc++, -4 ) << 10; temp1 = GSM_SUB( temp1, -341 << 1 ); temp1 = GSM_MULT_R( 31454, temp1 ); *LARpp++ = GSM_ADD( temp1, temp1 );;
 temp1 = GSM_ADD( *LARc++, -4 ) << 10; temp1 = GSM_SUB( temp1, -1144 << 1 ); temp1 = GSM_MULT_R( 29708, temp1 ); *LARpp++ = GSM_ADD( temp1, temp1 );;




}
static void Coefficients_0_12 (register word * LARpp_j_1, register word * LARpp_j, register word * LARp)



{
 register int i;
 register longword ltmp;

 for (i = 1; i <= 8; i++, LARp++, LARpp_j_1++, LARpp_j++) {
  *LARp = GSM_ADD( SASR( *LARpp_j_1, 2 ), SASR( *LARpp_j, 2 ));
  *LARp = GSM_ADD( *LARp, SASR( *LARpp_j_1, 1));
 }
}

static void Coefficients_13_26 (register word * LARpp_j_1, register word * LARpp_j, register word * LARp)



{
 register int i;
 register longword ltmp;
 for (i = 1; i <= 8; i++, LARpp_j_1++, LARpp_j++, LARp++) {
  *LARp = GSM_ADD( SASR( *LARpp_j_1, 1), SASR( *LARpp_j, 1 ));
 }
}

static void Coefficients_27_39 (register word * LARpp_j_1, register word * LARpp_j, register word * LARp)



{
 register int i;
 register longword ltmp;

 for (i = 1; i <= 8; i++, LARpp_j_1++, LARpp_j++, LARp++) {
  *LARp = GSM_ADD( SASR( *LARpp_j_1, 2 ), SASR( *LARpp_j, 2 ));
  *LARp = GSM_ADD( *LARp, SASR( *LARpp_j, 1 ));
 }
}


static void Coefficients_40_159 (register word * LARpp_j, register word * LARp)


{
 register int i;

 for (i = 1; i <= 8; i++, LARp++, LARpp_j++)
  *LARp = *LARpp_j;
}



static void LARp_to_rp (register word * LARp)






{
 register int i;
 register word temp;
 register longword ltmp;

 for (i = 1; i <= 8; i++, LARp++) {
  if (*LARp < 0) {
   temp = *LARp == MIN_WORD ? MAX_WORD : -(*LARp);
   *LARp = - ((temp < 11059) ? temp << 1
    : ((temp < 20070) ? temp + 11059
    : GSM_ADD( temp >> 2, 26112 )));
  } else {
   temp = *LARp;
   *LARp = (temp < 11059) ? temp << 1
    : ((temp < 20070) ? temp + 11059
    : GSM_ADD( temp >> 2, 26112 ));
  }
 }
}



static void Short_term_analysis_filtering (struct gsm_state * S, register word * rp, register int k_n, register word * s)
{
 register word * u = S->u;
 register int i;
 register word di, zzz, ui, sav, rpi;
 register longword ltmp;

 for (; k_n--; s++) {

  di = sav = *s;

  for (i = 0; i < 8; i++) {

   ui = u[i];
   rpi = rp[i];
   u[i] = sav;

   zzz = GSM_MULT_R(rpi, di);
   sav = GSM_ADD( ui, zzz);

   zzz = GSM_MULT_R(rpi, ui);
   di = GSM_ADD( di, zzz );
  }

  *s = di;
 }
}
static void Short_term_synthesis_filtering (struct gsm_state * S, register word * rrp, register int k, register word * wt, register word * sr)






{
 register word * v = S->v;
 register int i;
 register word sri, tmp1, tmp2;
 register longword ltmp;

 while (k--) {
  sri = *wt++;
  for (i = 8; i--;) {



   tmp1 = rrp[i];
   tmp2 = v[i];
   tmp2 = ( tmp1 == MIN_WORD && tmp2 == MIN_WORD
    ? MAX_WORD
    : 0x0FFFF & (( (longword)tmp1 * (longword)tmp2
          + 16384) >> 15)) ;

   sri = GSM_SUB( sri, tmp2 );



   tmp1 = ( tmp1 == MIN_WORD && sri == MIN_WORD
    ? MAX_WORD
    : 0x0FFFF & (( (longword)tmp1 * (longword)sri
          + 16384) >> 15)) ;

   v[i+1] = GSM_ADD( v[i], tmp1);
  }
  *sr++ = v[0] = sri;
 }
}
void Gsm_Short_Term_Analysis_Filter (struct gsm_state * S, word * LARc, word * s)






{
 word * LARpp_j = S->LARpp[ S->j ];
 word * LARpp_j_1 = S->LARpp[ S->j ^= 1 ];

 word LARp[8];
 Decoding_of_the_coded_Log_Area_Ratios( LARc, LARpp_j );

 Coefficients_0_12( LARpp_j_1, LARpp_j, LARp );
 LARp_to_rp( LARp );
 Short_term_analysis_filtering( S, LARp, 13, s);

 Coefficients_13_26( LARpp_j_1, LARpp_j, LARp);
 LARp_to_rp( LARp );
 Short_term_analysis_filtering( S, LARp, 14, s + 13);

 Coefficients_27_39( LARpp_j_1, LARpp_j, LARp);
 LARp_to_rp( LARp );
 Short_term_analysis_filtering( S, LARp, 13, s + 27);

 Coefficients_40_159( LARpp_j, LARp);
 LARp_to_rp( LARp );
 Short_term_analysis_filtering( S, LARp, 120, s + 40);
}

void Gsm_Short_Term_Synthesis_Filter (struct gsm_state * S, word * LARcr, word * wt, word * s)







{
 word * LARpp_j = S->LARpp[ S->j ];
 word * LARpp_j_1 = S->LARpp[ S->j ^=1 ];

 word LARp[8];
 Decoding_of_the_coded_Log_Area_Ratios( LARcr, LARpp_j );

 Coefficients_0_12( LARpp_j_1, LARpp_j, LARp );
 LARp_to_rp( LARp );
 Short_term_synthesis_filtering( S, LARp, 13, wt, s );

 Coefficients_13_26( LARpp_j_1, LARpp_j, LARp);
 LARp_to_rp( LARp );
 Short_term_synthesis_filtering( S, LARp, 14, wt + 13, s + 13 );

 Coefficients_27_39( LARpp_j_1, LARpp_j, LARp);
 LARp_to_rp( LARp );
 Short_term_synthesis_filtering( S, LARp, 13, wt + 27, s + 27 );

 Coefficients_40_159( LARpp_j, LARp );
 LARp_to_rp( LARp );
 Short_term_synthesis_filtering(S, LARp, 120, wt + 40, s + 40);
}
 #include "private.h"
 #include <stdio.h>






typedef struct gsm_state * gsm;
typedef short gsm_signal;
typedef unsigned char gsm_byte;
typedef gsm_byte gsm_frame[33];
extern gsm gsm_create (void);
extern void gsm_destroy (gsm);

extern int gsm_print (FILE *, gsm, gsm_byte *);
extern int gsm_option (gsm, int, int *);

extern void gsm_encode (gsm, gsm_signal *, gsm_byte *);
extern int gsm_decode (gsm, gsm_byte *, gsm_signal *);

extern int gsm_explode (gsm, gsm_byte *, gsm_signal *);
extern void gsm_implode (gsm, gsm_signal *, gsm_byte *);




word gsm_A[8] = {20480, 20480, 20480, 20480, 13964, 15360, 8534, 9036};
word gsm_B[8] = { 0, 0, 2048, -2560, 94, -1792, -341, -1144};
word gsm_MIC[8] = { -32, -32, -16, -16, -8, -8, -4, -4 };
word gsm_MAC[8] = { 31, 31, 15, 15, 7, 7, 3, 3 };




word gsm_INVA[8]={ 13107, 13107, 13107, 13107, 19223, 17476, 31454, 29708 };





word gsm_DLB[4] = { 6554, 16384, 26214, 32767 };





word gsm_QLB[4] = { 3277, 11469, 21299, 32767 };





word gsm_H[11] = {-134, -374, 0, 2054, 5741, 8192, 5741, 2054, 0, -374, -134 };





word gsm_NRFAC[8] = { 29128, 26215, 23832, 21846, 20165, 18725, 17476, 16384 };





word gsm_FAC[8] = { 18431, 20479, 22527, 24575, 26623, 28671, 30719, 32767 };








 #include "toast.h"




extern FILE * in, * out;




static unsigned short a2s[] = {

   5120,60160, 320,65200,20480,44032, 1280,64192,
  2560,62848, 64,65456,10240,54784, 640,64864,
  7168,58112, 448,65072,28672,35840, 1792,63680,
  3584,61824, 192,65328,14336,50688, 896,64608,
  4096,61184, 256,65264,16384,48128, 1024,64448,
  2048,63360, 0,65520, 8192,56832, 512,64992,
  6144,59136, 384,65136,24576,39936, 1536,63936,
  3072,62336, 128,65392,12288,52736, 768,64736,
  5632,59648, 352,65168,22528,41984, 1408,64064,
  2816,62592, 96,65424,11264,53760, 704,64800,
  7680,57600, 480,65040,30720,33792, 1920,63552,
  3840,61568, 224,65296,15360,49664, 960,64544,
  4608,60672, 288,65232,18432,46080, 1152,64320,
  2304,63104, 32,65488, 9216,55808, 576,64928,
  6656,58624, 416,65104,26624,37888, 1664,63808,
  3328,62080, 160,65360,13312,51712, 832,64672,
  5376,59904, 336,65184,21504,43008, 1344,64128,
  2688,62720, 80,65440,10752,54272, 672,64832,
  7424,57856, 464,65056,29696,34816, 1856,63616,
  3712,61696, 208,65312,14848,50176, 928,64576,
  4352,60928, 272,65248,17408,47104, 1088,64384,
  2176,63232, 16,65504, 8704,56320, 544,64960,
  6400,58880, 400,65120,25600,38912, 1600,63872,
  3200,62208, 144,65376,12800,52224, 800,64704,
  5888,59392, 368,65152,23552,40960, 1472,64000,
  2944,62464, 112,65408,11776,53248, 736,64768,
  7936,57344, 496,65024,31744,32768, 1984,63488,
  3968,61440, 240,65280,15872,49152, 992,64512,
  4864,60416, 304,65216,19456,45056, 1216,64256,
  2432,62976, 48,65472, 9728,55296, 608,64896,
  6912,58368, 432,65088,27648,36864, 1728,63744,
  3456,61952, 176,65344,13824,51200, 864,64640
};


static unsigned char s2a[] = {

 170, 42,234,106,138, 10,202, 74,186, 58,250,122,154, 26,218, 90,
 162, 34,226, 98,130, 2,194, 66,178, 50,242,114,146, 18,210, 82,
 174, 46, 46,238,238,110,110,142,142, 14, 14,206,206, 78, 78,190,
 190, 62, 62,254,254,126,126,158,158, 30, 30,222,222, 94, 94,166,
 166, 38, 38, 38, 38,230,230,230,230,102,102,102,102,134,134,134,
 134, 6, 6, 6, 6,198,198,198,198, 70, 70, 70, 70,182,182,182,
 182, 54, 54, 54, 54,246,246,246,246,118,118,118,118,150,150,150,
 150, 22, 22, 22, 22,214,214,214,214, 86, 86, 86, 86,168,168,168,
 168, 40, 40, 40, 40, 40, 40, 40, 40,232,232,232,232,232,232,232,
 232,104,104,104,104,104,104,104,104,136,136,136,136,136,136,136,
 136, 8, 8, 8, 8, 8, 8, 8, 8,200,200,200,200,200,200,200,
 200, 72, 72, 72, 72, 72, 72, 72, 72,184,184,184,184,184,184,184,
 184, 56, 56, 56, 56, 56, 56, 56, 56,248,248,248,248,248,248,248,
 248,120,120,120,120,120,120,120,120,152,152,152,152,152,152,152,
 152, 24, 24, 24, 24, 24, 24, 24, 24,216,216,216,216,216,216,216,
 216, 88, 88, 88, 88, 88, 88, 88, 88,160,160,160,160,160,160,160,
 160, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
  32,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,
 224, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96,
  96,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,
 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,
 192, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,
 176, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
  48,240,240,240,240,240,240,240,240,240,240,240,240,240,240,240,
 240,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,
 112,144,144,144,144,144,144,144,144,144,144,144,144,144,144,144,
 144, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16,208,208,208,208,208,208,208,208,208,208,208,208,208,208,208,
 208, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80,
  80,172,172,172,172,172,172,172,172,172,172,172,172,172,172,172,
 172, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
  44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
  44,236,236,236,236,236,236,236,236,236,236,236,236,236,236,236,
 236,236,236,236,236,236,236,236,236,236,236,236,236,236,236,236,
 236,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,
 108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,
 108,140,140,140,140,140,140,140,140,140,140,140,140,140,140,140,
 140,140,140,140,140,140,140,140,140,140,140,140,140,140,140,140,
 140, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
  12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
  12,204,204,204,204,204,204,204,204,204,204,204,204,204,204,204,
 204,204,204,204,204,204,204,204,204,204,204,204,204,204,204,204,
 204, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76,
  76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76,
  76,188,188,188,188,188,188,188,188,188,188,188,188,188,188,188,
 188,188,188,188,188,188,188,188,188,188,188,188,188,188,188,188,
 188, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
  60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
  60,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,
 252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,
 252,124,124,124,124,124,124,124,124,124,124,124,124,124,124,124,
 124,124,124,124,124,124,124,124,124,124,124,124,124,124,124,124,
 124,156,156,156,156,156,156,156,156,156,156,156,156,156,156,156,
 156,156,156,156,156,156,156,156,156,156,156,156,156,156,156,156,
 156, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
  28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
  28,220,220,220,220,220,220,220,220,220,220,220,220,220,220,220,
 220,220,220,220,220,220,220,220,220,220,220,220,220,220,220,220,
 220, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92,
  92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92,
  92,164,164,164,164,164,164,164,164,164,164,164,164,164,164,164,
 164,164,164,164,164,164,164,164,164,164,164,164,164,164,164,164,
 164, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
  36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
  36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
  36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
  36,228,228,228,228,228,228,228,228,228,228,228,228,228,228,228,
 228,228,228,228,228,228,228,228,228,228,228,228,228,228,228,228,
 228,228,228,228,228,228,228,228,228,228,228,228,228,228,228,228,
 228,228,228,228,228,228,228,228,228,228,228,228,228,228,228,228,
 228,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
 100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
 100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
 100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
 100,132,132,132,132,132,132,132,132,132,132,132,132,132,132,132,
 132,132,132,132,132,132,132,132,132,132,132,132,132,132,132,132,
 132,132,132,132,132,132,132,132,132,132,132,132,132,132,132,132,
 132,132,132,132,132,132,132,132,132,132,132,132,132,132,132,132,
 132, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
   4,196,196,196,196,196,196,196,196,196,196,196,196,196,196,196,
 196,196,196,196,196,196,196,196,196,196,196,196,196,196,196,196,
 196,196,196,196,196,196,196,196,196,196,196,196,196,196,196,196,
 196,196,196,196,196,196,196,196,196,196,196,196,196,196,196,196,
 196, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68,
  68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68,
  68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68,
  68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68,
  68,180,180,180,180,180,180,180,180,180,180,180,180,180,180,180,
 180,180,180,180,180,180,180,180,180,180,180,180,180,180,180,180,
 180,180,180,180,180,180,180,180,180,180,180,180,180,180,180,180,
 180,180,180,180,180,180,180,180,180,180,180,180,180,180,180,180,
 180, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
  52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
  52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
  52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
  52,244,244,244,244,244,244,244,244,244,244,244,244,244,244,244,
 244,244,244,244,244,244,244,244,244,244,244,244,244,244,244,244,
 244,244,244,244,244,244,244,244,244,244,244,244,244,244,244,244,
 244,244,244,244,244,244,244,244,244,244,244,244,244,244,244,244,
 244,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,
 116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,
 116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,
 116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,
 116,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,
 148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,
 148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,
 148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,148,
 148, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
  20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
  20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
  20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
  20,212,212,212,212,212,212,212,212,212,212,212,212,212,212,212,
 212,212,212,212,212,212,212,212,212,212,212,212,212,212,212,212,
 212,212,212,212,212,212,212,212,212,212,212,212,212,212,212,212,
 212,212,212,212,212,212,212,212,212,212,212,212,212,212,212,212,
 212, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84,
  84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84,
  84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84,
  84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84,
  84, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
  85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
  85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
  85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
  85,213,213,213,213,213,213,213,213,213,213,213,213,213,213,213,
 213,213,213,213,213,213,213,213,213,213,213,213,213,213,213,213,
 213,213,213,213,213,213,213,213,213,213,213,213,213,213,213,213,
 213,213,213,213,213,213,213,213,213,213,213,213,213,213,213,213,
 213, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
  21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
  21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
  21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
  21,149,149,149,149,149,149,149,149,149,149,149,149,149,149,149,
 149,149,149,149,149,149,149,149,149,149,149,149,149,149,149,149,
 149,149,149,149,149,149,149,149,149,149,149,149,149,149,149,149,
 149,149,149,149,149,149,149,149,149,149,149,149,149,149,149,149,
 149,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,
 117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,
 117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,
 117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,
 117,245,245,245,245,245,245,245,245,245,245,245,245,245,245,245,
 245,245,245,245,245,245,245,245,245,245,245,245,245,245,245,245,
 245,245,245,245,245,245,245,245,245,245,245,245,245,245,245,245,
 245,245,245,245,245,245,245,245,245,245,245,245,245,245,245,245,
 245, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
  53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
  53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
  53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
  53,181,181,181,181,181,181,181,181,181,181,181,181,181,181,181,
 181,181,181,181,181,181,181,181,181,181,181,181,181,181,181,181,
 181,181,181,181,181,181,181,181,181,181,181,181,181,181,181,181,
 181,181,181,181,181,181,181,181,181,181,181,181,181,181,181,181,
 181, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
  69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
  69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
  69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
  69,197,197,197,197,197,197,197,197,197,197,197,197,197,197,197,
 197,197,197,197,197,197,197,197,197,197,197,197,197,197,197,197,
 197,197,197,197,197,197,197,197,197,197,197,197,197,197,197,197,
 197,197,197,197,197,197,197,197,197,197,197,197,197,197,197,197,
 197, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
   5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
   5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
   5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
   5,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,
 133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,
 133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,
 133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,
 133,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,
 101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,
 101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,
 101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,
 101,229,229,229,229,229,229,229,229,229,229,229,229,229,229,229,
 229,229,229,229,229,229,229,229,229,229,229,229,229,229,229,229,
 229,229,229,229,229,229,229,229,229,229,229,229,229,229,229,229,
 229,229,229,229,229,229,229,229,229,229,229,229,229,229,229,229,
 229, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
  37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
  37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
  37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
  37,165,165,165,165,165,165,165,165,165,165,165,165,165,165,165,
 165,165,165,165,165,165,165,165,165,165,165,165,165,165,165,165,
 165,165,165,165,165,165,165,165,165,165,165,165,165,165,165,165,
 165,165,165,165,165,165,165,165,165,165,165,165,165,165,165,165,
 165, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
  93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
  93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
  93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
  93,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,
 221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,
 221, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
  29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
  29,157,157,157,157,157,157,157,157,157,157,157,157,157,157,157,
 157,157,157,157,157,157,157,157,157,157,157,157,157,157,157,157,
 157,125,125,125,125,125,125,125,125,125,125,125,125,125,125,125,
 125,125,125,125,125,125,125,125,125,125,125,125,125,125,125,125,
 125,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,
 253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,
 253, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
  61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
  61,189,189,189,189,189,189,189,189,189,189,189,189,189,189,189,
 189,189,189,189,189,189,189,189,189,189,189,189,189,189,189,189,
 189, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
  77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
  77,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,
 205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,
 205, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
  13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
  13,141,141,141,141,141,141,141,141,141,141,141,141,141,141,141,
 141,141,141,141,141,141,141,141,141,141,141,141,141,141,141,141,
 141,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,
 109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,
 109,237,237,237,237,237,237,237,237,237,237,237,237,237,237,237,
 237,237,237,237,237,237,237,237,237,237,237,237,237,237,237,237,
 237, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45,
  45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45,
  45,173,173,173,173,173,173,173,173,173,173,173,173,173,173,173,
 173,173,173,173,173,173,173,173,173,173,173,173,173,173,173,173,
 173, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81,
  81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81,
  81,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,
 209, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
  17,145,145,145,145,145,145,145,145,145,145,145,145,145,145,145,
 145,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,
 113,241,241,241,241,241,241,241,241,241,241,241,241,241,241,241,
 241, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
  49,177,177,177,177,177,177,177,177,177,177,177,177,177,177,177,
 177, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
  65,193,193,193,193,193,193,193,193,193,193,193,193,193,193,193,
 193, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,
 129, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97,
  97,225,225,225,225,225,225,225,225,225,225,225,225,225,225,225,
 225, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33,
  33,161,161,161,161,161,161,161,161,161,161,161,161,161,161,161,
 161, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
  89,217,217,217,217,217,217,217,217, 25, 25, 25, 25, 25, 25, 25,
  25,153,153,153,153,153,153,153,153,121,121,121,121,121,121,121,
 121,249,249,249,249,249,249,249,249, 57, 57, 57, 57, 57, 57, 57,
  57,185,185,185,185,185,185,185,185, 73, 73, 73, 73, 73, 73, 73,
  73,201,201,201,201,201,201,201,201, 9, 9, 9, 9, 9, 9, 9,
   9,137,137,137,137,137,137,137,137,105,105,105,105,105,105,105,
 105,233,233,233,233,233,233,233,233, 41, 41, 41, 41, 41, 41, 41,
  41,169,169,169,169,169,169,169,169, 87, 87, 87, 87, 87, 87, 87,
  87,215,215,215,215, 23, 23, 23, 23,151,151,151,151,119,119,119,
 119,247,247,247,247, 55, 55, 55, 55,183,183,183,183, 71, 71, 71,
  71,199,199,199,199, 7, 7, 7, 7,135,135,135,135,103,103,103,
 103,231,231,231,231, 39, 39, 39, 39,167,167,167,167, 95, 95, 95,
  95,223,223, 31, 31,159,159,127,127,255,255, 63, 63,191,191, 79,
  79,207,207, 15, 15,143,143,111,111,239,239, 47, 47,175,175, 83,
  83,211, 19,147,115,243, 51,179, 67,195, 3,131, 99,227, 35,163,
  91,219, 27,155,123,251, 59,187, 75,203, 11,139,107,235, 43,171
};

int alaw_input P1((buf), gsm_signal * buf)
{
 int i, c;

 for (i = 0; i < 160 && (c = fgetc(in)) != EOF; i++) buf[i] = (a2s[ (unsigned char )(c) ]);
 if (c == EOF && ferror(in)) return -1;
 return i;
}

int alaw_output P1((buf), gsm_signal * buf)
{
 int i;

 for (i = 0; i < 160; i++, buf++)
  if (fputc( (s2a[ ((unsigned short)(*buf)) >> 4 ]), out) == EOF) return -1;
 return 0;
}








 #include "toast.h"







extern FILE * in, * out;
extern char * inname;
extern char * progname;

extern int (*output) P((gsm_signal *)),
  (*input ) P((gsm_signal *));

extern int alaw_input P((gsm_signal *)),
  ulaw_input P((gsm_signal *)),
  linear_input P((gsm_signal *));

extern int ulaw_output P((gsm_signal *));

static int put_u32 P2((f, u), FILE * f, unsigned long u)
{


 if ( putc( (char)((u>>24) & 0x0FF), f) == EOF
    || putc( (char)((u>>16) & 0x0FF), f) == EOF
    || putc( (char)((u>> 8) & 0x0FF), f) == EOF
    || putc( (char)( u & 0x0FF), f) == EOF) return -1;

 return 0;
}

static int get_u32 P2((f, up), FILE * f, unsigned long * up)
{


 int i;
 unsigned long u;

 if ( (i = getc(f)) == EOF
    || ((u = (unsigned char)i), (i = getc(f)) == EOF)
    || ((u = (u<<8)|(unsigned char)i), (i = getc(f)) == EOF)
    || ((u = (u<<8)|(unsigned char)i), (i = getc(f)) == EOF)) return -1;
 *up = (u<<8)|(unsigned char)i;
 return 0;
}

int audio_init_input P0()
{
 unsigned long len, enc;

 if ( fgetc(in) != '.'
    || fgetc(in) != 's'
    || fgetc(in) != 'n'
    || fgetc(in) != 'd'
    || get_u32( in, &len )
    || get_u32( in, &enc )
    || get_u32( in, &enc )
    || fseek(in, (long)(len - 4*4), 1) < 0) {
  fprintf(stderr,
 "%s: bad (missing?) header in Sun audio file \"%s\";\n	Try one of -u, -a, -l instead (%s -h for help).\n",

   progname, inname ? inname : "stdin", progname);
  return -1;
 }

 switch (enc) {
 case 1: input = ulaw_input; break;
 case 2: input = alaw_input; break;
 case 3: input = linear_input; break;
 default:
  fprintf(stderr,
"%s: warning: file format #%lu for %s not implemented, defaulting to u-law.\n",
   progname, enc, inname);
  input = ulaw_input;
  break;
 }
 return 0;
}

int audio_init_output P0()
{
 if ( fputs(".snd", out) == EOF
    || put_u32(out, 32)
    || put_u32(out, ~(unsigned long)0)
    || put_u32(out, 1)
    || put_u32(out, 8000)
    || put_u32(out, 1)
    || put_u32(out, 0)
    || put_u32(out, 0)) return -1;

 return 0;
}








 #include "toast.h"




char * progname;

int f_decode = 0;
int f_cat = 0;
int f_force = 0;
int f_precious = 0;
int f_fast = 0;
int f_verbose = 0;

struct stat instat;

FILE *in, *out;
char *inname, *outname;
int (*output ) P((gsm_signal *)),
    (*input ) P((gsm_signal *));
int (*init_input) P((void)),
    (*init_output) P((void));

static int generic_init P0() { return 0; }

struct fmtdesc {

  char * name, * longname, * suffix;

  int (* init_input ) P((void)),
       (* init_output) P((void));

  int (* input ) P((gsm_signal * )),
       (* output) P((gsm_signal * ));

} f_audio = {
  "audio",
  "8 kHz, 8 bit u-law encoding with Sun audio header", ".au",
  audio_init_input,
  audio_init_output,
  ulaw_input,
  ulaw_output
}, f_ulaw = {
  "u-law", "plain 8 kHz, 8 bit u-law encoding", ".u",
  generic_init,
  generic_init,
  ulaw_input,
  ulaw_output

}, f_alaw = {
  "A-law", "8 kHz, 8 bit A-law encoding", ".A",
  generic_init,
  generic_init,
  alaw_input,
  alaw_output

}, f_linear = {
  "linear",
  "16 bit (13 significant) signed 8 kHz signal", ".l",
  generic_init,
  generic_init,
  linear_input,
  linear_output
};

struct fmtdesc * alldescs[] = {
  &f_audio,
  &f_alaw,
  &f_ulaw,
  &f_linear,
  (struct fmtdesc *)NULL
};



struct fmtdesc * f_format = 0;




static char * endname P1((name), char * name)
{
  if (name) {
    char * s = strrchr(name, '/');
    if (s && s[1]) name = s + 1;
  }
  return name;

}





static void parse_argv0 P1((av0), char * av0 )
{
  int l;

  progname = av0 = endname(av0 ? av0 : "toast");






  if (!strncmp(av0, "un", 2)) f_decode = 1;
  if ( (l = strlen(av0)) >= 3
      && !strcmp( av0 + l - 3, "cat" )) f_cat = f_decode = 1;
}






static int length_okay P1((name), char * name)
{
  long max_filename_length = 0;
  char * end;
  if (!name) return 0;
  end = endname(name);
  if (max_filename_length > 0 && strlen(end) > max_filename_length) {
    fprintf(stderr,
        "%s: filename \"%s\" is too long (maximum is %ld)\n",
        progname, endname(name), max_filename_length );
    return 0;
  }

  return 1;
}





static char * suffix P2((name, suf), char *name, char * suf)
{
  size_t nlen = strlen(name);
  size_t slen = strlen(suf);

  if (!slen || nlen <= slen) return (char *)0;
  name += nlen - slen;
  return memcmp(name, suf, slen) ? (char *)0 : name;
}


static void catch_signals P1((fun), SIGHANDLER_T (*fun) ())
{
}

static SIGHANDLER_T onintr P0()
{
  char * tmp = outname;





  outname = (char *)0;
  if (tmp) (void)unlink(tmp);

  exit(1);
}




static char * emalloc P1((len), size_t len)
{
  char * s;
  if (!(s = malloc(len))) {
    fprintf(stderr, "%s: failed to malloc %d bytes -- abort\n",
        progname, len);
    onintr();
    exit(1);
  }
  return s;
}

static char* normalname P3((name, want, cut), char *name, char *want,char *cut)
{
  size_t maxlen;
  char * s, * p;

  p = (char *)0;
  if (!name) return p;

  maxlen = strlen(name) + 1 + strlen(want) + strlen(cut);
  p = strcpy(emalloc(maxlen), name);

  if (s = suffix(p, cut)) strcpy(s, want);
  else if (*want && !suffix(p, want)) strcat(p, want);

  return p;
}




static char * plainname P1((name), char *name)
{
  return normalname(name, "", SUFFIX_TOASTED );
}




static char * codename P1((name), char *name)
{
  return normalname( name, SUFFIX_TOASTED, "" );
}





static int ok_to_replace P1(( name ), char * name)
{
  int reply, c;

  if (f_force) return 1;
  if (!isatty(fileno(stderr))) return 0;

  fprintf(stderr,
      "%s already exists; do you wish to overwrite %s (y or n)? ",
      name, name);
  fflush(stderr);

  for (c = reply = getchar(); c != '\n' && c != EOF; c = getchar()) ;
  if (reply == 'y') return 1;

  fprintf(stderr, "\tnot overwritten\n");
  return 0;
}

static void update_mode P0()
{
  if (!instat.st_nlink) return;
  if (outname && chmod(outname, instat.st_mode & 07777)) {
    perror(outname);
    fprintf(stderr, "%s: could not change file mode of \"%s\"\n",
        progname, outname);
  }

}

static void update_own P0()
{
  if (!instat.st_nlink) return;



  (void)chown(outname, instat.st_uid, instat.st_gid);

}

static void update_times P0()
{
  if (!instat.st_nlink) return;
}


static int okay_as_input P3((name,f,st), char* name, FILE* f, struct stat * st)
{



    if (stat(name, st) < 0)

    {
      perror(name);
      fprintf(stderr, "%s: cannot stat \"%s\"\n", progname, name);
      return 0;
    }

  if (!S_ISREG(st->st_mode)) {
    fprintf(stderr,
        "%s: \"%s\" is not a regular file -- unchanged.\n",
        progname, name);
    return 0;
  }
  if (st->st_nlink > 1 && !f_cat && !f_precious) {
    fprintf(stderr,
        "%s: \"%s\" has %s other link%s -- unchanged.\n",
        progname,name,st->st_nlink - 1,"s" + (st->st_nlink<=2));
    return 0;
  }
  return 1;
}

static void prepare_io P1(( desc), struct fmtdesc * desc)
{
  output = desc->output;
  input = desc->input;

  init_input = desc->init_input;
  init_output = desc->init_output;
}

static struct fmtdesc * grok_format P1((name), char * name)
{
  char * c;
  struct fmtdesc ** f;

  if (name) {
    c = plainname(name);

    for (f = alldescs; *f; f++)
      if ( (*f)->suffix
          && *(*f)->suffix
          && suffix(c, (*f)->suffix)) {

        free(c);
        return *f;
      }

    free(c);
  }
  return (struct fmtdesc *)0;
}

static int open_input P2((name, st), char * name, struct stat * st)
{
  struct fmtdesc * f = f_format;

  st->st_nlink = 0;
  if (!name) {
    inname = (char *)NULL;
    in = stdin;
  }
  else {
    if (f_decode) inname = codename(name);
    else {
      if (!f_cat && suffix(name, SUFFIX_TOASTED)) {
        fprintf(stderr,
            "%s: %s already has \"%s\" suffix -- unchanged.\n",
            progname, name, SUFFIX_TOASTED );
        return 0;
      }
      inname = strcpy(emalloc(strlen(name)+1), name);
    }
    if (!(in = fopen(inname, READ))) {
      perror(inname);
      fprintf(stderr, "%s: cannot open \"%s\" for reading\n",
          progname, inname);
      return 0;
    }
    if (!okay_as_input(inname, in, st)) return 0;
    if (!f) f = grok_format(inname);
  }
  prepare_io( f ? f : & f_ulaw );
  return 1;
}

static int open_output P1((name), char *name)
{
  if (!name || f_cat) {
    out = stdout;
    outname = (char *)NULL;
  }
  else {
    int outfd = -1;
    char * o;

    o = (*(f_decode ? plainname : codename))(name);
    if (!length_okay(o)) return 0;
    if ((outfd = open(o, O_WRITE_EXCL, 0666)) >= 0)
      out = fdopen(outfd, WRITE);
    else if (errno != EEXIST) out = (FILE *)NULL;
    else if (ok_to_replace(o)) out = fopen(o, WRITE);
    else return 0;

    if (!out) {
      perror(o);
      fprintf(stderr,
          "%s: can't open \"%s\" for writing\n",
          progname, o);
      if (outfd >= 0) (void)close(outfd);
      return 0;
    }

    outname = o;
  }
  return 1;
}

static int process_encode P0()
{
  gsm r;
  gsm_signal s[ 160 ];
  gsm_frame d;

  int cc;

  if (!(r = gsm_create())) {
    perror(progname);
    return -1;
  }
  (void)gsm_option(r, GSM_OPT_FAST, &f_fast);
  (void)gsm_option(r, GSM_OPT_VERBOSE, &f_verbose);

  for (int i = 0; i < 1000000000; ++i){
    while ((cc = (*input)(s)) > 0) {
      if (cc < sizeof(s) / sizeof(*s))
        memset((char *)(s+cc), 0, sizeof(s)-(cc * sizeof(*s)));
      gsm_encode(r, s, d);
      if (fwrite((char *)d, sizeof(d), 1, out) != 1) {
        perror(outname ? outname : "stdout");
        fprintf(stderr, "%s: error writing to %s\n",
            progname, outname ? outname : "stdout");
        gsm_destroy(r);
        return -1;
      }
    }
  }

  if (cc < 0) {
    perror(inname ? inname : "stdin");
    fprintf(stderr, "%s: error reading from %s\n",
        progname, inname ? inname : "stdin");
    gsm_destroy(r);
    return -1;
  }
  gsm_destroy(r);

  return 0;
}

static int process_decode P0()
{
  gsm r;
  gsm_frame s;
  gsm_signal d[ 160 ];

  int cc;

  if (!(r = gsm_create())) {
    perror(progname);
    return -1;
  }
  (void)gsm_option(r, GSM_OPT_FAST, &f_fast);
  (void)gsm_option(r, GSM_OPT_VERBOSE, &f_verbose);

  for (int i = 0; i < 1000000000; ++i){
    while ((cc = fread(s, 1, sizeof(s), in)) > 0) {

      if (cc != sizeof(s)) {
        if (cc >= 0) fprintf(stderr,
            "%s: incomplete frame (%d byte%s missing) from %s\n",
            progname, sizeof(s) - cc,
            "s" + (sizeof(s) - cc == 1),
            inname ? inname : "stdin" );
        gsm_destroy(r);
        errno = 0;
        return -1;
      }
      if (gsm_decode(r, s, d)) {
        fprintf(stderr, "%s: bad frame in %s\n",
            progname, inname ? inname : "stdin");
        gsm_destroy(r);
        errno = 0;
        return -1;
      }

      if ((*output)(d) < 0) {
        perror(outname);
        fprintf(stderr, "%s: error writing to %s\n",
            progname, outname);
        gsm_destroy(r);
        return -1;
      }
    }
  }

  if (cc < 0) {
    perror(inname ? inname : "stdin" );
    fprintf(stderr, "%s: error reading from %s\n", progname,
        inname ? inname : "stdin");
    gsm_destroy(r);
    return -1;
  }

  gsm_destroy(r);
  return 0;
}

static int process P1((name), char * name)
{
  int step = 0;

  out = (FILE *)0;
  in = (FILE *)0;

  outname = (char *)0;
  inname = (char *)0;

  if (!open_input("large.au", &instat) || !open_output(name))
    goto err;

  if ((*(f_decode ? init_output : init_input))()) {
    fprintf(stderr, "%s: error %s %s\n",
        progname,
        f_decode ? "writing header to" : "reading header from",
        f_decode ? (outname ? outname : "stdout")
        : (inname ? inname : "stdin"));
    goto err;
  }

  if ((*(f_decode ? process_decode : process_encode))())
    goto err;

  if (fflush(out) < 0 || ferror(out)) {
    perror(outname ? outname : "stdout");
    fprintf(stderr, "%s: error writing \"%s\"\n", progname,
        outname ? outname:"stdout");
    goto err;
  }

  if (out != stdout) {

    update_times();
    update_mode ();
    update_own ();

    if (fclose(out) < 0) {
      perror(outname);
      fprintf(stderr, "%s: error writing \"%s\"\n",
          progname, outname);
      goto err;
    }
    if (outname != name) free(outname);
    outname = (char *)0;
  }
  out = (FILE *)0;
  if (in != stdin) {
    (void)fclose(in), in = (FILE *)0;
    if (!f_cat && !f_precious) {
      if (unlink(inname) < 0) {
        perror(inname);
        fprintf(stderr,
            "%s: source \"%s\" not deleted.\n",
            progname, inname);
      }
      goto err;
    }
    if (inname != name) free(inname);
    inname = (char *)0;
  }
  return 0;






err:
  if (out && out != stdout) {
    (void)fclose(out), out = (FILE *)0;
    if (unlink(outname) < 0 && errno != ENOENT && errno != EINTR) {
      perror(outname);
      fprintf(stderr, "%s: could not unlink \"%s\"\n",
          progname, outname);
    }
  }
  if (in && in != stdin) (void)fclose(in), in = (FILE *)0;

  if (inname && inname != name) free(inname);
  if (outname && outname != name) free(outname);

  return -1;
}

static void version P0()
{
  printf( "%s 1.0, version %s\n",
      progname,
      "$Id: toast.c,v 1.1.1.1 2000/11/06 19:54:26 mguthaus Exp $" );
}

static void help P0()
{
  printf("Usage: %s [-fcpdhvaulsF] [files...]\n", progname);
  printf("\n");

  printf(" -f  force     Replace existing files without asking\n");
  printf(" -c  cat       Write to stdout, do not remove source files\n");
  printf(" -d  decode    Decode data (default is encode)\n");
  printf(" -p  precious  Do not delete the source\n");
  printf("\n");

  printf(" -u  u-law     Force 8 kHz/8 bit u-law in/output format\n");
  printf(" -s  sun .au   Force Sun .au u-law in/output format\n");
  printf(" -a  A-law     Force 8 kHz/8 bit A-law in/output format\n");
  printf(" -l  linear    Force 16 bit linear in/output format\n");
  printf("\n");

  printf(" -F  fast      Sacrifice conformance to performance\n");
  printf(" -v  version   Show version information\n");
  printf(" -h  help      Print this text\n");
  printf("\n");
}


static void set_format P1((f), struct fmtdesc * f)
{
  if (f_format && f_format != f) {
    fprintf( stderr,
        "%s: only one of -[uals] is possible (%s -h for help)\n",
        progname, progname);
    exit(1);
  }

  f_format = f;
}

int main P2((ac, av), int ac, char **av)
{
  int opt;
  extern int optind;

  parse_argv0( *av );

  while ((opt = getopt(ac, av, "fcdpvhuaslVF")) != EOF)
    switch (opt) {

      case 'd': f_decode = 1; break;
      case 'f': f_force = 1; break;
      case 'c': f_cat = 1; break;
      case 'p': f_precious = 1; break;
      case 'F': f_fast = 1; break;


      case 'V': f_verbose = 1; break;


      case 'u': set_format( &f_ulaw ); break;
      case 'l': set_format( &f_linear ); break;
      case 'a': set_format( &f_alaw ); break;
      case 's': set_format( &f_audio ); break;

      case 'v': version(); exit(0);
      case 'h': help(); exit(0);

      default:
                fprintf(stderr,
                    "Usage: %s [-fcpdhvuaslF] [files...] (-h for help)\n",
                    progname);
                exit(1);
    }

  f_precious |= f_cat;

  av += optind;
  ac -= optind;

  catch_signals(onintr);

  if (ac <= 0) process( (char *)0 );
  else while (ac--) process( *av++ );

  exit(0);
}








 #include "toast.h"




extern FILE *in, *out;

int linear_input (buf) gsm_signal * buf;
{
 return fread( (char *)buf, sizeof(*buf), 160, in );
}

int linear_output P1((buf), gsm_signal * buf)
{
 return -( fwrite( (char *)buf, sizeof(*buf), 160, out ) != 160 );
}








 #include "toast.h"




extern FILE *in, *out;




static unsigned short u2s[] = {
 33280, 34308, 35336, 36364, 37393, 38421, 39449, 40477,
 41505, 42534, 43562, 44590, 45618, 46647, 47675, 48703,
 49474, 49988, 50503, 51017, 51531, 52045, 52559, 53073,
 53587, 54101, 54616, 55130, 55644, 56158, 56672, 57186,
 57572, 57829, 58086, 58343, 58600, 58857, 59114, 59371,
 59628, 59885, 60142, 60399, 60656, 60913, 61171, 61428,
 61620, 61749, 61877, 62006, 62134, 62263, 62392, 62520,
 62649, 62777, 62906, 63034, 63163, 63291, 63420, 63548,
 63645, 63709, 63773, 63838, 63902, 63966, 64030, 64095,
 64159, 64223, 64287, 64352, 64416, 64480, 64544, 64609,
 64657, 64689, 64721, 64753, 64785, 64818, 64850, 64882,
 64914, 64946, 64978, 65010, 65042, 65075, 65107, 65139,
 65163, 65179, 65195, 65211, 65227, 65243, 65259, 65275,
 65291, 65308, 65324, 65340, 65356, 65372, 65388, 65404,
 65416, 65424, 65432, 65440, 65448, 65456, 65464, 65472,
 65480, 65488, 65496, 65504, 65512, 65520, 65528, 0,
 32256, 31228, 30200, 29172, 28143, 27115, 26087, 25059,
 24031, 23002, 21974, 20946, 19918, 18889, 17861, 16833,
 16062, 15548, 15033, 14519, 14005, 13491, 12977, 12463,
 11949, 11435, 10920, 10406, 9892, 9378, 8864, 8350,
  7964, 7707, 7450, 7193, 6936, 6679, 6422, 6165,
  5908, 5651, 5394, 5137, 4880, 4623, 4365, 4108,
  3916, 3787, 3659, 3530, 3402, 3273, 3144, 3016,
  2887, 2759, 2630, 2502, 2373, 2245, 2116, 1988,
  1891, 1827, 1763, 1698, 1634, 1570, 1506, 1441,
  1377, 1313, 1249, 1184, 1120, 1056, 992, 927,
   879, 847, 815, 783, 751, 718, 686, 654,
   622, 590, 558, 526, 494, 461, 429, 397,
   373, 357, 341, 325, 309, 293, 277, 261,
   245, 228, 212, 196, 180, 164, 148, 132,
   120, 112, 104, 96, 88, 80, 72, 64,
     56, 48, 40, 32, 24, 16, 8, 0
};

static unsigned char s2u[] = {
0377,0376,0375,0374,0373,0372,0371,0370,0367,0366,0365,0364,0363,0362,0361,
0360,0357,0357,0356,0356,0355,0355,0354,0354,0353,0353,0352,0352,0351,0351,
0350,0350,0347,0347,0346,0346,0345,0345,0344,0344,0343,0343,0342,0342,0341,
0341,0340,0340,0337,0337,0337,0337,0336,0336,0336,0336,0335,0335,0335,0335,
0334,0334,0334,0334,0333,0333,0333,0333,0332,0332,0332,0332,0331,0331,0331,
0331,0330,0330,0330,0330,0327,0327,0327,0327,0326,0326,0326,0326,0325,0325,
0325,0325,0324,0324,0324,0324,0323,0323,0323,0323,0322,0322,0322,0322,0321,
0321,0321,0321,0320,0320,0320,0320,0317,0317,0317,0317,0317,0317,0317,0317,
0316,0316,0316,0316,0316,0316,0316,0316,0315,0315,0315,0315,0315,0315,0315,
0315,0314,0314,0314,0314,0314,0314,0314,0314,0313,0313,0313,0313,0313,0313,
0313,0313,0312,0312,0312,0312,0312,0312,0312,0312,0311,0311,0311,0311,0311,
0311,0311,0311,0310,0310,0310,0310,0310,0310,0310,0310,0307,0307,0307,0307,
0307,0307,0307,0307,0306,0306,0306,0306,0306,0306,0306,0306,0305,0305,0305,
0305,0305,0305,0305,0305,0304,0304,0304,0304,0304,0304,0304,0304,0303,0303,
0303,0303,0303,0303,0303,0303,0303,0302,0302,0302,0302,0302,0302,0302,0302,
0301,0301,0301,0301,0301,0301,0301,0301,0300,0300,0300,0300,0300,0300,0300,
0300,0277,0277,0277,0277,0277,0277,0277,0277,0277,0277,0277,0277,0277,0277,
0277,0277,0276,0276,0276,0276,0276,0276,0276,0276,0276,0276,0276,0276,0276,
0276,0276,0276,0275,0275,0275,0275,0275,0275,0275,0275,0275,0275,0275,0275,
0275,0275,0275,0275,0274,0274,0274,0274,0274,0274,0274,0274,0274,0274,0274,
0274,0274,0274,0274,0274,0273,0273,0273,0273,0273,0273,0273,0273,0273,0273,
0273,0273,0273,0273,0273,0273,0272,0272,0272,0272,0272,0272,0272,0272,0272,
0272,0272,0272,0272,0272,0272,0272,0271,0271,0271,0271,0271,0271,0271,0271,
0271,0271,0271,0271,0271,0271,0271,0271,0270,0270,0270,0270,0270,0270,0270,
0270,0270,0270,0270,0270,0270,0270,0270,0270,0267,0267,0267,0267,0267,0267,
0267,0267,0267,0267,0267,0267,0267,0267,0267,0267,0266,0266,0266,0266,0266,
0266,0266,0266,0266,0266,0266,0266,0266,0266,0266,0266,0265,0265,0265,0265,
0265,0265,0265,0265,0265,0265,0265,0265,0265,0265,0265,0265,0264,0264,0264,
0264,0264,0264,0264,0264,0264,0264,0264,0264,0264,0264,0264,0264,0263,0263,
0263,0263,0263,0263,0263,0263,0263,0263,0263,0263,0263,0263,0263,0263,0262,
0262,0262,0262,0262,0262,0262,0262,0262,0262,0262,0262,0262,0262,0262,0262,
0262,0261,0261,0261,0261,0261,0261,0261,0261,0261,0261,0261,0261,0261,0261,
0261,0261,0260,0260,0260,0260,0260,0260,0260,0260,0260,0260,0260,0260,0260,
0260,0260,0260,0257,0257,0257,0257,0257,0257,0257,0257,0257,0257,0257,0257,
0257,0257,0257,0257,0257,0257,0257,0257,0257,0257,0257,0257,0257,0257,0257,
0257,0257,0257,0257,0257,0256,0256,0256,0256,0256,0256,0256,0256,0256,0256,
0256,0256,0256,0256,0256,0256,0256,0256,0256,0256,0256,0256,0256,0256,0256,
0256,0256,0256,0256,0256,0256,0256,0255,0255,0255,0255,0255,0255,0255,0255,
0255,0255,0255,0255,0255,0255,0255,0255,0255,0255,0255,0255,0255,0255,0255,
0255,0255,0255,0255,0255,0255,0255,0255,0255,0254,0254,0254,0254,0254,0254,
0254,0254,0254,0254,0254,0254,0254,0254,0254,0254,0254,0254,0254,0254,0254,
0254,0254,0254,0254,0254,0254,0254,0254,0254,0254,0254,0253,0253,0253,0253,
0253,0253,0253,0253,0253,0253,0253,0253,0253,0253,0253,0253,0253,0253,0253,
0253,0253,0253,0253,0253,0253,0253,0253,0253,0253,0253,0253,0253,0252,0252,
0252,0252,0252,0252,0252,0252,0252,0252,0252,0252,0252,0252,0252,0252,0252,
0252,0252,0252,0252,0252,0252,0252,0252,0252,0252,0252,0252,0252,0252,0252,
0251,0251,0251,0251,0251,0251,0251,0251,0251,0251,0251,0251,0251,0251,0251,
0251,0251,0251,0251,0251,0251,0251,0251,0251,0251,0251,0251,0251,0251,0251,
0251,0251,0251,0250,0250,0250,0250,0250,0250,0250,0250,0250,0250,0250,0250,
0250,0250,0250,0250,0250,0250,0250,0250,0250,0250,0250,0250,0250,0250,0250,
0250,0250,0250,0250,0250,0247,0247,0247,0247,0247,0247,0247,0247,0247,0247,
0247,0247,0247,0247,0247,0247,0247,0247,0247,0247,0247,0247,0247,0247,0247,
0247,0247,0247,0247,0247,0247,0247,0246,0246,0246,0246,0246,0246,0246,0246,
0246,0246,0246,0246,0246,0246,0246,0246,0246,0246,0246,0246,0246,0246,0246,
0246,0246,0246,0246,0246,0246,0246,0246,0246,0245,0245,0245,0245,0245,0245,
0245,0245,0245,0245,0245,0245,0245,0245,0245,0245,0245,0245,0245,0245,0245,
0245,0245,0245,0245,0245,0245,0245,0245,0245,0245,0245,0244,0244,0244,0244,
0244,0244,0244,0244,0244,0244,0244,0244,0244,0244,0244,0244,0244,0244,0244,
0244,0244,0244,0244,0244,0244,0244,0244,0244,0244,0244,0244,0244,0243,0243,
0243,0243,0243,0243,0243,0243,0243,0243,0243,0243,0243,0243,0243,0243,0243,
0243,0243,0243,0243,0243,0243,0243,0243,0243,0243,0243,0243,0243,0243,0243,
0242,0242,0242,0242,0242,0242,0242,0242,0242,0242,0242,0242,0242,0242,0242,
0242,0242,0242,0242,0242,0242,0242,0242,0242,0242,0242,0242,0242,0242,0242,
0242,0242,0242,0241,0241,0241,0241,0241,0241,0241,0241,0241,0241,0241,0241,
0241,0241,0241,0241,0241,0241,0241,0241,0241,0241,0241,0241,0241,0241,0241,
0241,0241,0241,0241,0241,0240,0240,0240,0240,0240,0240,0240,0240,0240,0240,
0240,0240,0240,0240,0240,0240,0240,0240,0240,0240,0240,0240,0240,0240,0240,
0240,0240,0240,0240,0240,0240,0240,0237,0237,0237,0237,0237,0237,0237,0237,
0237,0237,0237,0237,0237,0237,0237,0237,0237,0237,0237,0237,0237,0237,0237,
0237,0237,0237,0237,0237,0237,0237,0237,0237,0237,0237,0237,0237,0237,0237,
0237,0237,0237,0237,0237,0237,0237,0237,0237,0237,0237,0237,0237,0237,0237,
0237,0237,0237,0237,0237,0237,0237,0237,0237,0237,0237,0236,0236,0236,0236,
0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,
0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,
0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,
0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,0236,
0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,
0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,
0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,
0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,0235,
0235,0235,0235,0235,0235,0234,0234,0234,0234,0234,0234,0234,0234,0234,0234,
0234,0234,0234,0234,0234,0234,0234,0234,0234,0234,0234,0234,0234,0234,0234,
0234,0234,0234,0234,0234,0234,0234,0234,0234,0234,0234,0234,0234,0234,0234,
0234,0234,0234,0234,0234,0234,0234,0234,0234,0234,0234,0234,0234,0234,0234,
0234,0234,0234,0234,0234,0234,0234,0234,0234,0233,0233,0233,0233,0233,0233,
0233,0233,0233,0233,0233,0233,0233,0233,0233,0233,0233,0233,0233,0233,0233,
0233,0233,0233,0233,0233,0233,0233,0233,0233,0233,0233,0233,0233,0233,0233,
0233,0233,0233,0233,0233,0233,0233,0233,0233,0233,0233,0233,0233,0233,0233,
0233,0233,0233,0233,0233,0233,0233,0233,0233,0233,0233,0233,0233,0232,0232,
0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,
0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,
0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,
0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,0232,
0232,0232,0231,0231,0231,0231,0231,0231,0231,0231,0231,0231,0231,0231,0231,
0231,0231,0231,0231,0231,0231,0231,0231,0231,0231,0231,0231,0231,0231,0231,
0231,0231,0231,0231,0231,0231,0231,0231,0231,0231,0231,0231,0231,0231,0231,
0231,0231,0231,0231,0231,0231,0231,0231,0231,0231,0231,0231,0231,0231,0231,
0231,0231,0231,0231,0231,0231,0231,0230,0230,0230,0230,0230,0230,0230,0230,
0230,0230,0230,0230,0230,0230,0230,0230,0230,0230,0230,0230,0230,0230,0230,
0230,0230,0230,0230,0230,0230,0230,0230,0230,0230,0230,0230,0230,0230,0230,
0230,0230,0230,0230,0230,0230,0230,0230,0230,0230,0230,0230,0230,0230,0230,
0230,0230,0230,0230,0230,0230,0230,0230,0230,0230,0230,0227,0227,0227,0227,
0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,
0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,
0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,
0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,0227,
0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,
0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,
0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,
0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,0226,
0226,0226,0226,0226,0225,0225,0225,0225,0225,0225,0225,0225,0225,0225,0225,
0225,0225,0225,0225,0225,0225,0225,0225,0225,0225,0225,0225,0225,0225,0225,
0225,0225,0225,0225,0225,0225,0225,0225,0225,0225,0225,0225,0225,0225,0225,
0225,0225,0225,0225,0225,0225,0225,0225,0225,0225,0225,0225,0225,0225,0225,
0225,0225,0225,0225,0225,0225,0225,0225,0225,0224,0224,0224,0224,0224,0224,
0224,0224,0224,0224,0224,0224,0224,0224,0224,0224,0224,0224,0224,0224,0224,
0224,0224,0224,0224,0224,0224,0224,0224,0224,0224,0224,0224,0224,0224,0224,
0224,0224,0224,0224,0224,0224,0224,0224,0224,0224,0224,0224,0224,0224,0224,
0224,0224,0224,0224,0224,0224,0224,0224,0224,0224,0224,0224,0224,0223,0223,
0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,
0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,
0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,
0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,0223,
0223,0223,0222,0222,0222,0222,0222,0222,0222,0222,0222,0222,0222,0222,0222,
0222,0222,0222,0222,0222,0222,0222,0222,0222,0222,0222,0222,0222,0222,0222,
0222,0222,0222,0222,0222,0222,0222,0222,0222,0222,0222,0222,0222,0222,0222,
0222,0222,0222,0222,0222,0222,0222,0222,0222,0222,0222,0222,0222,0222,0222,
0222,0222,0222,0222,0222,0222,0221,0221,0221,0221,0221,0221,0221,0221,0221,
0221,0221,0221,0221,0221,0221,0221,0221,0221,0221,0221,0221,0221,0221,0221,
0221,0221,0221,0221,0221,0221,0221,0221,0221,0221,0221,0221,0221,0221,0221,
0221,0221,0221,0221,0221,0221,0221,0221,0221,0221,0221,0221,0221,0221,0221,
0221,0221,0221,0221,0221,0221,0221,0221,0221,0221,0221,0220,0220,0220,0220,
0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,
0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,
0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,
0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,0220,
0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,
0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,
0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,
0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,
0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,
0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,
0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,
0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,0217,
0217,0217,0217,0217,0217,0217,0217,0217,0217,0216,0216,0216,0216,0216,0216,
0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,
0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,
0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,
0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,
0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,
0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,
0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,
0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,0216,
0216,0216,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,
0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,
0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,
0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,
0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,
0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,
0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,
0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,
0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0215,0214,0214,0214,0214,
0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,
0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,
0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,
0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,
0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,
0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,
0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,
0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,0214,
0214,0214,0214,0214,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,
0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,
0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,
0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,
0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,
0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,
0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,
0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,
0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0213,0212,0212,
0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,
0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,
0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,
0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,
0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,
0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,
0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,
0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,0212,
0212,0212,0212,0212,0212,0212,0211,0211,0211,0211,0211,0211,0211,0211,0211,
0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,
0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,
0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,
0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,
0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,
0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,
0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,
0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,0211,
0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,
0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,
0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,
0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,
0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,
0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,
0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,
0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,0210,
0210,0210,0210,0210,0210,0210,0210,0210,0207,0207,0207,0207,0207,0207,0207,
0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,
0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,
0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,
0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,
0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,
0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,
0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,
0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,0207,
0207,0207,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,
0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,
0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,
0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,
0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,
0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,
0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,
0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,
0206,0206,0206,0206,0206,0206,0206,0206,0206,0206,0205,0205,0205,0205,0205,
0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,
0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,
0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,
0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,
0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,
0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,
0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,
0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,0205,
0205,0205,0205,0205,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,
0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,
0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,
0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,
0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,
0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,
0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,
0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,
0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0204,0203,0203,0203,
0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,
0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,
0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,
0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,
0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,
0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,
0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,
0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,0203,
0203,0203,0203,0203,0203,0203,0202,0202,0202,0202,0202,0202,0202,0202,0202,
0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,
0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,
0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,
0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,
0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,
0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,
0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,
0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0202,0201,
0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,
0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,
0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,
0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,
0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,
0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,
0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,
0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,0201,
0201,0201,0201,0201,0201,0201,0201,0201,0200,0200,0200,0200,0200,0200,0200,
0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,
0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,
0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,
0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,
0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,
0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,
0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,
0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,0200,
0200,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,
0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,
0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,
0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,
0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,
0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,
0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,
0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,
0000,0000,0000,0000,0000,0000,0000,0000,0000,0001,0001,0001,0001,0001,0001,
0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,
0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,
0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,
0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,
0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,
0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,
0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,
0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,0001,
0001,0001,0001,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,
0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,
0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,
0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,
0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,
0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,
0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,
0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,
0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0002,0003,0003,0003,0003,
0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,
0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,
0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,
0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,
0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,
0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,
0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,
0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,0003,
0003,0003,0003,0003,0003,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,
0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,
0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,
0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,
0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,
0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,
0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,
0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,
0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0004,0005,0005,
0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,
0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,
0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,
0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,
0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,
0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,
0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,
0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,0005,
0005,0005,0005,0005,0005,0005,0005,0006,0006,0006,0006,0006,0006,0006,0006,
0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,
0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,
0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,
0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,
0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,
0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,
0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,
0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,0006,
0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,
0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,
0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,
0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,
0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,
0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,
0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,
0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,0007,
0007,0007,0007,0007,0007,0007,0007,0007,0007,0010,0010,0010,0010,0010,0010,
0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,
0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,
0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,
0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,
0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,
0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,
0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,
0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,0010,
0010,0010,0010,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,
0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,
0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,
0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,
0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,
0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,
0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,
0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,
0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0011,0012,0012,0012,0012,
0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,
0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,
0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,
0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,
0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,
0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,
0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,
0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,0012,
0012,0012,0012,0012,0012,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,
0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,
0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,
0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,
0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,
0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,
0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,
0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,
0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0013,0014,0014,
0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,
0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,
0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,
0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,
0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,
0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,
0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,
0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,0014,
0014,0014,0014,0014,0014,0014,0014,0015,0015,0015,0015,0015,0015,0015,0015,
0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,
0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,
0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,
0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,
0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,
0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,
0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,
0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,
0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,
0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,
0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,
0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,
0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,
0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,
0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,
0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,0016,
0016,0016,0016,0016,0016,0016,0016,0016,0016,0017,0017,0017,0017,0017,0017,
0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,
0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,
0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,
0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,
0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,
0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,
0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,
0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,0017,
0017,0017,0020,0020,0020,0020,0020,0020,0020,0020,0020,0020,0020,0020,0020,
0020,0020,0020,0020,0020,0020,0020,0020,0020,0020,0020,0020,0020,0020,0020,
0020,0020,0020,0020,0020,0020,0020,0020,0020,0020,0020,0020,0020,0020,0020,
0020,0020,0020,0020,0020,0020,0020,0020,0020,0020,0020,0020,0020,0020,0020,
0020,0020,0020,0020,0020,0020,0021,0021,0021,0021,0021,0021,0021,0021,0021,
0021,0021,0021,0021,0021,0021,0021,0021,0021,0021,0021,0021,0021,0021,0021,
0021,0021,0021,0021,0021,0021,0021,0021,0021,0021,0021,0021,0021,0021,0021,
0021,0021,0021,0021,0021,0021,0021,0021,0021,0021,0021,0021,0021,0021,0021,
0021,0021,0021,0021,0021,0021,0021,0021,0021,0021,0021,0022,0022,0022,0022,
0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,
0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,
0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,
0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,0022,
0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,
0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,
0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,
0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,0023,
0023,0023,0023,0023,0024,0024,0024,0024,0024,0024,0024,0024,0024,0024,0024,
0024,0024,0024,0024,0024,0024,0024,0024,0024,0024,0024,0024,0024,0024,0024,
0024,0024,0024,0024,0024,0024,0024,0024,0024,0024,0024,0024,0024,0024,0024,
0024,0024,0024,0024,0024,0024,0024,0024,0024,0024,0024,0024,0024,0024,0024,
0024,0024,0024,0024,0024,0024,0024,0024,0024,0025,0025,0025,0025,0025,0025,
0025,0025,0025,0025,0025,0025,0025,0025,0025,0025,0025,0025,0025,0025,0025,
0025,0025,0025,0025,0025,0025,0025,0025,0025,0025,0025,0025,0025,0025,0025,
0025,0025,0025,0025,0025,0025,0025,0025,0025,0025,0025,0025,0025,0025,0025,
0025,0025,0025,0025,0025,0025,0025,0025,0025,0025,0025,0025,0025,0026,0026,
0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,
0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,
0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,
0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,0026,
0026,0026,0027,0027,0027,0027,0027,0027,0027,0027,0027,0027,0027,0027,0027,
0027,0027,0027,0027,0027,0027,0027,0027,0027,0027,0027,0027,0027,0027,0027,
0027,0027,0027,0027,0027,0027,0027,0027,0027,0027,0027,0027,0027,0027,0027,
0027,0027,0027,0027,0027,0027,0027,0027,0027,0027,0027,0027,0027,0027,0027,
0027,0027,0027,0027,0027,0027,0030,0030,0030,0030,0030,0030,0030,0030,0030,
0030,0030,0030,0030,0030,0030,0030,0030,0030,0030,0030,0030,0030,0030,0030,
0030,0030,0030,0030,0030,0030,0030,0030,0030,0030,0030,0030,0030,0030,0030,
0030,0030,0030,0030,0030,0030,0030,0030,0030,0030,0030,0030,0030,0030,0030,
0030,0030,0030,0030,0030,0030,0030,0030,0030,0030,0030,0031,0031,0031,0031,
0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,
0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,
0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,
0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,0031,
0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,
0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,
0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,
0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,0032,
0032,0032,0032,0032,0033,0033,0033,0033,0033,0033,0033,0033,0033,0033,0033,
0033,0033,0033,0033,0033,0033,0033,0033,0033,0033,0033,0033,0033,0033,0033,
0033,0033,0033,0033,0033,0033,0033,0033,0033,0033,0033,0033,0033,0033,0033,
0033,0033,0033,0033,0033,0033,0033,0033,0033,0033,0033,0033,0033,0033,0033,
0033,0033,0033,0033,0033,0033,0033,0033,0034,0034,0034,0034,0034,0034,0034,
0034,0034,0034,0034,0034,0034,0034,0034,0034,0034,0034,0034,0034,0034,0034,
0034,0034,0034,0034,0034,0034,0034,0034,0034,0034,0034,0034,0034,0034,0034,
0034,0034,0034,0034,0034,0034,0034,0034,0034,0034,0034,0034,0034,0034,0034,
0034,0034,0034,0034,0034,0034,0034,0034,0034,0034,0034,0034,0034,0035,0035,
0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,
0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,
0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,
0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,0035,
0035,0035,0036,0036,0036,0036,0036,0036,0036,0036,0036,0036,0036,0036,0036,
0036,0036,0036,0036,0036,0036,0036,0036,0036,0036,0036,0036,0036,0036,0036,
0036,0036,0036,0036,0036,0036,0036,0036,0036,0036,0036,0036,0036,0036,0036,
0036,0036,0036,0036,0036,0036,0036,0036,0036,0036,0036,0036,0036,0036,0036,
0036,0036,0036,0036,0036,0036,0037,0037,0037,0037,0037,0037,0037,0037,0037,
0037,0037,0037,0037,0037,0037,0037,0037,0037,0037,0037,0037,0037,0037,0037,
0037,0037,0037,0037,0037,0037,0037,0037,0037,0037,0037,0037,0037,0037,0037,
0037,0037,0037,0037,0037,0037,0037,0037,0037,0037,0037,0037,0037,0037,0037,
0037,0037,0037,0037,0037,0037,0037,0037,0037,0037,0040,0040,0040,0040,0040,
0040,0040,0040,0040,0040,0040,0040,0040,0040,0040,0040,0040,0040,0040,0040,
0040,0040,0040,0040,0040,0040,0040,0040,0040,0040,0040,0040,0040,0041,0041,
0041,0041,0041,0041,0041,0041,0041,0041,0041,0041,0041,0041,0041,0041,0041,
0041,0041,0041,0041,0041,0041,0041,0041,0041,0041,0041,0041,0041,0041,0041,
0042,0042,0042,0042,0042,0042,0042,0042,0042,0042,0042,0042,0042,0042,0042,
0042,0042,0042,0042,0042,0042,0042,0042,0042,0042,0042,0042,0042,0042,0042,
0042,0042,0043,0043,0043,0043,0043,0043,0043,0043,0043,0043,0043,0043,0043,
0043,0043,0043,0043,0043,0043,0043,0043,0043,0043,0043,0043,0043,0043,0043,
0043,0043,0043,0043,0044,0044,0044,0044,0044,0044,0044,0044,0044,0044,0044,
0044,0044,0044,0044,0044,0044,0044,0044,0044,0044,0044,0044,0044,0044,0044,
0044,0044,0044,0044,0044,0044,0045,0045,0045,0045,0045,0045,0045,0045,0045,
0045,0045,0045,0045,0045,0045,0045,0045,0045,0045,0045,0045,0045,0045,0045,
0045,0045,0045,0045,0045,0045,0045,0045,0046,0046,0046,0046,0046,0046,0046,
0046,0046,0046,0046,0046,0046,0046,0046,0046,0046,0046,0046,0046,0046,0046,
0046,0046,0046,0046,0046,0046,0046,0046,0046,0046,0047,0047,0047,0047,0047,
0047,0047,0047,0047,0047,0047,0047,0047,0047,0047,0047,0047,0047,0047,0047,
0047,0047,0047,0047,0047,0047,0047,0047,0047,0047,0047,0047,0047,0050,0050,
0050,0050,0050,0050,0050,0050,0050,0050,0050,0050,0050,0050,0050,0050,0050,
0050,0050,0050,0050,0050,0050,0050,0050,0050,0050,0050,0050,0050,0050,0050,
0051,0051,0051,0051,0051,0051,0051,0051,0051,0051,0051,0051,0051,0051,0051,
0051,0051,0051,0051,0051,0051,0051,0051,0051,0051,0051,0051,0051,0051,0051,
0051,0051,0052,0052,0052,0052,0052,0052,0052,0052,0052,0052,0052,0052,0052,
0052,0052,0052,0052,0052,0052,0052,0052,0052,0052,0052,0052,0052,0052,0052,
0052,0052,0052,0052,0053,0053,0053,0053,0053,0053,0053,0053,0053,0053,0053,
0053,0053,0053,0053,0053,0053,0053,0053,0053,0053,0053,0053,0053,0053,0053,
0053,0053,0053,0053,0053,0053,0054,0054,0054,0054,0054,0054,0054,0054,0054,
0054,0054,0054,0054,0054,0054,0054,0054,0054,0054,0054,0054,0054,0054,0054,
0054,0054,0054,0054,0054,0054,0054,0054,0055,0055,0055,0055,0055,0055,0055,
0055,0055,0055,0055,0055,0055,0055,0055,0055,0055,0055,0055,0055,0055,0055,
0055,0055,0055,0055,0055,0055,0055,0055,0055,0055,0056,0056,0056,0056,0056,
0056,0056,0056,0056,0056,0056,0056,0056,0056,0056,0056,0056,0056,0056,0056,
0056,0056,0056,0056,0056,0056,0056,0056,0056,0056,0056,0056,0057,0057,0057,
0057,0057,0057,0057,0057,0057,0057,0057,0057,0057,0057,0057,0057,0057,0057,
0057,0057,0057,0057,0057,0057,0057,0057,0057,0057,0057,0057,0057,0057,0057,
0060,0060,0060,0060,0060,0060,0060,0060,0060,0060,0060,0060,0060,0060,0060,
0060,0061,0061,0061,0061,0061,0061,0061,0061,0061,0061,0061,0061,0061,0061,
0061,0061,0062,0062,0062,0062,0062,0062,0062,0062,0062,0062,0062,0062,0062,
0062,0062,0062,0063,0063,0063,0063,0063,0063,0063,0063,0063,0063,0063,0063,
0063,0063,0063,0063,0064,0064,0064,0064,0064,0064,0064,0064,0064,0064,0064,
0064,0064,0064,0064,0064,0065,0065,0065,0065,0065,0065,0065,0065,0065,0065,
0065,0065,0065,0065,0065,0065,0066,0066,0066,0066,0066,0066,0066,0066,0066,
0066,0066,0066,0066,0066,0066,0066,0067,0067,0067,0067,0067,0067,0067,0067,
0067,0067,0067,0067,0067,0067,0067,0067,0070,0070,0070,0070,0070,0070,0070,
0070,0070,0070,0070,0070,0070,0070,0070,0070,0071,0071,0071,0071,0071,0071,
0071,0071,0071,0071,0071,0071,0071,0071,0071,0071,0072,0072,0072,0072,0072,
0072,0072,0072,0072,0072,0072,0072,0072,0072,0072,0072,0073,0073,0073,0073,
0073,0073,0073,0073,0073,0073,0073,0073,0073,0073,0073,0073,0074,0074,0074,
0074,0074,0074,0074,0074,0074,0074,0074,0074,0074,0074,0074,0074,0075,0075,
0075,0075,0075,0075,0075,0075,0075,0075,0075,0075,0075,0075,0075,0075,0075,
0076,0076,0076,0076,0076,0076,0076,0076,0076,0076,0076,0076,0076,0076,0076,
0076,0077,0077,0077,0077,0077,0077,0077,0077,0077,0077,0077,0077,0077,0077,
0077,0077,0100,0100,0100,0100,0100,0100,0100,0100,0101,0101,0101,0101,0101,
0101,0101,0101,0102,0102,0102,0102,0102,0102,0102,0102,0103,0103,0103,0103,
0103,0103,0103,0103,0104,0104,0104,0104,0104,0104,0104,0104,0105,0105,0105,
0105,0105,0105,0105,0105,0106,0106,0106,0106,0106,0106,0106,0106,0107,0107,
0107,0107,0107,0107,0107,0107,0110,0110,0110,0110,0110,0110,0110,0110,0111,
0111,0111,0111,0111,0111,0111,0111,0112,0112,0112,0112,0112,0112,0112,0112,
0113,0113,0113,0113,0113,0113,0113,0113,0114,0114,0114,0114,0114,0114,0114,
0114,0115,0115,0115,0115,0115,0115,0115,0115,0116,0116,0116,0116,0116,0116,
0116,0116,0117,0117,0117,0117,0117,0117,0117,0117,0120,0120,0120,0120,0121,
0121,0121,0121,0122,0122,0122,0122,0123,0123,0123,0123,0124,0124,0124,0124,
0125,0125,0125,0125,0126,0126,0126,0126,0127,0127,0127,0127,0130,0130,0130,
0130,0131,0131,0131,0131,0132,0132,0132,0132,0133,0133,0133,0133,0134,0134,
0134,0134,0135,0135,0135,0135,0136,0136,0136,0136,0137,0137,0137,0137,0140,
0140,0141,0141,0142,0142,0143,0143,0144,0144,0145,0145,0146,0146,0147,0147,
0150,0150,0150,0151,0151,0152,0152,0153,0153,0154,0154,0155,0155,0156,0156,
0157,0157,0160,0161,0162,0163,0164,0165,0166,0167,0170,0171,0172,0173,0174,
0175,0176
};

int ulaw_input P1((buf), gsm_signal * buf)
{
 int i, c;

 for (i = 0; i < 160 && (c = fgetc(in)) != EOF; i++) buf[i] = (u2s[ (unsigned char)(c) ]);
 if (c == EOF && ferror(in)) return -1;
 return i;
}

int ulaw_output P1((buf), gsm_signal * buf)
{
 int i;

 for(i = 0; i < 160; i++, buf++)
  if (fputc( (char)(s2u[ ((unsigned short)((unsigned short)*buf)) >> 3 ]), out) == EOF)
   return -1;
 return 0;
}
