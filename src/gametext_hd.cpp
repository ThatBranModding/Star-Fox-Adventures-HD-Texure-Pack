#include "foxhollow_mod_api.h"
#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#if defined(_WIN32)
#include <windows.h>
#endif

static FhMod* M{}; static const FhModHost* H{};
using FinalizeFn=void(*)(void*); using SelectFn=void(*)(void*,int);
static void* tFinalize{}; static void* tSelect{}; static FinalizeFn oFinalize{}; static SelectFn oSelect{};
static void log(FhLogLevel l,const std::string&s){if(H&&H->log)H->log(M,l,s.c_str());}
static uint16_t be16(const uint8_t*p){return uint16_t(p[0])<<8|p[1];}
static uint32_t be32(const uint8_t*p){return uint32_t(p[0])<<24|uint32_t(p[1])<<16|uint32_t(p[2])<<8|p[3];}
static uint64_t hash64(const uint8_t*p,size_t n){uint64_t h=1469598103934665603ull;while(n--){h^=*p++;h*=1099511628211ull;}return h;}
#if defined(_WIN32)
static bool readable(const void*p,size_t n){if(!p||!n)return false;uintptr_t c=(uintptr_t)p,e=c+n;if(e<c)return false;while(c<e){MEMORY_BASIC_INFORMATION m{};if(!VirtualQuery((void*)c,&m,sizeof(m))||m.State!=MEM_COMMIT||(m.Protect&PAGE_GUARD)||(m.Protect&0xff)==PAGE_NOACCESS)return false;uintptr_t q=(uintptr_t)m.BaseAddress+m.RegionSize;if(q<=c)return false;c=q;}return true;}
#else
static bool readable(const void*p,size_t n){return p&&n;}
#endif
struct Sprite{uint8_t w{},h{};std::vector<uint8_t> px;}; static std::unordered_map<uint32_t,Sprite> sprites;
struct Rep{uint16_t w{},h{};std::vector<uint8_t> px,obj;}; static std::unordered_map<uint64_t,Rep> reps;
static std::string path(const char*leaf){std::string p=H->modDir(M);if(!p.empty()&&p.back()!='/'&&p.back()!='\\')p+='/';return p+leaf;}
static bool loadSprites(){std::ifstream f(path("assets/glyphs.bin"),std::ios::binary);if(!f)return false;char m[4];uint32_t n=0;f.read(m,4);f.read((char*)&n,4);if(std::memcmp(m,"GTHD",4)||n>256)return false;for(uint32_t i=0;i<n;i++){uint32_t c;uint8_t w,h;uint16_t z;f.read((char*)&c,4);f.read((char*)&w,1);f.read((char*)&h,1);f.read((char*)&z,2);Sprite s;s.w=w;s.h=h;s.px.resize(size_t(w)*h);f.read((char*)s.px.data(),s.px.size());if(!f)return false;sprites.emplace(c,std::move(s));}return sprites.size()>=94;}
static size_t i4off(uint32_t w,uint32_t x,uint32_t y){return ((y>>3)*(w>>3)+(x>>3))*32+(y&7)*4+((x&7)>>1);}
static uint8_t getI4(const uint8_t*p,uint32_t w,uint32_t x,uint32_t y){uint8_t b=p[i4off(w,x,y)];return (x&1)?(b&15):(b>>4);}
static void setI4(uint8_t*p,uint32_t w,uint32_t x,uint32_t y,uint8_t v){auto o=i4off(w,x,y);if(x&1)p[o]=(p[o]&0xf0)|(v&15);else p[o]=(p[o]&0x0f)|(v<<4);}
struct G{uint32_t c;uint16_t x,y;uint8_t width,height,font,page;};
static void prepare(const uint8_t*b,size_t sz){
 if(!b||sz<32)return;uint32_t gc=be32(b);if(!gc||gc>4096||4ull+uint64_t(gc)*16>sz)return;
 std::vector<G> gs;gs.reserve(gc);for(uint32_t i=0;i<gc;i++){auto*r=b+4+i*16;G g{be32(r),be16(r+4),be16(r+6),r[12],r[13],r[14],r[15]};if(g.font==4&&sprites.count(g.c))gs.push_back(g);}if(gs.empty())return;
 size_t hdr=uint64_t(gc)*16;if(hdr+8>sz)return;uint16_t ec=be16(b+hdr+4),to=be16(b+hdr+6);size_t st=hdr+8+size_t(ec)*12;if(st+4>sz)return;uint32_t sc=be32(b+st);size_t text=st+4+size_t(sc)*4;if(text>sz||to>sz-text)return;size_t th=text+to;if(th+4>sz)return;uint32_t rel=be32(b+th);size_t pos=th+rel+4;
 for(unsigned page=0;page<16&&pos+8<=sz;page++){
   uint16_t kind=be16(b+pos),bpp=be16(b+pos+2),w=be16(b+pos+4),h=be16(b+pos+6);pos+=8;if(!w&&!h)break;size_t bytes=size_t(w)*h*bpp/8;if(pos+bytes>sz)break;
   bool latin=false;for(auto&g:gs)if(g.page==page){latin=true;break;}
   if(latin&&bpp==4&&w%8==0&&h%8==0){uint64_t sig=hash64(b+pos,bytes);uint32_t nw=w*2,nh=h*2;Rep r;r.w=w;r.h=h;r.px.assign(size_t(nw)*nh/2,0);
     for(uint32_t y=0;y<nh;y++)for(uint32_t x=0;x<nw;x++)setI4(r.px.data(),nw,x,y,getI4(b+pos,w,x/2,y/2));
     for(auto&g:gs)if(g.page==page){auto it=sprites.find(g.c);if(it==sprites.end())continue;auto&s=it->second;int cw=int(g.width)*2,ch=int(g.height)*2;int cellX=int(g.x)*2,cellY=int(g.y)*2;
       // Never let HD artwork escape the logical 2x GameText glyph rectangle. Oversize
       // source-sheet glyphs can contaminate a neighbouring glyph's UV sampling (notably
       // ">" immediately after "%" in the dynamic Latin atlas). Keep retail for those rare
       // glyphs rather than crop/rescale the supplied HD artwork.
       if(int(s.w)>cw||int(s.h)>ch)continue;
       // The 2x atlas starts as an upscaled copy of the retail GameText atlas. Clear the
       // complete Latin glyph rectangle first, otherwise the retail glyph remains underneath
       // the HD glyph and looks like a second, offset text layer.
       for(int yy=0;yy<ch;yy++)for(int xx=0;xx<cw;xx++){int dx=cellX+xx,dy=cellY+yy;if(dx>=0&&dy>=0&&dx<(int)nw&&dy<(int)nh)setI4(r.px.data(),nw,dx,dy,0);}
       // Use the retail glyph's *actual ink bounds* as the placement reference.  The HD
       // sprites are alpha-cropped, so cell-centering or bottom-anchoring throws away the
       // original bearings/baseline.  Measuring the retail pixels before clearing preserves
       // those relationships automatically (digits, W, x-height, ascenders and descenders).
       int minX=int(g.width),minY=int(g.height),maxX=-1,maxY=-1;
       for(int yy=0;yy<int(g.height);yy++)for(int xx=0;xx<int(g.width);xx++){
         if(getI4(b+pos,w,int(g.x)+xx,int(g.y)+yy)){if(xx<minX)minX=xx;if(yy<minY)minY=yy;if(xx>maxX)maxX=xx;if(yy>maxY)maxY=yy;}
       }
       int ox=cellX+(cw-int(s.w))/2,oy=cellY+(ch-int(s.h))/2;
       if(maxX>=minX&&maxY>=minY){
         int inkX=cellX+minX*2,inkY=cellY+minY*2;
         int inkW=(maxX-minX+1)*2,inkH=(maxY-minY+1)*2;
         ox=inkX+(inkW-int(s.w))/2;
         oy=inkY+(inkH-int(s.h))/2;
       }
       for(int yy=0;yy<s.h;yy++)for(int xx=0;xx<s.w;xx++){int dx=ox+xx,dy=oy+yy;if(dx>=0&&dy>=0&&dx<(int)nw&&dy<(int)nh)setI4(r.px.data(),nw,dx,dy,s.px[size_t(yy)*s.w+xx]);}}
     r.obj.resize(0x90+r.px.size());reps[sig]=std::move(r);
   }
   pos+=bytes;
 }
}
static void finalizeHook(void*slot){if(slot&&readable(slot,0x28)){auto*s=(uint8_t*)slot;uint32_t n=0;uint64_t p=0;std::memcpy(&n,s+0x1c,4);std::memcpy(&p,s+0x20,8);if(p&&n>16&&n<64u*1024u*1024u&&readable((void*)(uintptr_t)p,n))prepare((uint8_t*)(uintptr_t)p,n);}oFinalize(slot);}
static void selectHook(void*tex,int unit){
 if(tex&&unit==0&&readable(tex,0x90)){auto*b=(uint8_t*)tex;uint16_t w=0,h=0;std::memcpy(&w,b+0x0e,2);std::memcpy(&h,b+0x10,2);if(w&&h&&w%8==0&&h%8==0){size_t n=size_t(w)*h/2;if(n<8u*1024u*1024u&&readable(b+0x90,n)){uint64_t sig=hash64(b+0x90,n);auto it=reps.find(sig);if(it!=reps.end()){Rep&r=it->second;std::memcpy(r.obj.data(),b,0x90);std::memcpy(r.obj.data()+0x90,r.px.data(),r.px.size());uint16_t nw=w*2,nh=h*2;uint32_t nw32=nw,nh32=nh,ds=(uint32_t)r.px.size();std::memcpy(r.obj.data()+0x0e,&nw,2);std::memcpy(r.obj.data()+0x10,&nh,2);std::memcpy(r.obj.data()+0x44,&nw32,4);std::memcpy(r.obj.data()+0x48,&nh32,4);std::memcpy(r.obj.data()+0x70,&ds,4);uintptr_t self=(uintptr_t)r.obj.data(),data=self+0x90;std::memcpy(r.obj.data()+0x34,&self,8);std::memcpy(r.obj.data()+0x3c,&data,8);oSelect(r.obj.data(),unit);return;}}}}
 oSelect(tex,unit);
}
extern "C" FH_MOD_EXPORT int fh_mod_initialize(FhMod*m,const FhModHost*h){if(!m||!h||h->abiVersion!=FH_MOD_ABI_VERSION||!h->symbolAddress||!h->hookInstall)return FH_MOD_ERROR;M=m;H=h;if(!loadSprites()){log(FH_LOG_ERROR,"HD Text Pack GameText: HD glyph asset missing/corrupt.");return FH_MOD_ERROR;}tFinalize=h->symbolAddress(m,"gameTextFinalizeLoad");tSelect=h->symbolAddress(m,"selectTexture");if(!tFinalize||!tSelect){log(FH_LOG_ERROR,"GameText HD 0.3.6: required Foxhollow exports missing.");return FH_MOD_ERROR;}void*a=nullptr,*b=nullptr;if(h->hookInstall(m,tFinalize,(void*)finalizeHook,&a)!=FH_MOD_OK||!a)return FH_MOD_ERROR;oFinalize=(FinalizeFn)a;if(h->hookInstall(m,tSelect,(void*)selectHook,&b)!=FH_MOD_OK||!b){if(h->hookRemove)h->hookRemove(m,tFinalize);return FH_MOD_ERROR;}oSelect=(SelectFn)b;log(FH_LOG_INFO,"HD Text Pack GameText loaded. Dynamic map-independent full Latin GameText replacement active.");return FH_MOD_OK;}
extern "C" FH_MOD_EXPORT void fh_mod_update(FhMod*){}
extern "C" FH_MOD_EXPORT void fh_mod_shutdown(FhMod*){if(H&&H->hookRemove){if(tSelect)H->hookRemove(M,tSelect);if(tFinalize)H->hookRemove(M,tFinalize);}reps.clear();sprites.clear();M=nullptr;H=nullptr;tFinalize=tSelect=nullptr;oFinalize=nullptr;oSelect=nullptr;}
