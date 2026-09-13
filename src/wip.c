#include "wip.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <process.h>
#include <windows.h>
#include <wincrypt.h>
#pragma comment(lib,"advapi32.lib")
static FILE *journal;
static int continue_wip;
static RRJMemory *bound_memory;
static uint32_t current_frame;
static unsigned long sequence,tainted;
static int executable_hash(char out[65]){
 char path[MAX_PATH];unsigned char data[16384],digest[32];DWORD size=32,n;size_t count;FILE *f;
 HCRYPTPROV provider=0;HCRYPTHASH hash=0;int ok=0,i;
 n=GetModuleFileNameA(NULL,path,sizeof(path));if(!n||n>=sizeof(path))return 0;
 f=fopen(path,"rb");if(!f)return 0;
 if(!CryptAcquireContextA(&provider,NULL,NULL,PROV_RSA_AES,CRYPT_VERIFYCONTEXT)||!CryptCreateHash(provider,CALG_SHA_256,0,0,&hash))goto done;
 while((count=fread(data,1,sizeof(data),f))!=0)if(!CryptHashData(hash,data,(DWORD)count,0))goto done;
 if(ferror(f)||!CryptGetHashParam(hash,HP_HASHVAL,digest,&size,0)||size!=32)goto done;
 for(i=0;i<32;i++)sprintf(out+2*i,"%02x",digest[i]);ok=1;
done:
 if(hash)CryptDestroyHash(hash);if(provider)CryptReleaseContext(provider,0);fclose(f);return ok;
}

static void close_journal(void){if(journal){fclose(journal);journal=NULL;}}
static void quoted(const char *s){
 fputc('"',journal);
 for(;*s;s++){unsigned char c=(unsigned char)*s;if(c=='"'||c=='\\')fputc('\\',journal);if(c<32)fprintf(journal,"\\u%04x",c);else fputc(c,journal);}
 fputc('"',journal);
}
/* Logging must never call rrj_at: invalid pointers are themselves loggable. */
static uint32_t peek(RRJMemory *m,uint32_t address,unsigned bytes){
 uint32_t p=address&0x1fffffff,value=0;unsigned i;
 if(!m||!m->ram||p>0x200000||bytes>0x200000-p)return 0;
 for(i=0;i<bytes;i++)value|=(uint32_t)m->ram[p+i]<<(8*i);
 return value;
}
void rrj_wip_frame(uint32_t frame){current_frame=frame;}
int rrj_wip_options(int *argc,char **argv,RRJMemory *m){
 const char *path=NULL;char hash[65],default_path[MAX_PATH];int i,out=1,debug=0,explicit_continue=0;
 bound_memory=m;
 for(i=1;i<*argc;i++){
  if(!strcmp(argv[i],"--wip-continue"))explicit_continue=1;
  else if(!strcmp(argv[i],"--debug"))debug=1;
  else if(!strcmp(argv[i],"--wip-log")){
   if(++i>=*argc||!argv[i][0]){fputs("--wip-log requires a file path\n",stderr);return 0;}path=argv[i];
  }else argv[out++]=argv[i];
 }
 *argc=out;argv[out]=NULL;
 /* Old differential probes stay strict, including probes invoked by historical tools. */
 continue_wip=!debug&&(explicit_continue||out==1||!strcmp(argv[1],"--menu-run"));
 if(!path&&!continue_wip)return 1;
 if(!executable_hash(hash)){fputs("Cannot fingerprint RRJ.exe for WIP journal\n",stderr);return 0;}
 if(!path){
  DWORD n=GetModuleFileNameA(NULL,default_path,sizeof default_path);char *slash;
  if(!n||n>=sizeof default_path)return 0;
  slash=strrchr(default_path,'\\');if(!slash||(size_t)(slash-default_path)+20>=sizeof default_path)return 0;
  strcpy(slash+1,"wip-branches.jsonl");path=default_path;
 }
 journal=fopen(path,"ab");if(!journal){fprintf(stderr,"Cannot open WIP journal: %s\n",path);return 0;}
 atexit(close_journal);
 fprintf(journal,"{\"event\":\"session\",\"pid\":%d,\"time\":%lld,\"continue\":%s,\"build\":",_getpid(),(long long)time(NULL),continue_wip?"true":"false");
 quoted(__DATE__ " " __TIME__);fputs(",\"exe_sha256\":",journal);quoted(hash);fputs("}\n",journal);
 if(fflush(journal)||ferror(journal)){fputs("Cannot write WIP journal\n",stderr);return 0;}
 if(continue_wip)fprintf(stderr,"WIP discovery enabled. Journal: %s\n",path);
 return 1;
}
void rrj_wip_site(RRJMemory *m,uint32_t pc,const char *kind,const char *subsystem,const char *function,const char *file,int line,int recoverable,const char *fallback,const uint32_t *args,unsigned count){
 unsigned i;int resume=continue_wip&&recoverable;
 if(journal){
  uint32_t context=peek(m,0x8005B2F8,4);
  fprintf(journal,"{\"event\":\"wip\",\"pid\":%d,\"sequence\":%lu,\"time\":%lld,\"pc\":\"%08X\",\"pc_kind\":",_getpid(),++sequence,(long long)time(NULL),pc);quoted(kind);
  fputs(",\"subsystem\":",journal);quoted(subsystem);fputs(",\"function\":",journal);quoted(function);fputs(",\"file\":",journal);quoted(file);
  fprintf(journal,",\"line\":%d,\"iteration\":%u,\"vblank\":%u,\"game_context\":\"%08X\",\"game_state\":%u,\"menu\":%u,\"continued\":%s,\"prior_skips\":%lu,\"fallback\":",line,current_frame,peek(m,0x8005B46C,4),context,peek(m,context,1),peek(m,0x8009C5D0,2),resume?"true":"false",tainted);quoted(resume?fallback:"abort");
  fputs(",\"args\":[",journal);for(i=0;i<count;i++)fprintf(journal,"%s\"%08X\"",i?",":"",args[i]);
  fputs("],\"input_snapshot_hex\":\"",journal);for(i=0;i<768;i++)fprintf(journal,"%02X",peek(m,0x800D7128+i,1));
  fputs("\"}\n",journal);
  if(fflush(journal)||ferror(journal)){fputs("WIP journal write failed\n",stderr);abort();}
 }
 if(resume){++tainted;return;}
 fprintf(stderr,"WIP/guard stop %08X %s (%s:%d)\n",pc,function,file,line);abort();
}
__declspec(noreturn) void rrj_wip_stop(const char *function,const char *file,int line){
 rrj_wip_site(bound_memory,0,"unknown","guard_or_unclassified",function,file,line,0,"abort",NULL,0);abort();
}
