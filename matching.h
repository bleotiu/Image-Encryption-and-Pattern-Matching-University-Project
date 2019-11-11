//
// Created by Bleo on 1/4/2019.
//

#include "pixel.h"
#include <math.h>
#ifndef PROIECT_PP_MATCHING_H
#define PROIECT_PP_MATCHING_H

int minim (int a,int b){
    if (a > b)
        return b;
    return a;
}

int maxim (int a,int b){
    if (a > b)
        return a;
    return b;
}
//converteste pozitia (i,j) din matrice in pozitia k in liniarizare
int ijtok (int i,int j,int latime){
    return i * latime + j;
}
//converteste pozitia k din liniarizare in pozitia (i,j) in matrice
void ktoij (int k,int *i,int *j,int latime){
    *i = k / latime;
    *j = k % latime;
}
//calculeaza intensitatea medie pentru vectorul v, care reprezinta imaginea grayscale liniarizata
double medie_intensitati (unsigned char *v,int dimensiune){
    double S = 0;
    for (int i = 0;i < dimensiune; ++i)
        S += v[i];
    S /= (double)dimensiune;
    return S;
}
//calculeaza deviatia standard pentru vectorul v, care reprezinta imaginea grayscale liniarizata
double deviatie (unsigned char *v,int dimensiune,double medie){
    double S = 0;
    for (int i = 0; i < dimensiune; ++i)
        S += (medie - (double)v[i]) * (medie - (double)v[i]);
    S /= (double)(dimensiune - 1);
    S = sqrt(S);
    return S;
}
unsigned char grayscale (struct Pixel A){
    unsigned char ans = (unsigned char)(0.299 * ((double)A.r) + 0.587 * ((double)A.g) + 0.114 * ((double)A.b));
    return ans;
}

double corelatie (unsigned char *imagine, unsigned char *sablon,int dimensiune){
    double I,S,deviatie_imagine,deviatie_sablon,corr = 0.0;
    I = medie_intensitati (imagine,dimensiune);
    S = medie_intensitati (sablon,dimensiune);
    deviatie_imagine = deviatie (imagine,dimensiune,I);
    deviatie_sablon = deviatie (sablon,dimensiune,S);
    for (int i = 0; i < dimensiune; ++i)
        corr += ((double)imagine[i] - I) * ((double)sablon[i] - S);
    corr /= deviatie_imagine * deviatie_sablon;
    corr /= (double)dimensiune;
    if (I > 0)
        return corr;
    return 0;
}

void vector_imagine (struct Pixel * imagine,int k,int inaltime_sablon,int latime_sablon,int inaltime_imagine,int latime_imagine, unsigned char **container){
    int i,j,x,y;
    ktoij(k,&i,&j,latime_imagine);
    *container = (unsigned char*)malloc (inaltime_sablon * latime_sablon * sizeof (unsigned char));
    for (x = 0; x < inaltime_sablon; ++x){
        for (y = 0; y < latime_sablon; ++y)
            (*container)[x * latime_sablon + y] = grayscale (imagine[ijtok (i + x,j + y,latime_imagine)]);
    }
}
//aria intersectiei a 2 dreptunghiuri de dimensiune H*W cu colturile
//stanga sus in punctele (x1,y1) si (x2,y2)
int intersectie_dreptunghiuri(int x1,int y1,int x2,int y2,int H,int W){
    int left,right,up,down;
    left = maxim (y1,y2);
    right = minim (y1,y2) + W;
    up = maxim (x1,x2);
    down = minim (x1,x2) + H;
    if (left < right && down > up){
        int intersectie = (right - left) * (down - up);
        return intersectie;
    }
    return 0;
}

#endif //PROIECT_PP_MATCHING_H
