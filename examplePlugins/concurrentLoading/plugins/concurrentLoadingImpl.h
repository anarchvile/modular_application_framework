#include "concurrentLoading.h"

#ifdef _WIN32
    #ifdef CONCURRENTLOADING_EXPORTS
        #define CONCURRENTLOADING __declspec(dllexport)
    #else
        #define CONCURRENTLOADING __declspec(dllimport)
    #endif
#elif __linux__
    #define CONCURRENTLOADING
#endif

class ConcurrentLoadingImpl : public ConcurrentLoading
{
public:
    void initialize(size_t identifier);
    void release();

    void start();
    void stop();

    void preupdate(double dt);
    void update(double dt);
    void postupdate(double dt);
};
