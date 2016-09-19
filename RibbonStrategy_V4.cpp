//+------------------------------------------------------------------+
//|                                            RibbonStrategy_V4.mq4 |
//|                                                   Dailen Spencer |
//|                                    https://www.dailenspencer.com |
//+------------------------------------------------------------------+

//Updates - Shorts have been further improved
//Five Spread conditions have been applied. All must be met when executing order. Three must be met when exiting
//If first moving average > middle moving average when the chart flips, she short will cut out reducing losses

#property copyright "Dailen Spencer"
#property link      "https://www.dailenspencer.com"
#property version   "1.00"
#property strict

extern double  Lots              = 1;
extern int     MACount           = 5;
extern int     MagicSell         = 59789101;
extern int     MagicBuy          = 59789100;
extern int     TakeProfit        = 40;
extern int     StopLoss          = 40;
extern int     FirstMAPeriod     = 8;
extern int     SecondMAPeriod    = 15;
extern int     ThirdMAPeriod     = 20;
extern int     FourthMAPeriod    = 25;
extern int     FifthMAPeriod     = 35;
extern bool    ECNExecution      = false;
extern bool    AutoAdjustToFiveDigits = false;

double MyPoint;

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
//| Expert deinitialization function                                 |
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
  {
//---
   
  }
//+------------------------------------------------------------------+
//| Expert tick function                                             |
//+------------------------------------------------------------------+
void OnTick()
  {
//---
   static int SellTicket = 0;
   static int BuyTicket = 0;
   
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
      //Least To Greatest
      SellTicket = FindTicket(MagicSell);
      bool sellOrderSelectResult = OrderSelect(SellTicket, SELECT_BY_TICKET);
      if(sellOrderSelectResult == true)
      {
         //Sell order  is in the works
         if(OrderCloseTime() == 0)
         {
            //Check for close conditions on sell order
            CheckSellCloseConditions(MovingAverages,SellTicket);
         }
         //No sell order is in place
         else
         {
            //Execute sell order if conditions are met
            BeginSellOrder(MovingAverages, SellTicket);
         }
      }
      else
      {
         //Execute buy order if conditions are met
         Alert("Begin Sell Order");
         BeginSellOrder(MovingAverages, SellTicket);
      }
      
   }
   else 
   {  
      ArraySort(MovingAveragesSorted, WHOLE_ARRAY, 0, MODE_DESCEND);
      int descendRes = ArrayCompare(MovingAverages,MovingAveragesSorted,0,0,WHOLE_ARRAY);
      if(descendRes == 0)
      {
         /*
         //Greatest To Least
         BuyTicket = FindTicket(MagicBuy);
         bool buyOrderSelectResult = OrderSelect(BuyTicket, SELECT_BY_TICKET);
         if(buyOrderSelectResult == true)
         {
            //Buy Order is in the works 
            if(OrderCloseTime() == 0)
            {
               CheckBuyCloseConditions(MovingAverages,BuyTicket);
            }
            //No buy order is in place
            else
            {
               //Execute Buy Order if conditions are met
               BeginBuyOrder(MovingAverages,BuyTicket);
            }
         }
         else
         {
            Alert("Begin Buy Order");
            BeginBuyOrder(MovingAverages,BuyTicket);
         }
         */
      }
      
   }
   
   //Cutting out orders to reduce losses
   /*
   if(MovingAverages[0] > MovingAverages[4])
   {
      SellTicket = FindTicket(MagicSell);
      CloseOrder(SellTicket);
   }
   */
   
  }
//+------------------------------------------------------------------+

//MarketOrder - SELL
void BeginSellOrder(double &MovingAverages[5], int &ticket)
{  
  double FirstSpread = ND(MovingAverages[1] - MovingAverages[0]);
  double SecondSpread = ND(MovingAverages[2] - MovingAverages[1]);
  double ThirdSpread = ND(MovingAverages[3] - MovingAverages[2]);
  double FourthSpread = ND(MovingAverages[4] - MovingAverages[3]);
  // Alert(FirstSpread, " - ", SecondSpread, " - ", ThirdSpread, " - ", FourthSpread);
  //Alert(Bid);
  /*
  if((FirstSpread > ND(Bid*.00142363)) && (SecondSpread > ND(Bid*.00056255)) && (ThirdSpread > ND(Bid*.00040273)) && (FourthSpread > ND(Bid*.00056255)))
  {
      ticket = MarketOrderSend(Symbol(), OP_SELL, Lots, ND(Bid), 10*int(MyPoint/Point()),0,0, "Set by Ribbon Strategy V2", MagicSell);
      if(ticket < 0)
      {
         Alert("Error Sending SELL ORDER");
      }
  }
  */
  if((FirstSpread > ND(Bid*.00142363)) && (SecondSpread > ND(Bid*.00056255)) && (ThirdSpread > ND(Bid*.00040273)) && (FourthSpread > ND(Bid*.00056255)))
  {
      //Bollinger Bands
      double BottomBand = iBands(Symbol(),0,20,2.0,0,PRICE_CLOSE,MODE_LOWER,0);
      Alert(BottomBand);
      if(Bid > BottomBand)
      {
         ticket = MarketOrderSend(Symbol(), OP_SELL, Lots, ND(Bid), 10*int(MyPoint/Point()),0,0, "Set by Ribbon Strategy V2", MagicSell);
         if(ticket < 0)
         {
            Alert("Error Sending SELL ORDER");
         }
      }
  }
  
}

void BeginBuyOrder(double &MovingAverages[5], int&ticket)
{
   /*
   double FirstSpread = ND(MovingAverages[0] - MovingAverages[1]);
   double SecondSpread = ND(MovingAverages[1] - MovingAverages[2]);
   double ThirdSpread = ND(MovingAverages[2] - MovingAverages[3]);
   double FourthSpread = ND(MovingAverages[3] - MovingAverages[4]);
   
   if((FirstSpread > ND(Bid*.00142363)) && (SecondSpread > ND(Bid*.00056255)) && (ThirdSpread > ND(Bid*.00040273)) && (FourthSpread > ND(Bid*.00056255)))
  {
      Alert("Start this buy order");
      ticket = MarketOrderSend(Symbol(), OP_BUY, Lots, ND(Ask), 10*int(MyPoint/Point()),0,0, "Set by Ribbon Strategy V2", MagicSell);
      if(ticket < 0)
      {
         Alert("Error Sending SELL ORDER");
      }
  }
  */
}


//Check For Sell Close Conditions
void CheckSellCloseConditions(double &MovingAverages[5], int &ticket)
{
   double FirstSpread = ND(MovingAverages[1] - MovingAverages[0]);
   double SecondSpread = ND(MovingAverages[2] - MovingAverages[1]);
   double ThirdSpread = ND(MovingAverages[3] - MovingAverages[2]);
   double FourthSpread = ND(MovingAverages[4] - MovingAverages[3]);
   
   int count = 0;
   if(FirstSpread < ND(Bid*.00142363))
   {
      count = count + 1;
   }
   if(SecondSpread < ND(Bid* .00056255))
   {
      count = count + 1;
   }
   if(ThirdSpread < ND(Bid*.00040273))
   {
      count = count + 1;
   }
   if(FourthSpread < ND(Bid*.00056255))
   {
      count = count + 1;
   }
   
   if(count >= 3)
   {
     CloseOrder(ticket);
   }
   
}

void CheckBuyCloseConditions(double &MovingAverages[5], int &ticket)
{
   /*
   double FirstSpread = ND(MovingAverages[0] - MovingAverages[1]);
   double SecondSpread = ND(MovingAverages[1] - MovingAverages[2]);
   double ThirdSpread = ND(MovingAverages[2] - MovingAverages[3]);
   double FourthSpread = ND(MovingAverages[3] - MovingAverages[4]);
   
   int count = 0;
   if(FirstSpread < ND(Bid*.00142363))
   {
      count = count + 1;
   }
   if(SecondSpread < ND(Bid*.00056255))
   {
      count = count + 1;
   }
   if(ThirdSpread < ND(Bid*.00040273))
   {
      count = count + 1;
   }
   if(FourthSpread < ND(Bid*.00056255))
   {
      count = count + 1;
   }
   
   if(count >= 3)
   {
     CloseOrder(ticket);
   }
   */
}

void CloseOrder(int &ticket)
{
   bool OrderCloseResult = OrderClose(ticket, Lots, OrderClosePrice(), 10);
}


//Market Order
int MarketOrderSend(string symbol, int cmd, double volume, double price, int slippage, double stoploss, double takeprofit, string comment, int magic)
{
   int ticket;
   
   if(ECNExecution == false)
   {
      ticket = OrderSend(symbol, cmd, volume, price, slippage, stoploss, takeprofit, comment, magic);
      if(ticket <= 0) Alert("OrderSend Error: ", GetLastError());
      return(ticket);         
   }
   else
   {
      ticket = OrderSend(symbol, cmd, volume, price, slippage, 0, 0, comment, magic);
      if(ticket <= 0) Alert("OrderSend Error: ", GetLastError());
      else
      {
         bool res = OrderModify(ticket, 0, stoploss, takeprofit, 0);
         if(!res) {  Alert("OrderModify Error: ", GetLastError());
                     Alert("!!! ORDER #", ticket, " HAS NO STOPLOSS AND TAKEPROFIT --- ORDER MODIFY ERROR: GetLastError()");}
      }
      return(ticket);
   }
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
//+-----------------------------------------------------------------+
//| Mathmatical Calculations Functions                              |
//+-----------------------------------------------------------------+
double CalculateSpread()
{
   return(0.0);
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
