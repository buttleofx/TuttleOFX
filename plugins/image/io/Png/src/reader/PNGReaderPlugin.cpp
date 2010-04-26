/**
 * @file PNGReaderPlugin.cpp
 * @brief PNG reader ofx plugin
 * @author Eloi Du Bois
 * @date    27/09/09 10:42
 *
 */
#include "PNGReaderPlugin.hpp"
#include "PNGReaderProcess.hpp"
#include "PNGReaderDefinitions.hpp"

#include <ofxsImageEffect.h>
#include <ofxsMultiThread.h>
#include <boost/gil/gil_all.hpp>
#include <boost/filesystem.hpp>
#include "PngEngine/png_adds.hpp"
#include "tuttle/plugin/context/ReaderPlugin.hpp"

namespace tuttle {
namespace plugin {
namespace png {
namespace reader {

namespace bfs = boost::filesystem;
using namespace boost::gil;

PNGReaderPlugin::PNGReaderPlugin( OfxImageEffectHandle handle )
: ReaderPlugin( handle )
{
}

PNGReaderProcessParams PNGReaderPlugin::getProcessParams(const OfxTime time)
{
	PNGReaderProcessParams params;
	params._filepath = _filePattern.getFilenameAt(time);
	return params;
}

/**
 * @brief The overridden render function
 * @param[in]   args     Rendering parameters
 */
void PNGReaderPlugin::render( const OFX::RenderArguments& args )
{
	// instantiate the render code based on the pixel depth of the dst clip
	OFX::BitDepthEnum dstBitDepth         = this->_dstClip->getPixelDepth();
	OFX::PixelComponentEnum dstComponents = this->_dstClip->getPixelComponents();
	// do the rendering
	if( dstComponents == OFX::ePixelComponentRGBA )
	{
		switch( dstBitDepth )
		{
			case OFX::eBitDepthUByte:
			{
				PNGReaderProcess<rgba8_view_t> fred( *this );
				fred.setupAndProcess( args );
				break;
			}
			case OFX::eBitDepthUShort:
			{
				PNGReaderProcess<rgba16_view_t> fred( *this );
				fred.setupAndProcess( args );
				break;
			}
			case OFX::eBitDepthFloat:
			{
				PNGReaderProcess<rgba32f_view_t> fred( *this );
				fred.setupAndProcess( args );
				break;
			}
			case OFX::eBitDepthNone:
				COUT_FATALERROR( "BitDepthNone not recognize." );
				return;
			case OFX::eBitDepthCustom:
				COUT_FATALERROR( "BitDepthCustom not recognize." );
				return;
		}
	}
	else if( dstComponents == OFX::ePixelComponentAlpha )
	{
		switch( dstBitDepth )
		{
			case OFX::eBitDepthUByte:
			{
				PNGReaderProcess<gray8_view_t> fred( *this );
				fred.setupAndProcess( args );
				break;
			}
			case OFX::eBitDepthUShort:
			{
				PNGReaderProcess<gray16_view_t> fred( *this );
				fred.setupAndProcess( args );
				break;
			}
			case OFX::eBitDepthFloat:
			{
				PNGReaderProcess<gray32f_view_t> fred( *this );
				fred.setupAndProcess( args );
				break;
			}
			case OFX::eBitDepthNone:
				COUT_FATALERROR( "BitDepthNone not recognize." );
				return;
			case OFX::eBitDepthCustom:
				COUT_FATALERROR( "BitDepthCustom not recognize." );
				return;
		}
	}
	else
	{
		COUT_FATALERROR( dstComponents << " not recognize." );
	}
}

void PNGReaderPlugin::changedParam( const OFX::InstanceChangedArgs& args, const std::string& paramName )
{
	if( paramName == "Help" )
	{
		sendMessage( OFX::Message::eMessageMessage,
		             "", // No XML resources
		             kPNGReaderHelpString );
	}
	else
	{
		ReaderPlugin::changedParam(args, paramName);
	}
}

bool PNGReaderPlugin::getRegionOfDefinition( const OFX::RegionOfDefinitionArguments& args, OfxRectD& rod )
{
	point2<ptrdiff_t> pngDims = png_read_dimensions( _filePattern.getFilenameAt(args.time) );
	rod.x1 = 0;
	rod.x2 = pngDims.x * this->_dstClip->getPixelAspectRatio();
	rod.y1 = 0;
	rod.y2 = pngDims.y;
	return true;
}

void PNGReaderPlugin::getClipPreferences( OFX::ClipPreferencesSetter& clipPreferences )
{
	ReaderPlugin::getClipPreferences( clipPreferences );
	// Check if exist
	if( bfs::exists( _filePattern.getFirstFilename() ) )
	{
		if ( _explicitConv->getValue() )
		{
			switch( _explicitConv->getValue() )
			{
				case 1:
				{
					clipPreferences.setClipBitDepth( *this->_dstClip, OFX::eBitDepthUByte );
					break;
				}
				case 2:
				{
					clipPreferences.setClipBitDepth( *this->_dstClip, OFX::eBitDepthUShort );
					break;
				}
				case 3:
				{
					clipPreferences.setClipBitDepth( *this->_dstClip, OFX::eBitDepthFloat );
					break;
				}
			}
		}
		else
		{
			OFX::BitDepthEnum bd = OFX::eBitDepthNone;
			int bitDepth = png_read_precision( _filePattern.getFirstFilename() );
			switch( bitDepth )
			{
				case 8:
					bd = OFX::eBitDepthUByte;
					break;
				case 16:
					bd = OFX::eBitDepthUShort;
					break;
				default:
					throw OFX::Exception::Suite( kOfxStatErrImageFormat );
					break;
			}
			clipPreferences.setClipBitDepth( *this->_dstClip, bd );
		}
		clipPreferences.setClipComponents( *this->_dstClip, OFX::ePixelComponentRGBA );
		clipPreferences.setPixelAspectRatio( *this->_dstClip, 1.0 );
	}
}

}
}
}
}
