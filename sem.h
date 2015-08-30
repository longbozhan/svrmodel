#pragma once
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/*! according to X/OPEN we have to define it ourselves */

/*!
 *\brief  �Զ����semun����μ�man semctl. 
 */	
union semun {
  int val;                    /*!< value for SETVAL */
  struct semid_ds *buf;       /*!< buffer for IPC_STAT, IPC_SET */
  unsigned short int *array;  /*!< array for GETALL, SETALL */
  struct seminfo *__buf;      /*!< buffer for IPC_INFO */
};
#endif


/*!
 *
 *\brief ��ϵͳV�ź������Ƶķ�װ�����ڽ��̼�ͬ��.
 *       �����װ����һ���ź�������,һ������£�
 *       ʹ�ö�Ԫ�ź���ͬ����Դ��ʹ�ü���.
 *       ����m_iSems����Ϊ1.
 *
 *\as CSem  see  testiShmAndSem.cpp
 */
class CSem
{
protected:
   
	/*!
	 *\brief �ź����������ź�����Ŀ.
	 */
	int m_iSems;

	/*!
	 *\brief ��ʾ�ź������ϵ�id.
	 *
	 */
	int m_iSemId;

	/*!
	 *\brief ϵͳ�ڲ�ʹ�õļ�ֵ��һ����ftok����.
	 */
	key_t m_tKey;

	/*!
	 *
	 *\brief ���ź��������Ǵ����������ӵ�.
	 *       true:������fasle:����.
	 */
	bool m_bCreate;

public:

    /*!
	 *\brief ���캯��, �������������ź�������.
	 *\param tKey key_t,��ֵ.
	 *\param iSems int ,�����е��ź�����Ŀ����0��ʼ��ţ�Ĭ��ֵΪ1.
	 *\param iMode int ,�ź�������Ȩ��ֵ,Ĭ��Ϊ0600.
	 *
	 */	
	CSem(key_t tKey, int iSems = 1, int iMode = 0600);
 

	/*!
	 *\brief ��������.
	 *
	 */
	virtual ~CSem();


    /*!
	 *\brief  �������������ź�������. 
	 *\param iMode int ,�ź������Ϸ�������,Ĭ��ֵΪ0600.
	 *\exception ʧ�����׳�CException�쳣.
	 *
	 */	
	void GetSem(int iMode = 0600);
	
	/*!
	 *\brief  ��ϵͳ��ɾ�����ź������ϣ�����ɾ����������Ч��.
	 *        һ�����ź������ϵĴ����߸���ɾ��.
	 *\exception ʧ�����׳�CException�쳣.
	 *
	 */
	void RemoveSem();

	/*!
	 *\brief ��ȡ�����ź������ϣ���ȡ��Դ,����.
	 *       ����Դ�ò�������ʱ���ú�����������. 
	 *
	 *\retval true:�����ɹ���false:���ñ��ź��ж�/��Դæ,��ʱ����Ҳ��ʧ��.
	 *
	 *\exception ʧ�����׳�CException�쳣.
	 */
	bool Lock();

	/*!
	 *
	 *\brief �ͷ������ź������ϣ�����. 
	 *\exception ʧ�����׳�CException�쳣.
	 *
	 */
	void Unlock();


	/*!
	 *\brief ���ź���������ָ����Ŀ���ź������в�����flag��־�Ѿ�����ΪSEM_UNDO.
     *	     ���ݽ�ȥ�Ĳ����Ƕ�sembuf�ṹ��ĸ�ֵ���ýṹ���ֶ��Լ��ֶ�����
	 *	     ��ο�man semop.������Ϊ����ӿ�û�з�װ�ĺã������罫sembufֱ��
	 *	     ��¶���û�,������д.���ӿڸ�Ϊ:
	 *	     bool SemOpWithUndo(struct sembuf *ptr,size_t nops);
	 *\param  iSems int ��Ҫ�������ź�����Ŀ����Ӧ�ó���������Ԫ����Ŀ.
	 *\param  ...,�ɱ����.Ϊ(int ,int)�ԣ����������Ϊ��
	 *        (�ź������,sem_op) sem_op��ֵ  ����������ο�man semop.
	 *\retval true:semop���óɹ���fasle:��Դ��ʱ�����û��ߵ��ñ��ź��ж�.
     *
	 *\exception   ʧ�����׳��쳣CException.
	 *
	 */
	bool SemOpWithUndo(int iSems, ...);

    /*!
	 *\brief ���ź���������ָ����Ŀ���ź������в�����
     *	     ���ݽ�ȥ�Ĳ����Ƕ�sembuf�ṹ��ĸ�ֵ���ýṹ���ֶ��Լ��ֶ�����
	 *	     ��ο�man semop.������Ϊ����ӿ�û�з�װ�ĺã������罫sembufֱ��
	 *	     ��¶���û�,������д.���ӿڸ�Ϊ:
	 *	     bool SemOp(struct sembuf *ptr,size_t nops);
	 *\param  iSems int ��Ҫ�������ź�����Ŀ����Ӧ�ó���������Ԫ����Ŀ.
	 *\param  ...,�ɱ����.Ϊ(int ,int,int)�ԣ����������Ϊ��
	 *        (�ź������,�ź���ֵ,sem_flg��־ֵ) ��־ֵ��ѡΪSEM_UNDO,IPC_NOWAIT.
	 *        ����������ο�man semop.
	 *\retval true:semop���óɹ���fasle:��Դ��ʱ�����û��ߵ��ñ��ź��ж�.

     *
	 *\exception   ʧ�����׳��쳣CException.
	 *
	 */

	bool SemOp(int iSems, ...);



	/*!
	 *\brief  �������ź��������е��ź���ֵ����Ϊ�û�ָ����ֵ.
	 *\param  iSems int ,�����е��ź�����Ŀ���������m_iSems,�����캯���д��ݽ�ȥ��ֵ��
	 *         �����׳�CException�쳣.
	 *\param  ...,�ɱ��������������ֵ������ΪiSems��.         
	 *\exception  ������ƥ��ʱ�׳��쳣CException.
     *
	 */
	void SetAll(int iSems, ...);


    /*!
	 *\brief ��ָ����ŵ��ź���ֵ����Ϊ�û�ָ����ֵ.
	 *\param iSemNum int ,�ź������,��ΧΪ[0,m_iSems-1].
	 *\param iValue  int ,�û�ָ�����ź���ֵ.
	 *\exception ʧ�����׳�CException�쳣.
	 *
	 */
	void SetValue(int iSemNum, int iValue);



	/*!
	 *\brief ȡ���ź��������������ź���ֵ.
	 *\param pusiArray unsigned short int *,�洢����ֵ.
	 *       ע���ָ��ָ����ڴ��С����Ҫ���Դ洢
	 *       m_iSems���ź�����ֵ.
	 *\exception ʧ�����׳�CException�쳣.
	 *
	 */
	void GetAll(unsigned short int* pusiArray);



	/*!
	 *\brief ����iSemNum��ŵ��ź���ֵ.
	 *\param iSemNum int ,�ź������.ȡֵ��ΧΪ[0,m_iSems-1].
	 *\return �ɹ�ʱ����ֵΪ���iSemNum���ź���ֵ.
	 *\exception ʧ�����׳�CException�쳣.
	 */
	int GetValue(int iSemNum);


	/*!
	 *
	 *\brief �ж��ź��������Ǵ����Ļ������ӵ�.
	 *       true:������false:����.
	 *
	 */
	bool IfCreate() { return m_bCreate; }

    /*!
	 *\brief ���ź��������е��ź�����ʼֵȫ������Ϊ1.
	 *       ��ʾ����Դ�Ŀ�����ĿΪ1.
	 *\exception  ʧ�����׳�CException�쳣.
	 *
	 */
	void InitSem();
};
