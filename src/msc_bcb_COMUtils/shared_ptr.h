////////////////////////////////////////////////////////////////////////////////
//
// Shared_ptr.h -  Shared smart pointer class
//
// Copyright (c) 2002, Elite Information Systems
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __SHARED_PTR_H__
#define __SHARED_PTR_H__

#include <windows.h>

namespace shared_ptr
{

class RefCounterBase
{
public:
  RefCounterBase() : m_lRefCount(1) {}
  long AddRef();
  long Release();

protected:
  long m_lRefCount;
};


//thread unsafe refcounting class
class UnsafeRefCounter : public RefCounterBase
{
public:
  inline long AddRef()
  {
    return ++m_lRefCount;
  }

  inline long Release()
  {
    return --m_lRefCount;
  }
};

//thread safe refcounting class
class SafeRefCounter : public RefCounterBase
{
public:
  inline long AddRef()
  {
    return ::InterlockedIncrement(&m_lRefCount);
  }

  inline long Release()
  {
    return ::InterlockedDecrement(&m_lRefCount);
  }
};


//shared smart pointer template
//RefCounter should be derived from RefCounterBase!
template<class T, class RefCounter = UnsafeRefCounter>
class shared_ptr
{
public:
  //default constructor
  shared_ptr(): m_pT(NULL), m_pRefCounter(NULL)
  {
  }

  //constructor from class pointer
  shared_ptr(T* pT) : m_pT(pT)
  {
    if (NULL != pT)
      m_pRefCounter = new RefCounter;
  }

  //copy constructor from other smart pointer of the same type
  shared_ptr(const shared_ptr<T, RefCounter>& ptr) : m_pT(ptr.m_pT), m_pRefCounter(ptr.m_pRefCounter)
  {
    if (NULL != m_pRefCounter)
      m_pRefCounter->AddRef();
  }
  //operator= from class pointer
  const shared_ptr<T, RefCounter>& operator=(T* pT)
  {
    if(m_pT != pT)
    {
      if (NULL != m_pT)
      {
        if (0 == m_pRefCounter->Release())
        {
          delete m_pT;
          if (NULL == pT)
          {
            delete m_pRefCounter;
            m_pRefCounter = NULL;
          }
          else
            m_pRefCounter->AddRef();
        }
        else
        {
          if (NULL != pT)
            m_pRefCounter = new RefCounter;
          else
            m_pRefCounter = NULL;
        }
      }
      else
        m_pRefCounter = new RefCounter;
      m_pT = pT;
    }
    return *this;
  }
  //operator= from other smart pointer
  const shared_ptr<T, RefCounter>& operator=(const shared_ptr<T, RefCounter>& ptr)
  {
    if (m_pT != ptr.m_pT)
    {
      this->shared_ptr<T, RefCounter>::~shared_ptr();
      m_pT = ptr.m_pT;
      m_pRefCounter = ptr.m_pRefCounter;
      if (NULL != m_pRefCounter)
        m_pRefCounter->AddRef();
    }
    return *this;
  }
  //destructor
  ~shared_ptr()
  {
    if (NULL != m_pT)
    {
      if (0 == m_pRefCounter->Release())
      {
        delete m_pT;
        delete m_pRefCounter;
      }
    }
  }
  //operator*
  inline T& operator*() const
  {
    return *m_pT;
  }
  //operator->
  inline T* operator->() const
  {
    return m_pT;
  }
  //Attaches pointer to the shared_ptr
  inline const shared_ptr<T, RefCounter>& Attach(T* pT)
  {
    return this->operator=(pT);
  }
  //Detaches (removes one reference) pointer from shared_ptr
  inline T* Detach()
  {
    T* pT = m_pT;
    this->operator=(NULL);
    return pT;
  }

protected:
  T* m_pT;
  RefCounter* m_pRefCounter;
};
}

#endif
