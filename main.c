#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crypting.h"
#include "pixel.h"
#include "matching.h"
#define P(x,y,z) (struct Pixel){(unsigned char)x,(unsigned char)y,(unsigned char)z}

struct detectie{
    int k;
    double corelatie;
    struct Pixel culoare;
};

int comparare_detectii (const void * a, const void * b) {
    if( (*(struct detectie*)a).corelatie - (*(struct detectie*)b).corelatie > 0 )
        return -1;
    return 1;
}

struct Pixel* citire_imagine (char nume_fisier[],struct Pixel**container,int*inaltime,int*latime){
    FILE*fin = fopen (nume_fisier,"rb");
    int i,j,padding;
    struct Pixel P;
    if (fin == NULL){
        printf ("Nu am gasit imaginea sursa\n");
        return NULL;
    }
    fseek (fin, 18, SEEK_SET);
    fread (latime, sizeof(int), 1, fin);
    fread (inaltime, sizeof(int), 1, fin);
    *container = (struct Pixel*) malloc((*inaltime) * (*latime) * sizeof(struct Pixel));
    if ((*latime) % 4 != 0)
        padding = 4 - (3 * (*latime)) % 4;
    else
        padding = 0;
    fseek (fin, 54, SEEK_SET);
    for (i = (*inaltime) - 1; i > -1; --i){
        for (j = 0; j < *latime; ++j){
            fread (&P.b, sizeof (unsigned char),1,fin);
            fread (&P.g, sizeof (unsigned char),1,fin);
            fread (&P.r, sizeof (unsigned char),1,fin);
            (*container)[i * (*latime) + j] = P;
        }
        fseek (fin,padding,SEEK_CUR);
    }
    fclose(fin);
    return *container;
}

void citire_sablon (char nume_fisier[],int *inaltime,int *latime, unsigned char **container){
    FILE*fin = fopen (nume_fisier,"rb");
    int i,j,padding;
    struct Pixel P;
    if (fin == NULL){
        printf ("Nu am gasit sablonul\n");
        return;
    }
    fseek (fin, 18, SEEK_SET);
    fread (latime, sizeof(int), 1, fin);
    fread (inaltime, sizeof(int), 1, fin);
    *container = (unsigned char*) malloc((*inaltime) * (*latime) * sizeof(unsigned char));
    if ((*latime) % 4 != 0)
        padding = 4 - (3 * (*latime)) % 4;
    else
        padding = 0;
    fseek (fin, 54, SEEK_SET);
    for (i = (*inaltime) - 1; i > -1; --i){
        for (j = 0; j < *latime; ++j){
            fread (&P.b, sizeof (unsigned char),1,fin);
            fread (&P.g, sizeof (unsigned char),1,fin);
            fread (&P.r, sizeof (unsigned char),1,fin);
            (*container)[i * (*latime) + j] = grayscale(P);
        }
        fseek (fin,padding,SEEK_CUR);
    }
    fclose(fin);
}

void afisare_imagine (char nume_fisier[],char nume_fisier_sursa[],struct Pixel*container){
    FILE *fin =fopen (nume_fisier_sursa,"rb");
    FILE *fout = fopen (nume_fisier,"wb");
    /*if (fin == NULL){
        printf ("Nu am gasit imaginea sursa");
        return ;
    }*/
    int latime,inaltime,padding,i,j;
    unsigned char c;
    fseek (fin,0,SEEK_SET);
    //copiaza header - ul imaginii sursa in imaginea noua
    for (i = 0; i < 54; ++i){
        fread (&c,1,1,fin);
        fwrite (&c,1,1,fout);
        fflush (fout);
    }
    fseek (fin,18,SEEK_SET);
    fread (&latime, sizeof (int),1,fin);
    fread (&inaltime, sizeof (int),1,fin);
    if (latime % 4 == 0)
        padding = 0;
    else
        padding = 4 - (3 * latime) % 4;
    fseek (fout,54,SEEK_SET);
    c = 0;
    //scrierea imaginii noi in memorie
    for (i = inaltime - 1; i > -1; --i){
        for (j = 0; j < latime; ++j){
            fwrite (&container[i * latime + j].b,1,1,fout);
            fwrite (&container[i * latime + j].g,1,1,fout);
            fwrite (&container[i * latime + j].r,1,1,fout);
            fflush(fout);
        }
        for (j = 0; j < padding; ++j){
            fwrite (&c,1,1,fout);
            fflush (fout);
        }
    }
    fclose (fin);
    fclose (fout);
}

void criptare_imagine (struct Pixel*imagine_initiala,struct Pixel**imagine_criptata,char nume_fisier_cheie[],int dimensiune){
    FILE*fin = fopen (nume_fisier_cheie,"r");
    unsigned int R0,SV,*R;
    int i,*sigma;
    fscanf (fin,"%u%u",&R0,&SV);
    //generare numere pseudo-aleatoare
    numbers_generator (&R,2 * dimensiune,R0);
    //generare permutare
    sigma_generator (R,&sigma,dimensiune);
    //construire imagine intermediara
    struct Pixel *aux = malloc (dimensiune * sizeof (struct Pixel));
    for (i = 0; i < dimensiune; ++i)
        aux[sigma[i]] = imagine_initiala[i];
    //construire imagine criptata
    *imagine_criptata = (struct Pixel*) malloc (dimensiune * sizeof (struct Pixel));
    (*imagine_criptata)[0] = XOR_Scalar(aux[0],SV ^ R[dimensiune]);
    for (i = 1; i < dimensiune; ++i){
        (*imagine_criptata)[i] = XOR_Pixel ((*imagine_criptata)[i - 1],aux[i]);
        (*imagine_criptata)[i] = XOR_Scalar ((*imagine_criptata)[i],R[dimensiune + i]);
    }
    free(aux);
    free(R);
    free(sigma);
    fclose(fin);
}

void decriptare_imagine (struct Pixel *imagine_criptata,struct Pixel **imagine_decriptata,char nume_fisier_cheie[],int dimensiune){
    FILE*fin = fopen (nume_fisier_cheie,"r");
    unsigned int R0,SV,*R,*sigma;
    int i;
    fscanf (fin,"%u%u",&R0,&SV);
    //generare numere pseudo-aleatoare
    numbers_generator (&R,2 * dimensiune,R0);
    //generare permutare
    sigma_generator (R,&sigma,dimensiune);
    //construire imagine intermediara
    struct Pixel *aux = malloc (dimensiune * sizeof (struct Pixel));
    aux[0] = XOR_Scalar (imagine_criptata[0],SV ^ R[dimensiune]);
    for (i = 1; i < dimensiune; ++i){
        aux[i] = XOR_Pixel (imagine_criptata[i],imagine_criptata[i - 1]);
        aux[i] = XOR_Scalar (aux[i],R[dimensiune + i]);
    }
    //generare inversa permutarii
    reverse_sigma (&sigma,dimensiune);
    //generare imagine decriptata
    *imagine_decriptata = (struct Pixel*)malloc (dimensiune * sizeof(struct Pixel));
    for (i = 0; i < dimensiune; ++i)
        (*imagine_decriptata)[sigma[i]] = aux[i];
    free(aux);
    free(R);
    free(sigma);
    fclose(fin);
}

void test_chi_patrat (char nume_fisier[]){
    int H,W,i;
    double frecventa[256][3],estimated,red = 0,blue = 0,yellow = 0;
    struct Pixel *imagine;
    citire_imagine (nume_fisier,&imagine,&H,&W);
    estimated = (double)(H * W) / 256;
    for (i = 0; i < 256; ++i){
        frecventa[i][0] = 0;
        frecventa[i][1] = 0;
        frecventa[i][2] = 0;
    }
    for (i = 0; i < H * W; ++i){
        ++frecventa[imagine[i].b][0];
        ++frecventa[imagine[i].g][1];
        ++frecventa[imagine[i].r][2];
    }
    for (i = 0; i < 256; ++i){
        blue += (frecventa[i][0] - estimated) * (frecventa[i][0] - estimated) / estimated;
        yellow += (frecventa[i][1] - estimated) * (frecventa[i][1] - estimated) / estimated;
        red += (frecventa[i][2] - estimated) * (frecventa[i][2] - estimated) / estimated;
    }
    printf("Chi-squared test on RGB channels for %s:\n",nume_fisier);
    printf("R: %.2f\nG: %.2f\nB: %.2f\n\n",red,yellow,blue);
    free(imagine);
}

void adauga_detectie (int poz,double corr,struct Pixel culoare,struct detectie **container,int*dimensiune,int*ocupat){
    struct detectie D;
    D.corelatie = corr;
    D.culoare = culoare;
    D.k = poz;
    if (*dimensiune > *ocupat){
        (*container)[*ocupat] = D;
        ++(*ocupat);
    }
    else{
        *container = (struct detectie*)realloc(*container,2 * (*dimensiune) * sizeof(struct detectie) );
        (*dimensiune) *= 2;
        (*container)[*ocupat] = D;
        ++(*ocupat);
    }
}
//returneaza 1 daca detectiile A si B se suprapun prea mult
int intersectie_detectii (struct detectie A,struct detectie B,int latime_imagine,int H,int W){
    int i,j,x,y;
    ktoij (A.k,&i,&j,latime_imagine);
    ktoij (B.k,&x,&y,latime_imagine);
    double intersectie = intersectie_dreptunghiuri(i,j,x,y,H,W);
    double suprapunere = 2 * H * W - intersectie;
    suprapunere = intersectie / suprapunere;
    if (suprapunere > 0.2)
        return 1;
    return 0;
}

void alege_maxime(struct detectie**container,int *dimensiune,int *ocupat,struct detectie **maxime,
        int latime_imagine,int H,int W) {
    int i, j, cnt = 0;
    for (i = 0; i < *ocupat; ++i) {
        if ((*container)[i].corelatie > 0) {
            ++cnt;
            for (j = i + 1; j < *ocupat; ++j)
                if ((*container)[j].corelatie > 0 && intersectie_detectii((*container)[i], (*container)[j], latime_imagine, H, W)) {
                    (*container)[j].corelatie = 0;
                }
        }
    }
    *maxime = (struct detectie *) malloc(cnt * sizeof(struct detectie));
    j = 0;
    for (i = 0; i < *ocupat; ++i) {
        if ((*container)[i].corelatie > 0) {
            (*maxime)[j] = (*container)[i];
            ++j;
        }
    }
    *ocupat = j;
    free(*container);
}
//pune un chenar colorat cu coltul stanga sus pe pozitia k in liniarizarea imaginii
void coloreaza_fereastra (struct Pixel **imagine,int k,struct Pixel culoare,
        int inaltime_fereastra,int latime_fereastra,int latime_imagine){
    int i,x,y,poz;
    ktoij(k,&x,&y,latime_imagine);
    for (i = 0; i < latime_fereastra; ++i){
        poz = ijtok(x,y + i,latime_imagine);
        (*imagine)[poz] = culoare;
        poz = ijtok(x + inaltime_fereastra - 1,y + i,latime_imagine);
        (*imagine)[poz] = culoare;
    }
    for (i = 0; i < inaltime_fereastra; ++i){
        poz = ijtok(x + i,y,latime_imagine);
        (*imagine)[poz] = culoare;
        poz = ijtok(x+i,y + latime_fereastra - 1,latime_imagine);
        (*imagine)[poz] = culoare;
    }
}
//adauga in vectorul container toate detectiile sablonului pe imagine cu corelatie mai mare decat pragul
void template_matching (struct Pixel *imagine,unsigned char *sablon,struct Pixel culoare,double prag,
                        int inaltime_imagine,int latime_imagine,int H,int W,
                        struct detectie **container,int*size,int*cnt){
    int i,j,k,it;
    unsigned char *curr;
    double corr;
    for (i = 0; i < inaltime_imagine - H; ++i){
        for (j = 0; j < latime_imagine - W; ++j){
            k = ijtok (i,j,latime_imagine);
            vector_imagine(imagine,k,H,W,inaltime_imagine,latime_imagine,&curr);
            corr = corelatie (curr,sablon,H*W);
            if (corr > prag){
                adauga_detectie(k,corr,culoare,container,size,cnt);
            }
        }
    }
}
//aplica template_matching pentru toate sabloanele, elimina non-maximele si reprezinta maximele pe imagine
void prelucrare (struct Pixel **imagine,char sabloane[10][201],double prag,
        int inaltime_imagine,int latime_imagine,struct Pixel *culori){
    unsigned char *sablon;
    struct detectie *container = malloc (sizeof (struct detectie));
    struct detectie *maxime;
    int it,i,H,W,size = 1,cnt = 0;
    citire_sablon ((char*)sabloane[0],&H,&W,&sablon);
    for (it = 0;it < 10; ++it){
        template_matching(*imagine,sablon,culori[it],prag,inaltime_imagine,latime_imagine,H,W,&container,&size,&cnt);
        free(sablon);
        if (it < 9)
            citire_sablon(sabloane[it + 1],&H,&W,&sablon);
    }
    printf("\nAm gasit %d detectii, dintre care",cnt);
    qsort(container,cnt,sizeof (struct detectie),comparare_detectii);
    alege_maxime(&container,&size,&cnt,&maxime,latime_imagine,H,W);
    printf(" %d maxime\n",cnt);
    for (i =0 ;i<cnt;++i)
        coloreaza_fereastra(imagine,maxime[i].k,maxime[i].culoare,H,W,latime_imagine);
    free(maxime);
}

int main() {
    int H = 0,W = 0;

    struct Pixel *image = NULL;
    char image_name[201],reziduu[100];
    while (image == NULL){
        printf("Introduceti numele imaginii sau calea catre ea:");
        fgets (image_name,201,stdin);
        image_name[strlen (image_name) - 1] = '\0';
        ///citirea imaginii initiale
        image = citire_imagine (image_name,&image,&H,&W);
    }
    int task = 0;
    printf ("Introduceti 1 pentru criptare/decriptare si 2 pentru recunoasterea pattern-urilor\n");
    while (task != 1 && task != 2){
        scanf ("%d",&task);
        if (task != 1 && task != 2)
            printf("Cod cerinta invalid");
    }
    fgets (reziduu,100,stdin); // elimina input suplimentar, de exemplu '\n'
    ///Modulul de criptare/decriptare
    if (task == 1){
        char enc_image_name[201];
        char key[201];
        char dec_image_name[201];
        struct Pixel *enc_image, *d_image;
        printf ("\nIntroduceti numele imaginii criptate dorite sau calea catre ea:");
        fgets (enc_image_name,201,stdin);
        enc_image_name[strlen (enc_image_name) - 1] = '\0';
        printf ("\nIntroduceti numele fisierului ce contine cheia sau calea catre el:");
        fgets (key,201,stdin);
        key[strlen (key) - 1] = '\0';
        printf ("\nIntroduceti numele imaginii decriptate dorite sau calea catre ea:");
        fgets (dec_image_name,201,stdin);
        dec_image_name[strlen (dec_image_name) - 1] = '\0';

        ///criptarea
        criptare_imagine (image,&enc_image,key,H*W);
        ///salvarea in memoria externa a imaginii criptate
        afisare_imagine (enc_image_name,image_name,enc_image);
        ///decriptarea imaginii
        //enc_image = citire_imagine (enc_image_name,&enc_image,&H,&W);
        decriptare_imagine (enc_image,&d_image,key,H*W);
        ///salvarea in memoria externa a imaginii decriptate
        afisare_imagine (dec_image_name,enc_image_name,d_image);
        test_chi_patrat (image_name);
        test_chi_patrat (enc_image_name);
        test_chi_patrat (dec_image_name);
        free(image);
        free(enc_image);
        free(d_image);
        printf("Finished succesfully!\n");
    }
    ///Modulul de recunoastere de pattern-uri intr-o imagine
    else{
        int i;
        double prag;
        unsigned char D = 255;
        printf("Introduceti pragul de similaritate:");
        scanf ("%lf",&prag);
        char nume_sabloane[10][201],nume_fisier_sabloane[201] = "sabloane.txt";
        struct Pixel culori[10] = {P(255,0,0),P(255,255,0),P(0,255,0),
                                   P(0,255,255),P(255,0,255),P(0,0,255),P(192,192,192),
                                   P(D,140,0),P(128,0,128),P(128,0,0)};
        FILE *sabloane = fopen (nume_fisier_sabloane,"r");
        for (i = 0;i < 10; ++i){
            fgets(nume_sabloane[i],201,sabloane);
            nume_sabloane[i][strlen (nume_sabloane[i]) - 1] = '\0';
        }
        prelucrare (&image,nume_sabloane,prag,H,W,culori);
        afisare_imagine("colorat.bmp",image_name,image);
        free(image);
        printf("Finished succesfully!\n");
    }
    return 0;
}