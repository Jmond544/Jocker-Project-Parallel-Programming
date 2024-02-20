# PROJECT `IMAGES TRANSFORMATION`

`Author:` Mondalgo Tapia, Juan Carlos

`Course:` Parallel Programming

`Date:` 2024-02-20

`Institution:` Universidad Nacional Mayor de San Marcos.

`Faculty:` Ingeniería de sistemas.

## Description

Tool for image transformation. From `grayscale`, `resizing`, to convolution filters (`gaussian blur`). Comparing the performance of the sequential and parallel version.

- `Language`: C++

## Generalities

The operations implemented are:

- [x] **Grayscale**: Convert the image to grayscale.
- [x] **Resize**: Change the size of the image.
- [x] **Gaussian blur**: Apply a gaussian blur to the image.
- [x] **Ascii**: Convert the image to ascii art.

For more information, you can check the slides [here](./slides/), I use marp for the slides.

## How to use

Compile the project:

```bash
# Compile
make
# Also you can use the command
g++ ./src/main.cpp -o ./build/main -lboost_program_options
```

Run the project with the help command to see the options available:

```bash
# Run
./build/main -h
```
