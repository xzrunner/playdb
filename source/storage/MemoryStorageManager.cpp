#include "playdb/storage/MemoryStorageManager.h"
#include "playdb/Exception.h"

namespace playdb
{
namespace storage
{

MemoryStorageManager::MemoryStorageManager()
{
}

MemoryStorageManager::~MemoryStorageManager()
{
	for (auto& entry : m_buffer) {
		delete entry;
	}
}

void MemoryStorageManager::LoadByteArray(const id_type id, size_t& len, byte** data) const
{
	Entry* e;
	try {
		e = m_buffer.at(id);
		if (e == nullptr) {
			throw InvalidPageException(id);
		}
	} catch (std::out_of_range) {
		throw InvalidPageException(id);
	}

	len = e->m_len;

	*data = new byte[len];
	memcpy(*data, e->m_data, len);
}

void MemoryStorageManager::StoreByteArray(id_type& id, const size_t len, const byte* const data)
{
	if (id == NEW_PAGE)
	{
		Entry* e = new Entry(len, data);
		if (m_freelist.empty()) {
			m_buffer.push_back(e);
			id = m_buffer.size() - 1;
		} else {
			id = m_freelist.top(); m_freelist.pop();
			m_buffer[id] = e;
		}
	}
	else
	{
		Entry* e_old;
		try {
			e_old = m_buffer.at(id);
			if (e_old == nullptr) {
				throw InvalidPageException(id);
			}
		} catch (std::out_of_range) {
			throw InvalidPageException(id);
		}

		Entry* e = new Entry(len, data);

		delete e_old;
		m_buffer[id] = e;
	}
}

void MemoryStorageManager::DeleteByteArray(const id_type id)
{
	Entry* e;
	try {
		e = m_buffer.at(id);
		if (e == nullptr) {
			throw InvalidPageException(id);
		}
	} catch (std::out_of_range) {
		throw InvalidPageException(id);
	}

	m_buffer[id] = 0;
	m_freelist.push(id);

	delete e;
}

}
}