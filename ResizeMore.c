// Copies a BMP file

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include "bmp.h"

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "n infile outfile\n");
        return 1;
    }

    // remember filenames
    float n = atof(argv[1]);

    // if the user value is smaller than 1, clamp it to 0.5f, so we only need to half the picture
    if (n < 1)
    {
        n = 0.5f;
    }

    char *infile = argv[2];
    char *outfile = argv[3];

    // open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER old_bf;
    fread(&old_bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    BITMAPFILEHEADER new_bf;
    new_bf = old_bf;

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER old_bi;
    fread(&old_bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    BITMAPINFOHEADER new_bi;
    new_bi = old_bi;

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (old_bf.bfType != 0x4d42 || old_bf.bfOffBits != 54 || old_bi.biSize != 40 ||
        old_bi.biBitCount != 24 || old_bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }

    // Calculate new values
    new_bi.biWidth *= n;
    new_bi.biHeight *= n;

    // determine padding for scanlines
    int old_padding = (4 - (old_bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    int new_padding = (4 - (new_bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    new_bi.biSizeImage = ((sizeof(RGBTRIPLE) * new_bi.biWidth) + new_padding) * abs(new_bi.biHeight);
    new_bf.bfSize = new_bi.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // write outfile's BITMAPFILEHEADER
    fwrite(&new_bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&new_bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(old_bi.biHeight); i < biHeight; i++)
    {
        RGBTRIPLE trippleAry[new_bi.biWidth * sizeof(RGBTRIPLE)];
        // iterate over pixels in scanline of old file
        int counter = 0;
        for (int j = 0; j < old_bi.biWidth; j++)
        {
            // temporary storage
            RGBTRIPLE triple;

            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);


            for (int p = 0; p < n; p++)
            {
                trippleAry[counter] = triple;
                counter++;
            }
        }

        // the step decides if we take every i in the loop, or every second
        int step = 1;

        if (n == .5f)
        {
            step = 2;
        }

        for (int o = 0; o < n; o++)
        {
            // if we increase the size, we take every scanline, if we half the size, we only take every second scanline
            if (i % step == 0)
            {
                // looping through the new img's width
                for (int t = 0; t < new_bi.biWidth; t++)
                {
                    // calculate the positions of our trippleArray we want to read in. The array hold ALL pixels of the original
                    // If we half the picture size, we only want every second pixel
                    int stepOver = step * t;

                    fwrite(&trippleAry[stepOver], sizeof(RGBTRIPLE), 1, outptr);
                }

                for (int k = 0; k < new_padding; k++)
                {
                    fputc(0x00, outptr);
                }
            }
        }
        // skip over padding, if any
        fseek(inptr, old_padding, SEEK_CUR);
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}
