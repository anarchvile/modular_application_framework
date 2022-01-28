#include "plugin.h"

class ConcurrentLoading : public Plugin
{
public:
    virtual void initialize(size_t identifier) = 0;
    virtual void preupdate(double dt) = 0;
    virtual void update(double dt) = 0;
    virtual void postupdate(double dt) = 0;
    virtual void release() = 0;
};
