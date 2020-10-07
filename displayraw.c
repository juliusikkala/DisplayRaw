/*
Copyright (c) 2014, 2020 Julius Ikkala

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define get_bit(data, off) (((data)>>(off))&1)
#define set_bit(data, off, on) (((data)|(1<<(off)))^((!on)<<(off)))

typedef enum pixel_format
{
    PF_ERROR=0,
    BW1,
    BW2,
    BW4,
    BW8,
    BW16,
    BWA8,
    BWA16,
    R8,
    R16,
    RG8,
    RG16,
    RGB8,
    RGB16,
    RGBA8,
    RGBA16,
    RGBA32F,
} pixel_format;

typedef struct colour
{
    float r, g, b, a;
} colour;

colour formatted_to_intermediate(
    pixel_format data_format,
    const uint8_t *data,
    size_t bit_offset
){
    colour col;
    switch(data_format)
    {
    case BW1:
        col.a=1;
        col.r=get_bit(data[0], bit_offset);
        col.b=col.g=col.r;
        break;
    case BW2:
        col.a=1;
        col.r=get_bit(data[0], bit_offset)|(get_bit(data[0], bit_offset+1)<<1);
        col.r/=3;
        col.b=col.g=col.r;
        break;
    case BW4:
        col.a=1;
        col.r=get_bit(data[0], bit_offset)|(get_bit(data[0], bit_offset+1)<<1)|
            (get_bit(data[0], bit_offset+2)<<2)|(get_bit(data[0], bit_offset+3)<<3);
        col.r/=15;
        col.b=col.g=col.r;
        break;
    case BW8:
        col.a=1;
        col.r=data[0];
        col.r/=255;
        col.b=col.g=col.r;
        break;
    case BW16:
        col.a=1;
        col.r=data[0]|((data[1])<<8);
        col.r/=65535;
        col.b=col.g=col.r;
        break;
    case BWA8:
        col.r=data[0];
        col.a=data[1];
        col.r/=255;
        col.a/=255;
        col.b=col.g=col.r;
        break;
    case BWA16:
        col.r=data[0]|((data[1])<<8);
        col.a=data[2]|((data[3])<<8);
        col.r/=65535;
        col.a/=65535;
        col.b=col.g=col.r;
        break;
    case R8:
        col.r=data[0]/255.0;
        col.g=col.b=0;
        col.a=1;
        break;
    case R16:
        col.r=(data[0]|((data[1])<<8))/65535.0;
        col.g=col.b=0;
        col.a=1;
        break;
    case RG8:
        col.r=data[0]/255.0;
        col.g=data[1]/255.0;
        col.b=0;
        col.a=1;
        break;
    case RG16:
        col.r=(data[0]|((data[1])<<8))/65535.0;
        col.g=(data[2]|((data[3])<<8))/65535.0;
        col.b=0;
        col.a=0;
        break;
    case RGB8:
        col.a=1;
        col.r=data[0]/255.0;
        col.g=data[1]/255.0;
        col.b=data[2]/255.0;
        break;
    case RGBA8:
        col.r=data[0]/255.0;
        col.g=data[1]/255.0;
        col.b=data[2]/255.0;
        col.a=data[3]/255.0;
        break;
    case RGB16:
        col.a=1;
        col.r=(data[0]|((data[1])<<8))/65535.0;
        col.g=(data[2]|((data[3])<<8))/65535.0;
        col.b=(data[4]|((data[5])<<8))/65535.0;
        break;
    case RGBA16:
        col.r=(data[0]|((data[1])<<8))/65535.0;
        col.g=(data[2]|((data[3])<<8))/65535.0;
        col.b=(data[4]|((data[5])<<8))/65535.0;
        col.a=(data[6]|((data[7])<<8))/65535.0;
        break;
    case RGBA32F:
        col.r=((float*)data)[0];
        col.g=((float*)data)[1];
        col.b=((float*)data)[2];
        col.a=((float*)data)[3];
        break;
    default:
        col.r=col.g=col.b=col.a=0;
        break;
    }
    return col;
}

size_t get_pixel_format_bits(pixel_format pf)
{
    switch(pf)
    {
    case BW1:
        return 1;
    case BW2:
        return 2;
    case BW4:
        return 4;
    case BW8:
    case R8:
        return 8;
    case BW16:
    case BWA8:
    case R16:
    case RG8:
        return 16;
    case RGB8:
        return 24;
    case BWA16:
    case RG16:
    case RGBA8:
        return 32;
    case RGB16:
        return 48;
    case RGBA16:
        return 64;
    case RGBA32F:
        return 4*sizeof(float)*8;
    default:
        return 0;
    };
}

void display_help()
{
    printf(
        "Usage: displayraw -w=[width] -h=[height] -p=[pixel format] -f=[filename]\n"
        "0<[width]<65535\n0<[height]<65535\n"
        "[pixel format] is one of the following:\n"
        "\tBW1     ( 1-bit black & white)\n"
        "\tBW2     ( 2-bit grayscale)\n"
        "\tBW4     ( 4-bit grayscale)\n"
        "\tBW8     ( 8-bit grayscale)\n"
        "\tBW16    (16-bit grayscale)\n"
        "\tBWA8    ( 8-bit grayscale with alpha)\n"
        "\tBWA16   (16-bit grayscale with alpha)\n"
        "\tR8      ( 8-bit redscale)\n"
        "\tR16     (16-bit redscale)\n"
        "\tRG8     ( 8-bit red-green)\n"
        "\tRG16    (16-bit red-green)\n"
        "\tRGB8    ( 8-bit RGB)\n"
        "\tRGB16   (16-bit RGB)\n"
        "\tRGBA8   ( 8-bit RGBA)\n"
        "\tRGBA16  (16-bit RGBA)\n"
        "\tRGBA32F  (32-bit float RGBA)\n"
    );
}

int parse_args(
    int argc, char *argv[],
    int *w, int *h,
    pixel_format *pf,
    char **filename
){
    int width_set=0;
    int height_set=0;
    int pf_set=0;
    int filename_set=0;
    int parse_fail=0;
    int string_length=0;
    int i=0;
    
    if(argc!=5)
        return 1;

    for(i=1; i<argc; i++)
    {
        if(argv[i][0]!='-'||strlen(argv[i])<4)
        {
            parse_fail=1;
            break;
        }

        switch(argv[i][1])
        {
        case 'w':
            *w=atoi(argv[i]+3);
            width_set=1;
            break;
        case 'h':
            *h=atoi(argv[i]+3);
            height_set=1;
            break;
        case 'f':
            if(filename_set)
                free(*filename);
            string_length=strlen(argv[i]+3);
            *filename=calloc(string_length+1, 1);
            memcpy(*filename, argv[i]+3, string_length);
            filename_set=1;
            break;
        case 'p':
            pf_set=1;
            if(!strcmp("BW1", argv[i]+3)) *pf=BW1;
            else if(!strcmp("BW2", argv[i]+3)) *pf=BW2;
            else if(!strcmp("BW4", argv[i]+3)) *pf=BW4;
            else if(!strcmp("BW8", argv[i]+3)) *pf=BW8;
            else if(!strcmp("BW16", argv[i]+3)) *pf=BW16;
            else if(!strcmp("BWA8", argv[i]+3)) *pf=BWA8;
            else if(!strcmp("BWA16", argv[i]+3)) *pf=BWA16;
            else if(!strcmp("R8", argv[i]+3)) *pf=R8;
            else if(!strcmp("R16", argv[i]+3)) *pf=R16;
            else if(!strcmp("RG8", argv[i]+3)) *pf=RG8;
            else if(!strcmp("RG16", argv[i]+3)) *pf=RG16;
            else if(!strcmp("RGB8", argv[i]+3)) *pf=RGB8;
            else if(!strcmp("RGB16", argv[i]+3)) *pf=RGB16;
            else if(!strcmp("RGBA8", argv[i]+3)) *pf=RGBA8;
            else if(!strcmp("RGBA16", argv[i]+3)) *pf=RGBA16;
            else if(!strcmp("RGBA32F", argv[i]+3)) *pf=RGBA32F;
            else pf_set=0;
            break;
        }
    }

    if(!width_set||!height_set||!pf_set||!filename_set||parse_fail)
    {
        if(filename_set)
        {
            free(*filename);
            *filename=NULL;
        }
        return 2;
    }
    return 0;
}

SDL_Window *win;
SDL_Renderer *ren;
void init_graphics(int w, int h)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    win=SDL_CreateWindow(
        "Displayraw",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        w, h,
        SDL_WINDOW_SHOWN
    );
    ren=SDL_CreateRenderer(win, -1, 0);
}

void blit_image(int w, int h, pixel_format pf, uint8_t *data)
{
    SDL_RenderClear(ren);
    
    size_t src_byte=0;
    uint8_t src_bit=0;
    uint8_t src_bit_step=get_pixel_format_bits(pf);
    colour intermediate;
    size_t i=0;

    for(i=0;i<w*h;i++)
    {
        intermediate=formatted_to_intermediate(
            pf,
            data+src_byte,
            src_bit
        );
        src_bit+=src_bit_step;
        src_byte+=src_bit/8;
        src_bit-=(src_bit/8)*8;
        SDL_SetRenderDrawColor(
            ren,
            intermediate.r*255+0.5,
            intermediate.g*255+0.5,
            intermediate.b*255+0.5,
            intermediate.a*255+0.5
        );
        SDL_RenderDrawPoint(ren, i%w, i/w);
    }
    
    SDL_RenderPresent(ren);
}

void destroy_graphics()
{
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

int main(int argc, char *argv[])
{
    int width=0;
    int height=0;
    pixel_format pf=PF_ERROR;
    char *filename=NULL;
    uint8_t *image_data=NULL;
    FILE *image_file=NULL;
    int i=0, c=0, image_bytes=0;
    SDL_Event e;
    int quit=0;
    
    if(parse_args(argc, argv, &width, &height, &pf, &filename))
    {
        display_help();
        goto end;
    }

    image_bytes=width*height*get_pixel_format_bits(pf)/8;
    image_data=(uint8_t *)malloc(image_bytes);
    image_file=fopen(filename, "rb");

    if(!image_file)
    {
        printf("File %s does not exist\n", filename);
        goto end;
    }

    for(i=0; (c=fgetc(image_file)) != EOF && i < image_bytes; i++)
        image_data[i]=(uint8_t)c;
    fclose(image_file);

    init_graphics(width, height);
    blit_image(width, height, pf, image_data);

    while(!quit)
    {
        SDL_Delay(1.0/15.0f);
        while(SDL_PollEvent(&e))
        {
            if(e.type==SDL_QUIT)
                quit=1;
        }
    }
    destroy_graphics();

end:
    if(image_data)
        free(image_data);
    if(filename)
        free(filename);
    return 0;
}
