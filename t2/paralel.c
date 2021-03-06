/* Sequencial.c (Roland Teodorowitsch) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mpi.h"

/* CONSTANTES */
#define GRAU 400
#define TAM_INI 100
#define TAM_INC 100
#define TAM_MAX 1000

/* VARIAVEIS GLOBAIS */
double x[TAM_MAX], y[TAM_MAX], gabarito[TAM_MAX];

/* PROTOTIPOS */
double polinomio(double v[], int grau, double x);
void erro(char *msg_erro);

double polinomio(double a[], int grau, double x)
{
  int i;
  double res = a[0], pot = x;
  for (i = 1; i <= grau; ++i)
  {
    res += a[i] * pot;
    pot = pot * x;
  }
  return res;
}

void erro(char *msg_erro)
{
  fprintf(stderr, "ERRO: %s\n", msg_erro);
  MPI_Finalize();
  exit(1);
}

int main(int argc, char **argv)
{
  int id; /* Identificador do processo */
  int n;  /* Numero de processos */
  int i, size;
  double *vet, valor, *vresp, resposta, tempo, a[GRAU + 1];
  int hostsize; /* Tamanho do nome do nodo */
  char hostname[MPI_MAX_PROCESSOR_NAME];
  MPI_Status status; /* Status de retorno */

  MPI_Init(&argc, &argv);
  MPI_Get_processor_name(hostname, &hostsize);
  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  MPI_Comm_size(MPI_COMM_WORLD, &n);
  if (id == 0)
  {
    /* Gera os coeficientes do polinomio */
    for (i = 0; i <= GRAU; ++i)
    {
      // printf("[%d] %f\n", i, ((i % 3 == 0) ? -1.0 : 1.0));
      a[i] = (i % 3 == 0) ? -1.0 : 1.0;
    }

    MPI_Bcast(a, GRAU + 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    for (i = 0; i < TAM_MAX; ++i)
    {
      x[i] = 0.1 + 0.1 * (double)i / TAM_MAX;
      gabarito[i] = polinomio(a, GRAU, x[i]);
    }

    for (size = TAM_INI; size <= TAM_MAX; size += TAM_INC)
    {

      tempo = -MPI_Wtime();

      int limiteBloco = size / (n - 1);
      int remainder = size % (n - 1);
      for (i = 1; i < n; i++)
      {
        int tamanho = limiteBloco + ((i == n - 1) ? remainder : 0);
        MPI_Send(&tamanho, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
        MPI_Send(x + ((i - 1) * limiteBloco), tamanho, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
      }

      vresp = malloc(sizeof(double) * (limiteBloco + remainder));
      for (i = 1; i < n; i++)
      {
        MPI_Recv(vresp, limiteBloco + remainder, MPI_DOUBLE, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &status);
        int inicio = ((status.MPI_SOURCE - 1) * limiteBloco);
        int fim = inicio + limiteBloco + ((status.MPI_SOURCE == n - 1) ? remainder : 0);
        for (int j = inicio; j < fim; j++)
          y[j] = vresp[j - inicio];
      }
      free(vresp);

      /* Verificacao */
      for (i = 0; i < size; ++i)
      {
        if (y[i] != gabarito[i])
        {
          printf("Verificacao falhou para size %d em y[%d]!\n", size, i);
        }
      }

      tempo += MPI_Wtime();
      //   /* Mostra tempo */
      printf("%d %lf\n", size, tempo);
    }

    // Encerra Filhos
    for (i = 1; i < n; i++)
        MPI_Send(&id, 1, MPI_INT, i, 5, MPI_COMM_WORLD);
  }
  else
  {
    MPI_Bcast(a, GRAU + 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    while (1)
    {
      int tamanho;
      MPI_Recv(&tamanho, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      if(status.MPI_TAG == 5)
        break;
      vet = malloc(sizeof(double) * tamanho);
      vresp = malloc(sizeof(double) * tamanho);
      MPI_Recv(vet, tamanho, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
      for (int i = 0; i < tamanho; i++)
        vresp[i] = polinomio(a, GRAU, vet[i]);

      MPI_Send(vresp, tamanho, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD);
      free(vet);
      free(vresp);
    }
  }
  MPI_Finalize();
  return 0;
}
