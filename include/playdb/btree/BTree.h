#ifndef _PLAYDB_BTREE_BTREE_H_
#define _PLAYDB_BTREE_BTREE_H_

#include "playdb/typedef.h"
#include "playdb/btree/BTreeNode.h"
#include "playdb/btree/tools.h"

namespace playdb
{
namespace btree
{

template <typename T>
class BTree
{
public:
	// degree = order / 2
	BTree(IStorageManager* storage_mgr, size_t degree);
	BTree(IStorageManager* storage_mgr);
	~BTree();

	void InsertData(const T& key, size_t len, const byte* data);

	void LayerTraverse(IVisitor& visitor);

	bool Query(const T& key, Data<T>& result);

private:
	id_type WriteNode(BTreeNode<T>& node);
	NodePtr<T> ReadNode(id_type id);
	void DeleteNode(const BTreeNode<T>& node);

	void StoreHeader();
	void LoadHeader();

private:
	size_t MaxKeys() const {
		return m_degree * 2 - 1;
	}
	size_t MinKeys() const {
		return m_degree - 1;
	}

private:
	struct Statistics
	{
		size_t reads;
		size_t writes;
		size_t splits;
		size_t hits;
		size_t misses;

		size_t nodes;
		size_t adjustments;
		size_t query_results;
		size_t data;
		size_t tree_height;
	};

private:
	IStorageManager* m_storage_mgr;

	id_type m_root_id;
	id_type m_header_id;

	size_t m_degree;

	mutable Statistics m_stats;

	friend class BTreeNode<T>;

}; // BTree

}
}

#include "playdb/btree/BTree.inl"

#endif // _PLAYDB_BTREE_BTREE_H_