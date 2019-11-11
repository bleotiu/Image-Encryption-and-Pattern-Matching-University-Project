//
// Created by Bleo on 1/3/2019.
//

#ifndef PROIECT_PP_PIXEL_H
#define PROIECT_PP_PIXEL_H

typedef struct Pixel{
    unsigned char r,g,b;
};
//operatia de xor intre un pixel si un scalar
struct Pixel XOR_Scalar (struct Pixel P,unsigned int x){
    struct Pixel Q;
    Q.b = P.b ^ (unsigned char)(x & 255);
    Q.g = P.g ^ (unsigned char)((x >> 8) & 255);
    Q.r = P.r ^ (unsigned char)((x >> 16) & 255);
    return Q;
}
//operatia de xor intre 2 pixeli
struct Pixel XOR_Pixel (struct Pixel P,struct Pixel Q){
    struct Pixel P1;
    P1.b = P.b ^ Q.b;
    P1.g = P.g ^ Q.g;
    P1.r = P.r ^ Q.r;
    return P1;
}

#endif //PROIECT_PP_PIXEL_H
