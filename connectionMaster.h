#ifndef _PARAMETER_H_
#define _PARAMETER_H_
#include "parameter.h"
#endif

class ConnectionMaster
{
    public:
        ConnectionMaster(Parameter * a_ptPara);
        ~ConnectionMaster();

        void run();

    private:
        Parameter * m_ptPara;
};
