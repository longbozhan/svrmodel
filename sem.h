#pragma once
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/*! according to X/OPEN we have to define it ourselves */

/*!
 *\brief  自定义的semun，请参见man semctl. 
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
 *\brief 对系统V信号量机制的封装，用于进程间同步.
 *       这里封装的是一个信号量集合,一般情况下，
 *       使用二元信号量同步资源的使用即可.
 *       即将m_iSems设置为1.
 *
 *\as CSem  see  testiShmAndSem.cpp
 */
class CSem
{
protected:
   
	/*!
	 *\brief 信号量集合中信号量数目.
	 */
	int m_iSems;

	/*!
	 *\brief 标示信号量集合的id.
	 *
	 */
	int m_iSemId;

	/*!
	 *\brief 系统内部使用的键值，一般由ftok生成.
	 */
	key_t m_tKey;

	/*!
	 *
	 *\brief 该信号量集合是创建还是链接的.
	 *       true:创建，fasle:链接.
	 */
	bool m_bCreate;

public:

    /*!
	 *\brief 构造函数, 创建或者链接信号量集合.
	 *\param tKey key_t,键值.
	 *\param iSems int ,集合中的信号量数目，从0开始编号，默认值为1.
	 *\param iMode int ,信号量集合权限值,默认为0600.
	 *
	 */	
	CSem(key_t tKey, int iSems = 1, int iMode = 0600);
 

	/*!
	 *\brief 析构函数.
	 *
	 */
	virtual ~CSem();


    /*!
	 *\brief  创建或者链接信号量集合. 
	 *\param iMode int ,信号量集合访问掩码,默认值为0600.
	 *\exception 失败则抛出CException异常.
	 *
	 */	
	void GetSem(int iMode = 0600);
	
	/*!
	 *\brief  从系统中删除该信号量集合，这种删除是立即起效的.
	 *        一般由信号量集合的创建者负责删除.
	 *\exception 失败则抛出CException异常.
	 *
	 */
	void RemoveSem();

	/*!
	 *\brief 获取整个信号量集合，获取资源,加锁.
	 *       当资源得不到满足时，该函数将会阻塞. 
	 *
	 *\retval true:加锁成功，false:调用被信号中断/资源忙,此时加锁也算失败.
	 *
	 *\exception 失败则抛出CException异常.
	 */
	bool Lock();

	/*!
	 *
	 *\brief 释放整个信号量集合，解锁. 
	 *\exception 失败则抛出CException异常.
	 *
	 */
	void Unlock();


	/*!
	 *\brief 对信号量集合中指定数目的信号量进行操作，flag标志已经设置为SEM_UNDO.
     *	     传递进去的参数是对sembuf结构体的赋值，该结构的字段以及字段意义
	 *	     请参考man semop.个人认为这个接口没有封装的好，还不如将sembuf直接
	 *	     暴露给用户,建议重写.将接口改为:
	 *	     bool SemOpWithUndo(struct sembuf *ptr,size_t nops);
	 *\param  iSems int 需要操作的信号量数目，不应该超过集合中元素数目.
	 *\param  ...,可变参数.为(int ,int)对，代表的意义为：
	 *        (信号量编号,sem_op) sem_op的值  具体意义请参考man semop.
	 *\retval true:semop调用成功，fasle:资源暂时不可用或者调用被信号中断.
     *
	 *\exception   失败则抛出异常CException.
	 *
	 */
	bool SemOpWithUndo(int iSems, ...);

    /*!
	 *\brief 对信号量集合中指定数目的信号量进行操作，
     *	     传递进去的参数是对sembuf结构体的赋值，该结构的字段以及字段意义
	 *	     请参考man semop.个人认为这个接口没有封装的好，还不如将sembuf直接
	 *	     暴露给用户,建议重写.将接口改为:
	 *	     bool SemOp(struct sembuf *ptr,size_t nops);
	 *\param  iSems int 需要操作的信号量数目，不应该超过集合中元素数目.
	 *\param  ...,可变参数.为(int ,int,int)对，代表的意义为：
	 *        (信号量编号,信号量值,sem_flg标志值) 标志值可选为SEM_UNDO,IPC_NOWAIT.
	 *        具体意义请参考man semop.
	 *\retval true:semop调用成功，fasle:资源暂时不可用或者调用被信号中断.

     *
	 *\exception   失败则抛出异常CException.
	 *
	 */

	bool SemOp(int iSems, ...);



	/*!
	 *\brief  将整个信号量集合中的信号量值设置为用户指定的值.
	 *\param  iSems int ,集合中的信号量数目，必须等于m_iSems,即构造函数中传递进去的值，
	 *         否则抛出CException异常.
	 *\param  ...,可变参数个数，整数值，个数为iSems个.         
	 *\exception  参数不匹配时抛出异常CException.
     *
	 */
	void SetAll(int iSems, ...);


    /*!
	 *\brief 将指定编号的信号量值设置为用户指定的值.
	 *\param iSemNum int ,信号量编号,范围为[0,m_iSems-1].
	 *\param iValue  int ,用户指定的信号量值.
	 *\exception 失败则抛出CException异常.
	 *
	 */
	void SetValue(int iSemNum, int iValue);



	/*!
	 *\brief 取得信号量集合中所有信号量值.
	 *\param pusiArray unsigned short int *,存储返回值.
	 *       注意该指针指向的内存大小至少要可以存储
	 *       m_iSems个信号量的值.
	 *\exception 失败则抛出CException异常.
	 *
	 */
	void GetAll(unsigned short int* pusiArray);



	/*!
	 *\brief 返回iSemNum编号的信号量值.
	 *\param iSemNum int ,信号量编号.取值范围为[0,m_iSems-1].
	 *\return 成功时返回值为编号iSemNum的信号量值.
	 *\exception 失败则抛出CException异常.
	 */
	int GetValue(int iSemNum);


	/*!
	 *
	 *\brief 判断信号量集合是创建的还是链接的.
	 *       true:创建，false:链接.
	 *
	 */
	bool IfCreate() { return m_bCreate; }

    /*!
	 *\brief 将信号量集合中的信号量初始值全部设置为1.
	 *       表示该资源的可用数目为1.
	 *\exception  失败则抛出CException异常.
	 *
	 */
	void InitSem();
};
