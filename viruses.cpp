#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <random>
#include <chrono>
#include <math.h>
#include <string.h>  
#include <atomic>
#include <tuple>
#include <sys/mman.h>
#include <stdlib.h>  // para strtol


// #define THRESHOLD 0.3
// #define THRESHOLD_SUPER 0.1
// #define MAX_DAYS 3
// #define M 5 // Es con cuantas personas se reune el infectado

using namespace std;


void* share_mem(int size); // PREGUNTAR POR QUE ESTA ACA PERO SE DEFINE ABAJO// 

// Los detalles de esta funcion NO son relevantes para el TP!! Pero los aclaro por las dudas
// Esta funcion pide size * sizeof(atomic<int>) bytes dinámicamente
// Este espacio permite reservar suficiente memoria para un vector de size enteros atomicos
// Nos devuelve un puntero al comienzo de este espacio contiguo de memoria
// Nota: sizeof es una funcion que nos dice el tamaño de un tipo, en este caso atomic int.


tuple<int, int> simulate(int day,atomic<int> *sick, double threshold, double threshold_super, int m); // PREGUNTAR POR QUE ESTA ACA PERO SE DEFINE ABAJO// 

// Esta funcion simula el proceso de contagio que se lleva a cabo en el dia day en UNA reunion
// Ademas del dia, recibe como parametro el arreglo sick (expresado como puntero) que debe
// ser actualizado con los infectados resultantes de la reunion, y el treshold de infeccion.
// La funcion debe crear los procesos necesarios y devolver una tupla cuya primera componente
// permita saber si el proceso que retorna es el spreader o uno de los procesos recien infecta 


int main(int argc, char* argv[]) 	
{	
	int MAX_DAYS = atoi(argv[1]);
	int M = atoi(argv[2]);
	double THRESHOLD = atof(argv[3]);
	double THRESHOLD_SUPER = atof(argv[4]);
	
	
	int pid_original = getpid();
	// Utilizamos los parámetros que recibe main para obtener los valores de la simulacion (ver orden en pdf)
	
	// int max_days = atoi(argv[1]);
	// int M;
	// int threshold;

	// cout << THRESHOLD << endl;
	// cout << THRESHOLD_SUPER << endl;
 	// Vamos a almacenar la cantidad de infectados nuevos al termino de cada dia en un arreglo de enteros
 	// Es probable que necesiten que este arreglo sea compartido por todos los procesos
 	// Notar que en realidad estamos usando un entero "atomico". Esto nos da propiedades interesantes
	// para evitar algunos problemas, pero pueden asumir que funciona como cualquier otro entero. 
 	// NOTA: A llamar a share_mem hay que hacer algo llamado "casteo" para hacer una conversion de punteros
 	// eso lo que está a la izquierda de la invocación y no hace falta que lo toquen

 	int size = MAX_DAYS;
	atomic<int> *sick = (atomic<int> *) share_mem(size);
	
	
	sick[0] = 1; // Inicializamos 1 infectado para el dia 0

  	
  	// Declaramos las variables que representan los dias y los enfermos por dia
 	int day; // Representa el dia de la simulacion
 	int nsick; // Representa el numero de infectado que infecta el spreader
	int total_infectados = 1; //Sirve para llevar la cuenta de infectados
 	

	for(day = 0; day < MAX_DAYS; day++) // Corremos la simulacion
	{
		tuple<int,int> result = simulate(day,sick, THRESHOLD, THRESHOLD_SUPER, M);
		int pid = get<0>(result); // Obtiene -1 o 355
		nsick = get<1>(result); // El numero de chabones que va a infectar
		
		/*if(pid == 0) // Caso particular, soy el spreader inicial
		{	
			sick[day] = 1;
			break;
		}*/
		if(pid > 0) // Si el infectado infecta
		{
			printf("[%d][dia %d]Infecté a %d procesos, fui infectado por %d \n",getpid(),day, nsick, getppid());
			// fflush(stdout);
			
			for (int z = 0; z < nsick; z++) 
			{
				if (pid > 0)
				{
					total_infectados++;
					pid = fork();
				}
			}
			if(pid > 0)
			{	
				if(day > 0)
				{
					sick[day] = sick[day - 1] + nsick;
				}
				else
				{
					sick[day] = nsick;
				}

				cout << "Cantidad de infectados hasta ahora: " << sick[day] << endl;
				for(int z = 0; z < nsick; z++)
				{
					wait(NULL);
				}
                if(pid > 0)
                {
                    if(day == 0)
                    {
                        break;
                    }
                }    
				exit(0);	
			}
			
            // if(pid > 0)
            // {
            //      //recuento total por dia

            // }
			
		}
		else if(pid == -1) // Si el infectado no infecta
		{
			printf("[%d][dia %d] No Infecté a nadie, fui infectado por %d \n", getpid(), day, getppid());
			fflush(stdout);
			exit(0);
		}
		
		// Si llego aca, es porque fui infectado en este dia, tengo que seguir en la simulacion por un dia mas
		// El continue no es necesario, lo dejo para que quede mas claro
		// continue;
		 
	}
	
	

	if(	getpid() == pid_original) // Solo un proceso debe ser capaz de poder recolectar toda la informacion final y procesarla
	{

		int infectados_por_dia;   // Mostrar total de infectados
		cout << "El total de infectados es: [" << sick[MAX_DAYS - 1] << "]" << endl;
		// for (int x = 0; x < MAX_DAYS; x++)
		// {
		// 	total_infectados = total_infectados + sick[x];
		// }
		
		// printf("Hubo un total de [%d] infectados \n", total_infectados);

		for (int x = 1; x <= MAX_DAYS; x++) 	// Mostrar infectados por dia
		{
			infectados_por_dia = sick[x] - sick[x-1];	//printf("En el dia [%d] se infectaron [%d] \n", x, sick[x]);
		}

		int acumulado = 0;   // Mostrar recuento de infectados segun el dia

		for (int x = 0; x < MAX_DAYS; x++)
		{
			cout << "En el dia " << x + 1 << "hubo un total de " << sick[x];
		}
			
		// A. Media de infectados y maxima cantidad de infectados en un dia

		int media = (sick[MAX_DAYS - 1] / MAX_DAYS);
		printf("La media de infectados es [%d] \n", media);

		int aux = 0;		
		int dia = 0;
		for(int x = 0; x < MAX_DAYS; x++)
		{
			if(sick[x] > aux)
			{
				aux = sick[x];		
				dia = x;		
			}
		}

		printf("El día con más infectados fue el [%d] con una cantidad de [%d] \n", dia, aux);
        exit(0);		
	}
	else // Si soy alguno de los infectados del último dia
	{
		printf("[%d][dia %d] No Infecté a nadie, fui infectado por %d \n",getpid(), MAX_DAYS,getppid());
		fflush(stdout);
		exit(0);	
	}
	
	return 0;
}


void* share_mem(int size)
{
    void * mem;
    if( MAP_FAILED == (mem = (atomic<int>*)mmap(NULL, sizeof(atomic<int>)*size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0) ) )
    {
        perror( "mmap failed" );
        exit( EXIT_FAILURE );
    }
    return mem;	
}



tuple<int, int> simulate(int day,atomic<int> *sick, double threshold, double threshold_super, int m) // Simula el contagio con las personas que se reune puede ser Super Spreader
{
	int nsick = 0;
	int pid = -1;

	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count(); // La semilla es aleatoria, pero puede ser util fijarla para facilitar las pruebas
	std::default_random_engine generator (seed);
	uniform_real_distribution<double> distribution(0.0,1.0); // Inicializamos un generador con distribucion uniforme en el intervalo (0,1)
 	
	float random_super = distribution(generator);  // Distribution(generator) sortea un numero con la distribucion anterior

	if (random_super <= threshold)
	{ 
		nsick = m;
		pid = 355;
	}
	else
	{
		for(int i = 0; i < m; i++)
		{
			double random = distribution(generator); // La probabilidad de que contagie a cada persona por separada
			if (random <= threshold) // Simulamos un experimento de contagio. Si el resultado es <= a 0.2, la persona i resulta contagiada de la reunion				
			{
				nsick++;
				pid = 355;
			}
		}
	}
	return make_tuple(pid,nsick);
}
