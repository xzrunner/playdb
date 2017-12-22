#include "playdb/btree/BTree.h"
#include "playdb/btree/tools.h"
#include "playdb/storage/DiskStorageManager.h"

#include <sstream>
#include <memory>

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

void test_write()
{
	auto storage_mgr = std::make_unique<playdb::storage::DiskStorageManager>(
		"test_disk.idx", "test_disk.dat", true, 1024);
	playdb::btree::BTree<int> tree(storage_mgr.get(), 3);
	
	for (int i = 1; i < 10; ++i) {
		insert_node(tree, i);
	}

	PrintVisitor visitor;
	tree.LayerTraverse(visitor);
}

void test_read()
{
	auto storage_mgr = std::make_unique<playdb::storage::DiskStorageManager>(
		"test_disk.idx", "test_disk.dat");
	playdb::btree::BTree<int> tree(storage_mgr.get());

	PrintVisitor visitor;
	tree.LayerTraverse(visitor);
}

int main()
{
//	test_write();
	test_read();

	return 0;
}