#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../library/stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../library/stb_image/stb_image_write.h"
#include <iostream>
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

unsigned char *convertToGrayscale(unsigned char *data, int width, int height, int channels)
{
    unsigned char *grayscaleData = (unsigned char *)malloc(width * height);

    for (int y = 0; y < height; y++)
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

    return grayscaleData;
}

unsigned char *resizeImage(unsigned char *data, int width, int height, int channels, int newWidth, int newHeight)
{
    unsigned char *resizedData = (unsigned char *)malloc(newWidth * newHeight * channels);
    float x_ratio = width / (float)newWidth;
    float y_ratio = height / (float)newHeight;
    float px, py, fx, fy, weight1, weight2, weight3, weight4;
    int ix, iy;

    for (int y = 0; y < newHeight; ++y)
    {
        for (int x = 0; x < newWidth; ++x)
        {
            px = x * x_ratio;
            py = y * y_ratio;
            ix = floor(px);
            iy = floor(py);
            fx = px - ix;
            fy = py - iy;

            // Comprobamos los límites
            int ix1 = ix + 1 < width ? ix + 1 : ix;
            int iy1 = iy + 1 < height ? iy + 1 : iy;

            if (channels == 1)
            {
                weight1 = (1 - fx) * (1 - fy);
                weight2 = fx * (1 - fy);
                weight3 = (1 - fx) * fy;
                weight4 = fx * fy;

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
                    weight1 = (1 - fx) * (1 - fy);
                    weight2 = fx * (1 - fy);
                    weight3 = (1 - fx) * fy;
                    weight4 = fx * fy;

                    resizedData[(y * newWidth + x) * channels + c] =
                        weight1 * data[(iy * width + ix) * channels + c] +
                        weight2 * data[(iy * width + ix1) * channels + c] +
                        weight3 * data[(iy1 * width + ix) * channels + c] +
                        weight4 * data[(iy1 * width + ix1) * channels + c];
                }
            }
        }
    }

    return resizedData;
}

float **calculateGaussianKernel(int size, float sigma)
{
    float **kernel = new float *[size];
    int halfSize = size / 2;
    float sum = 0;
    for (int i = 0; i < size; ++i)
    {
        kernel[i] = new float[size];
        for (int j = 0; j < size; ++j)
        {
            int x = i - halfSize;
            int y = j - halfSize;
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
    std::cout << "Sum: " << sum << std::endl;
    return kernel;
}

unsigned char *applyGaussianBlur(unsigned char *data, int width, int height, int channels)
{
    unsigned char *blurredData = (unsigned char *)malloc(width * height * channels);
    // Kernel de desenfoque gaussiano
    int sizeKernel = 13, limit = (sizeKernel - 1) / 2;
    float **kernel = calculateGaussianKernel(sizeKernel, 10.0f);
    int neighborhood = (sizeKernel - 1) / 2;

    // Iterar sobre cada píxel de la imagen
    for (int y = limit; y < height - limit; ++y)
    {
        for (int x = limit; x < width - limit; ++x)
        {
            // Para cada canal de color
            for (int c = 0; c < channels; ++c)
            {
                float sum = 0.0f;


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
