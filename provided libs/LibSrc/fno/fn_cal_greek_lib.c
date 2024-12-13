/********************************************************************************/
/*  Library						: FN_CAL_GREEK_LIB																				*/
/*                                                                              */
/*  Description       : This library calculates all greeks delta,theta,gamma,		*/
/*											vega, implied volatility and Premium price							*/
/*                                                                              */
/*  Log               : 1.0   22-Sep-2010  Shailesh Hinge												*/
/********************************************************************************/
/*  1.0    -  New Release                                                       */
/********************************************************************************/

#include<fn_cal_greek_lib.h>
#include <stdio.h>
#include <string.h>
#include <fo.h>
#include <userlog.h>
#include <fn_log.h>
#include <fn_read_debug_lvl.h>

/* The cumulative normal distribution function */
double NC( char* c_ServiceName,double X )
{

  double L, K, w ;

  double const a1 = 0.31938153, a2 = -0.356563782, a3 = 1.781477937;
  double const a4 = -1.821255978, a5 = 1.330274429;

  if (X < -7)
  {
    w = NDF(c_ServiceName,X)/(1+(2*X*X)+pow(X,4));
  }
  else if ( X > 7)
  {
    w = 1 - NC(c_ServiceName,-1*X);
  }
  else
  {
    L = fabs(X);
    K = 1.0 / (1.0 + 0.2316419 * L);
    w = 1.0 - NDF(c_ServiceName,L) * (a1 * K + a2 * K *K + a3 * pow(K,3) + a4 * pow(K,4) + a5 * pow(K,5));

    if (X <= 0 )
    {
      w = 1.0 - w;
    }
  }

if(DEBUG_MSG_LVL_3){
	fn_userlog(c_ServiceName,"Function NC:w:%lf:",w);
}
  return w;
}

double NDF( char* c_ServiceName,double X)
{
	double n=0.00;
  n = ((1.0 / sqrt(2 * PI))*(exp(-X * X * 0.5)));

if(DEBUG_MSG_LVL_3){
	fn_userlog(c_ServiceName,"Function NDF:n:%lf:",n);
}
	return n;
}

/* The Implied volatility calculation function */
double eurpput( char* c_ServiceName,double spot,
							 double strike,
							 double maturity,
							 double vol,
							 double interest 
               )
{
double d1,d2;
double e;
maturity = maturity / 365;

  d1=(log((spot)/strike)+(interest+vol*vol/2)*maturity)/(vol*sqrt(maturity));
if(DEBUG_MSG_LVL_3){
	fn_userlog(c_ServiceName,"Function eurput:d1:%lf:",d1);
}

	d2 = d1 - (vol * sqrt(maturity));
if(DEBUG_MSG_LVL_3){
	fn_userlog(c_ServiceName,"Function eurput:d2:%lf:",d2);
}

	e = (strike * exp(-interest * maturity) *  NC(c_ServiceName,-d2) - spot *  NC(c_ServiceName,-d1));
if(DEBUG_MSG_LVL_3){
	fn_userlog(c_ServiceName,"Function eurput:e:%lf:",e);
}

	return e;
}

double vol_eput(char* c_ServiceName,double spot, 
								double strike, 
								double maturity, 
								double interest, 
								double mktprice)
{
double	leastcount = 0.005;
double	uppervol = 2.5;	
double	lowervol = 0.01;
double	vol= 0.00;
double midvol=0.00,midprice=0.00;
int	iteration = 0;
		
			do 
			{
				if((uppervol - lowervol) > leastcount) 
				{
					midvol = (uppervol + lowervol) / 2;
					midprice = eurpput(c_ServiceName,spot, strike, maturity, midvol, interest);

					if (midprice > mktprice) 
					{
			 			uppervol = midvol;
					}
					else
					{
			    	lowervol = midvol;
					}
   				iteration = iteration + 1;
				}
   			else
				{
					break;
				}
			}while (iteration < 60);

   		if(iteration < 60) 
			{
   			vol = (uppervol + lowervol) / 2;
				if(DEBUG_MSG_LVL_3){
					fn_userlog(c_ServiceName,"Function eurput iteration less than 60:%d:",iteration);
				}
			}

if(DEBUG_MSG_LVL_3){
	fn_userlog(c_ServiceName,"Function eurput:vol:%lf:",vol);
}

			return vol;
}

double delta_eurpput(char* c_ServiceName,double spot, 
										 double strike, 
										 double maturity, 
									   double vol, 
										 double interest)
{
double price0 = 0.00;
double price1 = 0.00;
double price2 = 0.00;
double spot2= 0.00;
double spot0= 0.00;
double delta = 0.0;
double delta0 = 0.0;
double delta2 = 0.0;

		price1 = eurpput(c_ServiceName,spot, strike, maturity, vol, interest);
		spot2 = 1.005 * spot;
		spot0 = 0.995 * spot;
		price2 = eurpput(c_ServiceName,spot2, strike, maturity, vol, interest);
		price0 = eurpput(c_ServiceName,spot0, strike, maturity, vol, interest);

		delta0 = (price0 - price1) / (spot0 - spot);
		delta2 = (price2 - price1) / (spot2 - spot);

		delta = (delta0 + delta2) / 2;

if(DEBUG_MSG_LVL_3){
	fn_userlog(c_ServiceName,"Function delta_eurpput:price0:%lf:price1:%lf:price2:%lf:",price0,price1,price2);
	fn_userlog(c_ServiceName,"Function delta_eurpput:spot0:%lf:spot2:%lf:",spot0,spot2);
	fn_userlog(c_ServiceName,"Function delta_eurpput:delta0:%lf:delta2:%lf:delta:%lf:",delta0,delta2,delta);
}
		return delta;
}

double vega_eurpput(char* c_ServiceName,double spot, 
										double strike, 
										double maturity, 
										double vol, 
										double interest)
{

double price1 = 0.00;
double vol2= 0.00;
double vol0= 0.00;
double vega = 0.00;
double price2=0.00,price0=0.00,vega0=0.00,vega2=0.00;

	price1 = eurpput(c_ServiceName,spot, strike, maturity, vol, interest);

	vol2 = 1.01 * vol;
	vol0 = 0.99 * vol;

	price2 = eurpput(c_ServiceName,spot, strike, maturity, vol2, interest);
	price0 = eurpput(c_ServiceName,spot, strike, maturity, vol0, interest);

	vega0 = (price0 - price1) / (vol0 - vol);
	vega2 = (price2 - price1) / (vol2 - vol);

	vega = (vega0 + vega2) / 200;

if(DEBUG_MSG_LVL_3){
	fn_userlog(c_ServiceName,"Function vega_eurpput:price0:%lf:price1:%lf:price2:%lf:",price0,price1,price2);
	fn_userlog(c_ServiceName,"Function vega_eurpput:vol0:%lf:vol2:%lf:",vol0,vol2);
	fn_userlog(c_ServiceName,"Function veag_eurpput:vega0:%lf:vega2:%lf:vega:%lf:",vega0,vega2,vega);
}

	return vega;
}


double gamma_eurpput(char* c_ServiceName,double spot, 
										 double strike, 
										 double maturity, 
										 double vol, 
									   double interest)
{
double d1 = 0.00;
double nd1 = 0.00;
double gamma = 0.00;

    d1=(log(spot/strike) + (interest+ vol * vol/2) * (maturity/365)) / (vol*(sqrt(maturity/365)));
		nd1 = exp(-d1 * d1 / 2) / (sqrt(2 * 3.14));

		gamma = nd1 / (spot * vol * (sqrt(maturity / 365)));

if(DEBUG_MSG_LVL_3){
	fn_userlog(c_ServiceName,"Function gamma_eurpput:d1:%lf:nd1:%lf:",d1,nd1);
	fn_userlog(c_ServiceName,"Function gamma_eurpput:gamma:%lf:",gamma);
}

		return gamma;
}

double theta_eurpput(char* c_ServiceName,double spot, 
										 double strike, 
										 double maturity, 
										 double vol, 
										 double interest)
{

double sqrmat = 0.00;
double d1= 0.00;
double nd11= 0.00;
double d2= 0.00;
double nd2= 0.00;
double theta= 0.00;
double theta_eurpput1 = 0.00;
double sqrtmat=0.00;

	sqrtmat = (sqrt(maturity / 365));
	d1 = (log(spot / strike) + (interest + vol * vol / 2) * (maturity / 365)) / (vol * sqrtmat);
	nd11 = exp(-d1 * d1 / 2) /(sqrt(2 * 3.14));
	d2 = d1 - vol * sqrtmat;
	nd2 = NC(c_ServiceName,-d2);
	theta = -(spot * nd11 * vol/ (2 * sqrtmat)) + interest *strike *exp(-interest *maturity/365)* nd2;
	theta_eurpput1 = theta / 365;

if(DEBUG_MSG_LVL_3){
	fn_userlog(c_ServiceName,"Function theta_eurpput:sqrtmat:%lf:d1:%lf:nd11:%lf:",sqrtmat,d1,nd11);
	fn_userlog(c_ServiceName,"Function theta_eurpput:d2:%lf:nd2:",d2,nd2);
	fn_userlog(c_ServiceName,"Function theta_eurpput:theta:%lf:",theta_eurpput1);
}

	return theta_eurpput1;
}
/* The Implied volatility calculation function */
double eurpcall( char* c_ServiceName,double spot,
							 double strike,
							 double maturity,
							 double vol,
							 double interest 
               )
{
double d1=0.00,d2=0.00;
double e=0.00;
double maturity1=0.00;
double temp=0.00;

	maturity1 = maturity / 365;
	temp = spot/strike;

  d1=(log(temp)+(interest+vol*vol/2)*maturity1)/(vol*sqrt(maturity1));
	d2 = d1 - (vol * sqrt(maturity1));

	e = exp(-interest * maturity1) * (spot * NC(c_ServiceName,d1) * exp( interest * maturity1) - strike* NC(c_ServiceName,d2));

if(DEBUG_MSG_LVL_3){
	fn_userlog(c_ServiceName,"Function eurpcall:maturity1:%lf:d1:%lf:d2:%lf:",maturity1,d1,d2);
	fn_userlog(c_ServiceName,"Function eurpcall:e:%lf:",e);
}
	return e;
}

double vol_ecall(char* c_ServiceName,double spot, 
								double strike, 
								double maturity, 
								double interest, 
								double mktprice)
{
double	leastcount = 0.005;
double	uppervol = 2.5;	
double	lowervol = 0.01;
double	vol= 0.00;
double midvol=0.00,midprice=0.00;
int	iteration = 0;
		
			do 
			{
				if((uppervol - lowervol) > leastcount) 
				{
					midvol = (uppervol + lowervol) / 2;
					midprice = eurpcall(c_ServiceName,spot, strike, maturity, midvol, interest);

					if (midprice > mktprice) 
					{
			 			uppervol = midvol;
					}
					else
					{
			    	lowervol = midvol;
					}
   				iteration = iteration + 1;
				}
   			else
				{
					break;
				}
			}while (iteration < 60);

   		if(iteration < 60) 
			{
   			vol = (uppervol + lowervol) / 2;
        if(DEBUG_MSG_LVL_3){
          fn_userlog(c_ServiceName,"Function eurput iteration less than 60:%d:",iteration);
        }
			}

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function eurput:vol:%lf:",vol);
}

			return vol;
}

double delta_eurpcall(char* c_ServiceName,double spot, 
										 double strike, 
										 double maturity, 
									   double vol, 
										 double interest)
{
double price0 = 0.00;
double price1 = 0.00;
double price2 = 0.00;
double spot2= 0.00;
double spot0= 0.00;
double delta = 0.0;
double delta0 = 0.0;
double delta2 = 0.0;

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function delta_eurpcall:spot:%lf:strike:%lf:",spot,strike);
  fn_userlog(c_ServiceName,"Function delta_eurpcall:maturity:%lf:vol:%lf:",maturity,vol);
  fn_userlog(c_ServiceName,"Function delta_eurpcall:interest:%lf:",interest);
}
		price1 = eurpcall(c_ServiceName,spot, strike, maturity, vol, interest);
		spot2 = 1.005 * spot;
		spot0 = 0.995 * spot;
		price2 = eurpcall(c_ServiceName,spot2, strike, maturity, vol, interest);
		price0 = eurpcall(c_ServiceName,spot0, strike, maturity, vol, interest);

		delta0 = (price0 - price1) / (spot0 - spot);
		delta2 = (price2 - price1) / (spot2 - spot);

		delta = (delta0 + delta2) / 2;

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function delta_eurpcall:price0:%lf:price1:%lf:price2:%lf:",price0,price1,price2);
  fn_userlog(c_ServiceName,"Function delta_eurpcall:spot0:%lf:spot2:%lf:",spot0,spot2);
  fn_userlog(c_ServiceName,"Function delta_eurpcall:delta0:%lf:delta2:%lf:delta:%lf:",delta0,delta2,delta);
}

		return delta;
}

double vega_eurpcall(char* c_ServiceName,double spot, 
										double strike, 
										double maturity, 
										double vol, 
										double interest)
{

double price1 = 0.00;
double vol2= 0.00;
double vol0= 0.00;
double vega = 0.00;
double price2=0.00,price0=0.00,vega0=0.00,vega2=0.00;

	price1 = eurpcall(c_ServiceName,spot, strike, maturity, vol, interest);

	vol2 = 1.01 * vol;
	vol0 = 0.99 * vol;

	price2 = eurpcall(c_ServiceName,spot, strike, maturity, vol2, interest);
	price0 = eurpcall(c_ServiceName,spot, strike, maturity, vol0, interest);

	vega0 = (price0 - price1) / (vol0 - vol);
	vega2 = (price2 - price1) / (vol2 - vol);

	vega = (vega0 + vega2) / 200;

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function vega_eurpcall:price0:%lf:price1:%lf:price2:%lf:",price0,price1,price2);
  fn_userlog(c_ServiceName,"Function vega_eurpcall:vol0:%lf:vol2:%lf:",vol0,vol2);
  fn_userlog(c_ServiceName,"Function veag_eurpcall:vega0:%lf:vega2:%lf:vega:%lf:",vega0,vega2,vega);
}

	return vega;
}


double gamma_eurpcall(char* c_ServiceName,double spot, 
										 double strike, 
										 double maturity, 
										 double vol, 
									   double interest)
{
double d1 = 0.00;
double nd1 = 0.00;
double gamma = 0.00;

    d1=(log(spot/strike) + (interest+ vol * vol/2) * (maturity/365)) / (vol*(sqrt(maturity/365)));
		nd1 = exp(-d1 * d1 / 2) / (sqrt(2 * 3.14));

		gamma = nd1 / (spot * vol * (sqrt(maturity / 365)));

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function gamma_eurpcall:d1:%lf:nd1:%lf:",d1,nd1);
  fn_userlog(c_ServiceName,"Function gamma_eurpcall:gamma:%lf:",gamma);
}

		return gamma;
}

double theta_eurpcall(char* c_ServiceName,double spot, 
										 double strike, 
										 double maturity, 
										 double vol, 
										 double interest)
{

double sqrtmat = 0.00;
double d1= 0.00;
double nd11= 0.00;
double d2= 0.00;
double nd2= 0.00;
double theta= 0.00;
double theta_eurpcall1 = 0.00;

	sqrtmat = (sqrt(maturity / 365));
	d1 = (log(spot / strike) + (interest + vol * vol / 2) * (maturity / 365)) / (vol * sqrtmat);
	nd11 = exp(-d1 * d1 / 2) /(sqrt(2 * 3.14));
	d2 = d1 - vol * sqrtmat;
	nd2 = NC(c_ServiceName,d2);
	theta = -(spot * nd11 * vol/ (2 * sqrtmat)) - interest *strike *exp(-interest *maturity/365)* nd2;
	theta_eurpcall1 = theta / 365;

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function theta_eurpcall:sqrtmat:%lf:d1:%lf:nd11:%lf:",sqrtmat,d1,nd11);
  fn_userlog(c_ServiceName,"Function theta_eurpcall:d2:%lf:nd2:",d2,nd2);
  fn_userlog(c_ServiceName,"Function theta_eurpcall:theta:%lf:",theta_eurpcall1);
}

	return theta_eurpcall1;
}
/* The Implied volatility calculation function */
double americancall( char* c_ServiceName,double spot,
										 double strike,
										 double maturity,
										 double vol,
										 double interest,
										 double dividend 
									 )
{
double h=0.00,alpha=0.00,beta=0.00,gamma1=0.00,gamma2=0.00,Nd1=0.00,Nd2=0.00;
double initialspot = spot;
double spotnext = 0.5 * spot;
double spotcritical =0.00;
double expression1 = 0.00, derivative = 0.00,callprice=0.00;
double d1crit=0.00,d2crit=0.00,Nd1crit=0.00,Nd2crit=0.00,A2=0.00,d1=0.00,d2=0.00,delta=0.00;
double gamma=0.00;
double americancall1=0.00;

	h = 1 - exp(-interest * maturity / 365);
	alpha = 2 * interest / (vol * vol); 
	beta = 2 * (interest - dividend) / (vol * vol); 
	gamma1 = -0.5 * ((beta - 1) + sqrt(fabs(((beta - 1) * (beta - 1)) + 4 * alpha / h)));
	gamma2 = -0.5 * ((beta - 1) - sqrt(fabs(((beta - 1) * (beta - 1)) + 4 * alpha / h)));

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function americancall:h:%lf:alpha:%lf:beta:%lf:gamma1:%lf:gamma2:%lf:",h,alpha,beta,gamma1,gamma2);
}
	while(fabs((spot - spotnext) / spot) > 0.0025)
	{
		 spotnext = spot;
		 d1 = (log(spot / strike) + (interest - dividend + 0.5 * vol * vol) * maturity / 365) / (vol * sqrt(fabs(maturity / 365)));
		 d2 = (log(spot / strike) + (interest - dividend - 0.5 * vol * vol) * maturity / 365) / (vol * sqrt(fabs(maturity / 365)));
				
		 Nd1 = NC(c_ServiceName,d1);
		 Nd2 = NC(c_ServiceName,d2);
				
		 callprice = spot * exp(-dividend * maturity / 365) * Nd1 - strike * exp(-interest * maturity / 365) * Nd2;
		 delta = exp(-dividend * maturity / 365) * Nd1;
		 gamma = (exp(-dividend * maturity / 365) * exp(-0.5 * d1 * d1)) / (sqrt(2*3.14)*(spot * vol * sqrt(fabs(maturity / 365))));

		 expression1 = spot - strike - callprice - (1 - exp(-dividend * maturity / 365) * Nd1) * spot / gamma2;
		 derivative = (1 - 1 / gamma2) - delta * (1 - 1 / gamma2) + spot * gamma / gamma2;

		 spot = spot - (expression1 / derivative)	;

	}

	spotcritical = spot;

	if (initialspot < spotcritical) 
	{
	   d1crit = (log(spotcritical / strike) + (interest - dividend + 0.5 * vol * vol) * maturity / 365) / (vol * sqrt(fabs(maturity / 365)));
	   d2crit = (log(spotcritical / strike) + (interest - dividend - 0.5 * vol * vol) * maturity / 365) / (vol * sqrt(fabs(maturity / 365)));

	    	Nd1crit =NC(c_ServiceName,d1crit);
	    	Nd2crit =NC(c_ServiceName,d2crit);

	    	A2 = (spotcritical / gamma2) * (1 - exp(-dividend * maturity / 365) * Nd1crit);
		    
	    	d1 = (log(initialspot / strike) + (interest - dividend + 0.5 * vol * vol) * maturity / 365) / (vol * sqrt(fabs(maturity / 365)));
	    	d2 = (log(initialspot / strike) + (interest - dividend - 0.5 * vol * vol) * maturity / 365) / (vol * sqrt(fabs(maturity / 365)));

	    	Nd1 = NC(c_ServiceName,d1);
	    	Nd2 = NC(c_ServiceName,d2);

	    	callprice = initialspot * exp(-dividend * maturity / 365) * Nd1 - strike * exp(-interest * maturity / 365) * Nd2;

	    	americancall1 = callprice + A2 * pow((initialspot / spotcritical) , gamma2);
	}
	else
	{
	    	americancall1 = initialspot - strike;
	}

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function americancall:americancall1:%lf:",americancall1);
}
		return americancall1;

}


double vol_acall(char* c_ServiceName,double spot, 
								double strike, 
								double maturity, 
								double interest, 
								double dividend, 
								double mktprice)
{
double	leastcount = 0.005;
double	uppervol = 2.5;	
double	lowervol = 0.01;
double	vol = 0.00;
double midvol=0.00,midprice=0.00;
int	iteration = 0;
		
			do 
			{
				if((uppervol - lowervol) > leastcount) 
				{
					midvol = (uppervol + lowervol) / 2;
					midprice = americancall(c_ServiceName,spot, strike, maturity, midvol, interest,dividend);

					if (midprice > mktprice) 
					{
			 			uppervol = midvol;
					}
					else
					{
			    	lowervol = midvol;
					}
   				iteration = iteration + 1;
				}
   			else
				{
					break;
				}
			}while (iteration < 60);

   		if(iteration < 60) 
			{
   			vol = (uppervol + lowervol) / 2;

      	if(DEBUG_MSG_LVL_3){
          fn_userlog(c_ServiceName,"Function vol_acall iteration less than 60:%d:",iteration);
        }
			}

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function vol_acall:vol_amcall:%lf:",vol);
}

			return vol;
}

double delta_americancall(char* c_ServiceName,double spot, 
													double strike, 
													double interest, 
													double maturity, 
													double vol, 
													double dividend)
{

double initialspot = spot;
double spotnext = 0.5 * spot;
double h =0.00,alpha=0.00,beta=0.00,gamma1=0.00,gamma2=0.00,d1crit=0.00,spotcritical=0.00,Nd1crit=0.00,A2=0.00,d1=0.00,d2=0.00,Nd1=0.00,Nd2=0.00,calldelta=0.00,delta=0.00;


	h = 1 - exp(-interest * maturity / 365);
	alpha = 2 * interest / (vol * vol);
	beta = 2 * (interest - dividend) / (vol * vol);
	gamma1 = -0.5 * ((beta - 1) + sqrt(fabs(((beta - 1)*(beta - 1)) + 4 * alpha / h)));
	gamma2 = -0.5 * ((beta - 1) - sqrt(fabs(((beta - 1)*(beta - 1)) + 4 * alpha / h)));

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function delta_americancall:h:%lf:alpha:%lf:beta:%lf:gamma1:%lf:gamma2:%lf:",h,alpha,beta,gamma1,gamma2);
}

	spotcritical = callcritical(c_ServiceName,spot, strike, interest, maturity, vol, dividend);

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function delta_americancall spotcritical:%lf:initialspot:%lf:",spotcritical,initialspot);
}

		if(initialspot < spotcritical)
		{
		    d1crit = (log(spotcritical / strike) + (((interest - dividend) + (0.5 * vol * vol)) * maturity / 365)) / (vol * sqrt(fabs(maturity / 365)));
		    Nd1crit = NC(c_ServiceName,d1crit);
		    
		    A2 = (spotcritical / gamma2) * (1 - exp(-dividend * maturity / 365) * Nd1crit);
		    
		    d1 = (log(initialspot / strike) + (interest - dividend + (0.5 * vol * vol)) * maturity / 365) / (vol * sqrt(fabs(maturity / 365)));
		    d2 = d1 - (vol * sqrt(fabs(maturity / 365)));

		    Nd1 = NC(c_ServiceName,d1);
		    Nd2 = NC(c_ServiceName,d2);

		    calldelta = exp(-dividend * maturity / 365) * Nd1;

		    delta = (calldelta + (A2 * gamma2 / spotcritical) * pow((initialspot / spotcritical) , (gamma2 - 1)));

				if(DEBUG_MSG_LVL_3){
					fn_userlog(c_ServiceName,"Function delta_americancall Shailesh before Else:delta:%lf:",delta);
				}

		}
		else
		{
			if(DEBUG_MSG_LVL_3){
				fn_userlog(c_ServiceName,"Function delta_americancall:Shailesh : I am in else delta=1");
			}
		    	delta = 1;
		}

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function delta_americancall:delta:%lf:",delta);
}

		return delta;
}
    

double americancallgamma(char* c_ServiceName,double spot, 
												 double strike, 
										     double interest, 
												 double maturity, 
												 double vol, 
												 double dividend)
{
double initialspot=0.00,spotnext=0.00,spotcritical=0.00;
double h=0.00,alpha=0.00,beta=0.00,gamma1=0.00,gamma2=0.00,d1=0.00;
double d1crit=0.00,Nd1crit=0.00,A2=0.00,Nd1prime=0.00,callgamma=0.00;
double americancallgamma1=0.00;

	initialspot = spot;
	spotnext = 0.5 * spot;

	h = 1 - exp(-interest * maturity / 365);
	alpha = 2 * interest / (vol * vol) ;
	beta = 2 * (interest - dividend) / (vol * vol); 
	gamma1 = -0.5 * ((beta - 1) + sqrt(fabs(((beta - 1)*(beta -1)) + 4 * alpha / h)));
	gamma2 = -0.5 * ((beta - 1) - sqrt(fabs(((beta - 1)*(beta -1)) + 4 * alpha / h)));

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function americancallgamma:h:%lf:alpha:%lf:beta:%lf:gamma1:%lf:gamma2:%lf:",h,alpha,beta,gamma1,gamma2);
}

	spotcritical = callcritical(c_ServiceName,spot, strike, interest, maturity, vol, dividend);

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function americancall spotcritical:%lf:",spotcritical);
}

		if(initialspot < spotcritical) 
		{
		    
		    d1crit = (log(spotcritical / strike) + (interest - dividend + 0.5 * vol * vol) * maturity / 365) / (vol * sqrt(fabs(maturity / 365)));
		    Nd1crit = NC(c_ServiceName,d1crit);
		    
		    A2 = (spotcritical / gamma2) * (1 - exp(-dividend * maturity / 365) * Nd1crit);
		    
		    d1 = (log(initialspot / strike) + (interest - dividend + 0.5 * vol * vol) * maturity / 365) / (vol * sqrt(fabs(maturity / 365)));
		    
		    Nd1prime = exp(-0.5 * d1 * d1) / sqrt(2 * 3.14);

		    callgamma = exp(-dividend * maturity / 365) * Nd1prime / (vol * sqrt(fabs(maturity / 365)) * spot);

		    americancallgamma1 = callgamma + (A2 * gamma2 * (gamma2 - 1) / (spotcritical * spotcritical)) * pow((initialspot / spotcritical),(gamma2 - 2));
		}
		else
		{
		    americancallgamma1 = 0;
		}

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function americancall americancallgamma1:%lf:",americancallgamma1);
}
			return americancallgamma1;
}

double vega_amercall(char* c_ServiceName,double spot, 
										 double strike, 
									   double maturity, 
                     double vol, 
						         double interest, 
							       double dividend)
{

double vega =0.00,price1=0.00,vol2=0.00,vol0=0.00,price2=0.00,price0=0.00,vega0=0.00,vega2=0.00; 

	price1 = americancall(c_ServiceName,spot, strike, maturity, vol, interest, dividend);

	vol2 = 1.01 * vol;
	vol0 = 0.99 * vol;

	price2 = americancall(c_ServiceName,spot, strike, maturity, vol2, interest, dividend);
	price0 = americancall(c_ServiceName,spot, strike, maturity, vol0, interest, dividend);

	vega0 = (price0 - price1) / (vol0 - vol);
	vega2 = (price2 - price1) / (vol2 - vol);

	vega = (vega0 + vega2) / 200;

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function vega_amercall:price0:%lf:price1:%lf:price2:%lf:",price0,price1,price2);
  fn_userlog(c_ServiceName,"Function vega_amercall:vol0:%lf:vol2:%lf:",vol0,vol2);
  fn_userlog(c_ServiceName,"Function vega_amercall:vega0:%lf:vega2:%lf:vega:%lf:",vega0,vega2,vega);
}

	return vega;
}


double callcritical(char* c_ServiceName,double spot, 
								    	double strike, 
											double interest, 
								      double maturity, 
											double vol, 
											double dividend)
{
double initialspot=0.00,spotnext=0.00;
double alpha=0.00,beta=0.00,delta=0.00,gamma=0.00,gamma1=0.00,gamma2=0.00,callprice=0.00;
double callcritical1=0.00,d1=0.00,d2=0.00,Nd1=0.00,Nd2=0.00;
double expression1=0.00,h=0.00,derivative=0.00;

double temp =0.00;

	initialspot = spot;
	spotnext = 0.5 * spot;

	h = 1 - exp(-interest * maturity / 365);
	alpha = 2 * interest / (vol * vol);
	beta = 2 * (interest - dividend) / (vol * vol); 
	gamma1 = -0.5 * ((beta - 1) + sqrt(fabs(((beta - 1) * (beta - 1)) + 4 * alpha / h)));
	gamma2 = -0.5 * ((beta - 1) - sqrt(fabs(((beta - 1) * (beta - 1)) + 4 * alpha / h)));

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function callcritical:h:%lf:alpha:%lf:beta:%lf:gamma1:%lf:gamma2:%lf:",h,alpha,beta,gamma1,gamma2);
  fn_userlog(c_ServiceName,"Function callcritical spotnext:%lf:",spotnext);
}

temp = fabs((spot - spotnext) / spot);
if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function callcritical Shailesh temp:%lf:",temp);
}
		while (fabs((spot - spotnext) / spot) > 0.0025)
		{
			spotnext = spot;
			d1 = (log(spot / strike) + (interest - dividend + 0.5 * vol * vol) * maturity / 365) / (vol * sqrt(fabs(maturity / 365)));
			d2 = (log(spot / strike) + (interest - dividend - 0.5 * vol * vol) * maturity / 365) / (vol * sqrt(fabs(maturity / 365)));

			Nd1 = NC(c_ServiceName,d1);
			Nd2 = NC(c_ServiceName,d2);

			callprice = (spot * exp(-dividend * maturity / 365) * Nd1) - (strike * exp(-interest * maturity / 365) * Nd2);
			delta = exp(-dividend * maturity / 365) * Nd1;
			gamma = exp(-dividend * maturity / 365) * exp(-0.5 * d1 * d1) / (sqrt(2 * 3.14) * (spot * vol * sqrt(fabs(maturity / 365))));

			expression1 = spot - strike - callprice - ((1 - exp(-dividend * maturity / 365) * Nd1) * spot / gamma2);
			derivative = (1 - 1 / gamma2) - (delta * (1 - 1 / gamma2)) + (spot * gamma / gamma2);

			spot = spot - expression1 / derivative;

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function callcritical Shailesh spot:%lf:",spot);
}
temp = fabs((spot - spotnext) / spot);
if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function callcritical Shailesh temp1:%lf:",temp);
}
		}

	callcritical1 = spot;

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function callcritical:spot:%lf:callcritical1:%lf:",spot,callcritical1);
}
	return callcritical1;
}



double americancalltheta(char* c_ServiceName,double spot, 
											   double strike, 
											   double interest, 
											   double maturity, 
												 double vol, 
										     double dividend)
{
double pricedown=0.00,pricemid=0.00,theta=0.00;

	pricedown = americancall(c_ServiceName,spot, strike, maturity - 1, vol, interest, dividend);
	pricemid = americancall(c_ServiceName,spot, strike, maturity, vol, interest, dividend);

	theta = -(pricemid - pricedown);

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function americancalltheta:pricedown:%lf:pricemid:%lf:",pricedown,pricemid);
  fn_userlog(c_ServiceName,"Function americancalltheta:theta:%lf:",theta);
}
	return theta;
}

double vol_aput(char* c_ServiceName,double spot, 
								double strike, 
								double maturity, 	
								double interest, 
								double dividend, 
								double mktprice)
{
int iteration=0;
double vol=0.00,midprice=0.00,midvol=0.00;
double leastcount = 0.005;
double uppervol = 3;
double lowervol = 0.01;

		do{
	    	if((uppervol - lowervol) > leastcount) 
				{
	        		midvol = (uppervol + lowervol) / 2;
	        		midprice = americanput(c_ServiceName,spot, strike, maturity, midvol, interest, dividend);
							if(midprice > mktprice)
							{
	     						uppervol = midvol;
							}
	        		else
							{
	            		lowervol = midvol;
							}
							iteration = iteration + 1;
				}
				else
				{
	        break;		
				}
			 }while (iteration < 60);

	if(iteration < 60) 
	{
		vol = (uppervol + lowervol) / 2;
		if(DEBUG_MSG_LVL_3){
			fn_userlog(c_ServiceName,"Function vol_aput iteration less than 60:%d:",iteration);
		}
	}

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function vol_aput:vol:%lf:",vol);
}

	return vol;
}


double americanput(char* c_ServiceName,double spot, 
									 double strike, 
									 double maturity, 
								   double vol, 
						       double interest, 
								   double dividend)
{
double initialspot=0.00,spotnext=0.00,h=0.00,alpha=0.00,beta=0.00,gamma1=0.00,gamma2=0.00,expression1=0.00,derivative=0.00;
double d1=0.00,d2=0.00;
double Nd1=0.00,Nd2=0.00;
double putprice=0.00,delta=0.00,gamma=0.00;
double spotcritical=0.00,d1crit=0.00,d2crit=0.00,Nd1crit=0.00,Nd2crit=0.00,A1=0.00;
double americanput1=0.00;

	initialspot = spot;

	fn_userlog(c_ServiceName,"Maturity before is:%lf:",maturity);

	maturity=fabs(maturity);

	fn_userlog(c_ServiceName,"Maturity after is:%lf:",maturity);

	spotnext = 0.5 * spot;

	h = 1 - exp(-interest * maturity / 365);
	alpha = 2 * interest / (vol * vol);
	beta = 2 * (interest - dividend) / (vol * vol);

	if (h <= 0)
	{
		gamma1 = 0;
		gamma2 = 0;
	}
	else	
	{
		gamma1 = -0.5 * ((beta - 1) + sqrt((beta - 1) * (beta - 1) + 4 * alpha / h));
		gamma2 = -0.5 * ((beta - 1) - sqrt((beta - 1) * (beta - 1) + 4 * alpha / h));
	}

	if(spot == 0 )
	{
		spot =1;
	}

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function americanput:h:%lf:alpha:%lf:beta:%lf:gamma1:%lf:gamma2:%lf:",h,alpha,beta,gamma1,gamma2);
	fn_userlog(c_ServiceName,"Function americanput:spot:%lf:spotnext:%lf:initialspot:%lf:",spot,spotnext,initialspot);
}

		while (fabs((spot - spotnext) / spot) > 0.0025)
		{
			spotnext = spot;
			if( maturity == 0) 
			{
				d1 = 0;
				d2 = 0;
			}
			else
			{
				d1 = (log(spot/strike)+(interest-dividend+0.5* vol*vol)* maturity / 365)/ (vol * sqrt(fabs(maturity) / 365));
				d2 = (log(spot/strike)+(interest-dividend- 0.5*vol*vol)* maturity / 365)/ (vol * sqrt(fabs(maturity) / 365));
			}
			
			Nd1 = NC(c_ServiceName,d1);
			Nd2 = NC(c_ServiceName,d2);

			putprice = spot * exp(-dividend * maturity / 365) * (Nd1 - 1) - strike * exp(-interest * maturity / 365) * (Nd2 - 1);
			delta = exp(-dividend * maturity / 365) * (Nd1 - 1);
			if (maturity == 0 )
			{
				gamma = 0;
			}
			else
			{
				gamma = exp(-dividend * maturity / 365) * exp(-0.5 * d1 * d1) / (sqrt(2 * 3.14159) * spot * vol * sqrt(fabs(maturity) / 365));
			}
			if( gamma1 == 0 )
			{
				expression1 = 0;
				derivative = 0;
				spot = 0;
			}
			else
			{
				expression1 = strike - spot - putprice + (1 - (1 - Nd1) * exp(-dividend * maturity / 365)) * spot / gamma1;
				derivative = -(1 - 1 / gamma1) - delta * (1 - 1 / gamma1) + spot * gamma / gamma1;
				spot = spot - expression1 / derivative;
			}
			if (spot == 0 )
			{
				spot =1;
			}
		}

		spotcritical = spot;

		if (initialspot > spotcritical )
		{
			if(maturity==0) 
			{
				maturity=1;
			}

			if( gamma1==0) 
			{
				gamma1=1;
			}
		    d1crit = (log(spotcritical / strike) + (interest - dividend + 0.5 * vol * vol) * maturity / 365) / (vol * sqrt(maturity / 365));
		    d2crit = (log(spotcritical / strike) + (interest - dividend - 0.5 * vol * vol) * maturity / 365) / (vol * sqrt(maturity / 365));

		    Nd1crit = NC(c_ServiceName,-d1crit);
		    Nd2crit = NC(c_ServiceName,-d2crit);

		    A1 = -(spotcritical / gamma1) * (1 - exp(-dividend * maturity / 365) * Nd1crit);
		       
		    d1 = (log(initialspot / strike) + (interest - dividend + 0.5 * vol * vol) * maturity / 365) / (vol * sqrt(maturity / 365));
		    d2 = (log(initialspot / strike) + (interest - dividend - 0.5 * vol * vol) * maturity / 365) / (vol * sqrt(maturity / 365));

		    Nd1 = NC(c_ServiceName,d1);
		    Nd2 = NC(c_ServiceName,d2);

		    putprice = initialspot * exp(-dividend * maturity / 365) * (Nd1 - 1) - strike * exp(-interest * maturity / 365) * (Nd2 - 1);
		    americanput1 = putprice + A1 * pow((initialspot / spotcritical) , gamma1);
		}
		else
		{
		    americanput1 = strike - initialspot;
		}

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function americanput:americanput1:%lf:",americanput1);
}

	  return americanput1;
}    


double delta_americanput(char* c_ServiceName,double spot, 
											   double strike, 
											   double interest, 
								         double maturity, 
												 double vol, 
												 double dividend)
{
double initialspot=0.00,spotnext=0.00,delta=0.00,h=0.00,alpha=0.00,beta=0.00,gamma1=0.00,gamma2=0.00;
double spotcritical=0.00,d1crit=0.00;
double Nd1crit=0.00,A1=0.00,d1=0.00,d2=0.00,Nd1=0.00,Nd2=0.00,putdelta=0.00;

	initialspot = spot;
	spotnext = 0.5 * spot;

	h = 1 - exp(-interest * maturity / 365);
	alpha = 2 * interest / (vol * vol);
	beta = (2 * (interest - dividend)) / (vol * vol);
	gamma1 = -0.5 * ((beta - 1) + sqrt(fabs((fabs(beta - 1)) * (fabs(beta - 1)))  + 4 * alpha / h));
	gamma2 = -0.5 * ((beta - 1) - sqrt(fabs((fabs(beta - 1)) * (fabs(beta - 1)))  + 4 * alpha / h));

	spotcritical = putcritical(c_ServiceName,spot, strike, interest, maturity, vol, dividend);

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function delta_americanput:h:%lf:alpha:%lf:beta:%lf:gamma1:%lf:gamma2:%lf:",h,alpha,beta,gamma1,gamma2);
	fn_userlog(c_ServiceName,"Function delta_americanput:spotcritical:%lf:",spotcritical);
}
		if (initialspot > spotcritical) 
		{
			d1crit = (log(spotcritical / strike) + (interest - dividend + 0.5 * vol * vol) * maturity / 365) / (vol * sqrt(fabs(maturity) / 365));
	    	Nd1crit = NC(c_ServiceName,-d1crit);
			    
	    	A1 = -(spotcritical / gamma1) * (1 - exp(-dividend * maturity / 365) * Nd1crit);
			       
	    	d1 = (log(initialspot / strike) + (interest - dividend + 0.5 * vol * vol) * maturity / 365) / (vol * sqrt(fabs(maturity) / 365));
		   	d2 = (log(initialspot / strike) + (interest - dividend - 0.5 * vol * vol) * maturity / 365) / (vol * sqrt(fabs(maturity) / 365));

	    	Nd1 = NC(c_ServiceName,d1);
	    	Nd2 = NC(c_ServiceName,d2);

	    	putdelta = exp(-dividend * maturity / 365) * (Nd1 - 1);
	     
	    	delta= putdelta + (A1 * gamma1 / spotcritical) * pow((initialspot / spotcritical) , (gamma1 - 1));
		}
		else
		{
	    	delta = -1 ;
		}

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function delta_americanput:delta:%lf:",delta);
}
	
		return delta;
}



double americanputgamma(char* c_ServiceName,double spot, 
											  double strike, 
												double interest, 
												double maturity, 
												double vol, 
												double dividend)
{
double initialspot=0.00,spotnext=0.00,h=0.00,alpha=0.00,beta=0.00,gamma1=0.00,gamma2=0.00;
double spotcritical=0.00,d1crit=0.00,Nd1crit=0.00,Nd1prime=0.00,putgamma=0.00,americanputgamma1=0.00;
double A1=0.00,d1=0.00;

	initialspot = spot;
	spotnext = 0.5 * spot;

	h = 1 - exp(-interest * maturity / 365);
	alpha = 2 * interest / (vol * vol);
	beta = 2 * (interest - dividend) / (vol * vol);
	gamma1 = -0.5 * ((beta - 1) + sqrt(((beta - 1) * (beta - 1)) + 4 * alpha / h));
	gamma2 = -0.5 * ((beta - 1) - sqrt(((beta - 1) * (beta - 1)) + 4 * alpha / h));

	spotcritical = putcritical(c_ServiceName,spot, strike, interest, maturity, vol, dividend);

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function americanputgamma:h:%lf:alpha:%lf:beta:%lf:gamma1:%lf:gamma2:%lf:",h,alpha,beta,gamma1,gamma2);
	fn_userlog(c_ServiceName,"Function americanputgamma:spotcritical:%lf:",spotcritical);
}

		if (initialspot > spotcritical) 
		{
		   d1crit = (log(spotcritical / strike) + (interest - dividend + 0.5 * vol * vol) * maturity / 365) / (vol * sqrt(maturity / 365));
		   Nd1crit = NC(c_ServiceName,-d1crit);
		    
		   A1 = -(spotcritical / gamma1) * (1 - exp(-dividend * maturity / 365) * Nd1crit);
		       
		   d1 = (log(initialspot / strike) + (interest - dividend + 0.5 * vol * vol) * maturity / 365) / (vol * sqrt(maturity / 365));
		        
		   Nd1prime = exp(-0.5 * d1 * d1) / sqrt(2 * 3.14);

		   putgamma = exp(-dividend * maturity / 365) * Nd1prime / (vol * sqrt(maturity / 365) * spot);
		    
		   americanputgamma1 = putgamma + (A1 * gamma1 * (gamma1 - 1) / (spotcritical * spotcritical) * pow((initialspot / spotcritical) , (gamma1 - 2)));
		}
		else
		{
		    	americanputgamma1 = 0;
		}

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function americanputgamma:americanputgamma1:%lf:",americanputgamma1);
}
		return americanputgamma1;
}


double vega_amerput(char* c_ServiceName,double spot, 
										double strike, 
										double maturity, 
										double vol, 
										double interest, 
										double dividend)
{
double price1=0.00,vol2=0.00,vol0=0.00,price2=0.00,price0=0.00,vega0=0.00,vega2=0.00,vega=0.00;

	price1 = americanput(c_ServiceName,spot, strike, maturity, vol, interest, dividend);

	vol2 = 1.01 * vol;
	vol0 = 0.99 * vol;

	price2 = americanput(c_ServiceName,spot, strike, maturity, vol2, interest, dividend);
	price0 = americanput(c_ServiceName,spot, strike, maturity, vol0, interest, dividend);

	vega0 = (price0 - price1) / (vol0 - vol);
	vega2 = (price2 - price1) / (vol2 - vol);

	vega = (vega0 + vega2) / 200;

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function vega_amerput:price0:%lf:price1:%lf:price2:%lf:",price0,price1,price2);
  fn_userlog(c_ServiceName,"Function vega_amerput:vol0:%lf:vol2:%lf:",vol0,vol2);
  fn_userlog(c_ServiceName,"Function vega_amerput:vega0:%lf:vega2:%lf:vega:%lf:",vega0,vega2,vega);
}
	return vega;
}

double americanputtheta(char* c_ServiceName,double spot, 
												double strike, 
												double interest, 
												double maturity, 
												double vol, 
												double dividend)
{
double pricedown=0.00,pricemid=0.00,theta=0.00;

	pricedown = americanput(c_ServiceName,spot, strike, maturity - 1, vol, interest, dividend);
	pricemid = americanput(c_ServiceName,spot, strike, maturity, vol, interest, dividend);

	theta = -(pricemid - pricedown);

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function americanputtheta:pricedown:%lf:pricemid:%lf:",pricedown,pricemid);
  fn_userlog(c_ServiceName,"Function americanputtheta:theta:%lf:",theta);
}

	return theta;
}


double putcritical(char* c_ServiceName,double spot, 
									 double strike, 
									 double interest, 
						       double maturity, 
									 double vol, 
									 double dividend)
{

double initialspot=0.00,spotnext=0.00,h=0.00,alpha=0.00,beta=0.00,gamma1=0.00,gamma2=0.00,d1=0.00,d2=0.00,Nd1=0.00,Nd2=0.00,putprice=0.00,delta=0.00,expression1=0.00,derivative=0.00,putcritical1=0.00;
double gamma=0.00;

	int intLogParam=0;
	initialspot = spot;
	spotnext = 0.5 * spot;

	h = 1 - exp(-interest * maturity / 365);
	alpha = 2 * interest / (vol * vol); 
	beta = 2 * (interest - dividend) / (vol * vol);


	gamma1 = -0.5 * ((beta - 1) + sqrt(fabs(((fabs(beta - 1)) * (fabs(beta - 1))) + 4 * alpha / h)));
	gamma2 = -0.5 * ((beta - 1) - sqrt(fabs(((fabs(beta - 1)) * (fabs(beta - 1))) + 4 * alpha / h)));

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function putcritical:h:%lf:alpha:%lf:beta:%lf:gamma1:%lf:gamma2:%lf:",h,alpha,beta,gamma1,gamma2);
}
	
	while (fabs((spot - spotnext) / spot) > 0.0025 )
	{
		spotnext = spot;
	
		d1 = (log(spot/strike) + (interest - dividend + 0.5 * vol * vol) * maturity / 365) / (vol * sqrt(fabs(maturity)/ 365));
		d2 = (log(spot/strike) + (interest - dividend - 0.5 * vol * vol) * maturity / 365) / (vol * sqrt(fabs(maturity)/ 365));

		Nd1 = NC(c_ServiceName,d1);
		Nd2 = NC(c_ServiceName,d2);

		putprice = spot * exp(-dividend * maturity / 365) * (Nd1 - 1) - strike * exp(-interest * maturity / 365) * (Nd2 - 1);
		delta = exp(-dividend * maturity / 365) * (Nd1 - 1);
		gamma = exp(-dividend * maturity / 365) * exp(-0.5 * d1 * d1) / (sqrt(2 * 3.14159) * spot * vol * sqrt(fabs(maturity) / 365));

		expression1 = strike - spot - putprice + (1 - (1 - Nd1) * exp(-dividend * maturity / 365)) * spot / gamma1;
		derivative = -(1 - 1 / gamma1) - delta * (1 - 1 / gamma1) + spot * gamma / gamma1;
		spot = spot - expression1 / derivative;
	}
		
	putcritical1 = spot;

if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Function putcritical:putcritical:%lf:",putcritical1);
}

	return putcritical1;
}
