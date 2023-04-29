#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include "spng.h"
#include "inttypes.h"

const char* color_type_str(enum spng_color_type color_type)
{
    switch (color_type)
    {
        case SPNG_COLOR_TYPE_GRAYSCALE: return "grayscale";
        case SPNG_COLOR_TYPE_TRUECOLOR: return "truecolor";
        case SPNG_COLOR_TYPE_INDEXED: return "indexed color";
        case SPNG_COLOR_TYPE_GRAYSCALE_ALPHA: return "grayscale with alpha";
        case SPNG_COLOR_TYPE_TRUECOLOR_ALPHA: return "truecolor with alpha";
        default: return "(invalid)";
    }
}

int main(int argc, char** argv)
{
    int numprocs, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (numprocs < 2)
    {
        printf("Error: se necesitan al menos 2 procesos para ejecutar este programa\n");
        MPI_Finalize();
        return -1;
    }

    char* IMG_PATH = "./photo.png";
    char* OUT_PATH = "./out.png";
    int ret = 0;

    FILE* png;
    spng_ctx* ctx = NULL;
    unsigned char* image = NULL;
    png = fopen(IMG_PATH, "rb");

    ctx = spng_ctx_new(0);

    /* Ignorar y no calcular los CRC de los chunks */
    spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE);

    /* Establecer límites de uso de memoria para almacenar chunks estándar y desconocidos,
       esto es importante al leer archivos no confiables */
    size_t limit = 1024 * 1024 * 64;
    spng_set_chunk_limits(ctx, limit, limit);

    /* Establecer la fuente de la imagen PNG */
    spng_set_png_file(ctx, png); /* o _buffer(), _stream() */

    struct spng_ihdr ihdr;
    spng_get_ihdr(ctx, &ihdr);

    const char* color_name = color_type_str(ihdr.color_type);

    if (rank == 0)
    {
        printf("width: %u\n"
               "height: %u\n"
               "bit depth: %u\n"
               "color type: %u - %s\n",
               ihdr.width, ihdr.height, ihdr.bit_depth, ihdr.color_type, color_name);

        printf("compression method: %u\n"
               "filter method: %u\n"
               "interlace method: %u\n",
               ihdr.compression_method, ihdr.filter_method, ihdr.interlace_method);
    }

    struct spng_plte plte = { 0 };
    spng_get_plte(ctx, &plte);
