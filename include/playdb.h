#ifndef _PLAYDB_PLAYDB_H_
#define _PLAYDB_PLAYDB_H_

#include "playdb/typedef.h"

namespace playdb
{

class IStorageManager
{
public:
	virtual void LoadByteArray(const id_type id, size_t& len, byte** data) = 0;
	virtual void StoreByteArray(id_type& id, const size_t len, const byte* const data) = 0;
	virtual void DeleteByteArray(const id_type id) = 0;
	virtual ~IStorageManager() {}
}; // IStorageManager

class ISerializable
{
public:
	virtual size_t GetByteArraySize() const = 0;
	virtual void LoadFromByteArray(const byte* data) = 0;
	virtual void StoreToByteArray(byte** data, size_t& length) const = 0;
	virtual ~ISerializable() {}
}; // ISerializable

class INode : public ISerializable
{
public:
	virtual id_type GetID() const = 0;
	virtual bool IsLeaf() const = 0;
	virtual size_t GetChildrenCount() const = 0;
	virtual ~INode() {}
}; // INode

class IData
{
public:
	virtual ~IData() {}
}; // IData

class IVisitor
{
public:
	virtual void VisitNode(const INode&) = 0;
	virtual void VisitData(const IData&) = 0;
	virtual ~IVisitor() {}
}; // IVisitor

namespace storage
{

//static const id_type NULL_PAGE = -1;
static const id_type NEW_PAGE = -1;

}

}

#endif // _PLAYDB_PLAYDB_H_