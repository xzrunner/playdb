#ifndef _PLAYDB_BTREE_BTREE_TOOLS_H_
#define _PLAYDB_BTREE_BTREE_TOOLS_H_

#include "playdb.h"

namespace playdb
{
namespace btree
{

template <typename T>
class Data : public IData
{
public:
	Data()
		: id(storage::NEW_PAGE), data(nullptr), data_len(0)
	{}
	Data(id_type id, const T& key, byte* data, size_t data_len)
		: id(id), key(key), data(data), data_len(data_len)
	{}

public:
	id_type id;
	T       key;
	byte*   data;
	size_t  data_len;

}; // Data

}
}

#endif // 