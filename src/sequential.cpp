#include <stdio.h>
#include <iostream>
#include <boost/program_options.hpp>
#include "../library/myFunctions/readWriteImages.cpp"
#include "../library/myFunctions/imageTransformationSequential.cpp"

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
    po::options_description desc("Opciones");
    desc.add_options()("help,h", "Mostrar ayuda")("input,i", po::value<std::string>(), "Ruta de la imagen de entrada")("output,o", po::value<std::string>(), "Ruta de la imagen de salida")("grayscale,g", "Convertir a escala de grises")("resize,r", po::value<std::vector<int>>()->multitoken(), "Redimensionar imagen (ancho alto)")("blur,b", "Aplicar desenfoque")("ascii,a", "Convertir a arte ASCII");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    int width, height, channels;
    unsigned char *data;
    std::string input;
    std::string output;

    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        return 0;
    }

    if (!vm.count("input"))
    {
        std::cerr << "Debe especificar una imagen de entrada." << std::endl;
        return 1;
    }

    if (!vm.count("output"))
    {
        std::cerr << "Debe especificar una imagen de salida." << std::endl;
        return 1;
    }
    input = vm["input"].as<std::string>();
    output = vm["output"].as<std::string>();
    loadImage(input, width, height, channels, data);

    if (vm.count("blur"))
    {
        printf("Aplicar desenfoque\n");
        size_t dataSize = sizeof(unsigned char) * width * height * channels;
        printf("Data size: %ld\n", dataSize);
        unsigned char *blurredData = applyGaussianBlur(data, width, height, channels);
        saveImage(output.c_str(), blurredData, width, height, channels);
    }

    if (vm.count("grayscale"))
    {
        printf("Convertir a escala de grises\n");
        unsigned char *grayscaleData = convertToGrayscale(data, width, height, channels);
        saveImage(output.c_str(), grayscaleData, width, height, 1);
    }

    if (vm.count("resize"))
    {
        std::vector<int> values = vm["resize"].as<std::vector<int>>();
        printf("Redimensionar imagen (ancho alto)\n");
        int newWidth = values[0];
        int newHeight = values[1];
        unsigned char *resizedData = resizeImage(data, width, height, channels, newWidth, newHeight);
        saveImage(output.c_str(), resizedData, newWidth, newHeight, channels);
        free(resizedData);
    }

    if (vm.count("ascii"))
    {
        int newWidth = width / 2;
        int newHeight = height / 2;
        unsigned char *resizedData = resizeImage(data, width, height, channels, newWidth, newHeight);
        printf("Resized - Width: %d, Height: %d, Channels: %d\n", newWidth, newHeight, 1);
        unsigned char *grayscaleData = convertToGrayscale(resizedData, newWidth, newHeight, channels);
        printf("Converted to Grayscale\n");
        printf("Convertir a arte ASCII\n");
        printImageAsAscii(grayscaleData, newWidth, newHeight, 1);
        free(resizedData);
    }

    stbi_image_free(data);

    return 0;
}
