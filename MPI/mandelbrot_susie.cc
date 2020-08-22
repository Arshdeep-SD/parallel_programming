/**
 *  \file mandelbrot_susie.cc
 *
 *  \brief Implement your parallel mandelbrot set in this file.
 */

#include <iostream>
#include <cstdlib>
#include <mpi.h>

#include "render.hh"

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
	
	gil::png_write_view("mandelbrot_susie.png", const_view(img));
}

int
main (int argc, char* argv[])
{
  /* Lucky you, you get to write MPI code */
  double minX = -2.1;
  double maxX = 0.7;
  double minY = -1.25;
  double maxY = 1.25;
  
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

  double it = (maxY - minY)/height;
  double jt = (maxX - minX)/width;
  double x, y;

	int P, p;
	double t1, t2;

	
	MPI_Status status;
	MPI_Datatype block;
	
/////////////MPI/////////////

	MPI_Init(&argc, &argv);
	
	MPI_Comm_rank(MPI_COMM_WORLD, &p);
	MPI_Comm_size(MPI_COMM_WORLD, &P);
	
	printf("rank : %d\n",p); 
	printf("size : %d\n",P); 

	
	int **m_pts = (int **) malloc(height * sizeof(int *));
	for (int i=0; i<height; i++)
	{
		m_pts[i] = (int *) malloc(width * sizeof(int));
	}
	
	int **M_pts = NULL;
	if (p == 0)
	{
		M_pts = (int **) malloc(height * sizeof(int *));
		for (int i=0; i<height; i++)
		{
			M_pts[i] = (int *) malloc(width * sizeof(int));
		}
		
		t1 = MPI_Wtime();
	}
	
	y = minY + p * it;
	for (int i = 0; i < height; i += P)
	{
		x = minX;
		for (int j = 0; j < width; ++j)
		{
			m_pts[j][i] = mandelbrot(x, y);
			x += jt;
		}
		y += it * P;
	}
	
	MPI_Type_contiguous(width*height, MPI_INT, &block);
	MPI_Type_commit(&block);
	
	//MPI_Barrier(MPI_COMM_WORLD);
	

	MPI_Gather(m_pts, height*width, block, M_pts, height*width, block, 0, MPI_COMM_WORLD);
	
	if (p == 0)
	{
		render_image(M_pts, height, width);
		
		t2 = MPI_Wtime();
		printf("susie_time: %f\n", t2-t1); 
	}
	
	MPI_Finalize();
}

/* eof */
