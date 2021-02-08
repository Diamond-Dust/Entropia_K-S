#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define DATALINE "%.17g\n"

typedef struct Check {
    double maxY;            //maksimum danego przedzialu
    double minY;            //minimum danego przedzialu
    double compartmentY;    //rozgranicznik
    double result;          //wynik dla danego przedzialu
    double* Compartments;   //Wczytane dane danego przedzialu
    int* CompartmentsY;     //licznik ile jest wartosci w danym podprzedziale
    int compSize;           //rozmiar przedzialow
    int subcompNum;         //ilosc podprzedzialow
}Check;

int increment(int a)
{
    return ++a;
}

int read_all_from(char* address, int limit)
{
    FILE* ifile = fopen(address, "r");
    if (ifile == NULL)
    {
        perror(address);
        return -1;
    }

    int counter = 0;
    double data;
    while(fscanf(ifile, "%lf", &data) != EOF)
    {
        counter++;
        if(counter > limit)
        {
            printf("Wrong number format in file or file too big.\n");
            return -1;
        }
    }

    fclose(ifile);

    return counter;
}




//Wyliczenie nowego rozgranicznika
void calculate_compartment_y(
    double* target,             //adres rozgranicznika
    double* max_y_address,      //minimum danego przedzialu
    double* min_y_address,      //minimum danego przedzialu
    int     subcompartment_num  //ilosc podprzedzialow
)
{
    *target = (*max_y_address - *min_y_address) / subcompartment_num;
}


void set_data_info(
    double* data_address,
    double* data_data_address,
    int* data_index_address,
    int data_index_data
)
{
    *data_address = *data_data_address;
    *data_index_address = data_index_data;
}


int finalise_compartment_results(
    double* compartments,           //wczytane dane danego przedzialu
    int     compartment_size,       //rozmiar przedzialow
    int*    compartments_y,         //licznik ile jest wartosci w danym podprzedziale
    double* compartment_y_address,  //rozgranicznik
    int     subcompartment_num,     //ilosc podprzedzialow
    double* min_y_address,          //minimum danego przedzialu
    double* result_address          //wynik dla danego przedzialu
)
{
    if(*compartment_y_address == 0) //dla przedzialu stalego
        *result_address = log(subcompartment_num);
    else
    {
        for(int j = 0; j < compartment_size; j++) //Sortowanie kubelkowe do podprzedzialow
        {
            double comp = ((compartments[j] - *min_y_address) / *compartment_y_address);
            if((int)comp == subcompartment_num )   //lewostronne domkniecie
                comp--;
            compartments_y[(int)comp]++;
        }

        *result_address = 0; //zerowanie wyniku

        for(int i=0; i<subcompartment_num; i++)   //Wyliczenie wyniku
        {
            double temp = (double)compartments_y[i] / (double)compartment_size;
            *result_address -= (temp == 0) ? 0: temp*log(temp);
        }
    }
}

int first_compartment(
    FILE*   input,
    FILE*   output,
    double* compartments,           //wczytane dane danego przedzialu
    int     compartment_size,       //rozmiar przedzialow
    int*    compartments_y,         //licznik ile jest wartosci w danym podprzedziale
    double* compartment_y_address,  //rozgranicznik
    int     subcompartment_num,     //ilosc podprzedzialow
    double* min_y_address,          //minimum danego przedzialu
    double* max_y_address,          //maksimum danego przedzialu
    int*    min_y_index_address,    //indeks minimum danego przedzialu
    int*    max_y_index_address,    //indeks maksimum danego przedzialu
    int*    data_size_address,      //liczba danych
    double* result_address          //wynik dla danego przedzialu
)
{
    for(int i = 0; i < compartment_size; i++) //Pierwsze wczytanie
    {
        if(fscanf(input, "%lf", &compartments[i]) != EOF) //Wczytanie danych
        {
            (*data_size_address)++;
            if(i == 0)  //Znalezienie ekstremow
            {
                set_data_info(max_y_address, &compartments[i], max_y_index_address, 0);
                set_data_info(min_y_address, &compartments[i], min_y_index_address, 0);
            }
            else
            {
                if(*max_y_address < compartments[i])
                {
                    set_data_info(max_y_address, &compartments[i], max_y_index_address, i);
                }
                if(*min_y_address > compartments[i])
                {
                    set_data_info(min_y_address, &compartments[i], min_y_index_address, i);
                }
            }
        }
        else    //Przedzialy sa wieksze niz liczba danych
        {
            printf("Nie ma dosc  danych na takie przedzialy.\n");
            return -1;
        }
    }
    calculate_compartment_y(compartment_y_address, max_y_address, min_y_address, subcompartment_num);


    finalise_compartment_results(
        compartments,
        compartment_size,
        compartments_y,
        compartment_y_address,
        subcompartment_num,
        min_y_address,
        result_address
    );


    fprintf(output, DATALINE, *result_address);  //wypisanie wyniku dla pierwszego przedzialu
    fflush(output);
}


int following_compartments(
    FILE* output,
    double* compartments,           //wczytane dane danego przedzialu
    int     compartment_size,       //rozmiar przedzialow
    int*    compartments_y,         //licznik ile jest wartosci w danym podprzedziale
    double* compartment_y_address,  //rozgranicznik
    int     subcompartment_num,     //ilosc podprzedzialow
    double* latest_data_address,    //nowa wczytana dana
    double* min_y_address,          //minimum danego przedzialu
    double* max_y_address,          //maksimum danego przedzialu
    int*    min_y_index_address,    //indeks minimum danego przedzialu
    int*    max_y_index_address,    //indeks maksimum danego przedzialu
    int*    data_size_address,      //liczba danych wyjsciowych
    int*    input_size_address,     //liczba danych
    double* result_address          //wynik dla danego przedzialu
)
{
    (*data_size_address)++;
    for(int i = 0; i < subcompartment_num; i++)   //zerowanie podprzedzialow
    {
        compartments_y[i] = 0;
    }

    if(*max_y_address < *latest_data_address)
    {
        set_data_info(max_y_address, latest_data_address, max_y_index_address, compartment_size);
        calculate_compartment_y(compartment_y_address, max_y_address, min_y_address, subcompartment_num);
    }
    else if(*min_y_address > *latest_data_address)
    {
        set_data_info(min_y_address, latest_data_address, min_y_index_address, compartment_size);
        calculate_compartment_y(compartment_y_address, max_y_address, min_y_address, subcompartment_num);
    }

    if(*max_y_index_address == 0)  //Sprawdzenie czy trzeba zdobyc nowego maxa (optymalizacja)
    {
        set_data_info(max_y_address, latest_data_address, max_y_index_address, compartment_size - 1);
        for(int i = 0; i < compartment_size-1; i++)
        {
            compartments[i] = compartments[i+1];
            if(*max_y_address < compartments[i])
                {
                    set_data_info(max_y_address, &compartments[i], max_y_index_address, i);
                }
        }

        calculate_compartment_y(compartment_y_address, max_y_address, min_y_address, subcompartment_num);
    }
    else if(*min_y_index_address == 0)     //Sprawdzenie czy trzeba zdobyc nowego mina (optymalizacja)
    {
        set_data_info(min_y_address, latest_data_address, min_y_index_address, compartment_size-1);
        for(int i = 0; i < compartment_size-1; i++)
        {
            compartments[i] = compartments[i+1];
            if(*min_y_address > compartments[i])
                {
                    set_data_info(min_y_address, &compartments[i], min_y_index_address, i);
                }
        }

        calculate_compartment_y(compartment_y_address, max_y_address, min_y_address, subcompartment_num);
    }
    else    //jesli nie, tylko przesun wszystko w lewo
    {
        for(int i = 0; i < compartment_size-1; i++)
        {
            compartments[i] = compartments[i+1];
        }
        (*min_y_index_address)--;
        (*max_y_index_address)--;
    }

    compartments[compartment_size-1] = *latest_data_address;   //Wpisanie nowej wartosci na koniec przedzialu

    finalise_compartment_results(
        compartments,
        compartment_size,
        compartments_y,
        compartment_y_address,
        subcompartment_num,
        min_y_address,
        result_address
    );

    fprintf(output, DATALINE, *result_address);
    (*input_size_address)++;
    fflush(output);
}


int plot_koolplot(
    char*   irfile_name,
    char*   ifile_name,
    int     data_size,
    int     input_size,
    int     k               //wypelniacz, by moc pokazac oba wykresy naraz
)
{
    FILE* irfile;
    irfile = fopen(irfile_name, "r");
    if (irfile == NULL)
    {
        perror(irfile_name);
        return -1;
    }
    FILE* ifile;
    ifile = fopen(ifile_name, "r");
    if (ifile == NULL)
    {
        perror(ifile_name);
        return -1;
    }

    double *results, *X, *datas;
    results = (double*)malloc(sizeof(double)*(input_size+k));
    X       = (double*)malloc(sizeof(double)*(input_size+k));
    datas   = (double*)malloc(sizeof(double)*data_size);

    for(int i = 0; i < k; i++)
    {
        results[i] = NAN;//NOPLOT;
        X[i]=i;
    }
    for(int i = k; i < input_size+k; i++)
    {
        fscanf(ifile, "%lf", &results[i]);
        X[i] = i;
    }

    for(int i = 0; i < input_size+k; i++)
    {
        fscanf(irfile, "%lf", &datas[i]);
    }

    char option;
    printf("Czy pokazac wykres (d)anych, (e)ntropii? czy (o)bu?\n");
    scanf(" %c", &option);

    if(option == 'o')
    {
        //Plotdata x(X, input_size), y(results, input_size), z(datas, data_size);
        //setColor(x, y, GREEN);
        //x << x;
        //y << z;
        //plot(x, y, BLACK, "Dane i Entropia");
    }
    else if(option == 'e')
    {
        //Plotdata x(X, input_size), y(results, input_size);
        //plot(x, y, BLACK, "Entropia");
    }
    else if(option == 'd')
    {
        //Plotdata x(X, input_size), z(datas, data_size);
        //plot(x, z, GREEN, "Dane");
    }



    fclose(ifile);
    fclose(irfile);
    free(results);
    free(X);
    free(datas);
}


int full_refactored()
{
    char* data_file_name    = "data.txt";
    char* results_file_name = "result.txt";
    int k=0,imax=0,imin=0, rsize=1, dsize=0;
    double temporary=0, comp=0;
    Check check;
    check.maxY=0;
    check.minY=0;
    check.compartmentY=0;
    check.result=0;

    printf("Jak duze maja byc przedzialy?\n");
    scanf("%d", &check.compSize);
    printf("Ile ma byc byc podprzedzialow?\n");
    scanf("%d", &check.subcompNum);

    printf("Analiza data.txt\n");
    FILE* ifile;
    ifile = fopen(data_file_name, "r");
    FILE* ofile;
    ofile = fopen(results_file_name, "w");

    k = (check.compSize%2==0) ? check.compSize/2-1 : check.compSize/2;
    for(int i=0; i< k; i++)
        fprintf(ofile, "\n");


    check.Compartments = (double*)calloc(sizeof(double), check.compSize);
    check.CompartmentsY = (int*)calloc(sizeof(int), check.subcompNum);

    first_compartment(
        ifile,
        ofile,
        check.Compartments,
        check.compSize,
        check.CompartmentsY,
        &check.compartmentY,
        check.subcompNum,
        &check.minY,
        &check.maxY,
        &imin,
        &imax,
        &dsize,
        &check.result
    );


    while(fscanf(ifile, "%lf", &temporary) != EOF)  //reszta wczytan
    {
        following_compartments(
            ofile,
            check.Compartments,
            check.compSize,
            check.CompartmentsY,
            &check.compartmentY,
            check.subcompNum,
            &temporary,
            &check.minY,
            &check.maxY,
            &imin,
            &imax,
            &dsize,
            &rsize,
            &check.result
        );
    }


    fclose(ifile);
    fclose(ofile);
    free(check.Compartments);
    free(check.CompartmentsY);

    printf("Koniec obliczen\n");

    plot_koolplot(
        data_file_name,
        results_file_name,
        dsize,
        rsize,
        k
    );
}


int calculation(
    char* data_file_name,
    char* results_file_name,
    int compartment_size,
    int subcompartment_num
)
{
    int k=0,imax=0,imin=0, rsize=1, dsize=0;
    double temporary=0, comp=0;
    Check check;
    check.maxY=0;
    check.minY=0;
    check.compartmentY=0;
    check.result=0;

    check.compSize = compartment_size;
    check.subcompNum = subcompartment_num;

    FILE* ifile;
    ifile = fopen(data_file_name, "r");
    FILE* ofile;
    ofile = fopen(results_file_name, "w");

    k = (check.compSize%2==0) ? check.compSize/2-1 : check.compSize/2;
    for(int i=0; i< k; i++)
        fprintf(ofile, "\n");


    check.Compartments = (double*)calloc(sizeof(double), check.compSize);
    check.CompartmentsY = (int*)calloc(sizeof(int), check.subcompNum);

    first_compartment(
        ifile,
        ofile,
        check.Compartments,
        check.compSize,
        check.CompartmentsY,
        &check.compartmentY,
        check.subcompNum,
        &check.minY,
        &check.maxY,
        &imin,
        &imax,
        &dsize,
        &check.result
    );


    while(fscanf(ifile, "%lf", &temporary) != EOF)  //reszta wczytan
    {
        following_compartments(
            ofile,
            check.Compartments,
            check.compSize,
            check.CompartmentsY,
            &check.compartmentY,
            check.subcompNum,
            &temporary,
            &check.minY,
            &check.maxY,
            &imin,
            &imax,
            &dsize,
            &rsize,
            &check.result
        );
    }


    fclose(ifile);
    fclose(ofile);
    free(check.Compartments);
    free(check.CompartmentsY);
}


int main() {
    full_refactored();
    return 0;
}

int full()
{
    int k=0,imax=0,imin=0, rsize=1, dsize=0;
    double temporary=0, comp=0;
    Check check;
    check.maxY=0;
    check.minY=0;
    check.compartmentY=0;
    check.result=0;
    printf("Jak duze maja byc przedzialy?\n");
    scanf("%d", &check.compSize);
    printf("Ile ma byc byc podprzedzialow?\n");
    scanf("%d", &check.subcompNum);

    printf("Analiza data.txt\n");
    FILE* ifile;
    ifile = fopen("data.txt", "r");
    FILE* ofile;
    ofile = fopen("result.txt", "w");

    k = (check.compSize%2==0) ? check.compSize/2-1 : check.compSize/2;
    for(int i=0; i< k; i++)
        fprintf(ofile, "\n");


    check.Compartments = (double*)calloc(sizeof(double), check.compSize);
    check.CompartmentsY = (int*)calloc(sizeof(int), check.subcompNum);

    for(int i=0; i<check.compSize; i++) //Pierwsze wczytanie
        {
            if(fscanf(ifile, "%lf", &check.Compartments[i]) != EOF) //Wczytanie danych
            {
                dsize++;
                if(i == 0)  //Znalezienie ekstremow
                {
                    check.maxY=check.Compartments[i];
                    check.minY=check.Compartments[i];
                }
                else
                {
                    if(check.maxY<check.Compartments[i])
                    {
                        check.maxY = check.Compartments[i];
                        imax = i;
                    }
                    if(check.minY>check.Compartments[i])
                    {
                        check.minY = check.Compartments[i];
                        imin = i;
                    }
                }
            }
            else    //Przedzialy sa wieksze niz liczba danych
            {
                printf("Nie ma dosc  danych na takie przedzialy.\n");
                return 0;
            }
        }
    check.compartmentY = (check.maxY-check.minY)/check.subcompNum;  //wyliczenie rozgranicznika

    if(check.compartmentY == 0) //dla przedzialu stalego
        check.result = log(check.subcompNum);
    else
    {
        for(int j=0; j<check.compSize; j++) //Sortowanie kubelkowe do podprzedzialow
        {
            comp = ((check.Compartments[j] - check.minY) / check.compartmentY);
            if(comp == check.subcompNum )   //lewostronne domkniecie
                comp--;
            check.CompartmentsY[(int)comp]++;
        }

        check.result=0; //zerowanie wyniku

        for(int i=0; i<check.subcompNum; i++)   //Wyliczenie wyniku
        {
            double temp = (double)check.CompartmentsY[i] / (double)check.compSize;
            check.result -= (temp == 0) ? 0: temp*log(temp);
        }
    }


    fprintf(ofile, "%.17g\n", check.result);  //wypisanie wyniku dla pierwszego przedzialu
    fflush(ofile);
    printf("Koniec obliczania pierwszego przedzialu\n");
    while(fscanf(ifile, "%lf", &temporary) != EOF)  //reszta wczytan
    {
        dsize++;
        for(int i=0; i<check.subcompNum; i++)   //zerowanie podprzedzialow
        {
            check.CompartmentsY[i] = 0;
        }

        if(check.maxY < temporary)
        {
            check.maxY = temporary;
            imax = check.compSize;
            check.compartmentY = (check.maxY-check.minY)/check.subcompNum;  //Wyliczenie nowego rozgranicznika
        }
        else if(check.minY > temporary)
        {
            check.minY = temporary;
            imin = check.compSize;
            check.compartmentY = (check.maxY-check.minY)/check.subcompNum;  //Wyliczenie nowego rozgranicznika
        }

        if(imax == 0)  //Sprawdzenie czy trzeba zdobyc nowego maxa (optymalizacja)
        {
            check.maxY = temporary; //Branie nowego maxa
            imax = check.compSize-1;
            for(int i=0; i<check.compSize-1; i++)
            {
                check.Compartments[i] = check.Compartments[i+1];
                if(check.maxY<check.Compartments[i])
                    {
                        check.maxY = check.Compartments[i];
                        imax = i;
                    }
            }

            check.compartmentY = (check.maxY-check.minY)/check.subcompNum;  //Wyliczenie nowego rozgranicznika
        }
        else if(imin == 0)     //Sprawdzenie czy trzeba zdobyc nowego mina (optymalizacja)
        {
            check.minY = temporary;//Branie nowego mina
            imin = check.compSize-1;
            for(int i=0; i<check.compSize-1; i++)
            {
                check.Compartments[i] = check.Compartments[i+1];
                if(check.minY>check.Compartments[i])
                    {
                        check.minY = check.Compartments[i];
                        imin = i;
                    }
            }

            check.compartmentY = (check.maxY-check.minY)/check.subcompNum;  //Wyliczenie nowego rozgranicznika
        }
        else    //jesli nie, tylko przesun wszystko w lewo
        {
            for(int i=0; i<check.compSize-1; i++)
            {
                check.Compartments[i] = check.Compartments[i+1];
            }
            imin--;
            imax--;
        }

        check.Compartments[check.compSize-1] = temporary;   //Wpisanie nowej wartosci na koniec przedzialu

        if(check.compartmentY == 0) //dla przedzialu stalego
            check.result = log(check.subcompNum);
        else
        {
            for(int j=0; j<check.compSize; j++) //Sortowanie kubelkowe do podprzedzialow
            {
                comp = ((check.Compartments[j] - check.minY) / check.compartmentY);
                if(comp == check.subcompNum )   //lewostronne domkniecie
                    comp--;
                check.CompartmentsY[(int)comp]++;
            }

            check.result=0; //zerowanie wyniku

            for(int i=0; i<check.subcompNum; i++)   //Wyliczenie wyniku
            {
                double temp = check.CompartmentsY[i] / (double)check.compSize;
                check.result -= (temp == 0) ? 0 : temp*log(temp);
            }
        }

        fprintf(ofile, "%.17g\n", check.result);
        rsize++;
        fflush(ofile);

    }


    fclose(ofile);
    free(check.Compartments);
    free(check.CompartmentsY);

    printf("Koniec obliczen\n");

    FILE* irfile;
    irfile = fopen("data.txt", "r");
    ifile = fopen("result.txt", "r");
    double* results, *X, *datas;
    results = (double*)malloc(sizeof(double)*(rsize+k));
    X = (double*)malloc(sizeof(double)*(rsize+k));
    datas = (double*)malloc(sizeof(double)*dsize);

    for(int i=0; i<k; i++)
    {
        results[i] = NAN;//NOPLOT;
        X[i]=i;
    }
    for(int i=k; i<rsize+k; i++)
    {
        fscanf(ifile, "%lf", &results[i]);
        X[i] = i;
    }

    for(int i=0; i<rsize+k; i++)
    {
        fscanf(irfile, "%lf", &datas[i]);
    }

    char option;
    printf("Czy pokazac wykres (d)anych, (e)ntropii? czy (o)bu?\n");
    scanf(" %c", &option);

    if(option == 'o')
    {
        //Plotdata x(X, rsize), y(results, rsize), z(datas, dsize);
        //setColor(x, y, GREEN);
        //x << x;
        //y << z;
        //plot(x, y, BLACK, "Dane i Entropia");
    }
    else if(option == 'e')
    {
        //Plotdata x(X, rsize), y(results, rsize);
        //plot(x, y, BLACK, "Entropia");
    }
    else if(option == 'd')
    {
        //Plotdata x(X, rsize), z(datas, dsize);
        //plot(x, z, GREEN, "Dane");
    }



    fclose(ifile);
    fclose(irfile);
}
