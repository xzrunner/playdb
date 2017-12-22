#include "playdb/storage/DiskStorageManager.h"
#include "playdb/Exception.h"

#include <assert.h>

namespace playdb
{
namespace storage
{

DiskStorageManager::DiskStorageManager(const std::string& index_filepath,
	                                   const std::string& data_filepath,
	                                   bool overwrite, size_t page_size)
	: m_page_size(0)
	, m_next_page(NEW_PAGE)
	, m_buffer(nullptr)
{
	// check if file exists.
	bool exists = true;
	std::ifstream fin1(index_filepath.c_str(), std::ios::in | std::ios::binary);
	std::ifstream fin2(data_filepath.c_str(), std::ios::in | std::ios::binary);
	if (fin1.fail() || fin2.fail()) {
		exists = false;
	}
	fin1.close(); fin2.close();

	// check if file can be read/written.
	if (exists == true && overwrite == false)
	{
		m_data_file.open(index_filepath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
		m_index_file.open(data_filepath.c_str(), std::ios::in | std::ios::out | std::ios::binary);

		if (m_index_file.fail() || m_index_file.fail()) {
			throw IllegalArgumentException("DiskStorageManager: Index/Data file cannot be read/writen.");
		}
	}
	else
	{
		m_data_file.open(index_filepath.c_str(), std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
		m_index_file.open(data_filepath.c_str(), std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

		if (m_index_file.fail() || m_index_file.fail()) {
			throw IllegalArgumentException("DiskStorageManager: Index/Data file cannot be created.");
		}
	}

	// find page size.
	if (overwrite)
	{
		assert(page_size != 0);
		m_page_size = page_size;
		m_next_page = 0;
	}
	else
	{
		m_index_file.read(reinterpret_cast<char*>(&m_page_size), sizeof(size_t));
		if (m_index_file.fail()) {
			throw IllegalStateException("DiskStorageManager: Failed reading page size.");
		}

		m_index_file.read(reinterpret_cast<char*>(&m_next_page), sizeof(id_type));
		if (m_index_file.fail()) {
			throw IllegalStateException("DiskStorageManager: Failed reading next page.");
		}
	}

	// create buffer.
	m_buffer = new byte[m_page_size];
	memset(m_buffer, 0, m_page_size);

	if (!overwrite)
	{
		// load empty pages in memory.
		size_t count;
		m_index_file.read(reinterpret_cast<char*>(&count), sizeof(size_t));
		if (m_index_file.fail()) {
			throw IllegalStateException("DiskStorageManager: Corrupted storage manager index file.");
		}
		for (size_t i = 0; i < count; ++i)
		{
			id_type page;
			m_index_file.read(reinterpret_cast<char*>(&page), sizeof(id_type));
			if (m_index_file.fail()) {
				throw IllegalStateException("DiskStorageManager: Corrupted storage manager index file.");
			}
			m_empty_pages.push(page);
		}

		// load index table in memory.
		m_index_file.read(reinterpret_cast<char*>(&count), sizeof(size_t));
		if (m_index_file.fail()) {
			throw IllegalStateException("DiskStorageManager: Corrupted storage manager index file.");
		}
		for (size_t i = 0; i < count; ++i)
		{
			auto e = std::make_unique<Entry>();

			id_type id;
			m_index_file.read(reinterpret_cast<char*>(&id), sizeof(id_type));
			if (m_index_file.fail()) {
				throw IllegalStateException("DiskStorageManager: Corrupted storage manager index file.");
			}

			m_index_file.read(reinterpret_cast<char*>(&(e->m_length)), sizeof(size_t));
			if (m_index_file.fail()) {
				throw IllegalStateException("DiskStorageManager: Corrupted storage manager index file.");
			}

			size_t count2;
			m_index_file.read(reinterpret_cast<char*>(&count2), sizeof(size_t));
			if (m_index_file.fail()) {
				throw IllegalStateException("DiskStorageManager: Corrupted storage manager index file.");
			}
			for (size_t j = 0; j < count2; ++j)
			{
				id_type page;
				m_index_file.read(reinterpret_cast<char*>(&page), sizeof(id_type));
				if (m_index_file.fail()) {
					throw IllegalStateException("DiskStorageManager: Corrupted storage manager index file.");
				}
				e->m_pages.push_back(page);
			}

			m_page_index.insert(std::make_pair(id, std::move(e)));
		}
	}
}

DiskStorageManager::~DiskStorageManager()
{
	Flush();

	m_index_file.close();
	m_data_file.close();

	if (m_buffer) {
		delete[] m_buffer;
	}
}

void DiskStorageManager::LoadByteArray(const id_type id, size_t& len, byte** data)
{
	auto entry = m_page_index.find(id);
	if (entry == m_page_index.end()) {
		throw InvalidPageException(id);
	}

	auto& pages = entry->second->m_pages;
	size_t next = 0;
	size_t totle = pages.size();

	len = entry->second->m_length;
	*data = new byte[len];

	byte* ptr = *data;
	size_t rem = len;
	do {
		m_data_file.seekg(pages[next] * m_page_size, std::ios_base::beg);
		if (m_data_file.fail()) {
			throw IllegalStateException("DiskStorageManager: Corrupted data file.");
		}

		m_data_file.read(reinterpret_cast<char*>(m_buffer), m_page_size);
		if (m_data_file.fail()) {
			throw IllegalStateException("DiskStorageManager: Corrupted data file.");
		}

		size_t _len = (rem > m_page_size) ? m_page_size : rem;
		memcpy(ptr, m_buffer, _len);

		ptr += _len;
		rem -= _len;
		++next;
	} while (next < totle);
}

void DiskStorageManager::StoreByteArray(id_type& id, const size_t len, const byte* const data)
{
	if (id == NEW_PAGE)
	{
		auto entry = std::make_unique<Entry>();
		entry->m_length = len;

		const byte* ptr = data;
		size_t rem = len;
		while (rem > 0)
		{
			id_type page;
			if (!m_empty_pages.empty()) {
				page = m_empty_pages.top(); m_empty_pages.pop();
			} else {
				page = m_next_page++;
			}

			size_t _len = (rem > m_page_size) ? m_page_size : rem;
			memcpy(m_buffer, ptr, _len);
			m_data_file.seekp(page * m_page_size, std::ios_base::beg);
			if (m_data_file.fail()) {
				throw IllegalStateException("DiskStorageManager: Corrupted data file.");
			}
			m_data_file.write(reinterpret_cast<const char*>(m_buffer), m_page_size);
			if (m_data_file.fail()) {
				throw IllegalStateException("DiskStorageManager: Corrupted data file.");
			}

			ptr += _len;
			rem -= _len;
			entry->m_pages.push_back(page);
		}

		id = entry->m_pages[0];
		m_page_index.insert(std::make_pair(id, std::move(entry)));
	}
	else
	{
		auto entry = m_page_index.find(id);
		if (entry == m_page_index.end()) {
			throw IndexOutOfBoundsException(id);
		}

		auto old_entry = std::move(entry->second);
		
		m_page_index.erase(entry);
		
		auto new_entry = std::make_unique<Entry>();
		new_entry->m_length = len;

		const byte* ptr = data;
		size_t rem = len;
		size_t next = 0;
		while (rem > 0)
		{
			id_type page;
			if (next < old_entry->m_pages.size()) {
				page = old_entry->m_pages[next++];
			} else if (!m_empty_pages.empty()) {
				page = m_empty_pages.top(); m_empty_pages.pop();
			} else {
				page = m_next_page++;
			}

			size_t _len = (rem > m_page_size) ? m_page_size : rem;
			memcpy(m_buffer, ptr, _len);

			m_data_file.seekp(page * m_page_size, std::ios_base::beg);
			if (m_data_file.fail()) {
				throw IllegalStateException("DiskStorageManager: Corrupted data file.");
			}

			m_data_file.write(reinterpret_cast<const char*>(m_buffer), m_page_size);
			if (m_data_file.fail()) {
				throw IllegalStateException("DiskStorageManager: Corrupted data file.");
			}

			ptr += _len;
			rem -= _len;
			new_entry->m_pages.push_back(page);
		}

		while (next < old_entry->m_pages.size()) {
			m_empty_pages.push(old_entry->m_pages[next++]);
		}

		m_page_index.insert(std::make_pair(id, std::move(new_entry)));
	}
}

void DiskStorageManager::DeleteByteArray(const id_type id)
{
	auto entry = m_page_index.find(id);
	if (entry == m_page_index.end()) {
		throw InvalidPageException(id);
	}

	for (auto page : entry->second->m_pages) {
		m_empty_pages.push(page);
	}

	m_page_index.erase(entry);
}

void DiskStorageManager::Flush()
{
	m_index_file.seekp(0, std::ios_base::beg);
	if (m_index_file.fail()) {
		throw IllegalStateException("DiskStorageManager: Corrupted storage manager index file.");
	}

	// write page size.
	m_index_file.write(reinterpret_cast<const char*>(&m_page_size), sizeof(size_t));
	if (m_index_file.fail()) {
		throw IllegalStateException("DiskStorageManager: Corrupted storage manager index file.");
	}

	// write next page.
	m_index_file.write(reinterpret_cast<const char*>(&m_next_page), sizeof(id_type));
	if (m_index_file.fail()) {
		throw IllegalStateException("DiskStorageManager: Corrupted storage manager index file.");
	}

	// write empty pages.
	size_t count = m_empty_pages.size();
	m_index_file.write(reinterpret_cast<const char*>(&count), sizeof(size_t));
	if (m_index_file.fail()) {
		throw IllegalStateException("DiskStorageManager: Corrupted storage manager index file.");
	}
	while (!m_empty_pages.empty())
	{
		id_type page = m_empty_pages.top(); m_empty_pages.pop();
		m_index_file.write(reinterpret_cast<const char*>(&page), sizeof(id_type));
		if (m_index_file.fail()) {
			throw IllegalStateException("DiskStorageManager: Corrupted storage manager index file.");
		}
	}
	
	// write page index.
	count = m_page_index.size();
	m_index_file.write(reinterpret_cast<const char*>(&count), sizeof(size_t));
	if (m_index_file.fail()) {
		throw IllegalStateException("DiskStorageManager: Corrupted storage manager index file.");
	}
	for (auto& entry : m_page_index)
	{
		id_type id = entry.first;
		m_index_file.write(reinterpret_cast<const char*>(&id), sizeof(id_type));
		if (m_index_file.fail()) {
			throw IllegalStateException("DiskStorageManager: Corrupted storage manager index file.");
		}

		size_t length = entry.second->m_length;
		m_index_file.write(reinterpret_cast<const char*>(&length), sizeof(size_t));
		if (m_index_file.fail()) {
			throw IllegalStateException("DiskStorageManager: Corrupted storage manager index file.");
		}

		count = entry.second->m_pages.size();
		m_index_file.write(reinterpret_cast<const char*>(&count), sizeof(size_t));
		if (m_index_file.fail()) {
			throw IllegalStateException("DiskStorageManager: Corrupted storage manager index file.");
		}
		for (size_t i = 0; i < count; ++i)
		{
			id_type page = entry.second->m_pages[i];
			m_index_file.write(reinterpret_cast<const char*>(&page), sizeof(id_type));
			if (m_index_file.fail()) {
				throw IllegalStateException("DiskStorageManager: Corrupted storage manager index file.");
			}
		}
	}

	m_index_file.flush();
	m_data_file.flush();
}

}
}