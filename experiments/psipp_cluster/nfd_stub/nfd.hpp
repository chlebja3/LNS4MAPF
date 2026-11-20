// Headless stub of nativefiledialog-extended, only for cluster benchmark builds.
#pragma once
typedef struct { const char* name; const char* spec; } nfdfilteritem_t;
typedef enum { NFD_ERROR, NFD_OKAY, NFD_CANCEL } nfdresult_t;
namespace NFD {
struct Guard {};
struct UniquePath { const char* get() const { return ""; } };
inline nfdresult_t OpenDialog(UniquePath&, const nfdfilteritem_t*, int) { return NFD_CANCEL; }
inline const char* GetError() { return "headless"; }
}
