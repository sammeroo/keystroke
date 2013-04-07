#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_NO_OF_TEs 	12
#define INFINITE 	999999999

struct __template {		// SUBJECT TO CHANGE
	double MHT_U;
	double MLT_U;
	double MHT_P;
	double MLT_P;
	double ATS;
	double SD;
	double GF;
};

struct dist_struct {
	double dist;
	char name[120];
};

struct namecount {
	char name[120];
	int count;
};

void KNN(FILE *file, FILE *merged_log, int K);			// K = no. of nearest neighbours, returns the usernames that are closest in a file: "KNN_RETURN"
								// 'file' contains the input data to be predicted
double distance(struct __template t1, struct __template t2);

void KNN(FILE *file, FILE *merged_log, int K)
{
	rewind(merged_log);
	FILE *KNNret = fopen("tmp/KNN_RETURN", "a");
	struct __template input, current;

	fscanf(file, "%lf %lf %lf %lf %lf %lf %lf", &input.MHT_U, &input.MLT_U, &input.MHT_P, &input.MLT_P, &input.ATS, &input.SD, &input.GF);
printf("Data read in: %lf %lf %lf %lf %lf %lf %lf\n", input.MHT_U, input.MLT_U, input.MHT_P, input.MLT_P, input.ATS, input.SD, input.GF);
	int N, M, i, j, z;
	char dummy[120], name[120];
	fscanf(merged_log, "%d", &N);
	
	struct dist_struct d[N][MAX_NO_OF_TEs];
	struct namecount nc[N];
	int max_allow = 0;
printf("\n");
	for(i=0;i<N;i++)
	{	nc[i].count = 0;
		fscanf(merged_log, "%s %s %s %s %d", name, name, dummy, dummy, &M);
//		d[i] = calloc(M, sizeof(struct dist_struct));
		strcpy(nc[i].name, name);
		max_allow += M;
printf("%s --> ", name);
		for(j=0;j<MAX_NO_OF_TEs;j++)
		{
			if(j<M)
			{	fscanf(merged_log, "%lf %lf %lf %lf %lf %lf %lf", &current.MHT_U, &current.MLT_U, &current.MHT_P, &current.MLT_P, &current.ATS, &current.SD, &current.GF);
				d[i][j].dist = distance(input, current);
				strcpy(d[i][j].name, name);
			}
			else
			{	d[i][j].dist = (double)(-1);	strcpy(d[i][j].name, "N/A");	}
printf("%lf ", d[i][j].dist);
		}
		fscanf(merged_log, "%lf %lf %lf %lf %lf %lf %lf", &current.MHT_U, &current.MLT_U, &current.MHT_P, &current.MLT_P, &current.ATS, &current.SD, &current.GF);
printf("\n");
	}

	// ----------->>>>>> Distances calculated
	// Find K minimum distances
	int act_K;
printf("\nmaxallow = %d\n", max_allow);
	if(K > max_allow)	act_K = max_allow;
	else			act_K = K;
printf("act_K = %d\n", act_K);
	struct dist_struct K_min_dists[act_K];
	double current_min = INFINITE;
	double last_min = 0;
	for(i=0;i<act_K;i++)
	{	current_min = d[0][0].dist;
		for(j=0;j<N;j++)
		{	
			for(z=0;z<MAX_NO_OF_TEs;z++)
				if(d[j][z].dist != (double)(-1) && d[j][z].dist > last_min && d[j][z].dist < current_min)
				{	current_min = d[j][z].dist;	K_min_dists[i].dist = current_min;	strcpy(K_min_dists[i].name, d[j][z].name);	}
		}

		last_min = current_min;
printf("%d ::: min = %lf ; name = %s\n", i, current_min, K_min_dists[i].name);
		for(j=0;j<N;j++)
			if(strcmp(K_min_dists[i].name, nc[j].name) == 0)
				nc[j].count++;
	}
/*
	int max = 0;
	for(i=0;i<N;i++)
		if(nc[i].count > max)	max = nc[i].count;

	for(i=0;i<N;i++)
		if(nc[i].count == max)
			fprintf(KNNret, "%s\n", nc[i].name);
*/

	for(i=0;i<act_K;i++)
		fprintf(KNNret, "%s %lf\n", K_min_dists[i].name, K_min_dists[i].dist);

	fclose(KNNret);	
}

double distance(struct __template t1, struct __template t2)
{
	double ret = 	sqrt(
			(t1.MHT_U - t2.MHT_U)*(t1.MHT_U - t2.MHT_U) +
			(t1.MLT_U - t2.MLT_U)*(t1.MLT_U - t2.MLT_U) +
			(t1.MHT_P - t2.MHT_P)*(t1.MHT_P - t2.MHT_P) +
			(t1.MLT_P - t2.MLT_P)*(t1.MLT_P - t2.MLT_P) +
			(t1.ATS - t2.ATS)*(t1.ATS - t2.ATS) +
			(t1.SD - t2.SD)*(t1.SD - t2.SD) 
			//(t1.GF - t2.GF)*(t1.GF - t2.GF)
			);
	return ret;
}	
