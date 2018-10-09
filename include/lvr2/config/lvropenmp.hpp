#ifndef LVROPENMP
#define LVROPENMP

namespace lvr2
{

/***
 * @brief   Config class to get save access to information about OpenMP.
 * 			Build to prevent trouble with OpenMP includes on compilers
 * 			that do not support OpenMP
 */
class OpenMPConfig
{
public:

	/// True if OpenMP is supported
	static bool haveOpenMP();

	/// Returns the number of supported threads (or 1 if OpenMP is not supported)
	static int  getNumThreads();

	/// Sets the number of used threads if OpenMP is used for parallelization
	static void setNumThreads(int n);

	/// Enables the maximum number of parallel threads
	static void setMaxNumThreads();
};

} // namespace lvr2

#include "lvropenmp.cpp"

#endif // LVROPENMP

