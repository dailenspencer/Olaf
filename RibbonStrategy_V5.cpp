//+------------------------------------------------------------------+
//|                                            RibbonStrategy_V4.mq4 |
//|                                                   Dailen Spencer |
//|                                    https://www.dailenspencer.com |
//+------------------------------------------------------------------+

#property copyright "Dailen Spencer"
#property link      "https://www.dailenspencer.com"
#property version   "1.00"
#property strict

extern double  Lots                   = 1;
extern int     MACount                = 5;
extern int     StopLoss               = 40;
extern int     TakeProfit             = 40;
extern double  threadSpacing          = .00056;
extern double  VolatilityLimit        = .003;
extern int     VolatilityPeriod       = 10;
extern int     RSIPeriod              = 6;
extern int     RSILimit               = 14;
extern int     ADXLimit               = 40;

extern bool    AutoAdjustToFiveDigits = true;

int MagicSell                       = 59789101;

int     FirstShortMAPeriod          = 3;
int     SecondShortMAPeriod         = 5;
int     ThirdShortMAPeriod          = 7;
int     FourthShortMAPeriod         = 10;
int     FifthShortMAPeriod          = 12;
int     SixthShortMAPeriod          = 15;
int     FirstLongMAPeriod           = 30;
int     SecondLongMAPeriod          = 35;
int     ThirdLongMAPeriod           = 40;
int     FourthLongMAPeriod          = 45;
int     FifthLongMAPeriod           = 50;
int     SixthLongMAPeriod           = 60;
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
   static int SellTicket = 0;
   SellTicket = FindTicket(MagicSell);
   bool orderIsOpen = CheckOpenOrder(SellTicket);
   if(!orderIsOpen)
   {
      
      bool shortMovingAveragesMeetConditions = CheckShortMovingAveragesConditions();
      bool longMovingAveragesMeetConditions  = CheckLongMovingAveragesConditions();
      bool rsiIsAdequate = DetermineRSI();
      
      double spacingBetween = CalculateSpacing(FirstLongMAPeriod, SixthShortMAPeriod);
      bool spacingIsAdequate = spacingBetween > .00056;
      
      if(shortMovingAveragesMeetConditions && spacingIsAdequate && rsiIsAdequate)
      {
         SellTicket = MarketOrderSend(Symbol(), OP_SELL, Lots, ND(Bid), 10*int(MyPoint/Point()),ND(Ask+StopLoss*Point()),ND(Ask-TakeProfit*Point()), "Set by Ribbon Strategy V2", MagicSell);
      }
      
   }
    
    if(orderIsOpen)
    {
      CheckCloseConditions(SellTicket);
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
bool CheckShortMovingAveragesConditions()
{
   //Changing sell to occur as soon as all moving averages are inline and under the long moving average
   bool inline = CheckInlineConditions(FirstShortMAPeriod, SecondShortMAPeriod, ThirdShortMAPeriod, FourthShortMAPeriod, FifthShortMAPeriod, SixthShortMAPeriod);
   bool spread = CheckSpreadForMovingAverages(FirstShortMAPeriod, SecondShortMAPeriod, ThirdShortMAPeriod, FourthLongMAPeriod, FifthShortMAPeriod, SixthLongMAPeriod);
   bool openADXIsAdequate = CheckADXForOpen();
   
   if(inline == true && spread == true && openADXIsAdequate == true)
   {
      return true;
   }
   return false;
}

bool CheckLongMovingAveragesConditions()
{
   bool inline = CheckInlineConditions(FirstLongMAPeriod, SecondLongMAPeriod, ThirdLongMAPeriod, FourthLongMAPeriod, FifthLongMAPeriod, SixthLongMAPeriod);
   return inline;
}

bool CheckInlineConditions(int FirstMAPeriod,int SecondMAPeriod, int ThirdMAPeriod, int FourthMAPeriod, int FifthMAPeriod, int SixthMAPeriod)
{
   double firstMA = iMA(Symbol(), Period(), FirstMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double secondMA = iMA(Symbol(), Period(), SecondMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double thirdMA = iMA(Symbol(), Period(), ThirdMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double fourthMA = iMA(Symbol(), Period(), FourthMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   double fifthMA = iMA(Symbol(), Period(), FifthMAPeriod, 0, MODE_EMA, PRICE_MEDIAN, 0);
   
   //Make array from MA's. 
   double MovingAverages[5];
   MovingAverages[0] = ND(firstMA);
   MovingAverages[1] = ND(secondMA);
   MovingAverages[2] = ND(thirdMA);
   MovingAverages[3] = ND(fourthMA);
   MovingAverages[4] = ND(fifthMA);
   
   //Sort array least to greatest
   double MovingAveragesSorted[5];
   MovingAveragesSorted[0] = ND(firstMA);
   MovingAveragesSorted[1] = ND(secondMA);
   MovingAveragesSorted[2] = ND(thirdMA);
   MovingAveragesSorted[3] = ND(fourthMA);
   MovingAveragesSorted[4] = ND(fifthMA);
   ArraySort(MovingAveragesSorted,WHOLE_ARRAY,0,MODE_ASCEND);
   
   
   
   int ascendRes = ArrayCompare(MovingAverages,MovingAveragesSorted,0,0,WHOLE_ARRAY);
   if(ascendRes == 0)
   {
      return true;
   }
   return false;
}

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
   
   double FirstSpread = ND(MovingAverages[1] - MovingAverages[0]);
   double SecondSpread = ND(MovingAverages[2] - MovingAverages[1]);
   double ThirdSpread = ND(MovingAverages[3] - MovingAverages[2]);
   double FourthSpread = ND(MovingAverages[4] - MovingAverages[3]);
   double FifthSpread = ND(MovingAverages[5] - MovingAverages[4]);
   
   
   int count = 0; 
   if(FirstSpread > .00119)
   {
      count = count + 1;
   }
   if(SecondSpread > .00082)
   {
      count = count + 1;
   }
   if(ThirdSpread > .00079)
   {
      count = count + 1;
   }
   if(FourthSpread > .00036)
   {
      count = count + 1;
   }
   if(FifthSpread > .0004)
   {
      count = count + 1;
   }
   if(count >= 3)
   {
      return true;
   }
   //return false;
   //Returning true for testing purposee
   return false;
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
   /*
   double mainADX = iADX(Symbol(),0,14,PRICE_CLOSE,MODE_MAIN,0);
   double dPlusADX = iADX(Symbol(),0,14,PRICE_CLOSE,MODE_PLUSDI,0);
   double dMinusADX = iADX(Symbol(),0,14,PRICE_CLOSE,MODE_MINUSDI,0);
   if(mainADX > ADXLimit && dMinusADX > dPlusADX)
   {
      return true;
   }
   return false;
   */
   return true;
}
//+------------------------------------------------------------------+
//| Close Conditions Checkers                                        |
//+------------------------------------------------------------------+

void CheckCloseConditions(int &ticket)
{
  double SpacingBetween = CalculateSpacing(FirstLongMAPeriod,SixthShortMAPeriod);
  bool SmallMovingAveragesAreInline = CheckInlineConditions(FirstShortMAPeriod, SecondShortMAPeriod, ThirdShortMAPeriod, FourthShortMAPeriod, FifthLongMAPeriod, SixthShortMAPeriod);
  bool SmallMovingAveragesSpread = CheckSpreadForMovingAverages(FirstShortMAPeriod, SecondShortMAPeriod, ThirdShortMAPeriod, FourthShortMAPeriod, FifthShortMAPeriod, SixthShortMAPeriod);
  bool LongMovingAveragesAreInline = CheckInlineConditions(FirstLongMAPeriod, SecondLongMAPeriod, ThirdLongMAPeriod, FourthLongMAPeriod, FifthLongMAPeriod, SixthLongMAPeriod);
  bool hasCrossedOver = CheckCrossOver(FirstShortMAPeriod,SecondShortMAPeriod,FourthLongMAPeriod);
  bool ADXIsAdequate = CheckADXForClose();
  
  //Initial close conditions
  
  if(!SmallMovingAveragesAreInline && hasCrossedOver && ADXIsAdequate)
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