#ifndef _TUTTLE_HOST_CORE_IMEMORYPOOL_HPP_
#define _TUTTLE_HOST_CORE_IMEMORYPOOL_HPP_

#include <cstddef>
#include <stdexcept>
#include <boost/smart_ptr/intrusive_ptr.hpp>

namespace tuttle {
namespace host {
namespace core {

class IUnknown
{
public:
	virtual void addRef()  = 0;
	virtual void release() = 0;
};

class IPoolData : public IUnknown
{
public:
	virtual char*        data()               = 0;
	virtual const char*  data() const         = 0;
	virtual const size_t size() const         = 0;
	virtual const size_t reservedSize() const = 0;
};

void intrusive_ptr_add_ref( IPoolData* pData );
void intrusive_ptr_release( IPoolData* pData );

typedef ::boost::intrusive_ptr<IPoolData> IPoolDataPtr;

class IMemoryPool
{
public:
	virtual size_t       getUsedMemorySize() const                                                = 0;
	virtual size_t       getAllocatedMemorySize() const                                           = 0;
	virtual size_t       getAvailableMemorySize() const                                           = 0;
	virtual size_t       getWastedMemorySize() const                                              = 0;
	virtual size_t       getMaxMemorySize() const                                                 = 0;
	virtual void         clear( size_t size )                                                     = 0;
	virtual void         clearOne()                                                               = 0;
	virtual void         clearAll()                                                               = 0;
	virtual IPoolDataPtr allocate( const size_t size ) throw( std::bad_alloc, std::length_error ) = 0;
};

}
}
}

#endif