/**
 *  \file mergesort.cc
 *
 *  \brief Implement your mergesort in this file.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#include "sort.hh"

void p_mergesort(int N, keytype* A);

void p_combine(keytype* comb, int N_A, keytype* A, int N_B, keytype* B);

int b_search(keytype v, int N, keytype* B)
{
	int top = N-1;
	int bottom = 0;
	int k = (top + bottom)/2;
	
	while (bottom < top)
	{		
        if (B[k] == v)
			return k;
		
		if (B[k] < v)
			bottom = k+1;
		else 
			top = k;

		k = (top + bottom)/2;
	}
	if (B[k] > v)
		return k-1;
	else 
		return k;
}

void
mySort (int N, keytype* A)
{
  /* Lucky you, you get to start from scratch */  
	#pragma omp parallel
	{
		#pragma omp master
		{
			p_mergesort(N, A);
		}
	}
  return ;
}

void p_mergesort(int N, keytype* A)
{
	keytype *A_1, *A_2, *comb;
	int i;
	
	if (N <= 1)
	{
		return ;
	}
	else
	{
		A_1 = newCopy((N/2), A);
		A_2 = newCopy((N-(N/2)), (A+(N/2)));		
		comb = newKeys(N);
		
		
		#pragma omp task
		p_mergesort((N/2), A_1);
		#pragma omp task
		p_mergesort((N-(N/2)), A_2);
		#pragma omp taskwait
	
		p_combine(comb, (N/2), A_1, (N-(N/2)), A_2);
		
		//#pragma omp task
		p_mergesort((N/2), A_1);
		//#pragma omp task
		p_mergesort((N-(N/2)), A_2);
		//#pragma omp taskwait
		
		p_combine(comb, (N/2), A_1, (N-(N/2)), A_2);
		
		for(i=0; i<N; i++)
		{
			A[i] = comb[i];
		}
		
		//printf("freeing comb\n");
		free(comb);
		
		return ;
	}
}

void p_combine(keytype* comb, int N_A, keytype* A, int N_B, keytype* B)
{
	int i, k, N_temp;
	keytype *C_1, *C_2, *temp;
	keytype *A_1, *A_2, *B_1, *B_2;
	
	if (N_A < N_B)
	{
		temp = A;
		A = B;
		B = temp;
		N_temp = N_A;
		N_A = N_B;
		N_B = N_temp;
	}
		
	if (N_B == 0)
	{
		for(i=0; i<N_A; i++)
		{
			comb[i] = A[i];
		}
		
		return ;
	}
	if (N_B == 1)
	{		
		k = b_search(B[0], N_A, A);
		
		for(i=0; i<=k; i++)
		{
			comb[i] = A[i];
		}
		
		comb[k+1] = B[0];
		
		for(i=k+2; i<=N_A; i++)
		{
			comb[i] = A[i-1];
		}
		
		return ;
	}
	else 
	{
		A_1 = newCopy((N_A/2), A);
		A_2 = newCopy((N_A-(N_A/2)), (A+(N_A/2)));
		k = b_search(A[(N_A/2)-1], N_B, B);
		
		if (k == -1)
		{
			B_1 = NULL;
			B_2 = B;
		}
		else if (k == (N_B-1))
		{
			B_1 = B;
			B_2 = NULL;
		}
		else
		{
			B_1 = newCopy((k+1), B);
			B_2 = newCopy((N_B-(k+1)), (B+(k+1)));
		}

		C_1 = newKeys((N_A/2)+(k+1));
		C_2 = newKeys((N_A-(N_A/2))+(N_B-(k+1)));
		
	
		#pragma omp task
		p_combine(C_1, (N_A/2), A_1, (k+1), B_1);
		#pragma omp task
		p_combine(C_2, (N_A-(N_A/2)), A_2, (N_B-(k+1)), B_2);
		#pragma omp taskwait
	
		for(i=0; i<((N_A/2)+(k+1)); i++)
		{
			comb[i] = C_1[i];
		}
		for(i=0; i<((N_A-(N_A/2))+(N_B-(k+1))); i++)
		{
			comb[i+((N_A/2)+(k+1))] = C_2[i];
		}
		
		free(C_1);
		free(C_2);
		
		return ;
	}
}

/* eof */
