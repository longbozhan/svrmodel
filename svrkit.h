#include "parameter.h"

#include "processMaster.h"
#include "connectionMaster.h"

class SvrKit
{
    public:
        SvrKit();
        ~SvrKit();

        int init();
        void run();

    private:
        void processLayer();
        void connectionLayer();

        Parameter * m_ptPara;
};

