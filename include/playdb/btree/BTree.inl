#ifndef _PLAYDB_BTREE_BTREE_NODE_INL_
#define _PLAYDB_BTREE_BTREE_NODE_INL_

#include "playdb.h"
#include "playdb/btree/BTree.h"
#include "playdb/Exception.h"

#include <iostream>
#include <queue>

namespace playdb
{
namespace btree
{

template <typename T>
BTree<T>::BTree(IStorageManager* storage_mgr, size_t degree)
	: m_storage_mgr(storage_mgr)
	, m_root_id(storage::NEW_PAGE)
	, m_header_id(storage::NEW_PAGE)
	, m_degree(degree)
{
	StoreHeader();

	BTreeNode<T> root(this, storage::NEW_PAGE, true);
	m_root_id = WriteNode(root);
}

template <typename T>
BTree<T>::BTree(IStorageManager* storage_mgr)
	: m_storage_mgr(storage_mgr)
	, m_root_id(storage::NEW_PAGE)
	, m_header_id(0)
	, m_degree(0)
{
	LoadHeader();
}

template <typename T>
BTree<T>::~BTree()
{
	StoreHeader();
}

template <typename T>
void BTree<T>::InsertData(const T& key, size_t len, const byte* const data)
{
	NodePtr<T> root = ReadNode(m_root_id);
	if (root->m_entry_num < MaxKeys()) {
		root->InsertEntryNonFull(len, data, key, storage::NEW_PAGE);
		return;
	}

	auto new_root = std::make_shared<BTreeNode<T>>(this, storage::NEW_PAGE, false);
	new_root->m_children[0] = m_root_id;
	new_root->SplitChild(0, root);
	int i = 0;
	if (new_root->m_entry_key[i] < key) {
		++i;
	}
	auto child = ReadNode(new_root->m_children[i]);
	child->InsertEntryNonFull(len, data, key, storage::NEW_PAGE);
	WriteNode(*new_root);
	
	m_root_id = new_root->m_id;
}

template <typename T>
void BTree<T>::LayerTraverse(IVisitor& visitor)
{
	std::queue<NodePtr<T>> st;
	NodePtr<T> root = ReadNode(m_root_id);
	st.push(root);
	while (!st.empty())
	{
		NodePtr<T> n = st.front(); st.pop();
		visitor.VisitNode(*n);
		for (size_t i = 0; i < n->m_entry_num; ++i) {
			visitor.VisitData(Data<T>(
					n->m_entry_id[i], 
					n->m_entry_key[i],
					n->m_entry_data[i],
					n->m_entry_len[i]));
		}
		if (!n->m_leaf) {
			for (size_t i = 0; i < n->m_entry_num + 1; ++i) {
				NodePtr<T> child = ReadNode(n->m_children[i]);
				st.push(child);
			}
		}
	}
}

template <typename T>
bool BTree<T>::Query(const T& key, Data<T>& result)
{
	NodePtr<T> node = ReadNode(m_root_id);
	while (node)
	{
		size_t i = 0;
		while (i < node->m_entry_num && key > node->m_entry_key[i]) {
			++i;
		}
		if (node->m_entry_key[i] == key) {
			result = Data<T>(
				node->m_entry_id[i],
				node->m_entry_key[i],
				node->m_entry_data[i],
				node->m_entry_len[i]);
			return true;
		}
		if (node->m_leaf) {
			return false;
		}
		node = ReadNode(node->m_children[i]);
	}
	return false;
}

template <typename T>
id_type BTree<T>::WriteNode(BTreeNode<T>& node)
{
	byte* buf;
	size_t len;
	node.StoreToByteArray(&buf, len);

	id_type page;
	if (node.m_id < 0) {
		page = storage::NEW_PAGE;
	} else {
		page = node.m_id;
	}

	try {
		m_storage_mgr->StoreByteArray(page, len, buf);
		delete[] buf;
	} catch (InvalidPageException& e) {
		delete[] buf;
		std::cerr << e.what() << std::endl;
		throw IllegalStateException("WriteNode: failed with InvalidPageException");
	}

	if (node.m_id < 0) 
	{
		node.m_id = page;
		m_stats.nodes++;
	}

	m_stats.writes++;

	return page;
}

template <typename T>
NodePtr<T> BTree<T>::ReadNode(id_type id)
{
	size_t len;
	byte* buf;

	try {
		m_storage_mgr->LoadByteArray(id, len, &buf);
	} catch (InvalidPageException& e) {
		std::cerr << e.what() << std::endl;
		throw playdb::IllegalStateException("ReadNode: failed with InvalidPageException");
	}

	auto node = std::make_shared<BTreeNode<T>>(this, id, true);
	node->LoadFromByteArray(buf);

	m_stats.reads++;

	delete[] buf;
	return node;
}

template <typename T>
void BTree<T>::DeleteNode(const BTreeNode<T>& node)
{
	try {
		m_storage_mgr->DeleteByteArray(node->m_id);
	} catch (InvalidPageException& e) {
		std::cerr << e.what() << std::endl;
		throw IllegalStateException("DeleteNode: failed with InvalidPageException");
	}

	m_stats.nodes--;
}

template <typename T>
void BTree<T>::StoreHeader()
{
	size_t sz = 0;
	sz += sizeof(id_type);		// m_root_id
	sz += sizeof(size_t);		// m_degree

	byte* data = new byte[sz];
	byte* ptr = data;

	storage::pack(m_root_id, &ptr);
	storage::pack(m_degree, &ptr);

	m_storage_mgr->StoreByteArray(m_header_id, sz, data);

	delete[] data;
}

template <typename T>
void BTree<T>::LoadHeader()
{
	size_t len;
	byte* data = 0;
	m_storage_mgr->LoadByteArray(m_header_id, len, &data);

	byte* ptr = data;

	storage::unpack(m_root_id, &ptr);
	storage::unpack(m_degree, &ptr);

	delete[] data;
}

}
}

#endif // _PLAYDB_BTREE_BTREE_NODE_INL_