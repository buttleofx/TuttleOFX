#include <sam/common/utility.hpp>
#include <sam/common/color.hpp>
#include <sam/common/options.hpp>

#include <tuttle/host/Graph.hpp>
#include <tuttle/common/utils/global.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include <Sequence.hpp>

#include <limits>

using namespace tuttle::host;
namespace bfs = boost::filesystem;
namespace bpo = boost::program_options;
namespace sp  = sequenceParser;

namespace sam {
Color _color;
}

bool verbose = false;
static std::streambuf * const _stdCout = std::cout.rdbuf(); // back up cout's streambuf

static int _notNullImage = 0;
static int _nullFileSize = 0;
static int _corruptedImage = 0;
static int _missingFiles = 0;
static int _processedImages = 0;

enum EReturnCode {
	eReturnCodeOK = 0,
	eReturnCodeErrorInImages = 1,
	eReturnCodeApplicationError = 2
};

enum EImageStatus {
	eImageStatusDiffNull = 0,
	eImageStatusDiffNotNull,
	eImageStatusFileSizeError,
	eImageStatusNoFile,
	eImageStatusImageError
};

/**
 * @brief Process the difference between 2 readers and return status.
 */
EImageStatus diffImageStatus(Graph::Node& read1, Graph::Node& read2, Graph::Node& stat, Graph& graph, const bfs::path& filename1, const bfs::path& filename2)
{
	if (bfs::exists(filename1) == 0 || bfs::exists(filename2) == 0)
		return eImageStatusNoFile;

	if (bfs::file_size(filename1) == 0 || bfs::file_size(filename2) == 0)
		return eImageStatusFileSizeError;

	try
	{
		// Setup parameters
		read1.getParam("filename").setValue(filename1.string());
		read2.getParam("filename").setValue(filename2.string());
		
		if( !verbose )
			std::cout.rdbuf(0);
		graph.compute(stat);
		std::cout.rdbuf(_stdCout);
		if( verbose )
		{
			std::cout << "diff = ";
			for (unsigned int i = 0; i < 3; ++i)
			{
				std::cout << stat.getParam("quality").getDoubleValueAtIndex(i) << "  ";
			}
			std::cout << std::endl;
		}

		for (unsigned int i = 0; i < 3; ++i)
		{
			if (stat.getParam("quality").getDoubleValueAtIndex(i) != 0.0 )
				return eImageStatusDiffNotNull;
		}
		//std::cout << stat << std::endl;

		return eImageStatusDiffNull;
	}
	catch (...)
	{
		std::cerr << boost::current_exception() << std::endl;
		std::cerr << boost::current_exception_diagnostic_information() << std::endl;
		return eImageStatusImageError;
	}
}

/**
 * @brief Process difference between a reader and a generator.
 */
EImageStatus diffImageStatus(Graph::Node& read1, Graph::Node& read2, Graph::Node& stat, Graph& graph, const bfs::path& filename, const std::vector<std::string>& generatorOptions )
{
	if ( ! bfs::exists(filename))
		return eImageStatusNoFile;

	if ( bfs::file_size(filename) == 0 )
		return eImageStatusFileSizeError;

	try
	{
		// Setup parameters
		read1.getParam("filename").setValue(filename.string());
		for( size_t i=0; i<generatorOptions.size(); i++ )
		{
			std::vector<std::string> opt;
			boost::split(opt, generatorOptions.at(i), boost::is_any_of("="));
			int optvalue = atoi ( opt.at(1).c_str() );
			if( optvalue == 0 )
			{
				std::vector<std::string> optList;
				boost::split(optList, opt.at(1), boost::is_any_of(","));
				switch( optList.size() )
				{
					case 1 :
					{ // not a number, set a string parameter
						read2.getParam(opt.at(0)).setValue( opt.at(1) );
						break;
					}
					case 2 :
					{
						float opt0 = atof ( optList.at(0).c_str() );
						float opt1 = atof ( optList.at(1).c_str() );
						read2.getParam(opt.at(0)).setValue( opt0, opt1 );
						break;
					}
					case 3 :
					{
						float opt0 = atof ( optList.at(0).c_str() );
						float opt1 = atof ( optList.at(1).c_str() );
						float opt2 = atof ( optList.at(2).c_str() );
						read2.getParam(opt.at(0)).setValue( opt0, opt1, opt2 );
						break;
					}/*
					case 4 :
					{
						double opt0 = atof ( optList.at(0).c_str() );
						double opt1 = atof ( optList.at(1).c_str() );
						double opt2 = atof ( optList.at(2).c_str() );
						//double opt3 = atof ( optList.at(3).c_str() );
						read2.getParam(opt.at(0)).setValue( opt0, opt1, opt2 );
					}*/
					default :
					{
						for(size_t i=0; i<opt.size(); i++)
								TUTTLE_COUT_VAR(opt.at(i));
						TUTTLE_COUT("ERROR: unable to process " << optList.size() << " arguments");
						break;
					}
				}

				
			}
			else
			{
				read2.getParam(opt.at(0)).setValue( optvalue );
			}
		}

		if( !verbose )
			std::cout.rdbuf(0);
		graph.compute(stat);
		std::cout.rdbuf(_stdCout);

		if( verbose )
		{
			std::cout << "diff = ";
			for (unsigned int i = 0; i < 3; ++i)
			{
				std::cout << stat.getParam("quality").getDoubleValueAtIndex(i) << "  ";
			}
			std::cout << std::endl;
		}

		for (unsigned int i = 0; i < 3; ++i)
		{
			if (stat.getParam("quality").getDoubleValueAtIndex(i) != 0.0 )
				return eImageStatusDiffNotNull;
		}
		//std::cout << stat << std::endl;

		return eImageStatusDiffNull;
    }
	catch (...)
	{
		std::cerr << boost::current_exception() << std::endl;
		std::cerr << boost::current_exception_diagnostic_information() << std::endl;
		return eImageStatusImageError;
	}
}

/**
 * @brief Difference between 2 reader's node associated at 2 files
 */
EImageStatus diffFile(Graph::Node& read1, Graph::Node& read2, Graph::Node& stat, Graph& graph, const bfs::path& filename1, const bfs::path& filename2)
{
	EImageStatus s = diffImageStatus(read1, read2, stat, graph, filename1, filename2);

	//std::cout.rdbuf(_stdCout); // restore cout's original streambuf
	switch (s) {
		case eImageStatusDiffNull:
			break;
		case eImageStatusDiffNotNull:
			if( verbose )
				std::cout << "Diff not null: ";
			++_notNullImage;
			break;
		case eImageStatusFileSizeError:
			if( verbose )
				std::cout << "Null file size: ";
			++_nullFileSize;
			break;
		case eImageStatusNoFile:
			if( verbose )
				std::cout << "Missing file: ";
			++_missingFiles;
			break;
		case eImageStatusImageError:
			if( verbose )
				std::cout << "Corrupted image: ";
			++_corruptedImage;
			break;
	}
	++_processedImages;
	//std::cout << filename1 << "  |  " << filename2 << std::endl;
	//std::cout.rdbuf(0); // remove cout's streambuf
	return s;
}

/**
 * @brief Difference between 1 reader and 1 generator associated with 1 file and vector of options for the generator
 */
EImageStatus diffFile(Graph::Node& read1, Graph::Node& read2, Graph::Node& stat, Graph& graph, const bfs::path& filename1, const std::vector<std::string>& generatorOptions)
{
	EImageStatus s = diffImageStatus(read1, read2, stat, graph, filename1, generatorOptions);

	//std::cout.rdbuf(_stdCout); // restore cout's original streambuf
	switch (s) {
		case eImageStatusDiffNull:
			break;
		case eImageStatusDiffNotNull:
			if( verbose )
				std::cout << "Diff not null: ";
			++_notNullImage;
			break;
		case eImageStatusFileSizeError:
			if( verbose )
				std::cout << "Null file size: ";
			++_nullFileSize;
			break;
		case eImageStatusNoFile:
			if( verbose )
				std::cout << "Missing file: ";
			++_missingFiles;
			break;
		case eImageStatusImageError:
			if( verbose )
				std::cout << "Corrupted image: ";
			++_corruptedImage;
			break;
	}
	++_processedImages;
	//std::cout << filename1 << std::endl;
	//std::cout.rdbuf(0); // remove cout's streambuf
	return s;
}

void diffSequence(Graph::Node& read1, Graph::Node& read2, Graph::Node& stat, Graph& graph, const sp::Sequence& seq1, const sp::Sequence& seq2) {
    for (sp::Time t = seq1.getFirstTime(); t <= seq1.getLastTime(); ++t) {
        diffFile(read1, read2, stat, graph, seq1.getAbsoluteFilenameAt(t), seq2.getAbsoluteFilenameAt(t));
    }
}

void diffSequence(Graph::Node& read1, Graph::Node& read2, Graph::Node& stat, Graph& graph, const sp::Sequence& seq1, const sp::Sequence& seq2, const sp::Time first,
                  const sp::Time last) {
    for (sp::Time t = first; t <= last; ++t) {
        diffFile(read1, read2, stat, graph, seq1.getAbsoluteFilenameAt(t), seq2.getAbsoluteFilenameAt(t));
    }
}

void displayHelp(bpo::options_description &desc) {
    using namespace sam;
    //std::cout.rdbuf(_stdCout); // restore cout's original streambuf
    TUTTLE_COUT( _color._blue << "TuttleOFX project [http://sites.google.com/site/tuttleofx]" << _color._std << std::endl);
    TUTTLE_COUT( _color._blue << "NAME" << _color._std);
    TUTTLE_COUT( _color._green << "\tsam-diff - compute difference between 2 images/sequences" << _color._std << std::endl);
    TUTTLE_COUT( _color._blue << "SYNOPSIS" << _color._std);
    TUTTLE_COUT( _color._green << "\tsam-diff [reader] [input] [reader] [input] [options]" << _color._std << std::endl);
    TUTTLE_COUT( _color._blue << "DESCRIPTION" << _color._std);
    TUTTLE_COUT( _color._green << "\tDiff if sequence have black images." << _color._std );
    TUTTLE_COUT( _color._green << "\tThis tools process the PSNR of an image, and if it's null, the image is considered black." << _color._std );
    TUTTLE_COUT( _color._green << "\tOnly compare RGB layout, not Alpha." << _color._std << std::endl );

    TUTTLE_COUT( _color._blue << "OPTIONS" << _color._std << std::endl);
    TUTTLE_COUT( desc);
    //std::cout.rdbuf(0); // remove cout's streambuf

	TUTTLE_COUT( _color._blue << "EXAMPLES" << _color._std << std::left);
	SAM_EXAMPLE_TITLE_COUT( "Sequence possible definitions: ");
	SAM_EXAMPLE_LINE_COUT("Auto-detect padding : ", "seq.@.jpg");
	SAM_EXAMPLE_LINE_COUT("Padding of 8 (usual style): ", "seq.########.jpg");
	SAM_EXAMPLE_TITLE_COUT( "Compare two images: ");
	SAM_EXAMPLE_LINE_COUT("", "sam-diff --reader tuttle.jpegreader --input path/image.jpg --reader tuttle.jpegreader --input anotherPath/image.jpg");
	SAM_EXAMPLE_TITLE_COUT( "Compare two sequences: ");
	SAM_EXAMPLE_LINE_COUT("", "sam-diff --reader tuttle.jpegreader --input path/seq.@.jpg --reader tuttle.jpegreader --input anotherPath/seq.@.jpg --range 677836 677839");
	SAM_EXAMPLE_TITLE_COUT( "Compare one sequence with one generator (generator need to be every time the second node): ");
	SAM_EXAMPLE_LINE_COUT("", "sam-diff --reader tuttle.jpegreader --input path/seq.@.jpg --reader tuttle.constant --generator-args width=500 components=rgb --range 677836 677839" << std::endl );


}

int main(int argc, char** argv) {
    using namespace sam;

    //std::cout.rdbuf(0); // remove cout's streambuf
    //std::cerr.rdbuf(0); // remove cerr's streambuf

    try {
        std::vector<std::string> inputs;
        std::vector<std::string> nodeId;
        bool hasRange = false;
        bool script = false;
        bool enableColor = false;
        std::vector<int> range;
		
		std::vector<std::string> generator;

        bpo::options_description desc;
        bpo::options_description hidden;

		desc.add_options()(kHelpOptionString, kHelpOptionMessage)
		(kReaderOptionString, bpo::value(&nodeId), kReaderOptionMessage)
		(kInputOptionString, bpo::value(&inputs), kInputOptionMessage)
		(kRangeOptionString, bpo::value(&range)->multitoken(), kRangeOptionMessage)
		(kGeneratorArgsOptionString, bpo::value(&generator)->multitoken(), kGeneratorArgsOptionMessage)
		(kVerboseOptionString, kVerboseOptionMessage)
		(kBriefOptionString, kBriefOptionMessage)
		(kColorOptionString, kColorOptionMessage)
		(kScriptOptionString, kScriptOptionMessage);

        // describe hidden options
        hidden.add_options()(kEnableColorOptionString, bpo::value<std::string>(), kEnableColorOptionMessage);

        bpo::options_description cmdline_options;
        cmdline_options.add(desc).add(hidden);

        bpo::positional_options_description pod;
        pod.add(kInputOptionLongName, -1);

        bpo::variables_map vm;

        try {
            //parse the command line, and put the result in vm
            bpo::store(bpo::command_line_parser(argc, argv).options(cmdline_options).positional(pod).run(), vm);

            // get environment options and parse them
            if (const char* env_diff_options = std::getenv("SAM_DIFF_OPTIONS")) {
                const std::vector<std::string> vecOptions = bpo::split_unix(env_diff_options, " ");
                bpo::store(bpo::command_line_parser(vecOptions).options(cmdline_options).positional(pod).run(), vm);
            }
            if (const char* env_diff_options = std::getenv("SAM_OPTIONS")) {
                const std::vector<std::string> vecOptions = bpo::split_unix(env_diff_options, " ");
                bpo::store(bpo::command_line_parser(vecOptions).options(cmdline_options).positional(pod).run(), vm);
            }
            bpo::notify(vm);
        } catch (const bpo::error& e) {
            TUTTLE_COUT("sam-diff: command line error: " << e.what());
            exit(-2);
        } catch (...) {
            TUTTLE_COUT("sam-diff: unknown error in command line.");
            exit(-2);
        }

		if (vm.count(kScriptOptionLongName)) {
			// disable color, disable directory printing and set relative path by default
			script = true;
		}
		
		if (vm.count(kVerboseOptionLongName)) {
			// print verbose messages
			verbose = true;
		}

        if (vm.count(kColorOptionLongName) && !script) {
            enableColor = true;
        }
        if (vm.count(kEnableColorOptionLongName) && !script) {
            const std::string str = vm[kEnableColorOptionLongName].as<std::string>();
            enableColor = string_to_boolean(str);
        }

        if (enableColor) {
            _color.enable();
        }

        if (vm.count(kBriefOptionLongName)) {
            //std::cout.rdbuf(_stdCout);
            TUTTLE_COUT( _color._green << "diff image files" << _color._std);
            //std::cout.rdbuf(0);
            return 0;
        }

        if (vm.count(kHelpOptionLongName)) {
            displayHelp(desc);
            return 0;
        }
        if (!vm.count(kReaderOptionLongName)) {
            TUTTLE_COUT( _color._red << "sam-diff : no reader specified." << _color._std);
            displayHelp(desc);
            return 0;
        }
        if (!vm.count(kInputOptionLongName)) {
            TUTTLE_COUT( _color._red << "sam-diff : no input specified." << _color._std);
            displayHelp(desc);
            return 0;
        }
		
		if (vm.count(kGeneratorArgsOptionLongName)) {
			generator = vm[kGeneratorArgsOptionLongName].as<std::vector<std::string> >();
		}
		
		nodeId    = vm[kReaderOptionLongName].as<std::vector<std::string> >();
		inputs    = vm[kInputOptionLongName].as<std::vector<std::string> >();
		
		if( nodeId.size() != 2 )
		{
			TUTTLE_COUT( _color._red << "sam-diff : require 2 input nodes." << _color._std);
			displayHelp(desc);
			return 0;
		}
		if (vm.count(kRangeOptionLongName))
		{
			range = vm[kRangeOptionLongName].as<std::vector<int> >();
			hasRange = (range.size() == 2);
		}

		core().preload();
		Graph graph;
		if( verbose )
		{
			TUTTLE_COUT( "in1: " << nodeId.at(0) );
			TUTTLE_COUT( "in2: " << nodeId.at(1) );
		}
		Graph::Node& read1 = graph.createNode(nodeId.at(0));
		Graph::Node& read2 = graph.createNode(nodeId.at(1));
		//Graph::Node& viewer = graph.createNode( "tuttle.viewer" );
		Graph::Node& stat = graph.createNode("tuttle.diff");
		//graph.connect( viewer, stat );
		graph.connect(read1, stat);
		graph.connect(read2, stat.getAttribute("SourceB"));

		switch( inputs.size() )
		{
			case 0 :
			{
				break;
			}
			case 1 :
			{
				bfs::path path1 = inputs.at(0);
	
				if ( bfs::exists(path1)) {
					// process a file
					diffFile(read1, read2, stat, graph, path1, generator );
				}
				else
				{
					// process a sequence
					/*
					try {
						Sequence s1(path1);
						if (hasRange) {
							diffSequence(read1, read2, stat, graph, s1, generator, range[0], range[1]);
						} else {
							diffSequence(read1, read2, stat, graph, s1, generator);
						}
					} catch (...) {
						std::cerr << "Unrecognized pattern " << path1 << std::endl;
						return eReturnCodeApplicationError;
					}*/
				}
				break;
			}
			case 2 :
			{
				bfs::path path1 = inputs.at(0);
				bfs::path path2 = inputs.at(1);


				if (bfs::exists(path1)) {
				if (!bfs::exists(path2)) {
					TUTTLE_COUT( "could not find file or directory 2 (first is not a sequence).");
					return 0;
				}
				/*
				 if( bfs::is_directory( path1 ) )
				 {
				 std::list<boost::shared_ptr<FileObject> > fObjects;
				 fObjects = fileObjectsInDir( path );
				 BOOST_FOREACH( const boost::shared_ptr<FileObject> fObj, fObjects )
				 {
				 switch( fObj->getMaskType() )
				 {
				 case eMaskTypeSequence:
				 {
				 diffSequence( read1, read2, stat, graph, dynamic_cast<const Sequence&>( *fObj ) );
				 break;
				 }
				 case eMaskTypeFile:
				 {
				 const File fFile = dynamic_cast<const File&>( *fObj );
				 diffFile( read1, read2, stat, graph, fFile.getAbsoluteFilename(), fFile.getAbsoluteFilename() );
				 break;
				 }
				 case eMaskTypeDirectory:
				 case eMaskTypeUndefined:
				 break;
				 }
				 }
				 }
				 else
				 {*/
					diffFile(read1, read2, stat, graph, path1, path2);
				 //}
				} else {
					try {
						sp::Sequence s1(path1);
						sp::Sequence s2(path2);
						if (hasRange) {
							diffSequence(read1, read2, stat, graph, s1, s2, range[0], range[1]);
						} else {
							diffSequence(read1, read2, stat, graph, s1, s2);
						}
					} catch (...) {
						std::cerr << "Unrecognized pattern " << path1 << " or " << path2 << std::endl;
						return eReturnCodeApplicationError;
					}
				}
			}
		}
	}
	catch (...)
	{
		//std::cerr.rdbuf(_stdCerr); // restore cout's original streambuf
		std::cerr << "sam-diff error " << boost::current_exception_diagnostic_information() << std::endl;
		//std::cerr.rdbuf(0); // remove cerr's streambuf
		return -1;
	}
	
	//std::cout.rdbuf(_stdCerr); // restore cout's original streambuf
	TUTTLE_COUT( "________________________________________");
	TUTTLE_COUT( "Processed images: " << _processedImages);
	TUTTLE_COUT( "Different images: " << _notNullImage);
	TUTTLE_COUT( "Null file size: " << _nullFileSize);
	TUTTLE_COUT( "Corrupted images: " << _corruptedImage);
	TUTTLE_COUT( "Holes in sequence: " << _missingFiles);
	TUTTLE_COUT( "________________________________________");
	//std::cout.rdbuf(0); // remove cerr's streambuf

	return _notNullImage + _nullFileSize + _corruptedImage + _missingFiles;
}

