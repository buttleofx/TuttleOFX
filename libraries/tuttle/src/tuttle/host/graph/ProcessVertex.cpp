#include "ProcessVertex.hpp"

namespace tuttle {
namespace host {
namespace graph {

std::ostream& operator<<( std::ostream& os, const ProcessVertex& v )
{
	os << v.getName() ;
	return os;
}

}
}
}
