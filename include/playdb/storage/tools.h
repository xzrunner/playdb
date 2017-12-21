#ifndef _PLAYDB_STORAGE_TOOLS_H_
#define _PLAYDB_STORAGE_TOOLS_H_

namespace playdb
{
namespace storage
{

template<typename T> inline 
void pack(const T& d, uint8_t** ptr) {
	memcpy(*ptr, &d, sizeof(d));
	*ptr += sizeof(d);
}

template<typename T> inline 
void unpack(T& d, uint8_t** ptr) {
	memcpy(&d, *ptr, sizeof(d));
	*ptr += sizeof(d);
}

}
}

#endif // _PLAYDB_STORAGE_TOOLS_H_