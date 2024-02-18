#include <iostream>
#include <thread>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION
#include "../library/stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../library/stb_image/stb_image_write.h"
#include <stdio.h>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

const std::string ASCII_CHARS = " .:-=+*#%@";

void loadImage(const std::string &imagePath, int &width, int &height, int &channels, unsigned char *&data)
{
    data = stbi_load(imagePath.c_str(), &width, &height, &channels, STBI_rgb);
    if (data == NULL)
    {
        std::cerr << "Error: " << stbi_failure_reason() << std::endl;
        exit(1);
    }

    std::cout << "Image loaded - Width: " << width << ", Height: " << height << ", Channels: " << channels << std::endl;
}

void printImageAsAscii(unsigned char *data, int width, int height, int channels)
{
    int factor = 1;
    int sizeASCII = 2;

    // Asumimos que la imagen ya está en escala de grises
    for (int y = 0; y < height; y += (sizeASCII * 4))
    {
        for (int x = 0; x < width; x += (sizeASCII * 2))
        {
            // Calculamos el índice del píxel en el array de datos
            int index = y * width + x;

            // calcula el promedio de sus pixeles vecinos con un bucle
            int sum = 0;
            for (int i = 0; i < factor; i++)
            {
                for (int j = 0; j < factor; j++)
                {
                    sum += data[(y + i) * width + (x + j)];
                }
            }

            // Obtenemos el valor del píxel
            unsigned char pixelValue = sum / (factor * factor);

            // Mapeamos el valor del píxel a un carácter ASCII
            char asciiChar = ASCII_CHARS[pixelValue * ASCII_CHARS.size() / 256];

            // Imprimimos el carácter ASCII
            std::cout << asciiChar;
        }

        // Imprimimos un salto de línea al final de cada fila
        std::cout << '\n';
    }
}

void saveImage(const char *filename, unsigned char *data, int width, int height, int channels)
{
    stbi_write_png(filename, width, height, channels, data, width * channels);
}

void convertToGrayscaleThread(unsigned char *data, unsigned char *grayscaleData, int width, int height, int channels, int startRow, int endRow)
{
    for (int y = startRow; y < endRow; y++)
    {
        for (int x = 0; x < width; x++)
        {
            // Calcular el valor promedio de los canales de color
            int sum = 0;
            for (int c = 0; c < channels; c++)
            {
                sum += data[(y * width + x) * channels + c];
            }
            int average = sum / channels;
            // Asignar el mismo valor a cada canal en la imagen en escala de grises
            grayscaleData[y * width + x] = average;
        }
    }
}

unsigned char *convertToGrayscaleParallel(unsigned char *data, int width, int height, int channels)
{
    unsigned char *grayscaleData = (unsigned char *)malloc(width * height);

    int numThreads = std::thread::hardware_concurrency();
    int rowsPerThread = height / numThreads;
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i)
    {
        int startRow = i * rowsPerThread;
        int endRow = (i == numThreads - 1) ? height : startRow + rowsPerThread;
        threads.emplace_back(convertToGrayscaleThread, data, grayscaleData, width, height, channels, startRow, endRow);
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    return grayscaleData;
}

unsigned char *resizeImageParallel(unsigned char *data, int width, int height, int channels, int newWidth, int newHeight)
{
    unsigned char *resizedData = (unsigned char *)malloc(newWidth * newHeight * channels);
    float x_ratio = width / (float)newWidth;
    float y_ratio = height / (float)newHeight;
    int numThreads = std::thread::hardware_concurrency();
    int rowsPerThread = newHeight / numThreads;
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i)
    {
        int startRow = i * rowsPerThread;
        int endRow = (i == numThreads - 1) ? newHeight : startRow + rowsPerThread;
        threads.emplace_back([=, &data, &resizedData]()
                             {
            for (int y = startRow; y < endRow; ++y)
            {
                for (int x = 0; x < newWidth; ++x)
                {
                    float px = x * x_ratio;
                    float py = y * y_ratio;
                    int ix = floor(px);
                    int iy = floor(py);
                    float fx = px - ix;
                    float fy = py - iy;

                    // Comprobamos los límites
                    int ix1 = ix + 1 < width ? ix + 1 : ix;
                    int iy1 = iy + 1 < height ? iy + 1 : iy;

                    if (channels == 1)
                    {
                        float weight1 = (1 - fx) * (1 - fy);
                        float weight2 = fx * (1 - fy);
                        float weight3 = (1 - fx) * fy;
                        float weight4 = fx * fy;

                        resizedData[y * newWidth + x] =
                            weight1 * data[iy * width + ix] +
                            weight2 * data[iy * width + ix1] +
                            weight3 * data[iy1 * width + ix] +
                            weight4 * data[iy1 * width + ix1];
                    }
                    else
                    {
                        for (int c = 0; c < channels; ++c)
                        {
                            float weight1 = (1 - fx) * (1 - fy);
                            float weight2 = fx * (1 - fy);
                            float weight3 = (1 - fx) * fy;
                            float weight4 = fx * fy;

                            resizedData[(y * newWidth + x) * channels + c] =
                                weight1 * data[(iy * width + ix) * channels + c] +
                                weight2 * data[(iy * width + ix1) * channels + c] +
                                weight3 * data[(iy1 * width + ix) * channels + c] +
                                weight4 * data[(iy1 * width + ix1) * channels + c];
                        }
                    }
                }
            } });
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    return resizedData;
}

float **calculateGaussianKernel(int size, float sigma)
{
    float **kernel = new float *[size];
    float sum = 0;
    for (int i = 0; i < size; ++i)
    {
        kernel[i] = new float[size];
        for (int j = 0; j < size; ++j)
        {
            int x = i - size / 2;
            int y = j - size / 2;
            kernel[i][j] = exp(-(x * x + y * y) / (2 * sigma * sigma)) / (2 * M_PI * sigma * sigma);
            sum += kernel[i][j];
        }
    }
    // Normalizar el kernel dividiendo cada elemento por la suma total
    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < size; ++j)
        {
            kernel[i][j] /= sum;
        }
    }
    return kernel;
}

// Función para calcular el desenfoque gaussiano en un rango de píxeles
void blurRange(unsigned char *data, int width, int height, int channels, int start_y, int end_y, float **kernel, unsigned char *blurredData)
{
    // Kernel de desenfoque gaussiano
    int sizeKernel = 13, limit = (sizeKernel - 1) / 2;
    int neighborhood = (sizeKernel - 1) / 2;

    if (start_y == 0)
    {
        start_y = limit;
    }
    if (end_y == height)
    {
        end_y = height - limit;
    }

    for (int y = start_y; y < end_y; ++y)
    {
        for (int x = limit; x < width - limit; ++x)
        {
            // Para cada canal de color
            for (int c = 0; c < channels; ++c)
            {
                float sum = 0.0f;
                // Aplicar el kernel de desenfoque gaussiano al vecindario de nxn del píxel actual

                for (int ky = -neighborhood; ky <= neighborhood; ++ky)
                {
                    for (int kx = -neighborhood; kx <= neighborhood; ++kx)
                    {
                        // Obtener el valor del píxel vecino
                        int pixelX = x + kx;
                        int pixelY = y + ky;
                        int index = (pixelY * width + pixelX) * channels + c;
                        sum += data[index] * kernel[ky + neighborhood][kx + neighborhood];
                    }
                }
                // Asignar el valor calculado al píxel correspondiente en la imagen desenfocada
                blurredData[(y * width + x) * channels + c] = static_cast<unsigned char>(sum);
            }
        }
    }
}

// Función para aplicar el desenfoque gaussiano utilizando múltiples hilos
unsigned char *applyGaussianBlurParallel(unsigned char *data, int width, int height, int channels)
{
    unsigned char *blurredData = (unsigned char *)malloc(width * height * channels);
    float **kernel = calculateGaussianKernel(13, 10.0f);

    // Número de hilos a utilizar (puedes ajustar esto según la cantidad de núcleos de tu CPU)
    const int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;

    // Dividir el trabajo entre los hilos
    int chunk_size = height / num_threads;
    int start_y = 0;
    for (int i = 0; i < num_threads - 1; ++i)
    {
        int end_y = start_y + chunk_size;
        threads.emplace_back(blurRange, data, width, height, channels, start_y, end_y, kernel, blurredData);
        start_y = end_y;
    }
    // El último hilo maneja cualquier fila restante
    threads.emplace_back(blurRange, data, width, height, channels, start_y, height, kernel, blurredData);

    // Esperar a que todos los hilos terminen
    for (auto &thread : threads)
    {
        thread.join();
    }

    return blurredData;
}

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
        unsigned char *blurredData = applyGaussianBlurParallel(data, width, height, channels);
        saveImage(output.c_str(), blurredData, width, height, channels);
    }

    if (vm.count("grayscale"))
    {
        printf("Convertir a escala de grises\n");
        unsigned char *grayscaleData = convertToGrayscaleParallel(data, width, height, channels);
        saveImage(output.c_str(), grayscaleData, width, height, 1);
    }

    if (vm.count("resize"))
    {
        std::vector<int> values = vm["resize"].as<std::vector<int>>();
        printf("Redimensionar imagen (ancho alto)\n");
        int newWidth = values[0];
        int newHeight = values[1];
        unsigned char *resizedData = resizeImageParallel(data, width, height, channels, newWidth, newHeight);
        saveImage(output.c_str(), resizedData, newWidth, newHeight, channels);
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
        printImageAsAscii(grayscaleData, newWidth, newHeight, 1);
        free(resizedData);
    }

    stbi_image_free(data);

    return 0;
}
