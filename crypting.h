//
// Created by Bleo on 12/28/2018.
//

#ifndef PROIECT_PP_CRYPTING_H
#define PROIECT_PP_CRYPTING_H

void swap (unsigned int *a,unsigned int *b){
    (*a) ^= (*b);
    (*b) ^= (*a);
    (*a) ^= (*b);
}

unsigned int xorshift32 (unsigned int X){
    unsigned int t = X;
    t ^= t << 13;
    t ^= t >> 17;
    t ^= t << 5;
    return t;
}
//generarea numerelor pseudo-aleatoare
void numbers_generator (unsigned int **v,int dimensiune,unsigned int R0){
    *v = (unsigned int*)malloc (dimensiune * sizeof (unsigned int));
    (*v)[0] = R0;
    for (int i = 1; i < dimensiune; ++i)
        (*v)[i] = xorshift32((*v)[i - 1]);
}
//construirea permutarii folosind valorile pseudo-aleatoare din vectorul R
void sigma_generator (unsigned int *R,unsigned int **sigma,int dimensiune){
    *sigma = (unsigned int*)malloc (dimensiune * sizeof (unsigned int));
    int i,j;
    for (i = 0; i < dimensiune; ++i)
        (*sigma)[i] = i;
    for (i = dimensiune - 1; i > 0; --i){
        j = R[dimensiune - i] % (i + 1);
        if(j < i)
            swap (&(*sigma)[i],&(*sigma)[j]);
    }
}
//construirea inversei permutarii
void reverse_sigma (unsigned int **sigma,int dimensiune){
    int i;
    unsigned int *rev_sigma = malloc (dimensiune * sizeof(unsigned int));
    for (i = 0; i < dimensiune; ++i)
        rev_sigma[(*sigma)[i]] = i;
    for (i = 0; i < dimensiune; ++i)
        (*sigma)[i] = rev_sigma[i];
    free (rev_sigma);
}

#endif //PROIECT_PP_CRYPTING_H
