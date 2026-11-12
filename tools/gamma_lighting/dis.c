#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct { void **vt; } Blob;
typedef HRESULT (WINAPI *PFN)(const void*, SIZE_T, UINT, const char*, Blob**);
int main(int c, char**v){
  HMODULE m=LoadLibraryA("d3dcompiler_47.dll"); if(!m){printf("no dll\n");return 1;}
  PFN f=(PFN)GetProcAddress(m,"D3DDisassemble");
  for(int i=1;i<c;i++){ FILE*fp=fopen(v[i],"rb"); fseek(fp,0,2); long n=ftell(fp); fseek(fp,0,0); char*b=malloc(n); fread(b,1,n,fp); fclose(fp);
    Blob*o=0; HRESULT h=f(b,n,0,0,&o); char out[1024]; snprintf(out,sizeof out,"%s.asm",v[i]);
    if(h==0&&o){ void*(WINAPI*ptr)(Blob*)=(void*)o->vt[3]; SIZE_T(WINAPI*sz)(Blob*)=(void*)o->vt[4]; FILE*w=fopen(out,"wb"); fwrite(ptr(o),1,sz(o),w); fclose(w);} else printf("fail %s %lx\n",v[i],h); }
  return 0; }
