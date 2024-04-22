#include "LineDetection.h"


uint32_t Pixel_x_y(SDL_Surface *img, int j, int i)
{
    uint32_t pixel = *(uint32_t *) ((uint8_t *)img->pixels + i*img->pitch + j*img->format->BytesPerPixel);
    return pixel;
}

void draw2(SDL_Renderer* renderer, SDL_Texture* texture)
{
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void sort(int n, int* ptr)  
{  
    int i, j, t;
    for (i = 0; i < n; i++) {  
  
        for (j = i + 1; j < n; j++) {  
  
            if (*(ptr + j) < *(ptr + i)) {  
  
                t = *(ptr + i);  
                *(ptr + i) = *(ptr + j);  
                *(ptr + j) = t;  
            }  
        }  
    }  
}  

hough_lines* cv_hough(SDL_Surface* img)
{
    float rho = 1.0;
    float theta = 1.0;
    int width, height;
    int numangle, numrho;
    int total = 0;
    float ang;
    int r, n;
    int i, j;
    float irho = 1 / rho;
    double scale;
    int threshold = 400;
    int linesMax = 0;
    width = img->w;
    height = img->h;
    numangle = round(180/theta);
    numrho = round(2*sqrt(width*width+height*height)/ rho);
    printf("numangle = %d\n", numangle);
    printf("numrho = %d\n", numrho);
    int *accum = calloc((numangle+2)*(numrho+2), sizeof(int));
    int *sort_buf = calloc(numangle*numrho, sizeof(int));
    float *tab_sin =  calloc(numangle, sizeof(float));
    float *tab_cos =  calloc(numangle, sizeof(float));

    for( ang = 0, n = 0; n < numangle; ang += theta, n++ )
    {
        tab_sin[n] = (float)(sin(ang) * irho);
        tab_cos[n] = (float)(cos(ang) * irho);
    }

    for( i = 0; i < height; i++ )
    {
        for( j = 0; j < width; j++ )
        {
            if ((Pixel_x_y(img, i, j) & 0xFF) == 0xFF)
            {
                for( n = 0; n < numangle; n++ )
                {
                    r = round(j * tab_cos[n] + i * tab_sin[n]);
                    r += (numrho - 1) / 2;
                    accum[(n+1) * (numrho+2) + r+1]++;
                }
            }
        }
    }

    for( r = 0; r < numrho; r++ )
    {
        for( n = 0; n < numangle; n++ )
        {
            int base = (n+1) * (numrho+2) + r+1;
            if( accum[base] > threshold &&
                accum[base] > accum[base - 1] && accum[base] >= accum[base + 1] &&
                accum[base] > accum[base - numrho - 2] && accum[base] >= accum[base + numrho + 2] )
                sort_buf[total++] = base;
        }
    }
 
    sort(total,sort_buf);
    linesMax = total;
    scale = 1./(numrho+2);
    polar_line *res = malloc(linesMax * sizeof(polar_line));
    for( i = 0; i < linesMax; i++ )
    {
        polar_line line;
        int idx = sort_buf[i];
        int n = floor(idx*scale) - 1;
        int r = idx - (n+1)*(numrho+2) - 1;
        line.rho = (r - (numrho - 1)*0.5f) * rho;
        line.theta = n * theta;
        res[i] = line;
    }

    hough_lines* result = malloc(sizeof(*result));
    result->lines = res;
    result->nb_lines = linesMax;
    return result;
}


hough_lines* hough_transform(SDL_Surface* img)
{
    /*
        to modif : 
    
    */
    int Height = img->h;
    int Width = img->w;
    double p = 1.0;
    double t = 1.0;
    int nb_p = sqrt(Width*Width+Height*Height)/p; //defines the diagonal length for the range of rho
    int nb_t = 180/t; //theta from 0 to 180
    double dp = 1.0;
    double dt = 2*M_PI/(double)nb_t;


    long* res = calloc(nb_p * nb_t, sizeof(long));
    for (int i = 0; i < Height; ++i)
    {
        for (int j = 0; j < Width; ++j) 
        {
            if ((Pixel_x_y(img, i, j) & 0xFF) == 0xFF)
            {
                for (int uno= 0; uno < nb_t; ++uno)
                {
                    double t = uno * dt;
                    double p   = i * cos(t) + (Height - j) * sin(t);
                    int ip  = p / dp;
                    if (ip > 0 && ip < nb_p) ++res[uno * nb_p + ip];
                }
            }
        }
    }
    int threshold = 500;
    long nbLines = 0;
    for (int i = 0; i  < nb_t * nb_p; ++i)
    {
        if (res[i] < threshold) res[i] = 0; 
        else ++nbLines;
    }
    polar_line *lines = calloc(nbLines, sizeof(polar_line));
    int index = 0;
    for (int dos = 0; dos < nb_t; ++dos)
    {
        for (int ip = 0; ip < nb_p; ++ip)
        {
            if (res[dos * nb_p + ip] != 0)
            {
                lines[index].rho = ip * dp;
                lines[index].theta = dos * dt;
                index++;
            }
        }
    }
    hough_lines* result = malloc(sizeof(*result));
    result->lines = lines;
    result->nb_lines = nbLines;
    free(res);
    return result;
}


void drawLine2(SDL_Surface *Screen, int x0, int y0, int x1, int y1, uint32_t pixel)
{
    int i;
    double x = x1 - x0;
    double y = y1 - y0;
    double length = sqrt( x*x + y*y );
    double addx = x / length;
    double addy = y / length;
    x = x0;
    y = y0;
    for (i = 0; i < length; ++i) 
    {
        int bytes = Screen->format->BytesPerPixel;
        Uint8 *p = (Uint8 *)Screen->pixels + (int)y * Screen->pitch + (int)x * bytes;
        switch(bytes) 
        {
        case 1:
            *p = pixel;
            break;
        case 2:
            *(uint16_t *)p = pixel;
            break;
        case 3:
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            } 
            else 
            {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;
        case 4:
            *(Uint32 *)p = pixel;
            break;
        }
        x += addx;
        y += addy;
    }
}

int clamp(int value, int min, int max) 
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}
void Drawlines2(SDL_Surface *source, hough_lines *parameters) {
    int nbLines = parameters->nb_lines;
    polar_line *lines = parameters->lines;
    printf("detected %d lines\n", nbLines);
    for (int i = 0; i < nbLines; ++i) 
    {
        double rho = lines->rho;
        double theta = lines->theta;
        lines++;

        double angle_degrees = theta * 180.0 / M_PI;
        printf("Line %d: Rho = %f, Theta = %f degrees\n", i, rho, angle_degrees);

        int x0 = floor(cos(theta) * rho);
        int y0 = floor(sin(theta) * rho);
        int x1 = clamp(floor(x0 + 1000 * cos(theta + M_PI / 2)), 0, source->w - 1);
        int y1 = clamp(floor(y0 + 1000 * sin(theta + M_PI / 2)), 0, source->h - 1);
        int x2 = clamp(floor(x0 - 1000 * cos(theta + M_PI / 2)), 0, source->w - 1);
        int y2 = clamp(floor(y0 - 1000 * sin(theta + M_PI / 2)), 0, source->h - 1);

        drawLine2(source, x1, y1, x2, y2, 0xFF0000);
    }
}


void event_loop(SDL_Renderer* renderer, SDL_Texture* colored, SDL_Texture* grayscale)
{
    SDL_Event event;
    SDL_Texture* t = colored;
    draw2(renderer, t);
    while (1)
    {
        SDL_WaitEvent(&event);

        switch (event.type)
        {
        	case SDL_QUIT:
			return;
	    	case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				draw2(renderer, t);
			}
			break;
		case SDL_KEYDOWN:
			if (t==colored) t = grayscale;
			else t = colored;
			draw2(renderer, t);
        }
    }
}

SDL_Surface* load_image2(const char* path)
{
    SDL_Surface* surf =  IMG_Load(path);
    if (surf == NULL) errx(EXIT_FAILURE, "%s", SDL_GetError());
    return SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGB888, 0 );
}

