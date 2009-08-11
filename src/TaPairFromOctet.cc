//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaPairFromOctet.cc  (implementation)
//
// Author:  R. Holmes <http://mep1.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>, K.Paschke
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
//    Class which makes and analyzes opposite helicity event pairs
//    from a data stream structured as helicity window octets.
//    Derived from VaPair.
//
//////////////////////////////////////////////////////////////////////////

//#define NOISY

#include "VaEvent.hh"
#include "TaPairFromOctet.hh"
#include "TaRun.hh"

#ifndef NODICT
ClassImp(TaPairFromOctet)
#endif

// Static members
Int_t TaPairFromOctet::fgOctetCount;

TaPairFromOctet::TaPairFromOctet():VaPair()
{
}

TaPairFromOctet::TaPairFromOctet(const TaPairFromOctet& copy):VaPair(copy)
{
}

TaPairFromOctet &TaPairFromOctet::operator=(const TaPairFromOctet &assign)
{
  VaPair::operator=(assign);
  return *this;
}

TaPairFromOctet::~TaPairFromOctet()
{
}

ErrCode_t
TaPairFromOctet::RunInit(const TaRun& run)
{
  if (VaPair::RunInit(run) == fgVAP_ERROR)
    return fgVAP_ERROR;
  fgOctetCount = 5;
  return fgVAP_OK;
}

void TaPairFromOctet::CheckSequence( VaEvent& ThisEv, TaRun& run )
{
  // Look for sequence errors in the beam's window pair structure.
  // Errors include:
  //
  // Pairsynch unchanged from previous window
  // multipletsync == FirstMS when OtherMS expected
  // multipletsync == OtherMS when FirstMS expected
  // In first window of octet, helicity does not match expected value
  // In second window of octet, helicity unchanged from first window
  // In third window of octet, helicity changed from second window
  // In fourth window of octet, helicity unchanged from third window
  // In first or third window of octet, pairsynch is FirstPS
  // In second or fourth window of octet, pairsynch is SecondPS
  // In second and later events of window, pairsynch changed from first event
  // In second and later events of window, multipletsynch changed from first event
  // In second and later events of window, helicity changed from first event

  const Int_t EPSCHANGE  = 0x1;
  const Int_t WPSSAME    = 0x2;
  const Int_t EHELCHANGE = 0x3;
  const Int_t WHELSAME   = 0x4;
  const Int_t WHELWRONG  = 0x5;
  const Int_t WHELCHANGE = 0x6;
  const Int_t WMSFIRST   = 0x7;
  const Int_t WMSOTHER   = 0x8;
  const Int_t EMSCHANGE  = 0x9;
  const Int_t WMSPSWRONG = 0x10;

  Int_t val = 0;

  static UInt_t gLastTimeSlot;
  Bool_t newWin = ThisEv.GetTimeSlot() == 1 ||
    ThisEv.GetTimeSlot() <= gLastTimeSlot;
  gLastTimeSlot = ThisEv.GetTimeSlot();
  
  EPairSynch lps = ThisEv.GetPairSynch();
  EMultipletSynch lms = ThisEv.GetMultipletSynch();
  
  //    clog << "TaPairFromOctet::CheckSequence hel/ps/ms/ts="
  //         << " " << (ThisEv.GetHelicity() == RightHeli ? "R" : "L")
  //         << " " << (ThisEv.GetPairSynch() == FirstPS ? "F" : "S")
  //         << " " << (ThisEv.GetMultipletSynch() == FirstMS ? "F" : "O")
  //         << " " << ThisEv.GetTimeSlot()
  //         << endl;
  
  if (fgSequenceNo < fgNCuts)
    {
      if (newWin)
	{ 
	  // New window.
	  // Store event for comparison to later ones
	  fgLastWinEv = fgThisWinEv;
	  fgThisWinEv = ThisEv;
	  if (lms == FirstMS && fgOctetCount == 5)
	    fgOctetCount = 0;
	  else if (fgOctetCount < 5)
	    {
	      fgOctetCount = (fgOctetCount + 1) % 4;
	      if (lms == FirstMS && fgOctetCount != 0)
		{
		  cout << "TaPairFromOctet::CheckSequence ERROR: Event " 
		       << ThisEv.GetEvNumber() 
		       << " unexpected first window of octet" << endl;
		  val = WMSFIRST;
		  fgOctetCount = 0;
		}
	      else if (lms == OtherMS && fgOctetCount == 0)
		{
		  cout << "TaPairFromOctet::CheckSequence ERROR: Event " 
		       << ThisEv.GetEvNumber() 
		       << " unexpected non-first window of octet" << endl;
		  val = WMSOTHER;
		}
	    }
	  
	  if (fgOctetCount == 0)
	    // See if helicity is right
	    {
	      if (!HelSeqOK (ThisEv.GetHelicity()))
		{
		  cout << "TaPairFromOctet::CheckEvent ERROR: Event " 
		       << ThisEv.GetEvNumber() 
		       << " helicity sequence error" << endl;
		  val = WHELWRONG;
		}	      
	    } 
	  
	  if (((fgOctetCount == 0 || fgOctetCount == 2) && lps == SecondPS)
	      ||
	      ((fgOctetCount == 1 || fgOctetCount == 3) && lps == FirstPS))
	    // See if pairsynch is right
	    {
	      cout << "TaPairFromOctet::CheckEvent ERROR: Event " 
		   << ThisEv.GetEvNumber() 
		   << " pairsynch/multipletsynch mismatch" << endl;
	      val = WMSPSWRONG;
	    } 
	  
	  if ( fgLastWinEv.GetEvNumber() > 0 )
	    {
	      // Comparisons to last window
	      // See if pairsynch changed since last window
	      if ( lps == fgLastWinEv.GetPairSynch() )
		{
		  cout << "TaPairFromOctet::CheckSequence ERROR: Event " 
		       << ThisEv.GetEvNumber() 
		       << " pair synch unchanged" << endl;
		  val = WPSSAME;
		}
	      
	      if (fgOctetCount == 1 || fgOctetCount == 3)
		// See if helicity changed
		{
		  if ( ThisEv.GetHelicity() == fgLastWinEv.GetHelicity() )
		    {
		      cout << "TaPairFromOctet::CheckSequence ERROR: Event " 
			   << ThisEv.GetEvNumber() 
			   << " helicity unchanged from previous window" << endl;
		      val = WHELSAME;
		    }
		}
	      else if (fgOctetCount == 2)
		// See if helicity unchanged
		{
		  if ( ThisEv.GetHelicity() != fgLastWinEv.GetHelicity() )
		    {
		      cout << "TaPairFromOctet::CheckSequence ERROR: Event " 
			   << ThisEv.GetEvNumber() 
			   << " helicity changed from previous window" << endl;
		      val = WHELCHANGE;
		    }
		}
	    }
	}
      
      if ( !newWin && fgThisWinEv.GetEvNumber() != 0 )
	{
	  // Comparisons to last event
	  // See if pairsynch stayed the same
	  if ( lps != fgThisWinEv.GetPairSynch() )
	    {
	      cout << "TaPairFromOctet::CheckSequence ERROR: Event " 
		   << ThisEv.GetEvNumber()
		   << " pairsynch change in mid window\n";
	      val = EPSCHANGE;
	    }
	  // See if multipletsynch stayed the same
	  if ( lms != fgThisWinEv.GetMultipletSynch() )
	    {
	      cout << "TaPairFromOctet::CheckSequence ERROR: Event " 
		   << ThisEv.GetEvNumber()
		   << " multipletsynch change in mid window\n";
	      val = EMSCHANGE;
	    }
	  // See if helicity stayed the same
	  if ( ThisEv.GetHelicity() != fgThisWinEv.GetHelicity() )
	    {
	      cout << "TaPairFromOctet::CheckSequence ERROR: Event " 
		   << ThisEv.GetEvNumber()
		   << " helicity change in mid window\n";
	      val = EHELCHANGE;
	    }
	}
    }

#ifdef NOISY
  clog << "This Event  " << ThisEv.GetEvNumber()
       << " hel/ps/ms/ts " << (UInt_t)ThisEv.GetHelicity() 
       << " " << (UInt_t)ThisEv.GetPairSynch()
       << " " << (UInt_t)ThisEv.GetMultipletSynch()
       << " " << ThisEv.GetTimeSlot() << endl;
  clog << "This Window " << fgThisWinEv.GetEvNumber()
       << " hel/ps/ms/ts " << (UInt_t)fgThisWinEv.GetHelicity() 
       << " " << (UInt_t)fgThisWinEv.GetPairSynch()
       << " " << (UInt_t)fgThisWinEv.GetMultipletSynch()
       << " " << fgThisWinEv.GetTimeSlot() << endl;
  if(fgLastWinEv.GetEvNumber() != 0){
     clog << "Last Window " << fgLastWinEv.GetEvNumber()
          << " hel/ps/ms/ts " << (UInt_t)fgLastWinEv.GetHelicity() 
          << " " << (UInt_t)fgLastWinEv.GetPairSynch()
          << " " << (UInt_t)fgLastWinEv.GetMultipletSynch()
          << " " << fgLastWinEv.GetTimeSlot() << endl;
  }
#endif

  if (fgSequenceNo < fgNCuts)
    {
      ThisEv.AddCut (fgSequenceNo, val);
      run.UpdateCutList (fgSequenceNo, val, ThisEv.GetEvNumber());
    }
}


UInt_t 
TaPairFromOctet::RanBit (UInt_t hRead)
{
  // Pseudorandom bit predictor.  Following algorithm taken from
  // "Numerical Recipes in C" Press, Flannery, et al., 1988.  New bit
  // is returned.  This algorithm mimics the one implemented in
  // hardware in the helicity box and is used for random helicity mode
  // to set the helicity bit for the first window of each window octet.
  // Except: if the helicity bit actually read is passed as argument,
  // it is used to update the shift register, not the generated bit.

  const UInt_t IB1 = 0x1;	        // Bit 1 mask
  const UInt_t IB3 = 0x4;	        // Bit 3 mask
  const UInt_t IB4 = 0x8;	        // Bit 4 mask
  const UInt_t IB24 = 0x800000;         // Bit 24 mask
  const UInt_t MASK = IB1+IB3+IB4+IB24;	// 100000000000000000001101
 
  int hPred = (fgShreg & IB24) ? 1 : 0;

  if ((hRead == 2 ? hPred : hRead) == 1)
    fgShreg = ((fgShreg ^ MASK) << 1) | IB1;
  else
    fgShreg <<= 1;

  return hPred;
}