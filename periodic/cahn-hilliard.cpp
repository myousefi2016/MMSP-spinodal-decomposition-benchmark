// cahn-hilliard.hpp
// Algorithms for 2D and 3D Cahn-Hilliard model
// Questions/comments to trevor.keller@gmail.com (Trevor Keller)

#ifndef CAHNHILLIARD_UPDATE
#define CAHNHILLIARD_UPDATE
#include"MMSP.hpp"
#include<cmath>
#include"cahn-hilliard.hpp"
#include"../energy.hpp"

namespace MMSP {

template <int dim,typename T>
double Helmholtz(const grid<dim,T>& GRID)
{
	double dV = 1.0;
	for (int d=0; d<dim; d++)
		dV *= dx(GRID, d);

	double f = 0.0;
	double g = 0.0;

	for (int n=0; n<nodes(GRID); n++) {
		vector<int> x = position(GRID, n);
		vector<double> gradc = gradient(GRID, x);
		f += chemenergy(GRID(x));
		g += gradc*gradc;
	}

	double F = dV*(f + 0.5*kappa*g);
	#ifdef MPI_VERSION
	double myF(F);
        MPI::COMM_WORLD.Barrier();
	MPI::COMM_WORLD.Allreduce(&myF, &F, 1, MPI_DOUBLE, MPI_SUM);
	#endif
	return F;
}


void generate(int dim, const char* filename)
{
	int rank=0;
	#ifdef MPI_VERSION
	rank = MPI::COMM_WORLD.Get_rank();
	#endif

	if (dim!=2 && rank==0) {
		std::cerr<<"ERROR: CHiMaD problems are 2-D, only!"<<std::endl;
		std::exit(-1);
	}

	if (dim==2) {
		GRID2D initGrid(1,0,200,0,200);
		for (int d=0; d<dim; d++)
			dx(initGrid,d) = deltaX;

		for (int n=0; n<nodes(initGrid); n++) {
			vector<int> x = position(initGrid, n);
			initGrid(x) = cheminit(dx(initGrid,0)*x[0], dx(initGrid,1)*x[1]);
		}

		ghostswap(initGrid);
		output(initGrid,filename);

	}
}

template <int dim, typename T>
void update(grid<dim,T>& oldGrid, int steps)
{
	ghostswap(oldGrid);
	grid<dim,T> newGrid(oldGrid);
	grid<dim,T> lapGrid(oldGrid);

	for (int step=0; step<steps; step++) {
		for (int n=0; n<nodes(oldGrid); n++) {
			vector<int> x = position(oldGrid, n);
			lapGrid(x) = dfdc(oldGrid(x)) - kappa*laplacian(oldGrid, x);
		}

		ghostswap(lapGrid);

		for (int n=0; n<nodes(oldGrid); n++) {
			vector<int> x = position(oldGrid, n);
			newGrid(x) = oldGrid(x) + dt*M*laplacian(lapGrid, x);
		}

		swap(oldGrid,newGrid);
		ghostswap(oldGrid);
	}
}

} // MMSP
#endif

#include"../main.cpp"
