#ifndef _PARAMETER_H_
#define _PARAMETER_H_
#include "parameter.h"
#endif

#include <map>

using namespace std;

class ProcessMaster
{
    public:
        ProcessMaster(Parameter *);
        ~ProcessMaster();
        
        void run();
        void forkChildRun(int);
        void childRun();
    
    private:
        Parameter * m_ptPara;
        map<int, int> m_mapPidInfo;
};
