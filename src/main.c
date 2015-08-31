
#include <stdio.h>
#include <memory.h>
#include "crypto.h"
#include "timer.h"
#include <unistd.h>

typedef struct 
{
  char essid[36];
  unsigned char mac1[6];
  unsigned char mac2[6];
  unsigned char nonce1[32];
  unsigned char nonce2[32];
  
  unsigned char eapol[256];
  int eapol_size;
  
  int keyver;
  unsigned char keymic[16];
} hccap_t;

int hccap_load(char *fn,hccap_t *hc)
{
  FILE *fp;
  int res;

  fp = fopen(fn,"rb");
  if(fp)
    {
      if((res = fread(hc,1,sizeof(hccap_t),fp)) != sizeof(hccap_t))
	{
	  return -2; //something wrong
	}
      else
	return 0;
    }
  else
    return -1;
}

void show_wpa_stats(char *key, int keylen, unsigned char pmk[32], unsigned char ptk[64], 
unsigned char mic[16])
{
  int i;
  char tmpbuf[28];

  memset(tmpbuf, ' ',sizeof(tmpbuf));
  memcpy(tmpbuf,key,keylen > 27 ? 27 : keylen);
  tmpbuf[27] = '\0';
  printf("\33[8;24HCurrent passphrase: %s\n",tmpbuf);
  printf("\33[11;7HMaster Key     : ");
  for(i=0;i<32;i++)
    {
      if(i == 16) printf("\n\33[23C");
      printf("%02X ",pmk[i]);
    }
  printf( "\33[14;7HTransient Key  : " );
  for(i=0;i<64;i++)
    {
      if(i >0 && i % 16 == 0) printf("\n\33[23C");
      printf("%02X ", ptk[i]);
    }
 
  printf("\33[19;7HEAPOL HMAC     : ");
  for(i=0;i<16; i++)
    printf("%02X ",mic[i]);
  printf("\n");
}

void dump_key(char *name, unsigned char *key,int len)
{
  int i;
  printf("%s : ",name);
  for(i=0;i<len;i++)
    printf("%02X ",key[i]);
  printf("\n");
}

void prof_pmk(char *essid)
{
  TIME_STRUCT p1,p2;

  char key[16][128];
  unsigned char pmk_sol[16][40];
  unsigned char pmk_fast[16][40];

  int i,j;
  for(i=0;i<16;i++)
    {
      strcpy(key[i],"atest");
      key[i][0]+=i;
    }

  p1 = t_start();
  for(i=0;i<16;i++)
    calc_pmk(key[i],essid,pmk_sol[i]); //key값과 essid로부터 pmk 값을 계산함
  t_end(&p1);

  p2 = t_start();
  calc_16pmk(key,essid,pmk_fast);
  t_end(&p2);

  //diff
  for(i=0;i<16;i++)
    {
      if(memcmp(pmk_sol[i],pmk_fast[i],sizeof(pmk_sol[i])) != 0)
	{
	  printf("* %d wrong case (key:%s)\n",i,key[i]);
	  dump_key("pmk_sol",pmk_sol[i],sizeof(pmk_sol[i]));
	  dump_key("pmk_fst",pmk_fast[i],sizeof(pmk_fast[i]));	  
	}
    }  
  printf("original : %0.2lf ms\n",t_get(p1)/1000);
  printf("simd ver : %0.2lf ms\n",t_get(p2)/1000);
  printf("performance : x%0.2lf\n",t_get(p1)/t_get(p2));
}

void crack_wpa(char *fn)
{
  hccap_t hc;
  int res, i, itr;

  unsigned char pmk[128];

  unsigned char pke[100];
  unsigned char ptk[80];
  unsigned char mic[20];

  char key[128]; // passphase 키값
  int keylen;

  memset(key,0,sizeof(key));
  strcpy(key,"dekdekdek");

  keylen = strlen(key);
  // load from file
  if((res = hccap_load(fn,&hc)))
    {
      printf("hashcat file load failed! code : %d\n",res);
      return ;
    }
  
  memcpy( pke,"Pairwise key expansion", 23);
  if(memcmp(hc.mac2,hc.mac1,6) < 0) {
    memcpy( pke + 23, hc.mac2, 6);
    memcpy( pke + 29, hc.mac1,6 );
  } else {
    memcpy(pke + 23, hc.mac1, 6);
    memcpy(pke + 29, hc.mac2, 6);
  }
  
  if( memcmp( hc.nonce1,hc.nonce2, 32) < 0 ) {
    memcpy(pke + 35, hc.nonce1,32);
    memcpy(pke + 67, hc.nonce2,32);
  }
  else {
    memcpy(pke + 35, hc.nonce2,32);
    memcpy(pke + 67, hc.nonce1,32);    
  }

  #pragma omp parallel
  #pragma omp master
  printf("Initializing ... %d threads\n",omp_get_num_threads());


  //  prof_pmk(hc.essid);
  //return;
  //#pragma omp parallel for private(i) shared(key,hc,pmk,pke,mic)

  {
    char key[16][128];
    unsigned char pmk_sol[16][40];
    unsigned char pmk_fast[16][40];

    TIME_STRUCT p1;

    int i,j;
    for(i=0;i<16;i++)
      {
	strcpy(key[i],"atest");
	key[i][0]+=i;
      }
    p1 = t_start();
#pragma omp parallel for
    for(itr=0;itr<16000;itr+=16)
      {
	calc_16pmk(key,hc.essid,pmk_fast); //key값과 essid로부터 pmk 값을 계산함
	//t_end(&p1);

	//      p2 = t_start();
	for(i=0;i<4;i++)
	  {
	    pke[99] = i;
	    HMAC(EVP_sha1(), pmk, 32, pke, 100, ptk + i * 20, NULL);
	  }
      
	if(hc.keyver == 1)
	  HMAC(EVP_md5(), ptk, 16,hc.eapol,hc.eapol_size,mic,NULL);
	else
	  HMAC(EVP_sha1(), ptk, 16, hc.eapol,hc.eapol_size,mic,NULL);
      
	//      show_wpa_stats(key,keylen,pmk,ptk,mic);
      
	if(memcmp(mic,hc.keymic,16) == 0)
	  {
	    //	  printf("success\n");
	    //	  return ;
	  }
	//      t_end(&p2);
      }
    t_end(&p1);
    printf("time : %0.2lf ms\n",t_get(p1)/1000);
  }
  //  printf("failed\n");
  return;
}

void print_usage(char *fn)
{
	printf(" WPA-PSK Key Cracker for Xeon phi\n\n");
	printf(" usage : %s [hashcat file]\n",fn);
}

int main(int argc,char **argv)
{
	if(argc == 2)
		crack_wpa(argv[1]);
	else
		print_usage(argv[0]);
	
	return 0;
}
