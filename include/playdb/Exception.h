#ifndef _PLAYDB_EXCEPTION_H_
#define _PLAYDB_EXCEPTION_H_

#include <string>

namespace playdb
{

class Exception
{
public:
	virtual std::string what() const = 0;
	virtual ~Exception() {}
}; // Exception 

class IndexOutOfBoundsException : public Exception
{
public:
	IndexOutOfBoundsException(size_t i);
	virtual std::string what() const override;

private:
	std::string m_error;
}; // IndexOutOfBoundsException

class InvalidPageException : public Exception
{
public:
	InvalidPageException(size_t id);
	virtual std::string what() const override;

private:
	std::string m_error;
}; // InvalidPageException

class IllegalStateException : public Exception
{
public:
	IllegalStateException(const std::string& s);
	virtual std::string what() const override;

private:
	std::string m_error;
}; // IllegalStateException

class IllegalArgumentException : public Exception
{
public:
	IllegalArgumentException(const std::string& s);
	virtual std::string what() const override;

private:
	std::string m_error;
}; // IllegalArgumentException

}

#endif // _PLAYDB_EXCEPTION_H_