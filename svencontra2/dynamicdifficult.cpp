#include "dynamicdifficult.h"

using namespace std;

const char* CVAR_KNNAME = "sk_plr_crowbar";

CWeaponDMGBase g_WeaponDMG;

void CWeaponDMGBase::Tweak(float factor) {
    this->AR = factor * this->ARD;
    this->SG = factor * this->SGD;
    this->MG = factor * this->MGD;
    this->FG = factor * this->FGD;
    this->FGE = factor * this->FGED;
    this->LG = factor * this->LGD;
    this->KN = factor * this->KND;
    CVAR_SET_FLOAT(CVAR_KNNAME, this->KN);
}

void PlayerDMGTweak(){
    int iNowPlayerNum = 0;
    for(int i = 0; i < 33; i++){
        CBasePlayer* pEntity = UTIL_PlayerByIndex(i);
        if(pEntity){
            iNowPlayerNum++;
        }
    }
    /*
    * > x=c(1,2,3,4,5,6,7,8,9,10)
    * > y=c(1,1,1,0.8,0.75,0.72,0.7,0.68,0.65,0.6)
    * > head(df)
    *                                             
    * 1 function (x, df1, df2, ncp, log = FALSE)    
    * 2 {                                           
    * 3     if (missing(ncp))                       
    * 4         .Call(C_df, x, df1, df2, log)       
    * 5     else .Call(C_dnf, x, df1, df2, ncp, log)
    * 6 }                                           
    * > df=data.frame(x=x,y=y)
    * > pra.lm=lm(y~log(x),data=df)
    * > a=coefficients(pra.lm)[2]
    * > b=coefficients(pra.lm)[1]
    * > summary(pra.lm)
    * 
    * Call:
    * lm(formula = y ~ log(x), data = df)
    * 
    * Residuals:
    *     Min        1Q    Median        3Q       Max 
    * -0.086297 -0.019139 -0.009815  0.000070  0.129213 
    * 
    * Coefficients:
    *             Estimate Std. Error t value Pr(>|t|)    
    * (Intercept)  1.08630    0.04530  23.981 9.74e-09 ***
    * log(x)      -0.19617    0.02724  -7.201 9.24e-05 ***
    * ---
    * Signif. codes:  0 ‘***’ 0.001 ‘**’ 0.01 ‘*’ 0.05 ‘.’ 0.1 ‘ ’ 1
    * 
    * Residual standard error: 0.05991 on 8 degrees of freedom
    * Multiple R-squared:  0.8663,    Adjusted R-squared:  0.8496 
    * F-statistic: 51.86 on 1 && 8 DF,  p-value: 9.235e-05
    * 
    * > a
    *     log(x) 
    * -0.1961658 
    * > b
    * (Intercept) 
    * 1.086297 
    * > n=do.call(cbind,foreach(var=1:32) %do% (a*log(var)+b))
    * > n
    *         [,1]      [,2]      [,3]      [,4]      [,5]     [,6]
    * log(x) 1.086297 0.9503252 0.8707868 0.8143534 0.7705802 0.734815
    *             [,7]      [,8]      [,9]     [,10]     [,11]     [,12]
    * log(x) 0.7045759 0.6783816 0.6552766 0.6346085 0.6159119 0.5988432
    *         [,13]     [,14]     [,15]     [,16]     [,17]     [,18]
    * log(x) 0.5831416 0.5686041 0.5550701 0.5424098 0.5305173 0.5193048
    *         [,19]     [,20]     [,21]     [,22]     [,23]     [,24]
    * log(x) 0.5086987 0.4986367 0.4890657 0.4799401 0.4712202 0.4628714
    *         [,25]     [,26]     [,27]     [,28]     [,29]     [,30]
    * log(x) 0.4548636 0.4471698 0.4397664 0.4326323 0.4257486 0.4190983
    *         [,31]    [,32]
    * log(x) 0.4126661 0.406438
    */
    //float flTweakFactor = iNowPlayerNum <= 3 ? 1.0f : (-0.1961658f * log(iNowPlayerNum) + 1.086297f);
    //g_WeaponDMG.Tweak(flTweakFactor);
    g_WeaponDMG.Tweak(g_WeaponDMG.aryTweakFactors[iNowPlayerNum-1]);
}