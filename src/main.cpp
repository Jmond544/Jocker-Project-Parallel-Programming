#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../library/stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../library/stb_image/stb_image_write.h"
#include <iostream>

const std::string ASCII_CHARS = " .:-=+*#%@";

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
            std::cout << kernel[i][j] << " ";
            sum += kernel[i][j];
        }
        std::cout << std::endl;
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
    int sizeKernel = 13;
    float **kernel = calculateGaussianKernel(sizeKernel, 10.0f);
    int neighborhood = (sizeKernel - 1) / 2;

    // Iterar sobre cada píxel de la imagen
    for (int y = 0; y < height - 0; ++y)
    {
        for (int x = 0; x < width - 0; ++x)
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
    return blurredData;
}

int main()
{
    int width, height, channels;
    unsigned char *data = stbi_load("test.jpg", &width, &height, &channels, STBI_rgb);

    if (data == NULL)
    {
        printf("Error: %s\n", stbi_failure_reason());
        return 1;
    }

    printf("Original - Width: %d, Height: %d, Channels: %d\n", width, height, channels);

    // Reducir el tamaño de la imagen
    int newWidth = width / 2;
    int newHeight = height / 2;
    unsigned char *resizedData = resizeImage(data, width, height, channels, newWidth, newHeight);
    printf("Resized - Width: %d, Height: %d, Channels: %d\n", newWidth, newHeight, 1);

    // Convertir la imagen a escala de grises
    unsigned char *grayscaleData = convertToGrayscale(resizedData, newWidth, newHeight, channels);
    printf("Converted to Grayscale\n");

    // Guardar la imagen en escala de grises
    saveImage("grayscale_image.png", grayscaleData, newWidth, newHeight, 1); // 1 canal para la escala de grises

    // Guardar la imagen redimensionada
    saveImage("resized_image.png", resizedData, newWidth, newHeight, channels);

    // blur

    unsigned char *blurredData = applyGaussianBlur(grayscaleData, newWidth, newHeight, 1);
    printf("Applied Gaussian Blur\n");

    // Guardar la imagen desenfocada
    saveImage("blurred_image.png", blurredData, newWidth, newHeight, 1);

    // Imprimir la imagen en escala de grises como texto ASCII

    printImageAsAscii(grayscaleData, newWidth, newHeight, 1);

    // Liberar la memoria de la imagen original
    stbi_image_free(data);

    // Liberar la memoria de la imagen redimensionada
    free(resizedData);

    return 0;
}
