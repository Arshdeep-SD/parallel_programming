/**
 *  \file mandelbrot_ms.cc
 *
 *  \brief Implement your parallel mandelbrot set in this file.
 */

#include <iostream>
#include <cstdlib>
#include <mpi.h>

#include "render.hh"

const int DO = 1;
const int DIE = 2;

int
mandelbrot(double x, double y) {
  int maxit = 511;
  double cx = x;
  double cy = y;
  double newx, newy;

  int it = 0;
  for (it = 0; it < maxit && (x*x + y*y) < 4; ++it) {
    newx = x*x - y*y + cx;
    newy = 2*x*y + cy;
    x = newx;
    y = newy;
  }
  return it;
}

void 
render_image(int **image, int height, int width)
{
	int i, j;
	
	gil::rgb8_image_t img(height, width);
	auto img_view = gil::view(img);
	
	for (i=0; i<height; i++)
	{
		for (j=0; j<width; j++)
		{
			img_view(j, i) = render(image[j][i]/512.0);
		}
	}
	
	gil::png_write_view("mandelbrot.png", const_view(img));
}

void
add_row(int in_row, int *row, int **image, int width)
{
	for (int i=0; i<width; i++)
	{
		image[in_row][i] = row[i];
	}
}

void master(int height, int width)
{
	int P, row, in_row;
	int *recv_row = (int *) malloc(width * sizeof(int));
	MPI_Comm_size(MPI_COMM_WORLD, &P);
	MPI_Status status;
	
	int **image = (int **) malloc(height * sizeof(int *));
	for (int i=0; i<height; i++)
	{
		image[i] = (int *) malloc(width * sizeof(int));
	}
	
	row = 0;
	in_row = 0;
	
	for (int i=1; i<P; i++)
	{
		MPI_Send(&row,
				1,
				MPI_INT, 
				i, 
				DO, 
				MPI_COMM_WORLD);
		row++;
	}
	
	while (row != height)
	{
		MPI_Recv(recv_row, 
				width, 
				MPI_INT, 
				MPI_ANY_SOURCE, 
				MPI_ANY_TAG, 
				MPI_COMM_WORLD, 
				&status);
				
		add_row(in_row, recv_row, image, width);
		in_row++;
		
		MPI_Send(&row,
				1,
				MPI_INT, 
				status.MPI_SOURCE, 
				DO, 
				MPI_COMM_WORLD);
		row++;
	}
	
	for (int i=1; i<P; i++)
	{
		MPI_Recv(recv_row, 
				width, 
				MPI_INT, 
				MPI_ANY_SOURCE, 
				MPI_ANY_TAG, 
				MPI_COMM_WORLD, 
				&status);
		
		add_row(in_row, recv_row, image, width);
		in_row++;
	}
	
	for (int i=1; i<P; i++)
	{
		MPI_Send(0,
				0,
				MPI_INT, 
				i, 
				DIE, 
				MPI_COMM_WORLD);
		row++;
	}
	
	render_image(image, height, width);
}

void slave(int height, int width)
{
	double minX = -2.1;
	double maxX = 0.7;
	double minY = -1.25;
	double maxY = 1.25;
	
	double it = (maxY - minY)/height;
	double jt = (maxX - minX)/width;
	double x, y;
	
	MPI_Status status;
	
	int P, p, pos;
	
	int *send_row = (int *) malloc(width * sizeof(int));

	while(1)
	{
		MPI_Recv(&pos, 
				1, 
				MPI_INT, 
				0, 
				MPI_ANY_TAG,
				MPI_COMM_WORLD,
				&status);
		
		if (status.MPI_TAG == DIE)
		{
			return;
		}
		
		y = minY + pos * jt;
		x = minX;
		for (int i=0; i<width; i++)
		{
			send_row[i] = mandelbrot(x, y);
			x += it;
		}
		
		MPI_Send(send_row, width, MPI_INT, 0, 0, MPI_COMM_WORLD);

	}
}

int
main (int argc, char* argv[])
{
  /* Lucky you, you get to write MPI code */

  int height, width;
  if (argc == 3) {
    height = atoi (argv[1]);
    width = atoi (argv[2]);
    assert (height > 0 && width > 0);
  } else {
    fprintf (stderr, "usage: %s <height> <width>\n", argv[0]);
    fprintf (stderr, "where <height> and <width> are the dimensions of the image.\n");
    return -1;
  }
  
	int rank;
	
/////////////MPI/////////////

	MPI_Init(&argc, &argv);
	
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	if (rank == 0)
	{
		master(height, width);
		printf("master \n");
	}
	else
	{
		slave(height, width);
		printf("master \n");
	}
	
	MPI_Finalize();

}

/* eof */
