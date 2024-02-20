#include <iostream>
#include <thread>
#include <vector>
#include <cmath>
#include <algorithm>
#include <future>
#include <stdio.h>

const std::string ASCII_CHARS_PARALLEL = " .:-=+*#%@";

void printImageAsAsciiForParallel(unsigned char *data, int width, int height, int channels)
{
    int factor = 1;
    int sizeASCII = 2;

    // Asumimos que la imagen ya está en escala de grises
    for (int y = 0; y < height; y += (sizeASCII * 4))
    {
        for (int x = 0; x < width; x += (sizeASCII * 2))
        {

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
            char asciiChar = ASCII_CHARS_PARALLEL[pixelValue * ASCII_CHARS_PARALLEL.size() / 256];

            // Imprimimos el carácter ASCII
            std::cout << asciiChar;
        }

        // Imprimimos un salto de línea al final de cada fila
        std::cout << '\n';
    }
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
    printf("Number of threads: %d\n", numThreads);

    int rowsPerThread = height / numThreads;
    std::vector<std::future<void>> futures;

    for (int i = 0; i < numThreads; ++i)
    {
        int startRow = i * rowsPerThread;
        int endRow = (i == numThreads - 1) ? height : startRow + rowsPerThread;
        futures.emplace_back(std::async(std::launch::async, convertToGrayscaleThread, data, grayscaleData, width, height, channels, startRow, endRow));
    }

    for (auto &future : futures)
    {
        future.wait();
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
    std::vector<std::future<void>> futures;

    for (int i = 0; i < numThreads; ++i)
    {
        int startRow = i * rowsPerThread;
        int endRow = (i == numThreads - 1) ? newHeight : startRow + rowsPerThread;
        futures.emplace_back(std::async(std::launch::async, [=, &data, &resizedData]()
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
            } }));
    }

    for (auto &future : futures)
    {
        future.wait();
    }

    return resizedData;
}

float **calculateGaussianKernelForParallel(int size, float sigma)
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
    float **kernel = calculateGaussianKernelForParallel(13, 10.0f);

    // Número de hilos a utilizar (puedes ajustar esto según la cantidad de núcleos de tu CPU)
    const int num_threads = std::thread::hardware_concurrency();
    printf("Number of threads: %d\n", num_threads);

    std::vector<std::future<void>> futures;

    // Dividir el trabajo entre los hilos
    int chunk_size = height / num_threads;
    int start_y = 0;
    for (int i = 0; i < num_threads - 1; ++i)
    {
        int end_y = start_y + chunk_size;
        futures.emplace_back(std::async(std::launch::async, blurRange, data, width, height, channels, start_y, end_y, kernel, blurredData));
        start_y = end_y;
    }
    // El último hilo maneja cualquier fila restante
    futures.emplace_back(std::async(std::launch::async, blurRange, data, width, height, channels, start_y, height, kernel, blurredData));

    // Esperar a que todos los hilos terminen
    for (auto &future : futures)
    {
        future.wait();
    }

    return blurredData;
}