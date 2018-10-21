#ifndef _PLAYDB_DISK_STORAGE_MANAGER_H_
#define _PLAYDB_DISK_STORAGE_MANAGER_H_

#include "playdb.h"

#include <vector>
#include <map>
#include <queue>
#include <functional>
#include <fstream>
#include <memory>

namespace playdb
{
namespace storage
{

class DiskStorageManager : public IStorageManager
{
public:
	DiskStorageManager(const std::string& index_filepath,
		const std::string& data_filepath, bool overwrite = false, size_t page_size = 0);
	virtual ~DiskStorageManager();

	virtual void LoadByteArray(const id_type id, size_t& len, byte** data) override;
	virtual void StoreByteArray(id_type& id, const size_t len, const byte* const data) override;
	virtual void DeleteByteArray(const id_type id) override;

	void Flush();

private:
	class Entry
	{
	public:
		size_t m_length;
		std::vector<id_type> m_pages;
	};

private:
	std::fstream m_index_file;
	std::fstream m_data_file;

	size_t  m_page_size;
	id_type m_next_page;

	std::priority_queue<id_type, std::vector<id_type>, std::greater<id_type>> m_empty_pages;
	std::map<id_type, std::unique_ptr<Entry>> m_page_index;

	byte* m_buffer;

}; // DiskStorageManager

}
}

#endif // _PLAYDB_DISK_STORAGE_MANAGER_H_