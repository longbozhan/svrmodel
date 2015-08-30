#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "sem.h"

CSem::CSem(key_t tKey, int iSems, int iMode)
				: m_iSems(iSems), m_tKey(tKey)
{
	GetSem(iMode);
    printf("create(%d)\n", m_bCreate);
    printf("ERR: semget( %x, %d, %x ) %d, errno %d, %s\n",
					m_tKey, m_iSems, IPC_CREAT | IPC_EXCL | iMode,
					m_iSemId, errno, strerror( errno ) );
	
}


CSem::~CSem()
{
}


void CSem::GetSem(int iMode)
{
	if ((m_iSemId = semget(m_tKey, m_iSems, IPC_CREAT | IPC_EXCL | iMode)) < 0)
	{
		if (errno != EEXIST) {
			printf("ERR: semget( %x, %d, %x ) %d, errno %d, %s",
					m_tKey, m_iSems, IPC_CREAT | IPC_EXCL | iMode,
					m_iSemId, errno, strerror( errno ) );
			printf("err(%s) file(%s), line(%d)",strerror(errno), __FILE__, __LINE__);
            return;
		}

		if ((m_iSemId = semget(m_tKey, 0, iMode)) < 0) {
			printf("ERR: semget( %x, %d, %x ) %d, errno %d, %s",
					m_tKey, 0, IPC_CREAT | IPC_EXCL | iMode,
					m_iSemId, errno, strerror( errno ) );
			printf("err(%s) file(%s), line(%d)",strerror(errno), __FILE__, __LINE__);
            return;
		}
		
		m_bCreate = false;
		return;
	}

	m_bCreate = true;
}


void CSem::RemoveSem()
{
	union semun unSem;
	memset(&unSem, 0, sizeof(unSem));
	if (semctl(m_iSemId, 0, IPC_RMID, unSem) < 0)
    {
		printf("err(%s) file(%s), line(%d)",strerror(errno), __FILE__, __LINE__);
        return;
    }
}


bool CSem::SemOpWithUndo(int iSems, ...)
{
	struct sembuf* pstSemBuf = new struct sembuf [iSems];
	
	va_list ap;
	va_start(ap, iSems);
	for (int i = 0; i < iSems; i++)
	{
		pstSemBuf[i].sem_num = (unsigned short int)va_arg(ap, int);
		pstSemBuf[i].sem_op = (short int)va_arg(ap, int);
		pstSemBuf[i].sem_flg = SEM_UNDO;
	}
	va_end(ap);
	
	if (semop(m_iSemId, pstSemBuf, iSems) < 0)
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			delete [] pstSemBuf;
			return false;
		}
			
		delete [] pstSemBuf;
		printf("err(%s) file(%s), line(%d)",strerror(errno), __FILE__, __LINE__);
        return false;
	}
	
	delete [] pstSemBuf;
	return true;
}


bool CSem::SemOp(int iSems, ...)
{
	struct sembuf* pstSemBuf = new struct sembuf [iSems];
	
	va_list ap;
	va_start(ap, iSems);
	for (int i = 0; i < iSems; i++)
	{
		pstSemBuf[i].sem_num = (unsigned short int)va_arg(ap, int);
		pstSemBuf[i].sem_op = (short int)va_arg(ap, int);
		pstSemBuf[i].sem_flg = (short int)va_arg(ap, int);
	}
	va_end(ap);
	
	if (semop(m_iSemId, pstSemBuf, iSems) < 0)
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			delete [] pstSemBuf;
			return false;
		}
		
		delete [] pstSemBuf;
		printf("err(%s) file(%s), line(%d)",strerror(errno), __FILE__, __LINE__);
        return false;
	}
	
	delete [] pstSemBuf;
	return true;
}


bool CSem::Lock()
{
	struct sembuf* pstSemBuf = new struct sembuf [m_iSems];
	
	for (int i = 0; i < m_iSems; i++)
	{
		pstSemBuf[i].sem_num = i;
		pstSemBuf[i].sem_op = -1;
		pstSemBuf[i].sem_flg = SEM_UNDO;
	}
	
	if (semop(m_iSemId, pstSemBuf, m_iSems) < 0)
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			delete [] pstSemBuf;
			return false;
		}
		
		delete [] pstSemBuf;
		printf("err(%s) file(%s), line(%d)",strerror(errno), __FILE__, __LINE__);	
        return false;
	}
	
	delete [] pstSemBuf;
	return true;
}


void CSem::Unlock()
{
	struct sembuf* pstSemBuf = new struct sembuf [m_iSems];
	
	for (int i = 0; i < m_iSems; i++)
	{
		pstSemBuf[i].sem_num = i;
		pstSemBuf[i].sem_op = 1;
		pstSemBuf[i].sem_flg = SEM_UNDO;
	}
	
	if (semop(m_iSemId, pstSemBuf, m_iSems) < 0)
	{
		delete [] pstSemBuf;
		printf("err(%s) file(%s), line(%d)",strerror(errno), __FILE__, __LINE__);	
        return ;
	}
	
	delete [] pstSemBuf;
}


void CSem::SetAll(int iSems, ...)
{
	if (iSems != m_iSems)
    {
		printf("err(%s) file(%s), line(%d)","argument error.", __FILE__, __LINE__);
        return;
    }
	
	unsigned short int* pusiValue = new unsigned short int [iSems];
	
	va_list ap;
	va_start(ap, iSems);
	for (int i = 0; i < iSems; i++)
		pusiValue[i] = (unsigned short int)va_arg(ap, int);
	va_end(ap);
	
	semun unSem;
	unSem.array = pusiValue;
	if (semctl(m_iSemId, 0, SETALL, unSem) < 0)
	{
		delete [] pusiValue;
		printf("err(%s) file(%s), line(%d)",strerror(errno), __FILE__, __LINE__);
        return;
	}
	
	delete [] pusiValue;
}


void CSem::SetValue(int iSemNum, int iValue)
{
	semun unSem;
	unSem.val = iValue;
	if (semctl(m_iSemId, iSemNum, SETVAL, unSem) < 0)
    {
		printf("err(%s) file(%s), line(%d)",strerror(errno), __FILE__, __LINE__);
        return;
    }
}


void CSem::GetAll(unsigned short int* pusiArray)
{
	semun unSem;
	unSem.array = pusiArray;
	if (semctl(m_iSemId, 0, GETALL, unSem) < 0)
    {
        printf("err(%s) file(%s), line(%d)",strerror(errno), __FILE__, __LINE__);	
        return;
    }
}


int CSem::GetValue(int iSemNum)
{
	semun unSem;
	memset(&unSem, 0, sizeof(unSem));
	int iValue = semctl(m_iSemId, iSemNum, GETVAL, unSem);
	if (iValue < 0)
    {
        printf("err(%s) file(%s), line(%d)",strerror(errno), __FILE__, __LINE__);	
        return -1;
    }
	
	return iValue;
}


void CSem::InitSem()
{
	unsigned short int* pusiValue = new unsigned short int [m_iSems];
	
	for (int i = 0; i < m_iSems; i++)
		pusiValue[i] = (unsigned short int)1;

	semun unSem;
	unSem.array = pusiValue;
	if (semctl(m_iSemId, 0, SETALL, unSem) < 0)
	{
		delete [] pusiValue;
		printf("err(%s) file(%s), line(%d)",strerror(errno), __FILE__, __LINE__);
        return ;
	}
	
	delete [] pusiValue;
}
