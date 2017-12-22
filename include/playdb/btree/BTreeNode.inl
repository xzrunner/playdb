#ifndef _PLAYDB_BTREE_NODE_INL_
#define _PLAYDB_BTREE_NODE_INL_

#include "playdb.h"
#include "playdb/storage/tools.h"

#include <assert.h>

namespace playdb
{
namespace btree
{

template <typename T>
BTreeNode<T>::BTreeNode()
	: m_tree(nullptr)
	, m_id(storagemanager::NULL_PAGE)
	, m_leaf(true)
	, m_entry_num(0)
	, m_entry_id(nullptr)
	, m_entry_key(nullptr)
	, m_entry_data(nullptr)
	, m_entry_len(nullptr)
	, m_children(nullptr)
{
}

template <typename T>
BTreeNode<T>::BTreeNode(BTree<T>* tree, id_type id, bool leaf)
	: m_tree(tree)
	, m_id(id)
	, m_leaf(leaf)
	, m_entry_num(0)
	, m_entry_id(nullptr)
	, m_entry_key(nullptr)
	, m_entry_data(nullptr)
	, m_entry_len(nullptr)
	, m_children(nullptr)
{
	try {
		size_t cap = tree->MaxKeys();
		m_entry_id   = new id_type[cap];
		m_entry_key  = new T[cap];
		m_entry_data = new byte*[cap];
		m_entry_len  = new size_t[cap];
		m_children   = new id_type[cap + 1];
	} catch (...) {
		delete[] m_entry_id;
		delete[] m_entry_key;
		delete[] m_entry_data;
		delete[] m_entry_len;
		delete[] m_children;
	}
}

template <typename T>
size_t BTreeNode<T>::GetByteArraySize() const
{
	size_t sz = 0;
	sz += sizeof(m_leaf); // m_leaf
	sz += sizeof(size_t); // m_entry_num
	// entries
	sz += (sizeof(id_type) + sizeof(T) + sizeof(size_t)) * m_entry_num;
	for (size_t i = 0; i < m_entry_num; ++i) {
		sz += m_entry_len[i];
	}
	sz += sizeof(id_type) * (m_entry_num + 1); // children
	return sz;
}

template <>
size_t BTreeNode<std::string>::GetByteArraySize() const
{
	size_t sz = 0;
	sz += sizeof(m_leaf); // m_leaf
	sz += sizeof(size_t); // m_entry_num
	// entries
	sz += (sizeof(id_type) + sizeof(size_t)) * m_entry_num; 
	for (size_t i = 0; i < m_entry_num; ++i) {
		sz += storage::sizeof_pack_str(m_entry_key[i]);
		sz += m_entry_len[i];
	}
	sz += sizeof(id_type) * (m_entry_num + 1); // children
	return sz;
}

template <typename T>
void BTreeNode<T>::LoadFromByteArray(const byte* data)
{
	byte* ptr = const_cast<byte*>(data);

	storage::unpack(m_leaf, &ptr);     // m_leaf

	storage::unpack(m_entry_num, &ptr); // m_entry_num
	// entries
	for (size_t i = 0; i < m_entry_num; ++i)
	{
		storage::unpack(m_entry_id[i], &ptr);
		storage::unpack(m_entry_key[i], &ptr);
		storage::unpack(m_entry_len[i], &ptr);

		size_t len = m_entry_len[i];
		if (len > 0) 
		{
			m_entry_data[i] = new byte[len];
			memcpy(m_entry_data[i], ptr, len);
			ptr += len;
		}
		else
		{
			m_entry_data[i] = nullptr;
		}
	}

	// children
	for (size_t i = 0, n = m_entry_num + 1; i < n; ++i) {
		storage::unpack(m_children[i], &ptr);
	}
}

template <>
void BTreeNode<std::string>::LoadFromByteArray(const byte* data)
{
	byte* ptr = const_cast<byte*>(data);

	storage::unpack(m_leaf, &ptr);     // m_leaf

	storage::unpack(m_entry_num, &ptr); // m_entry_num
	// entries
	for (size_t i = 0; i < m_entry_num; ++i)
	{
		storage::unpack(m_entry_id[i], &ptr);
		storage::unpack_str(m_entry_key[i], &ptr);
		storage::unpack(m_entry_len[i], &ptr);

		size_t len = m_entry_len[i];
		if (len > 0) 
		{
			m_entry_data[i] = new byte[len];
			memcpy(m_entry_data[i], ptr, len);
			ptr += len;
		}
		else
		{
			m_entry_data[i] = nullptr;
		}
	}

	// children
	for (size_t i = 0, n = m_entry_num + 1; i < n; ++i) {
		storage::unpack(m_children[i], &ptr);
	}
}

template <typename T>
void BTreeNode<T>::StoreToByteArray(byte** data, size_t& len) const
{
	len = GetByteArraySize();

	*data = new byte[len];
	byte* ptr = *data;

	storage::pack(m_leaf, &ptr); // m_leaf

	storage::pack(m_entry_num, &ptr); // m_entry_num
	// entries
	for (size_t i = 0; i < m_entry_num; ++i)
	{
		storage::pack(m_entry_id[i], &ptr);
		storage::pack(m_entry_key[i], &ptr);

		size_t len = m_entry_len[i];
		storage::pack(len, &ptr);
		if (len > 0) {
			memcpy(ptr, m_entry_data[i], len);
			ptr += len;
		}
	}

	// children
	for (size_t i = 0, n = m_entry_num + 1; i < n; ++i) {
		storage::pack(m_children[i], &ptr);
	}
}

template <>
void BTreeNode<std::string>::StoreToByteArray(byte** data, size_t& len) const
{
	len = GetByteArraySize();

	*data = new byte[len];
	byte* ptr = *data;

	storage::pack(m_leaf, &ptr); // m_leaf

	storage::pack(m_entry_num, &ptr); // m_entry_num
	// entries
	for (size_t i = 0; i < m_entry_num; ++i)
	{
		storage::pack(m_entry_id[i], &ptr);
		storage::pack_str(m_entry_key[i], &ptr);

		size_t len = m_entry_len[i];
		storage::pack(len, &ptr);
		if (len > 0) {
			memcpy(ptr, m_entry_data[i], len);
			ptr += len;
		}
	}

	// children
	for (size_t i = 0, n = m_entry_num + 1; i < n; ++i) {
		storage::pack(m_children[i], &ptr);
	}
}

template <typename T>
void BTreeNode<T>::InsertEntryNonFull(size_t data_len, const byte* const data, const T& key, id_type id)
{
	size_t capacity = m_tree->MaxKeys();
	if (m_leaf)
	{
		assert(m_entry_num < capacity);

		int i = static_cast<int>(m_entry_num - 1);
		while (i >= 0 && m_entry_key[i] > key) {
			CopyKey(i + 1, i, *this);
			--i;
		}

		m_entry_id[i + 1]  = id;
		m_entry_key[i + 1] = key;

		m_entry_data[i + 1] = new byte[data_len];
		memcpy(m_entry_data[i + 1], data, data_len);
		m_entry_len[i + 1] = data_len;

		++m_entry_num;

		m_tree->WriteNode(*this);
	}
	else
	{
		// find
		int i = static_cast<int>(m_entry_num - 1);
		while (i >= 0 && m_entry_key[i] > key) {
			--i;
		}

		NodePtr<T> child = m_tree->ReadNode(m_children[i + 1]);
		if (child->m_entry_num == capacity) 
		{
			SplitChild(i + 1, child);
			if (m_entry_key[i + 1] < key) {
				child = m_tree->ReadNode(m_children[i + 2]);
			}
		}
		child->InsertEntryNonFull(data_len, data, key, id);
	}
}

template <typename T>
void BTreeNode<T>::DeleteEntry(size_t index)
{
	//assert(index >= 0 && index < m_entry_num);

	//m_tot_data_len -= m_entry_data[index];
	//if (m_entry_data[index]) {
	//	delete[] m_entry_data[index];
	//}

	//if (m_entry_num > 1 && index != m_entry_num - 1) 
	//{
	//	m_entry_id[index] = 
	//}
}

template <typename T>
void BTreeNode<T>::SplitChild(size_t idx, NodePtr<T>& node)
{
	auto other = std::make_shared<BTreeNode<T>>(m_tree, storage::NEW_PAGE, node->m_leaf);

	// copy entries
	size_t t = m_tree->m_degree;
	other->m_entry_num = t - 1; // min keys
	for (size_t i = 0; i < t - 1; ++i) {
		other->CopyKey(i, t + i, *node);
	}

	// copy children
	if (!node->m_leaf) {
		for (size_t i = 0; i < t; ++i) {
			other->m_children[i] = node->m_children[t + i];
		}
	}

	// store other
	m_tree->WriteNode(*other);

	node->m_entry_num = t - 1;
	m_tree->WriteNode(*node);

	// insert other
	for (int i = static_cast<int>(m_entry_num), n = static_cast<int>(idx + 1); i >= n; --i) {
		m_children[i + 1] = m_children[i];
	}
	m_children[idx + 1] = other->m_id;

	// add key
	for (int i = static_cast<int>(m_entry_num - 1), n = static_cast<int>(idx); i >= n; --i) {
		CopyKey(i + 1, i, *this);
	}
	CopyKey(idx, t - 1, *node);

	m_entry_num++;

	m_tree->WriteNode(*this);
}

template <typename T>
void BTreeNode<T>::CopyKey(size_t dst_idx, size_t src_idx, const BTreeNode<T>& src)
{
	m_entry_id[dst_idx]   = src.m_entry_id[src_idx];
	m_entry_key[dst_idx]  = src.m_entry_key[src_idx];
	m_entry_data[dst_idx] = src.m_entry_data[src_idx];
	m_entry_len[dst_idx]  = src.m_entry_len[src_idx];
}

}
}

#endif // _PLAYDB_BTREE_NODE_INL_