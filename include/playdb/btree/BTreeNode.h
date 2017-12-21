#ifndef _PLAYDB_BTREE_BTREE_NODE_H_
#define _PLAYDB_BTREE_BTREE_NODE_H_

#include "playdb/typedef.h"
//#include "playdb/btree/BTree.h"

#include <stack>
#include <memory>

namespace playdb
{
namespace btree
{

template<typename T>
class BTreeNode;

template <typename T>
using NodePtr = std::shared_ptr<BTreeNode<T>>;

template<typename T>
class BTree;

template <typename T>
class BTreeNode : public INode, public std::enable_shared_from_this<BTreeNode<T>>
{
public:
	BTreeNode();
	BTreeNode(BTree<T>* tree, id_type id, bool leaf);
	BTreeNode(const BTreeNode&) = delete;
	BTreeNode& operator = (const BTreeNode&) = delete;

	//
	// ISerializable interface
	//
	virtual size_t GetByteArraySize() const override;
	virtual void LoadFromByteArray(const byte* data) override;
	virtual void StoreToByteArray(byte** data, size_t& len) const override;

	//
	// INode interface
	//
	virtual id_type GetID() const override { return m_id; }
	virtual bool IsLeaf() const override { return m_leaf; }
	virtual size_t GetChildrenCount() const override { return m_entry_num; }

	void InsertEntryNonFull(size_t data_len, const byte* const data, const T& key, id_type id);
	void DeleteEntry(size_t index);
	
private:
	void SplitChild(size_t idx, NodePtr<T>& node);

	void CopyKey(size_t dst_idx, size_t src_idx, const BTreeNode<T>& src);

private:
	BTree<T>* m_tree;

	id_type m_id;

	bool m_leaf;

	size_t m_entry_num;

	// n - 1 entry, keys
	id_type* m_entry_id;
	T*       m_entry_key;
	byte**   m_entry_data;
	size_t*  m_entry_len;	
	
	// n child
	id_type* m_children;

	template <typename T> 
	friend class BTree;

}; // BTreeNode

}
}

#include "playdb/btree/BTreeNode.inl"

#endif // #define _PLAYDB_BTREE_BTREE_NODE_H_
