#include <boost/program_options.hpp>
#include <iostream>

// create program options namespace
namespace po = boost::program_options;


int main(int argc, char** argv)
{
    // Define command-line options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Show help message")
        ("something,s", po::value<std::string>(), "Some dummy string");

    // Parse command-line arguments
    po::variables_map vm;
    try 
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } 
    catch (const po::error& ex) 
    {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    // Handle --help
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    // Handle --something
    if (vm.count("something")) 
    {
      std::cout << "The something argument is " << vm["something"].as<std::string>() << std::endl;
    };

    return 0;
}
