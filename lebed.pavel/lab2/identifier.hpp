#ifndef __IDENTIFIER_H__
#define __IDENTIFIER_H__

#include <cstddef>

class Identifier
{
public:
	static size_t GetId()
	{
		static size_t id = 0;
		id++;
		return id;
	}
};

#endif // __IDENTIFIER_H__
