#include <iostream>
#include "../library/myFunctions/imageTransformationParallel.cpp"
#include "../library/myFunctions/imageTransformationSequential.cpp"
#include "../library/myFunctions/readWriteImages.cpp"
#include <stdio.h>
#include <boost/program_options.hpp>
#include <chrono>

namespace po = boost::program_options;
int numThreads = std::thread::hardware_concurrency();

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
        // Datos generales
        printf("Aplicar desenfoque\n");
        size_t dataSize = sizeof(unsigned char) * width * height * channels;
        printf("Data size: %ld\n", dataSize);

        // Parallel
        printf("\n> PARALLEL\n");
        auto start = std::chrono::high_resolution_clock::now();
        unsigned char *blurredDataParallel = applyGaussianBlurParallel(data, width, height, channels);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedParallel = end - start;
        printf("* Execution time (time): %f s\n", elapsedParallel.count());
        printf("* Throughput: %f pixel/s\n", (dataSize) / elapsedParallel.count());
        printf("* Latency: %f s/pixel\n", (elapsedParallel.count() / dataSize));
        // Save image
        std::string outputParallel = "parallel_" + std::string(output.c_str());
        saveImage(outputParallel.c_str(), blurredDataParallel, width, height, channels);

        // Sequential
        printf("\n> SEQUENTIAL\n");
        start = std::chrono::high_resolution_clock::now();
        unsigned char *blurredDataSequential = applyGaussianBlur(data, width, height, channels);
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedSequential = end - start;
        printf("* Execution time (time): %f s\n", elapsedSequential.count());
        float speedup = elapsedSequential.count() / elapsedParallel.count();

        printf("\n> ADITIONAL METRICS\n");
        printf("* Speedup: %f\n", speedup);
        printf("* Efficiency: %f\n", speedup / numThreads);
        // Save image
        std::string outputSequential = "sequential_" + std::string(output.c_str());
        saveImage(outputSequential.c_str(), blurredDataSequential, width, height, channels);
    }

    if (vm.count("grayscale"))
    {

        // Datos generales
        printf("Convertir a escala de grises\n");
        size_t dataSize = sizeof(unsigned char) * width * height * channels;
        printf("Data size: %ld\n", dataSize);

        // Parallel
        printf("\n> PARALLEL\n");
        auto start = std::chrono::high_resolution_clock::now();
        unsigned char *grayscaleDataParallel = convertToGrayscaleParallel(data, width, height, channels);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        printf("* Execution time (time): %f s\n", elapsed.count());
        printf("* Throughput: %f pixel/s\n", dataSize / elapsed.count());
        printf("* Latency: %f s/pixel\n", (elapsed.count() / dataSize));
        // Save image
        std::string outputParallel = "parallel_" + std::string(output.c_str());
        saveImage(outputParallel.c_str(), grayscaleDataParallel, width, height, 1);

        // Sequential
        printf("\n> SEQUENTIAL\n");
        start = std::chrono::high_resolution_clock::now();
        unsigned char *grayscaleDataSequential = convertToGrayscale(data, width, height, channels);
        end = std::chrono::high_resolution_clock::now();
        elapsed = end - start;
        printf("* Execution time (time): %f s\n", elapsed.count());
        float speedup = elapsed.count() / elapsed.count();

        printf("\n> ADITIONAL METRICS\n");
        printf("* Speedup: %f\n", speedup);
        printf("* Efficiency: %f\n", speedup / numThreads);
        // Save image
        std::string outputSequential = "sequential_" + std::string(output.c_str());
        saveImage(outputSequential.c_str(), grayscaleDataSequential, width, height, 1);

    }

    if (vm.count("resize"))
    {
        std::vector<int> values = vm["resize"].as<std::vector<int>>();
        printf("Redimensionar imagen (ancho alto)\n");
        int newWidth = values[0];
        int newHeight = values[1];
        size_t dataSize = sizeof(unsigned char) * width * height * channels;
        printf("Data size: %ld\n", dataSize);
        printf("New size: %d x %d\n", newWidth, newHeight);

        // Parallel
        printf("\n> PARALLEL\n");
        auto start = std::chrono::high_resolution_clock::now();
        unsigned char *resizedData = resizeImageParallel(data, width, height, channels, newWidth, newHeight);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        printf("* Execution time (time): %f s\n", elapsed.count());
        printf("* Throughput: %f pixel/s\n", dataSize / elapsed.count());
        printf("* Latency: %f s/pixel\n", (elapsed.count() / dataSize));
        // Save image
        std::string outputParallel = "parallel_" + std::string(output.c_str());
        saveImage(outputParallel.c_str(), resizedData, newWidth, newHeight, channels);

        // Sequential
        printf("\n> SEQUENTIAL\n");
        start = std::chrono::high_resolution_clock::now();
        unsigned char *resizedDataSequential = resizeImage(data, width, height, channels, newWidth, newHeight);
        end = std::chrono::high_resolution_clock::now();
        elapsed = end - start;
        printf("* Execution time (time): %f s\n", elapsed.count());
        float speedup = elapsed.count() / elapsed.count();

        printf("\n> ADITIONAL METRICS\n");
        printf("* Speedup: %f\n", speedup);
        printf("* Efficiency: %f\n", speedup / numThreads);
        // Save image
        std::string outputSequential = "sequential_" + std::string(output.c_str());
        saveImage(outputSequential.c_str(), resizedDataSequential, newWidth, newHeight, channels);

        free(resizedData);
    }

    if (vm.count("ascii"))
    {
        int newWidth = width / 2;
        int newHeight = height / 2;
        unsigned char *resizedData = resizeImageParallel(data, width, height, channels, newWidth, newHeight);
        printf("Resized - Width: %d, Height: %d, Channels: %d\n", newWidth, newHeight, 1);
        unsigned char *grayscaleData = convertToGrayscaleParallel(resizedData, newWidth, newHeight, channels);
        printf("Converted to Grayscale\n");
        printf("Convertir a arte ASCII\n");
        printImageAsAsciiForSequential(grayscaleData, newWidth, newHeight, 1);
        free(resizedData);
    }

    stbi_image_free(data);

    return 0;
}
