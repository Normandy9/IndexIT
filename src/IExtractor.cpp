#include "IExtractor.hpp"
#include "RAII.hpp"
#include "miniz.h"
#include <windows.h>
#include <algorithm>
#include <cctype>
#include <climits>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>

namespace indexit {
namespace {
static const size_t kMaxInput = 50u * 1024u * 1024u;
static const size_t kMaxEntry = 16u * 1024u * 1024u;
static const size_t kMaxArchiveOutput = 64u * 1024u * 1024u;

bool readFile(const std::wstring& path, std::string& out) {
    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                           nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER sz;
    if (!GetFileSizeEx(h, &sz) || sz.QuadPart < 0 || static_cast<uint64_t>(sz.QuadPart) > kMaxInput) { CloseHandle(h); return false; }
    out.resize(static_cast<size_t>(sz.QuadPart));
    size_t offset = 0;
    while (offset < out.size()) {
        DWORD got = 0, want = static_cast<DWORD>(std::min<size_t>(out.size() - offset, 1u << 20));
        if (!ReadFile(h, &out[offset], want, &got, nullptr) || got == 0) { CloseHandle(h); out.clear(); return false; }
        offset += got;
    }
    CloseHandle(h);
    return true;
}

std::string lower(std::string s) { for (size_t i=0;i<s.size();++i) s[i]=static_cast<char>(tolower(static_cast<unsigned char>(s[i]))); return s; }

void appendUtf8(std::string& out, unsigned cp) {
    if (cp < 0x80) out.push_back(static_cast<char>(cp));
    else if (cp < 0x800) { out.push_back(static_cast<char>(0xc0|(cp>>6))); out.push_back(static_cast<char>(0x80|(cp&63))); }
    else { out.push_back(static_cast<char>(0xe0|(cp>>12))); out.push_back(static_cast<char>(0x80|((cp>>6)&63))); out.push_back(static_cast<char>(0x80|(cp&63))); }
}

std::string xmlText(const std::string& xml) {
    std::string out; bool tag=false;
    for (size_t i=0;i<xml.size();++i) {
        char c=xml[i];
        if (c=='<') { tag=true; if (!out.empty() && out.back()!=' ') out.push_back(' '); continue; }
        if (c=='>') { tag=false; continue; }
        if (tag) continue;
        if (c=='&') {
            size_t e=xml.find(';',i+1);
            if (e!=std::string::npos && e-i<12) {
                std::string ent=xml.substr(i+1,e-i-1); unsigned cp=0; bool ok=true;
                if(ent=="amp")cp='&'; else if(ent=="lt")cp='<'; else if(ent=="gt")cp='>'; else if(ent=="quot")cp='"'; else if(ent=="apos")cp='\'';
                else if(ent.size()>1 && ent[0]=='#') { char* end=nullptr; cp=static_cast<unsigned>(strtoul(ent.c_str()+(ent[1]=='x'||ent[1]=='X'?2:1),&end,(ent[1]=='x'||ent[1]=='X')?16:10)); ok=end && *end==0 && cp<=0x10ffff; }
                else ok=false;
                if(ok) { if(cp<=0xffff) appendUtf8(out,cp); i=e; continue; }
            }
        }
        out.push_back(c);
    }
    return out;
}

uint16_t u16(const std::string& s,size_t p) { return static_cast<uint16_t>((unsigned char)s[p] | ((unsigned char)s[p+1]<<8)); }
uint32_t u32(const std::string& s,size_t p) { return (uint32_t)u16(s,p) | ((uint32_t)u16(s,p+2)<<16); }

bool inflateRaw(const unsigned char* src, size_t srcSize, std::string& out, size_t limit) {
    if (srcSize > UINT_MAX) return false;
    mz_stream z; memset(&z,0,sizeof(z)); z.next_in=src; z.avail_in=static_cast<unsigned int>(srcSize);
    if(mz_inflateInit2(&z,-MZ_DEFAULT_WINDOW_BITS)!=MZ_OK) return false;
    char buf[32768]; int rc=MZ_OK;
    while(rc==MZ_OK) {
        z.next_out=reinterpret_cast<unsigned char*>(buf); z.avail_out=sizeof(buf);
        rc=mz_inflate(&z,MZ_NO_FLUSH); size_t n=sizeof(buf)-z.avail_out;
        if(out.size()+n>limit){rc=MZ_BUF_ERROR;break;} out.append(buf,n);
    }
    mz_inflateEnd(&z); return rc==MZ_STREAM_END;
}

struct ZipEntry { std::string name, data; };
bool readZip(const std::string& bytes, std::vector<ZipEntry>& entries) {
    if(bytes.size()<22 || bytes.substr(0,2)!="PK") return false;
    size_t begin=bytes.size()>65557?bytes.size()-65557:0, eocd=std::string::npos;
    for(size_t p=bytes.size()-22;;--p){if(u32(bytes,p)==0x06054b50){eocd=p;break;}if(p==begin)break;}
    if(eocd==std::string::npos || eocd+22>bytes.size()) return false;
    uint16_t count=u16(bytes,eocd+10); uint32_t cdSize=u32(bytes,eocd+12), cdOff=u32(bytes,eocd+16);
    if(cdOff>bytes.size() || cdSize>bytes.size()-cdOff || count>10000) return false;
    size_t p=cdOff,total=0;
    for(unsigned i=0;i<count;++i){
        if(p+46>bytes.size()||u32(bytes,p)!=0x02014b50)return false;
        uint16_t method=u16(bytes,p+10), nl=u16(bytes,p+28), xl=u16(bytes,p+30), cl=u16(bytes,p+32);
        uint32_t csize=u32(bytes,p+20), usize=u32(bytes,p+24), lo=u32(bytes,p+42);
        if(p+46u+nl+xl+cl>bytes.size())return false;
        ZipEntry e; e.name=bytes.substr(p+46,nl); p+=46u+nl+xl+cl;
        if(e.name.empty()||e.name.back()=='/'||usize==0)continue;
        if(usize>kMaxEntry || csize>bytes.size() || (csize && usize/csize>200) || total+usize>kMaxArchiveOutput) continue;
        if(lo+30u>bytes.size()||u32(bytes,lo)!=0x04034b50)continue;
        uint16_t lnl=u16(bytes,lo+26), lxl=u16(bytes,lo+28); size_t data=lo+30u+lnl+lxl;
        if(data>bytes.size()||csize>bytes.size()-data)continue;
        if(method==0)e.data.assign(bytes.data()+data,usize);
        else if(method==8) { if(!inflateRaw(reinterpret_cast<const unsigned char*>(bytes.data()+data),csize,e.data,kMaxEntry)||e.data.size()!=usize)continue; }
        else continue;
        total+=e.data.size(); entries.push_back(e);
    }
    return true;
}

std::string extractXmlEntries(const std::wstring& path, const std::string& wantedPrefix, const std::string& exact) {
    std::string bytes,out; if(!readFile(path,bytes))return out; std::vector<ZipEntry> entries;
    if(!readZip(bytes,entries))return out;
    for(size_t i=0;i<entries.size();++i) { std::string n=lower(entries[i].name);
        bool take=exact.empty()? n.find(wantedPrefix)==0 : n==exact;
        if(take){out+=xmlText(entries[i].data);out.push_back(' ');}
    }
    return out;
}

std::string stripHtml(const std::string& s) {
    std::string out; bool tag=false,script=false; std::string tagbuf;
    for(size_t i=0;i<s.size();++i){char c=s[i]; if(c=='<'){tag=true;tagbuf.clear();continue;} if(tag){if(c=='>'){std::string t=lower(tagbuf); if(t.find("script")!=std::string::npos)script=t.find("/script")==std::string::npos; tag=false;if(!script)out.push_back(' ');}else tagbuf.push_back(c);continue;} if(!script)out.push_back(c);}
    return xmlText(out);
}
std::string stripRtf(const std::string& s) {
    std::string out; int depth=0;
    for(size_t i=0;i<s.size();++i){char c=s[i];if(c=='{'){++depth;continue;}if(c=='}'){if(depth)--depth;continue;}if(c=='\\'){if(i+1<s.size()&&(s[i+1]=='\\'||s[i+1]=='{'||s[i+1]=='}')){out.push_back(s[++i]);continue;}if(i+1<s.size()&&s[i+1]=='\''){if(i+3<s.size()){unsigned v=0;sscanf(s.substr(i+2,2).c_str(),"%x",&v);out.push_back(static_cast<char>(v));i+=3;}continue;}++i;while(i<s.size()&&isalpha(static_cast<unsigned char>(s[i])))++i;if(i<s.size()&&(s[i]=='-'||isdigit(static_cast<unsigned char>(s[i])))){while(i<s.size()&&(s[i]=='-'||isdigit(static_cast<unsigned char>(s[i]))))++i;}if(i<s.size()&&s[i]==' '){}else --i;continue;}if(depth>0)out.push_back(c);}
    return out;
}

std::string pdfText(const std::string& b) {
    std::string out;
    for(size_t p=0;(p=b.find("stream",p))!=std::string::npos;){size_t start=p+6;if(start<b.size()&&b[start]=='\r')++start;if(start<b.size()&&b[start]=='\n')++start;size_t end=b.find("endstream",start);if(end==std::string::npos)break;size_t dict=b.rfind("<<",p);std::string data=b.substr(start,end-start),decoded;
        if(dict!=std::string::npos && b.find("/FlateDecode",dict)<p){mz_stream z;memset(&z,0,sizeof(z));z.next_in=(const unsigned char*)data.data();z.avail_in=(unsigned int)data.size();if(mz_inflateInit(&z)==MZ_OK){char buf[8192];int rc=MZ_OK;while(rc==MZ_OK){z.next_out=(unsigned char*)buf;z.avail_out=sizeof(buf);rc=mz_inflate(&z,MZ_NO_FLUSH);size_t n=sizeof(buf)-z.avail_out;if(decoded.size()+n>kMaxEntry){decoded.clear();break;}decoded.append(buf,n);}mz_inflateEnd(&z);}}
        else decoded=data;
        for(size_t i=0;i<decoded.size();++i){if(decoded[i]=='('){for(++i;i<decoded.size()&&decoded[i]!=')';++i){if(decoded[i]=='\\'&&i+1<decoded.size())++i;out.push_back(decoded[i]);}out.push_back(' ');}}
        p=end+9;
    }
    return out;
}
}

std::string PlainExtractor::extract(const std::wstring& p) { std::string s; if(!readFile(p,s))return {}; if(s.size()>=2&&((unsigned char)s[0]==0xff&&(unsigned char)s[1]==0xfe)){std::wstring w(reinterpret_cast<const wchar_t*>(s.data()+2),(s.size()-2)/2); int n=WideCharToMultiByte(CP_UTF8,0,w.data(),(int)w.size(),nullptr,0,nullptr,nullptr);std::string o(n,'\0');if(n)WideCharToMultiByte(CP_UTF8,0,w.data(),(int)w.size(),&o[0],n,nullptr,nullptr);return o;} if(s.size()>=2&&(unsigned char)s[0]==0xfe&&(unsigned char)s[1]==0xff){std::string swapped=s;for(size_t i=2;i+1<swapped.size();i+=2)std::swap(swapped[i],swapped[i+1]);std::wstring w(reinterpret_cast<const wchar_t*>(swapped.data()+2),(swapped.size()-2)/2);int n=WideCharToMultiByte(CP_UTF8,0,w.data(),(int)w.size(),nullptr,0,nullptr,nullptr);std::string o(n,'\0');if(n)WideCharToMultiByte(CP_UTF8,0,w.data(),(int)w.size(),&o[0],n,nullptr,nullptr);return o;} if(s.size()>=3&&(unsigned char)s[0]==0xef&&(unsigned char)s[1]==0xbb&&(unsigned char)s[2]==0xbf)s.erase(0,3);return s; }
std::string HtmlExtractor::extract(const std::wstring& p) { std::string s; return readFile(p,s)?stripHtml(s):std::string(); }
std::string RtfExtractor::extract(const std::wstring& p) { std::string s; return readFile(p,s)?stripRtf(s):std::string(); }
std::string EmlExtractor::extract(const std::wstring& p) { std::string s;return readFile(p,s)?s:std::string(); }
std::string DocxExtractor::extract(const std::wstring& p) { return extractXmlEntries(p,"word/document.xml",""); }
std::string PptxExtractor::extract(const std::wstring& p) { return extractXmlEntries(p,"ppt/slides/slide",""); }
std::string XlsxExtractor::extract(const std::wstring& p) { return extractXmlEntries(p,"","xl/sharedstrings.xml")+extractXmlEntries(p,"xl/worksheets/sheet",""); }
std::string OdtExtractor::extract(const std::wstring& p) { return extractXmlEntries(p,"","content.xml"); }
std::string ZipExtractor::extract(const std::wstring& p) { std::string b,out;if(!readFile(p,b))return out;std::vector<ZipEntry> es;if(!readZip(b,es))return out;for(size_t i=0;i<es.size();++i){std::string n=lower(es[i].name);size_t dot=n.find_last_of('.');if(dot!=std::string::npos&&(n.substr(dot)==".txt"||n.substr(dot)==".md"||n.substr(dot)==".csv"||n.substr(dot)==".log"))out+=es[i].data+" ";}return out; }
std::string PdfExtractor::extract(const std::wstring& p) { std::string s;return readFile(p,s)?pdfText(s):std::string(); }

std::unique_ptr<IExtractor> ExtractorFactory::create(DocumentFormat f) {
    switch(f){case FORMAT_HTML:return std::unique_ptr<IExtractor>(new HtmlExtractor());case FORMAT_RTF:return std::unique_ptr<IExtractor>(new RtfExtractor());case FORMAT_EML:return std::unique_ptr<IExtractor>(new EmlExtractor());case FORMAT_DOCX:return std::unique_ptr<IExtractor>(new DocxExtractor());case FORMAT_PPTX:return std::unique_ptr<IExtractor>(new PptxExtractor());case FORMAT_XLSX:return std::unique_ptr<IExtractor>(new XlsxExtractor());case FORMAT_ODT:return std::unique_ptr<IExtractor>(new OdtExtractor());case FORMAT_PDF:return std::unique_ptr<IExtractor>(new PdfExtractor());case FORMAT_ZIP:return std::unique_ptr<IExtractor>(new ZipExtractor());case FORMAT_TXT:case FORMAT_CSV:case FORMAT_LOG:case FORMAT_MD:case FORMAT_JSON:case FORMAT_XML:case FORMAT_C:case FORMAT_CPP:case FORMAT_H:return std::unique_ptr<IExtractor>(new PlainExtractor());default:return std::unique_ptr<IExtractor>();}
}
} // namespace indexit
