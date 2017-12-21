#include "playdb/Exception.h"

#include <sstream>

namespace playdb
{

IndexOutOfBoundsException::IndexOutOfBoundsException(size_t i)
{
	std::ostringstream s;
	s << "Invalid index " << i;
	m_error = s.str();
}

std::string IndexOutOfBoundsException::what() const
{
	return "IndexOutOfBoundsException: " + m_error;
}
InvalidPageException::InvalidPageException(size_t id)
{
	std::ostringstream s;
	s << "Unknown page id " << id;
	m_error = s.str();
}

std::string InvalidPageException::what() const
{
	return "InvalidPageException: " + m_error;
}

IllegalStateException::IllegalStateException(std::string s) 
	: m_error(s)
{
}

std::string IllegalStateException::what() const
{
	return "IllegalStateException: " + m_error + "\nPlease contact "/* + PACKAGE_BUGREPORT*/;
}

}