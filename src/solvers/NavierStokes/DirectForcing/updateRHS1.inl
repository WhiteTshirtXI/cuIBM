#include <solvers/NavierStokes/kernels/updateRHS1.h>

template <>
void DirectForcingSolver<device_memory>::updateRHS1()
{
	int  nx = domInfo->nx,
	     ny = domInfo->ny;
	
	real dt = (*paramDB)["simulation"]["dt"].get<real>();
	
	real *rhs1_r   = thrust::raw_pointer_cast(&(rhs1[0]));
//	int  *tags_r = thrust::raw_pointer_cast(&(tagsD[0]));

	int  *tagsX_r = thrust::raw_pointer_cast(&(tagsXD[0])),
	     *tagsY_r = thrust::raw_pointer_cast(&(tagsYD[0]));

	real *dx_r = thrust::raw_pointer_cast( &(domInfo->dxD[0]) ),
	     *dy_r = thrust::raw_pointer_cast( &(domInfo->dyD[0]) ),
	     *coeffsX_r = thrust::raw_pointer_cast( &(coeffsXD[0]) ),
	     *coeffsY_r = thrust::raw_pointer_cast( &(coeffsYD[0]) ),
	     *uvX_r   = thrust::raw_pointer_cast( &(uvXD[0]) ),
	     *uvY_r   = thrust::raw_pointer_cast( &(uvYD[0]) );

	const int blocksize = 256;
	dim3 dimGrid( int( ((nx-1)*ny+nx*(ny-1)-0.5)/blocksize ) + 1, 1);
	dim3 dimBlock(blocksize, 1);
	
	// 1-d interpolation
//	kernels::updateRHS1 <<<dimGrid, dimBlock>>> (rhs1_r, numUV, tags_r);

	// 2-d interpolation
	//kernels::updateRHS1 <<<dimGrid, dimBlock>>> (rhs1_r, (nx-1)*ny+nx*(ny-1), tagsX_r, tagsY_r);
	
	kernels::updateRHS1X <<<dimGrid, dimBlock>>>(rhs1_r, nx, ny, dt, dx_r, tagsX_r, tagsY_r, coeffsX_r, coeffsY_r, uvX_r, uvY_r);
	kernels::updateRHS1Y <<<dimGrid, dimBlock>>>(rhs1_r, nx, ny, dt, dy_r, tagsX_r, tagsY_r, coeffsX_r, coeffsY_r, uvX_r, uvY_r);
}

template <>
void DirectForcingSolver<host_memory>::updateRHS1()
{
	int  nx = domInfo->nx,
	     ny = domInfo->ny;
	
	int  numU  = (nx-1)*ny,
	     I = 0;
	
	real dt = (*paramDB)["simulation"]["dt"].get<real>();
	
	for(int j=0; j<ny; j++)
	{
		for(int i=0; i<nx-1; i++)
		{
			I = j*(nx-1)+i;
			//if(tags[I]!=-1) // this is for 1-d interpolation
		
			// MODIFY THIS FOR MOVING BODIES
			
			// 2-d interpolation
			//if(tagsX[I]!=-1 || tagsY[I]!=-1)
			//	rhs1[I] = 0.0;

			rhs1[I] = (tagsX[I]==-1 && tagsY[I]==-1)*rhs1[I] + ( (tagsX[I]!=-1)*(1.0-coeffsX[I])*(1.0-coeffsY[I])*uvX[I] + (tagsY[I]!=-1)*coeffsX[I]*(1.0-coeffsY[I])*uvY[I] ) * 0.5*(domInfo->dx[i+1]+domInfo->dx[i]) / dt;
		}
	}
	
	for(int j=0; j<ny-1; j++)
	{
		for(int i=0; i<nx; i++)
		{
			I = numU + j*nx + i;
			//if(tags[i]!=-1) // this is for 1-d interpolation
		
			// MODIFY THIS FOR MOVING BODIES
			// 2-d interpolation
			//if(tagsX[i]!=-1 || tagsY[i]!=-1)
			//	rhs1[i] = 0.0;
		
			rhs1[I] = (tagsX[I]==-1 && tagsY[I]==-1)*rhs1[I] + ( (tagsX[I]!=-1)*(1.0-coeffsX[I])*(1.0-coeffsY[I])*uvX[I] + (tagsY[I]!=-1)*coeffsX[I]*(1.0-coeffsY[I])*uvY[I] ) * 0.5*(domInfo->dy[j+1]+domInfo->dy[j]) / dt;
		}
	}
}