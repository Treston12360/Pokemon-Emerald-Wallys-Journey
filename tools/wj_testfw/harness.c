#include <mgba/internal/arm/arm.h>
#include <mgba-util/vfs.h>
#include <fcntl.h>
#include <mgba/core/core.h>
#include <mgba/core/interface.h>
#include <mgba/core/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// usage: harness rom script rtc_epoch_seconds
static int keymask(const char *s){int k=0; if(strchr(s,'A'))k|=1; if(strchr(s,'B'))k|=2; if(strchr(s,'s'))k|=4; if(strchr(s,'S'))k|=8;
 if(strchr(s,'R'))k|=16; if(strchr(s,'L'))k|=32; if(strchr(s,'U'))k|=64; if(strchr(s,'D'))k|=128; if(strchr(s,'r'))k|=256; if(strchr(s,'l'))k|=512; return k;}
int main(int argc,char**argv){
 struct mCore*core=mCoreFind(argv[1]); if(!core){puts("nocore");return 1;}
 core->init(core); mCoreInitConfig(core,NULL);
 unsigned w=240,h=160;
 uint32_t*buf=malloc(w*h*4); core->setVideoBuffer(core,(void*)buf,w);
 if(!mCoreLoadFile(core,argv[1])){puts("noload");return 1;}
 if(argc>4){ struct VFile*sv=VFileOpen(argv[4],O_RDWR); if(!sv||!core->loadSave(core,sv)){puts("nosave");return 1;} }
 core->rtc.override=RTC_FIXED; core->rtc.value=atoll(argv[3])*1000LL;
 core->reset(core);
 FILE*f=fopen(argv[2],"r"); char line[256];
 while(fgets(line,sizeof line,f)){
  char a[64]="",b[64]=""; int n=sscanf(line,"%63s %63s",a,b);
  if(n<1||a[0]=='#') continue;
  if(!strcmp(a,"pc")){ struct ARMCore*c=core->cpu; fprintf(stderr,"PC"); for(int r=0;r<16;r++) fprintf(stderr," r%d=%08X",r,c->gprs[r]); fprintf(stderr," cpsr=%08X\n",c->cpsr.packed); continue;}
  if(!strcmp(a,"dump")){ unsigned long ad=strtoul(b,0,16); FILE*o=fopen("dump.bin","wb"); for(int i=0;i<4096;i++) fputc(core->busRead8(core,ad+i),o); fclose(o); continue;}
  if(!strcmp(a,"spr")){ unsigned long ad=strtoul(b,0,16); int c=0; for(int i=0;i<64;i++){ if(core->busRead8(core,ad+i*0x44+0x3E)&1) c++; } fprintf(stderr,"SPR %d\n",c); continue;}
  if(!strcmp(a,"peek")){ unsigned long ad=strtoul(b,0,16); fprintf(stderr,"PEEK %s:",b); for(int i=0;i<12;i++) fprintf(stderr," %02X",core->busRead8(core,ad+i)); fprintf(stderr,"\n"); continue;}
  if(!strcmp(a,"shot")){ char fn[300]; snprintf(fn,sizeof fn,"%s.ppm",b); FILE*o=fopen(fn,"wb"); fprintf(o,"P6\n%u %u\n255\n",w,h);
    for(unsigned i=0;i<w*h;i++){uint32_t p=buf[i]; unsigned char c[3]={p&0xFF,(p>>8)&0xFF,(p>>16)&0xFF}; fwrite(c,1,3,o);} fclose(o); continue;}
  int frames=atoi(a); int k=n>1?keymask(b):0;
  for(int i=0;i<frames;i++){ core->setKeys(core,k); core->runFrame(core);} 
 }
 core->deinit(core); return 0;}
