/* solution.c (Roland Teodorowitsch; 15 abr. 2021)
 * Compilation: gcc solution.c -o solution -fopenmp
 * Adapted from: https://www.geeksforgeeks.org/maximum-sum-subsequence-of-length-k/
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define MAX_VAL 9999

#define SIZE 100000
#define START 10000
#define STEP 10000

#define K 10

#define MAX(A, B) (((A) > (B)) ? (A) : (B))

void create_sequence(int *v, int size, int seed)
{
    int i;
    srand(seed);
    for (i = 0; i < size; ++i)
        v[i] = rand() % (MAX_VAL + 1);
}

int maximum_sum_subsequence(int *arr, int n, int k)
{
    int i, j, l, ans = -1;
    int **dp;

    dp = (int **)malloc(n * sizeof(int *));
    for (i = 0; i < n; i++)
        dp[i] = (int *)malloc((k + 1) * sizeof(int));
    for (i = 0; i < n; i++)
    {
        // dp[i][0] = -1; // NÃO UTILIZADO
        dp[i][1] = arr[i];
        for (j = 2; j <= k; j++)
            dp[i][j] = -1;
    }

    for (l = 1; l <= k - 1; l++)
    {
#pragma omp parallel for private(j) schedule(guided)
        for (i = 1; i < n; i++)
        {
            for (j = 0; j < i; j++)
            {
                if (arr[j] < arr[i] && dp[j][l] != -1)
                {
                    dp[i][l + 1] = MAX(dp[i][l + 1], dp[j][l] + arr[i]);
                }
            }
        }
    }

    for (i = 0; i < n; i++)
    {
        if (ans < dp[i][k])
            ans = dp[i][k];
    }

    for (i = 0; i < n; i++)
        free((void *)dp[i]);
    free((void *)dp);

    return (ans == -1) ? 0 : ans;
}

int main()
{
    int n, k, v[SIZE];
    double start, finish;

    create_sequence(v, SIZE, 1);
    k = K;
    for (n = START; n <= SIZE; n += STEP)
    {
        start = omp_get_wtime();
        printf("%d\n", maximum_sum_subsequence(v, n, k));
        finish = omp_get_wtime();
        fprintf(stderr, "%d %lf\n", n, finish - start);
    }

    return 0;
}
