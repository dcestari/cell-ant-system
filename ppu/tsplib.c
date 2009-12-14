#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "tsplib.h"

#define KEYWORD_BUFFER            100
#define NAME_BUFFER               100
#define TYPE_BUFFER               10
#define COMMENT_BUFFER            512
#define EDGE_WEIGHT_TYPE_BUFFER   20
#define EDGE_WEIGHT_FORMAT_BUFFER 20
#define EDGE_DATA_FORMAT_BUFFER   20
#define NODE_COORD_TYPE_BUFFER    20
#define DISPLAY_DATA_TYPE_BUFFER  20

int ** generate_distance_matrix(int n, float * x_coords, float * y_coords, float * z_coords)
{
  int i, j, aligned_n;
  int ** distances;
  float x_delta, y_delta, z_delta;

  aligned_n = ceil(n / 4.0) * 4;
  printf("nearest 4 multiple of %d is %d\n", n, aligned_n);
  
  if (posix_memalign(&distances, 128, aligned_n * sizeof(int *)) != 0)
  {
    fprintf(stderr, "allocating memory for distances matrix");
    exit(EXIT_FAILURE);
  }

  /* initializing distances matrix */
  for (i = 0; i < n; i++)
  {
    if (posix_memalign(&distances[i], 128, aligned_n * sizeof(int)) != 0)
    {
      fprintf(stderr, "allocating memory for distances matrix");
      exit(EXIT_FAILURE);
    }

    for (j = 0; j < i; j++)
    {
      x_delta = x_coords[i] - x_coords[j];
      y_delta = y_coords[i] - y_coords[j];

      if (z_coords == NULL)
      {
        z_delta = 0.0;
      }
      else
      {
        z_delta = z_coords[i] - z_coords[j];
      }
      
      distances[i][j] = floor(sqrtf(powf(x_delta, 2) + powf(y_delta, 2) + powf(z_delta, 2)));
      distances[j][i] = distances[i][j];
    }
  }
  
  return distances;
}

int readString(FILE * fp, char * name)
{
  return fscanf(fp, "%[^\n]\n", name);
}

int readInt(FILE * fp, int * value)
{
  return fscanf(fp, "%d\n", value);
}

int parse_tsp_file(char * filename, int *** distance_matrix)
{
  char keyword[KEYWORD_BUFFER];
  char name[NAME_BUFFER];
  char type[TYPE_BUFFER];
  char comment[COMMENT_BUFFER];
  char edge_weigth_type[EDGE_WEIGHT_TYPE_BUFFER];
  char edge_weigth_format[EDGE_WEIGHT_FORMAT_BUFFER];
  char edge_data_format[EDGE_DATA_FORMAT_BUFFER];
  char node_coord_type[NODE_COORD_TYPE_BUFFER];
  char display_data_type[DISPLAY_DATA_TYPE_BUFFER];
  int dimension;
  float * node_coord_x;
  float * node_coord_y;
  float * node_coord_z;
  float tmp_x, tmp_y, tmp_z;
  int i, j;
  
  FILE * fp;
  
  /* initialize */
  memset(keyword, '\0', KEYWORD_BUFFER);
  memset(name, '\0', NAME_BUFFER);
  memset(type, '\0', TYPE_BUFFER);
  memset(comment, '\0', COMMENT_BUFFER);
  memset(edge_weigth_type, '\0', EDGE_WEIGHT_TYPE_BUFFER);
  memset(edge_weigth_format, '\0', EDGE_WEIGHT_FORMAT_BUFFER);
  memset(edge_data_format, '\0', EDGE_DATA_FORMAT_BUFFER);
  memset(node_coord_type, '\0', NODE_COORD_TYPE_BUFFER);
  memset(display_data_type, '\0', DISPLAY_DATA_TYPE_BUFFER);
  dimension = 0;
  node_coord_x = NULL;
  node_coord_y = NULL;
  node_coord_z = NULL;

  if ((fp = fopen(filename, "r")) == NULL)
  {
    perror("Error while opening tsp file");
    exit(EXIT_FAILURE);
  }

  while (!feof(fp) && strcmp(keyword, "EOF") != 0)
  {
    fscanf(fp, "%[^ :\n]%*2[: \n]", keyword);
/*    printf("keyword: *%s*\n", keyword);*/

    if (strcmp(keyword, "NAME") == 0)
    {
      readString(fp, name);
    }
    else if (strcmp(keyword, "TYPE") == 0)
    {
      readString(fp, type);
    }
    else if (strcmp(keyword, "COMMENT") == 0)
    {
      readString(fp, comment);
    }
    else if (strcmp(keyword, "DIMENSION") == 0)
    {
      readInt(fp, &dimension);
    }
    else if (strcmp(keyword, "EDGE_WEIGHT_TYPE") == 0)
    {
      readString(fp, edge_weigth_type);
    }
    else if (strcmp(edge_weigth_type, "EXPLICITY") == 0 && strcmp(keyword, "EDGE_WEIGHT_FORMAT") == 0)
    {
      readString(fp, edge_weigth_type);
    }
    else if (strcmp(keyword, "EDGE_DATA_FORMAT") == 0)
    {
      readString(fp, edge_data_format);
    }
    else if (strcmp(keyword, "NODE_COORD_TYPE") == 0)
    {
      readString(fp, node_coord_type);
    }
    else if (strcmp(keyword, "DISPLAY_DATA_TYPE") == 0)
    {
      readString(fp, display_data_type);
    }
    else if (strcmp(keyword, "NODE_COORD_SECTION") == 0)
    {
      if (strcmp(node_coord_type, "TWOD_COORDS") == 0)
      {
        if ((node_coord_x = malloc(dimension * sizeof(double))) == NULL)
        {
          perror("allocating memory for node_coord_x");
          exit(EXIT_FAILURE);
        }
        if ((node_coord_y = malloc(dimension * sizeof(double))) == NULL)
        {
          perror("allocating memory for node_coord_y");
          exit(EXIT_FAILURE);
        }
        
        for (i = 0; i < dimension; i++)
        {
          fscanf(fp, "%d %f %f\n", &j, &tmp_x, &tmp_y);

          node_coord_x[i] = tmp_x;
          node_coord_y[i] = tmp_y;
        }
      }
      else if (strcmp(node_coord_type, "THREED_COORDS") == 0)
      {
        if ((node_coord_x = malloc(dimension * sizeof(double))) == NULL)
        {
          perror("allocating memory for node_coord_x");
          exit(EXIT_FAILURE);
        }
        if ((node_coord_y = malloc(dimension * sizeof(double))) == NULL)
        {
          perror("allocating memory for node_coord_y");
          exit(EXIT_FAILURE);
        }
        if ((node_coord_z = malloc(dimension * sizeof(double))) == NULL)
        {
          perror("allocating memory for node_coord_z");
          exit(EXIT_FAILURE);
        }
        
        for (i = 0; i < dimension; i++)
        {
          fscanf(fp, "%d %f %f %f\n", &j, &tmp_x, &tmp_y, &tmp_z);

          node_coord_x[i] = tmp_x;
          node_coord_y[i] = tmp_y;
          node_coord_z[i] = tmp_z;
        }
      }
    }
  }
  
  /* set defaults */
  if (strlen(node_coord_type) == 0)
  {
    strcpy(node_coord_type, "NO_COORDS");
  }
  
  if (strlen(display_data_type) == 0)
  {
    if (strcmp(node_coord_type, "NO_COORDS") == 0)
    {
      strcpy(display_data_type, "NO_DISPLAY");
    }
    else
    {
      strcpy(display_data_type, "COORD_DISPLAY");
    }
  }

/*  printf("NAME : %s\n", name);
  printf("TYPE : %s\n", type);
  printf("COMMENT : %s\n", comment);
  printf("DIMENSION : %d\n", dimension);
  printf("EDGE_WEIGHT_TYPE : %s\n", edge_weigth_type);
  
  if (strcmp(edge_weigth_type, "EXPLICITY") == 0)
  {
    printf("EDGE_WEIGHT_FORMAT : %s\n", edge_weigth_format);
  }
  
  printf("EDGE_DATA_FORMAT : %s\n", edge_data_format);
  printf("NODE_COORD_TYPE : %s\n", node_coord_type);
  printf("DISPLAY_DATA_TYPE : %s\n", display_data_type);
*/  

  if (fclose(fp) != 0)
  {
    perror("Error while closing tsp file");
    exit(EXIT_FAILURE);
  }
  
  *distance_matrix = generate_distance_matrix(dimension, node_coord_x, node_coord_y, node_coord_z);
  
  return dimension;
}


