#ifndef __TaThread__
#define __TaThread__ 

/////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <stdio.h>

#include "TThread.h" 
#include "TMutex.h" 
#include "TCondition.h" 
#include "TApplication.h"
#include "TROOT.h"
#include "TObject.h"

extern TMutex  conMutex;
extern TCondition mycondition;

using namespace std;

class TaThread{

 private:
  
  UInt_t mThreadnum;                    //number of threads in object
  TThread* fPanThread;            // thread pointer
  TThread* fRefreshThread;            // thread pointer
  // better: keep thread pointers within array; is omitted here
  // for tutorial reasons

 protected:
  Bool_t PanThreadrun; // flags for quick abort of loop within the pan thread
  Bool_t RefreshThreadrun;  // 

 public:
  
  TaThread();                     // ROOT dictionnary rule
  TaThread(UInt_t threadnum);
  virtual ~TaThread();
 
  static void Thread0(void* arg);    // functions running as threads
  static void Thread1(void* arg);    // 
    
  virtual Int_t PanThreadStart();       // launch all threads 
  virtual Int_t PanThreadStop();        // stop all threads
  virtual Int_t RefreshThreadStart();       // launch all threads 
  virtual Int_t RefreshThreadStop();        // stop all threads
 
  //  virtual void SendThreadSignal(TCondition* cond);
  virtual void DoPan();          // functions filled in derived classes
  virtual void DoRefreshing();
  virtual void EndThread();  


  Bool_t GetPanThreadRun() {return PanThreadrun;}
  Bool_t GetRefreshThreadRun() {return RefreshThreadrun;}


ClassDef(TaThread,0)

};

#endif