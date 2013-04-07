// Contains auxiliary functions like template-matching, and other calculation functions
#include <stdlib.h>
#include <gtk/gtk.h>
#include <math.h>

// Defining keyboard grouping
// The number definitions vary depending how difficult the keys are to reach from one another
// NEEDS CHANGE --- TOP PRIORITY

#define TOP1	1000
#define TOP2	1008
#define TOP3	1002
#define MIDA1	1001
#define MIDA2	1010
#define MIDA3	1007
#define MIDB1	1003
#define MIDB2	1012
#define MIDB3	1004
#define BOT1	1005
#define BOT2	1006
#define NONE	9999

int distance_map(char key1, char key2);

struct __input {	// Clone of the structure 'input' in functions.h
	char type[10];
	char key;
	guint time;
	int shift;
	int caps;
} *LRIT_in;

double calculate_GF(FILE *file)		// Calculates Grouping Factor time for a particular entry,
{	// whether username, password or large text entry. NOTE: Will work on a raw input file, i.e. file containing raw keystroke data
	// NOTE:
	//	Grouping Factor(G.F.) is defined as:
	//		G.F. = (CrossGroup Distance) * (Interkey Time) / Count
	//		
	//		where 	CrossGroup Distance = 
	//			InterKey Time = 
	//			Count = 
	
	int i, j, k = line_count(file);
	char dummy[110];
	double sum = 0;
	int count = 0;

	LRIT_in = calloc(k, sizeof(struct __input));
	fscanf(file, "%s %s", dummy, dummy);
	for(i=0;i<k;i++)
		fscanf(file, "%s %c %d %d %d", LRIT_in[i].type, &LRIT_in[i].key, &LRIT_in[i].time, &LRIT_in[i].shift, &LRIT_in[i].caps);

	for(i=0;i<k;i++)
		if(strcmp(LRIT_in[i].type, "press")==0)
		{
			for(j=i+1;j<k;j++)
				if(strcmp(LRIT_in[j].type, "press")==0)
					break;
			int d = distance_map(LRIT_in[i].key, LRIT_in[j].key);
			if(d != -1) {
				sum = sum + (double)d*(LRIT_in[j].time - LRIT_in[i].time);
				count++;
			}
		}
	
	free(LRIT_in);
	return (sum/(double)count);
}

int group(char key)
{
	if(key >= '1' && key <= '4')		return TOP1;
	else if(key >= '5' && key <= '7')	return TOP2;
	else if(key == '0' || key=='8' || key=='9' || key=='_')	return TOP3;
	else if(
		key == 'q' || key == 'Q' ||
		key == 'w' || key == 'W' ||
		key == 'e' || key == 'E'
		)	return MIDA1;
	else if(
		key == 'r' || key == 'R' ||
		key == 't' || key == 'T' ||
		key == 'y' || key == 'Y'
		)	return MIDA2;
	else if(
		key == 'u' || key == 'U' ||
		key == 'i' || key == 'I' ||
		key == 'o' || key == 'O' ||
		key == 'p' || key == 'P'
		)	return MIDA3;
	else if(
		key == 'a' || key == 'A' ||
		key == 's' || key == 'S' ||
		key == 'd' || key == 'D'
		)	return MIDB1;
	else if(
		key == 'f' || key == 'F' ||
		key == 'g' || key == 'G' ||
		key == 'h' || key == 'H'
		)	return MIDB2;
	else if(
		key == 'j' || key == 'J' ||
		key == 'k' || key == 'K' ||
		key == 'l' || key == 'L'
		)	return MIDB3;
	else if(
		key == 'z' || key == 'Z' ||
		key == 'x' || key == 'X' ||
		key == 'c' || key == 'C' ||
		key == 'v' || key == 'V'
		)	return BOT1;
	else if(
		key == 'b' || key == 'B' ||
		key == 'n' || key == 'N' ||
		key == 'm' || key == 'M'
		)	return BOT2;
	else		return NONE;
}

int distance_map(char key1, char key2)
{
	int g1, g2;
	g1 = group(key1);
	g2 = group(key2);
	if(g1!=NONE && g2!=NONE)
		return((int)fabs(g1-g2));
	else	return (-1);
}

int line_count(FILE *f)
{//printf("in count\n");
	rewind(f);
	char buf[100];
	int count = 0;
	while(fgets(buf, 100, f) != NULL)
		count++;
	rewind(f);
//	printf("count = %d\n", count);
	return count;
}

double SD(FILE *file)
{
	int i, j, k = line_count(file);
	char dummy[110];
	LRIT_in = calloc(k, sizeof(struct __input));
	fscanf(file, "%s %s", dummy, dummy);
	for(i=0;i<k;i++)
		fscanf(file, "%s %c %d %d %d", LRIT_in[i].type, &LRIT_in[i].key, &LRIT_in[i].time, &LRIT_in[i].shift, &LRIT_in[i].caps);	

	// Write Code
}
