# pragma once

# include "KaminoQuantity.h"

class KaminoSolver
{
private:
	// Handle for batched FFT
	cufftHandle kaminoPlan;

	// Buffer for U, the fouriered coefs
	// This pointer's for the pooled global memory (nTheta by nPhi)
	ComplexFourier* gpuUFourier;
	fReal* gpuUReal;
	fReal* gpuUImag;

	// Buffer for V, the fouriered coefs
	// This pointer's for the pooled global memory as well
	ComplexFourier* gpuFFourier;
	fReal* gpuFReal;
	fReal* gpuFImag;

	/// Precompute these!
	// nPhi by nTheta elements, but they should be retrieved by shared memories
	// in the TDM kernel we solve nTheta times with each time nPhi elements.
	fReal* gpuA;
	// Diagonal elements b (major diagonal);
	fReal* gpuB;
	// Diagonal elements c (upper);
	fReal* gpuC;
	void precomputeABCCoef();

	/* Grid dimensions */
	size_t nPhi;
	size_t nTheta;
	/* Radius of sphere */
	fReal radius;
	/* Grid size */
	fReal gridLen;
	/* Inverted grid size*/
	fReal invGridLen;

	/* harmonic coefficients for velocity field initializaton */
	fReal A;
	int B, C, D, E;

	/* So that it remembers all these attributes within */
	//std::map<std::string, KaminoQuantity*> centeredAttr;
	//std::map<std::string, KaminoQuantity*> staggeredAttr;
	
	KaminoQuantity* velTheta;
	KaminoQuantity* velPhi;
	KaminoQuantity* pressure;
	void copyVelocity2GPU();
	void copyVelocityBack2CPU();

	/* Something about time steps */
	fReal frameDuration;
	fReal timeStep;
	fReal timeElapsed;

	/// Kernel calling from here
	void advection();
	void geometric();
	void projection();
	void bodyForce();

	// Swap all these buffers of the attributes.
	void swapAttrBuffers();

	/* distribute initial velocity values at grid points */
	void initialize_velocity();
	/* which grids are solid? */
	void initialize_boundary();

	/* sum of sine functions for velocity initialization */
	fReal fPhi(const fReal x);
	/* */
	fReal gTheta(const fReal y);
	/* */
	fReal lPhi(const fReal x);
	/* */
	fReal mTheta(const fReal y);

	/* Tri-diagonal matrix solver */
	void TDMSolve(fReal* a, fReal* b, fReal* c, fReal* d);

	void mapPToSphere(vec3& pos) const;
	void mapVToSphere(vec3& pos, vec3& vel) const;
	/* Convert to texture */
	void setTextureParams(table2D* tex);
public:
	
	KaminoSolver(size_t nPhi, size_t nTheta, fReal radius, fReal frameDuration,
		fReal A, int B, int C, int D, int E);
	~KaminoSolver();

	void stepForward(fReal timeStep);

	void write_data_bgeo(const std::string& s, const int frame);
};

__device__ fReal kaminoLerp(fReal from, fReal to, fReal alpha);

__device__ fReal sampleAt
(fReal* attribute, fReal phi, fReal theta,
	fReal phiOffset, fReal thetaOffset, fReal gridLen,
	size_t nTheta, size_t nPhi, size_t nPitch);