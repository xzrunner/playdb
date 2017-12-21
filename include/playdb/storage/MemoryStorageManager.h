#ifndef _PLAYDB_MEMORY_STORAGE_MANAGER_H_
#define _PLAYDB_MEMORY_STORAGE_MANAGER_H_

#include "playdb.h"
#include "playdb/typedef.h"

#include <vector>
#include <stack>

#include <string.h>

namespace playdb
{
namespace storage
{

class MemoryStorageManager : public IStorageManager
{
public:
	MemoryStorageManager();
	virtual ~MemoryStorageManager();
	
	virtual void LoadByteArray(const id_type id, size_t& len, byte** data) const override;
	virtual void StoreByteArray(id_type& id, const size_t len, const byte* const data) override;
	virtual void DeleteByteArray(const id_type id) override;

private:
	class Entry
	{
	public:
		byte*  m_data;
		size_t m_len;

		Entry(size_t len, const byte* data)
			: m_data(nullptr)
			, m_len(len)
		{
			m_data = new byte[m_len];
			memcpy(m_data, data, len);
		}

		~Entry()
		{
			delete[] m_data;
		}

	}; // Entry

private:
	std::vector<Entry*> m_buffer;

	std::stack<id_type> m_freelist;

}; // MemoryStorageManager

}
}

#endif // _PLAYDB_MEMORY_STORAGE_MANAGER_H_