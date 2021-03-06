#define OFXPLUGIN_VERSION_MAJOR 1
#define OFXPLUGIN_VERSION_MINOR 0

#include "ColorWheelPluginFactory.hpp"
#include <tuttle/plugin/Plugin.hpp>

namespace OFX {
namespace Plugin {

void getPluginIDs( OFX::PluginFactoryArray& ids )
{
	mAppendPluginFactory( ids, tuttle::plugin::colorWheel::ColorWheelPluginFactory, "tuttle.colorwheel" );
}

}
}

