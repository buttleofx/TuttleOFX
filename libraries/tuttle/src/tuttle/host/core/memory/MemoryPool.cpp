#include "MemoryPool.hpp"

namespace tuttle {
namespace host {
namespace core {

unsigned long MemoryPool::Data::_count = 0;

MemoryPool::MemoryPool()
: _memoryUsed(0)
, _memoryAllocated(0)
, _memoryAuthorized(1000000000)
{
}

MemoryPool::~MemoryPool()
{
}

SizeInteger MemoryPool::clear(SizeInteger size)
{
}

SizeInteger MemoryPool::clearOne()
{
}

SizeInteger MemoryPool::clearAll()
{
	_datas.remove_if( data_is_unused );
}

}
}
}