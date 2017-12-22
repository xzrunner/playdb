#ifndef _PLAYDB_STORAGE_TOOLS_H_
#define _PLAYDB_STORAGE_TOOLS_H_

#include "playdb/Exception.h"

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

inline 
int sizeof_pack_str(const std::string& str) {
	if (str.empty()) {
		return sizeof(uint16_t);
	} else {
		if (str.size() > 0xffff) {
			throw IllegalArgumentException("string too long");
		}
		return sizeof(uint16_t) + str.size();
	}
}

inline 
void pack_str(const std::string& str, uint8_t** ptr) {
	if (str.empty()) {
		uint16_t c = 0xffff;
		pack(c, ptr);
	} else {
		if (str.size() > 0xffff) {
			throw IllegalArgumentException("string too long");
		}
		uint16_t sz = static_cast<uint16_t>(str.size());
		pack(sz, ptr);
		for (int i = 0; i < sz; ++i) {
			uint8_t c = str[i];
			pack(c, ptr);
		}
	}
}

inline 
void unpack_str(std::string& str, uint8_t** ptr) {
	uint16_t sz;
	unpack(sz, ptr);
	if (sz == 0xffff) {
		;
	} else {
		for (int i = 0; i < sz; ++i) {
			uint8_t c;
			unpack(c, ptr);
			str.push_back((char)c);
		}
	}
}

}
}

#endif // _PLAYDB_STORAGE_TOOLS_H_