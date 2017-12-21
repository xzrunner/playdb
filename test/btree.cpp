#include "playdb.h"
#include "playdb/btree/BTree.h"
#include "playdb/btree/tools.h"
#include "playdb/storage/MemoryStorageManager.h"

#include <sstream>

#include <string.h>

class PrintVisitor : public playdb::IVisitor
{
public:
	virtual void VisitNode(const playdb::INode& node)
	{
		printf("visit node: id %d, leaf %d, child_n %d\n", 
			node.GetID(), node.IsLeaf(), node.GetChildrenCount());
	}

	virtual void VisitData(const playdb::IData& data)
	{
		auto entry = dynamic_cast<const playdb::btree::Data<int>&>(data);
		printf("++ visit data: id %d, key %d, %s\n", 
			entry.id, entry.key, (const char*)entry.data);
	}

}; // PrintVisitor

void insert_node(playdb::btree::BTree<int>& tree, int n)
{
	std::ostringstream ss;
	ss << "data" << n;
	auto str = ss.str();
	int zz = str.size();
	tree.InsertData(n, str.size() + 1, (playdb::byte*)(str.c_str()));
}

int main()
{
	PrintVisitor visitor;

	auto storage_mgr = std::make_unique<playdb::storage::MemoryStorageManager>();
	playdb::btree::BTree<int> tree(storage_mgr.get(), 3);

	//for (int i = 1; i < 10; ++i) {
	//	insert_node(tree, i);
	//}

	insert_node(tree, 3);
	insert_node(tree, 10);
	insert_node(tree, 20);
	insert_node(tree, 5);
	insert_node(tree, 6);
	insert_node(tree, 12);
	insert_node(tree, 30);
	insert_node(tree, 7);
	insert_node(tree, 17);

	tree.LayerTraverse(visitor);

	playdb::btree::Data<int> data;
	if (tree.Query(6, data)) {
		printf("query 6: %s\n", data.data);
	}
	if (tree.Query(15, data)) {
		printf("query 15: %s\n", data.data);
	}

	return 0;
}