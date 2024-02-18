#include <iostream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char *argv[]) {
    po::options_description desc("Opciones");
    desc.add_options()
        ("help,h", "Mostrar ayuda")
        ("input,i", po::value<std::string>(), "Ruta de la imagen de entrada")
        ("output,o", po::value<std::string>(), "Ruta de la imagen de salida")
        ("grayscale,g", "Convertir a escala de grises")
        ("resize,r", po::value<std::vector<int>>()->multitoken(), "Redimensionar imagen (ancho alto)")
        ("ascii,a", "Convertir a arte ASCII");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    if (!vm.count("input")) {
        std::cerr << "Debe especificar una imagen de entrada." << std::endl;
        return 1;
    }

    // Cómo obtengo los valores del input
    std::string input = vm["input"].as<std::string>();
    std::cout << "Imagen de entrada: " << input << std::endl;
    if (vm.count("resize")) {
        std::vector<int> values = vm["resize"].as<std::vector<int>>();
        std::cout << "Ancho: " << values[0] << std::endl;
        std::cout << "Alto: " << values[1] << std::endl;
    }

    if (vm.count("grayscale")) {
        std::cout << "Convertir a escala de grises" << std::endl;
    }

    if (vm.count("ascii")) {
        std::cout << "Convertir a arte ASCII" << std::endl;
    }

    return 0;
}
