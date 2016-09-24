//+------------------------------------------------------------------+
//|                                            RibbonStrategy_V6.mq4 |
//|                                                   Dailen Spencer |
//|                                    https://www.dailenspencer.com |
//+------------------------------------------------------------------+

#property copyright "Dailen Spencer"
#property link      "https://www.dailenspencer.com"
#property version   "1.00"
#property strict

extern double  Lots                   = 1;
extern int     MACount                = 5;
extern int     StopLoss               = 5000;
extern int     TakeProfit             = 5000;

extern double  ShortLimit              = 750;

extern double  VolatilityLimit        = .003;
extern int     VolatilityPeriod       = 10;

extern double  ThreadSpacing          = .00056;

extern int     RSIPeriod              = 14;
extern int     RSILimit               = 14;
extern int     ADXPeriod              = 14;
extern int     ADXLimit               = 40;

extern bool    AutoAdjustToFiveDigits = true;

int     MagicSell                     = 59789101;

int     FirstShortMAPeriod            = 3;
int     SecondShortMAPeriod           = 5;
int     ThirdShortMAPeriod            = 7;
int     FourthShortMAPeriod           = 10;
int     FifthShortMAPeriod            = 12;
int     SixthShortMAPeriod            = 15;
int     FirstLongMAPeriod             = 30;
int     SecondLongMAPeriod            = 35;
int     ThirdLongMAPeriod             = 40;
int     FourthLongMAPeriod            = 45;
int     FifthLongMAPeriod             = 50;
int     SixthLongMAPeriod             = 60;
double  MyPoint;

//+------------------------------------------------------------------+
//| Expert initialization function                                   |
//+------------------------------------------------------------------+
int OnInit()
  {
//---
   SetMyPoint();
//---
   return(INIT_SUCCEEDED);
  }
//+------------------------------------------------------------------+
//| Expert tick function                                             |
//+------------------------------------------------------------------+
void OnTick()
  {
//---
   
   //Persistent Variables
   static int SellTicket = 0;
   
   static bool OrderHasBeenQualified = false;
   static double ShortPrice = 0;

   
   static double OpeningOBV;
   
   SellTicket = FindTicket(MagicSell);
   bool orderIsOpen = CheckOpenOrder(SellTicket);
   if(!orderIsOpen)
   {
      
      bool OrderQualification = QualifyOrder();
      if(OrderQualification)
      {
         OrderHasBeenQualified = true;
         Alert("Order has been quailified");
         if(ShortPrice == 0)
         {
            
            ShortPrice = ND(Bid-ShortLimit*Point);
         }
      }
      
      if(OrderHasBeenQualified)
      {
         Alert("Bid : ", ND(Bid), " | Short Price : ", ND(ShortPrice));
         if(Bid <= ShortPrice)
         {
            
            SellTicket = MarketOrderSend(Symbol(), OP_SELL, Lots, ND(Bid), 10*int(MyPoint/Point()),0,0,"Set by Ribbon Strategy V7", MagicSell);
           
            //Reset static variables
            OrderHasBeenQualified = false;
            ShortPrice = 0;
         }
      }
   }
    
   if(orderIsOpen)
   {
      CheckCloseConditions(SellTicket, OpeningOBV);
   }
   
  }




//+------------------------------------------------------------------+
//| Check Open Orders                                          |
//+------------------------------------------------------------------+
bool CheckOpenOrder(int &ticket)
{
   bool orderSelectResult = OrderSelect(ticket, SELECT_BY_TICKET);
   if(orderSelectResult == true)
   {
      //Sell order  is in the works
      if(OrderCloseTime() == 0)
      {
         //Check for close conditions on sell order
         return true;
      }
      //No sell order is in place
      else
      {
         return false;
      }
   }
   else
   {
      return false;
   }

}



//+------------------------------------------------------------------+
//| Open Conditions Checkers                                         |
//+------------------------------------------------------------------+
bool QualifyOrder()
{
   //Short Conditions
   bool SmallMovingAveragesAreInline = CheckInlineConditions(FirstShortMAPeriod, SecondShortMAPeriod, ThirdShortMAPeriod, FourthShortMAPeriod, FifthShortMAPeriod, SixthLongMAPeriod);
   bool SmallMovingAveragesAreUnder = CheckShortMAUnderLongMA();
   bool SmallMovingAveragesAreSpread = CheckSpreadForMovingAverages(FirstShortMAPeriod, SecondShortMAPeriod, ThirdShortMAPeriod, FourthShortMAPeriod, FifthShortMAPeriod, SixthShortMAPeriod);
  
   
   //Long Conditions
   bool LongMovingAveragesAreInline = CheckInlineConditions(FirstLongMAPeriod, SecondLongMAPeriod, ThirdLongMAPeriod, FourthLongMAPeriod, FifthLongMAPeriod, SixthLongMAPeriod);
   bool LongMovingAveragesAreSpread = CheckSpreadForMovingAverages(FirstLongMAPeriod, SecondLongMAPeriod, ThirdLongMAPeriod, FourthLongMAPeriod, FifthLongMAPeriod, SixthLongMAPeriod);
  
   
   bool SpacingBetweenThreads = CheckSpacingBetweenThreads();
   
   bool RSIStrong = DetermineRSI();
   bool ADXStrong = CheckADXForOpen();
   bool CurrentOBVIsLess = DetermineOBVForOpen();
   bool MACDIsLess = DetermineMACDForOpen();
   //Alert("SMA is spread : ", SmallMovingAveragesAreSpread);
   //Alert("LMA is spread : ", LongMovingAveragesAreSpread);
   //Alert("Spacing Between Threads : ", SpacingBetweenThreads);
   
   if(SmallMovingAveragesAreSpread && 
      LongMovingAveragesAreSpread  &&
      SpacingBetweenThreads)
    {
      return true;
    }
    return false;
   
}


//Verify Moving Averages Are Inline 
bool CheckInlineConditions(int FirstMAPeriod,int SecondMAPeriod, int ThirdMAPeriod, int FourthMAPeriod, int FifthMAPeriod, int SixthMAPeriod)
{
   double firstMA = iMA(Symbol(), Period(), FirstMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double secondMA = iMA(Symbol(), Period(), SecondMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double thirdMA = iMA(Symbol(), Period(), ThirdMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double fourthMA = iMA(Symbol(), Period(), FourthMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double fifthMA = iMA(Symbol(), Period(), FifthMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double sixthMA = iMA(Symbol(), Period(), SixthMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   //Make array from MA's. 
   double MovingAverages[6];
   MovingAverages[0] = ND(firstMA);
   MovingAverages[1] = ND(secondMA);
   MovingAverages[2] = ND(thirdMA);
   MovingAverages[3] = ND(fourthMA);
   MovingAverages[4] = ND(fifthMA);
   MovingAverages[5] = ND(sixthMA);
   
   //Sort array least to greatest
   double MovingAveragesSorted[6];
   MovingAveragesSorted[0] = ND(firstMA);
   MovingAveragesSorted[1] = ND(secondMA);
   MovingAveragesSorted[2] = ND(thirdMA);
   MovingAveragesSorted[3] = ND(fourthMA);
   MovingAveragesSorted[4] = ND(fifthMA);
   MovingAveragesSorted[5] = ND(sixthMA);
   
   ArraySort(MovingAveragesSorted,WHOLE_ARRAY,0,MODE_ASCEND);
   
   int ascendRes = ArrayCompare(MovingAverages,MovingAveragesSorted,0,0,WHOLE_ARRAY);
   if(ascendRes == 0)
   {
      return true;
   }
   return false;
}

//Verify Short Moving Averages Are Under Long Moving Averages
bool CheckShortMAUnderLongMA()
{
   double topShortMA = iMA(Symbol(), Period(), SixthShortMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double bottomLongMA = iMA(Symbol(), Period(), FirstLongMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   if(topShortMA < bottomLongMA)
   {
      return true;
   }
   return false;
}

//Verify Spread For Moving Averages Is Adequate
bool CheckSpreadForMovingAverages(int firstMAPeriod, int secondMAPeriod, int thirdMAPeriod, int fourthMAPeriod, int fifthMAPeriod, int sixthMAPeriod )
{
   
   double firstMA = iMA(Symbol(), Period(), firstMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double secondMA = iMA(Symbol(), Period(), secondMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double thirdMA = iMA(Symbol(), Period(), thirdMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double fourthMA = iMA(Symbol(), Period(), fourthMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double fifthMA = iMA(Symbol(), Period(), fifthMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double sixthMA = iMA(Symbol(), Period(), sixthMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);

   //Make array from MA's. 
   double MovingAverages[6];
   MovingAverages[0] = ND(firstMA);
   MovingAverages[1] = ND(secondMA);
   MovingAverages[2] = ND(thirdMA);
   MovingAverages[3] = ND(fourthMA);
   MovingAverages[4] = ND(fifthMA);
   MovingAverages[5] = ND(sixthMA);
   
   double FirstSpread = MathAbs(ND(MovingAverages[1] - MovingAverages[0]));
   double SecondSpread = MathAbs(ND(MovingAverages[2] - MovingAverages[1]));
   double ThirdSpread = MathAbs(ND(MovingAverages[3] - MovingAverages[2]));
   double FourthSpread = MathAbs(ND(MovingAverages[4] - MovingAverages[3]));
   double FifthSpread = MathAbs(ND(MovingAverages[5] - MovingAverages[4]));
  // Alert(ND(FirstSpread), " | ",ND(SecondSpread), " | ",ND(ThirdSpread), " | ",ND(FourthSpread), " | ", ND(FifthSpread));
   
   int count = 0; 
   if(FirstSpread > .000125)
   {
      return false;
   }
   if(SecondSpread > .000125)
   {
      return false;
   }
   if(ThirdSpread > .000125)
   {
      return false;
   }
   if(FourthSpread > .000125)
   {
      return false;
   }
   if(FifthSpread > .0003)
   {
      return false;
   }
   return true;
}


bool DetermineRSI()
{
   double RSI = iRSI(Symbol(),0,RSIPeriod,PRICE_CLOSE,0);
   if(RSI > RSILimit)
   {
      return true;
   }
   return false;
}

bool CheckADXForOpen()
{
   double mainADX = iADX(Symbol(),0,ADXPeriod,PRICE_CLOSE,MODE_MAIN,0);
   double dPlusADX = iADX(Symbol(),0,ADXPeriod,PRICE_CLOSE,MODE_PLUSDI,0);
   double dMinusADX = iADX(Symbol(),0,ADXPeriod,PRICE_CLOSE,MODE_MINUSDI,0);
   if(mainADX > ADXLimit && dMinusADX > dPlusADX)
   {
      return true;
   }
   return false;
}

bool CheckSpacingBetweenThreads()
{
   double firstShortMA = iMA(Symbol(), Period(), FirstShortMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double secondShortMA = iMA(Symbol(), Period(), SecondShortMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double thirdShortMA = iMA(Symbol(), Period(), ThirdShortMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double fourthShortMA = iMA(Symbol(), Period(), FourthShortMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double fifthShortMA = iMA(Symbol(), Period(), FifthShortMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double sixthShortMA = iMA(Symbol(), Period(), SixthShortMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double averageOfShortMA = (firstShortMA + secondShortMA + thirdShortMA + fourthShortMA + fifthShortMA + sixthShortMA) / 6;
   
   double firstLongMA = iMA(Symbol(), Period(), FirstLongMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double secondLongMA = iMA(Symbol(), Period(), SecondLongMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double thirdLongMA = iMA(Symbol(), Period(), ThirdLongMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double fourthLongMA = iMA(Symbol(), Period(), FourthLongMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double fifthLongMA = iMA(Symbol(), Period(), FifthLongMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double sixthLongMA = iMA(Symbol(), Period(), SixthLongMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double averageOfLongMA = (firstLongMA + secondLongMA + thirdLongMA + fourthLongMA + fifthLongMA + sixthLongMA) / 6;
   
   double spacingBetween;
   
   if(averageOfShortMA > averageOfLongMA)
   {
      spacingBetween = averageOfShortMA - averageOfLongMA;
   }
   else
   {
      spacingBetween = averageOfLongMA - averageOfShortMA;
   }
   //Alert(spacingBetween);
   if(ND(spacingBetween) > .003)
   {
      return false;
   }
   return true;
   
}

bool DetermineOBVForOpen()
{
   bool previousOBV = iOBV(Symbol(),PERIOD_H1,PRICE_CLOSE,5);
   bool currentOBV = iOBV(Symbol(), PERIOD_H1, PRICE_CLOSE, 0);
   if(currentOBV < previousOBV)
   {
      return true;
   }
   return false;
}

bool DetermineMACDForOpen()
{
   double MACD = iMACD(Symbol(),0,12,26,9,PRICE_CLOSE,MODE_SIGNAL,0);
   if(MACD < -.0025)
   {
      return true;
   }
   return false;
}
//+------------------------------------------------------------------+
//| Close Conditions Checkers                                        |
//+------------------------------------------------------------------+

void CheckCloseConditions(int &ticket, double &OBVIndicator)
{
  bool SmallMovingAveragesHaveCrossedOver = CheckCrossOver(FirstShortMAPeriod,FirstShortMAPeriod,FourthLongMAPeriod);
  bool OBVIsGreater = DetermineOBV(OBVIndicator);
  bool MACDIsGreater = DetermineMACDForClose();
  
  if(SmallMovingAveragesHaveCrossedOver && MACDIsGreater)
  {
      CloseOrder(ticket);
      return;
  }
}


bool CheckCrossOver(int FirstMAPeriod, int SecondMAPeriod, int MiddleMAPeriod)
{
   double firstShortMA = iMA(Symbol(), Period(), FirstMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double secondShortMA = iMA(Symbol(), Period(), SecondMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double middleLongMA = iMA(Symbol(), Period(), MiddleMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   if(firstShortMA > middleLongMA && secondShortMA > middleLongMA)
   {
      return true;
   }
   return false;
}

double CheckADXForClose()
{
   /*
   double mainADX = iADX(Symbol(),0,14,PRICE_CLOSE,MODE_MAIN,0);
   double dPlusADX = iADX(Symbol(),0,14,PRICE_CLOSE,MODE_PLUSDI,0);
   double dMinusADX = iADX(Symbol(),0,14,PRICE_CLOSE,MODE_MINUSDI,0);
   if(mainADX > ADXLimit && dPlusADX > dMinusADX)
   {
      return true;
   }
   return false;
   */
   return true;
}

bool CheckVolatility()
{
   double val = iStdDev(Symbol(),0,VolatilityPeriod,0,MODE_EMA,PRICE_CLOSE,0);
   Alert("Volatility : ", val);
   /*
   if(val < VolatilityLimit)
   {
      return false;
   }
   */
   return true;
}

bool DetermineOBV(double &previousOBV)
{
   double currentOBV = iOBV(Symbol(), 0, PRICE_CLOSE,1);
   if(currentOBV < previousOBV)
   {
      Alert("Current OBV is less than the previous we should reset the previous OBV to the current");
      return false;
   }
   else 
   {
      Alert("Previous OBV is less than the current, we should sell ", previousOBV, " || ", currentOBV);
      return true;
   }
  
}

bool DetermineMACDForClose()
{
   double MACD = iMACD(NULL,0,12,26,9,PRICE_CLOSE,MODE_MAIN,0);
   if(MACD > .001)
   {
      return true;
   }
   return false;
}

//+------------------------------------------------------------------+
//| Market Interaction Functions                                     |
//+------------------------------------------------------------------+
int MarketOrderSend(string symbol, int cmd, double volume, double price, int slippage, double stoploss, double takeprofit, string comment, int magic)
{
   int ticket;
   ticket = OrderSend(symbol, cmd, volume, price, slippage, stoploss, 0, comment, magic);
   if(ticket <= 0) Alert("OrderSend Error: ", GetLastError());
   return(ticket);         
}

void CloseOrder(int &ticket)
{
   bool OrderCloseResult = OrderClose(ticket, Lots, OrderClosePrice(), 20.0);
}



//+------------------------------------------------------------------+
//| Calculations                                             |
//+------------------------------------------------------------------+
double CalculateSpacing(int HigherMovingAveragePeriod, int LowerMovingAveragePeriod)
{
   double higherMA = iMA(Symbol(), Period(), HigherMovingAveragePeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double lowerMA = iMA(Symbol(), Period(), LowerMovingAveragePeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double spacing = higherMA - lowerMA;
   return spacing;
}

double AccountPercentStopPips(string symbol, double percent, double lots)
{
    double balance   = AccountBalance();
    double tickvalue = MarketInfo(symbol, MODE_TICKVALUE);
    double lotsize   = MarketInfo(symbol, MODE_LOTSIZE);
    double spread    = MarketInfo(symbol, MODE_SPREAD);

    double stopLossPips = percent * balance / (lots * lotsize * tickvalue) - spread;

    return (stopLossPips);
}






//Find Ticket
int FindTicket(int M)
{
   int ret = 0;
   for(int i = OrdersTotal()-1; i >= 0; i--)
   {
      bool res;
      res = OrderSelect(i, SELECT_BY_POS);
      if(res == true)
      {
         if(OrderMagicNumber() == M)
         {
            ret = OrderTicket();
            break;
         }
      }
   }
   return(ret);
}

//+------------------------------------------------------------------+
//| Preliminary Functions                                            |
//+------------------------------------------------------------------+
void SetMyPoint()
{
   MyPoint = Point();
   if(AutoAdjustToFiveDigits == true && (Digits() == 3 || Digits() == 5))
   {
      Alert("Digits=", Digits(), " Broker quotes given in 5-digit mode. Old values of SL, TP and slippage will be multiplied by 10");
      MyPoint = Point()*10;
   }
}

double ND(double val)
{
   return(NormalizeDouble(val, Digits()));
}