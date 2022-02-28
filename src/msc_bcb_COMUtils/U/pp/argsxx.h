//#pragma once
#ifndef __ARGSXX_H 
#define __ARGSXX_H 
//---------------------------------------------------------------------------
#define ARGS_DELIMITER ,
#define CQ ,
#define CQQ
#define CQQ0
#define CQQ1 ,
#define CQQ2 ,
#define CQQ3 ,
#define CQQ4 ,
#define CQQ5 ,
#define CQQ6 ,
#define CQQ7 ,
#define CQQ8 ,
#define CQQ9 ,
#define CQQA ,
#define CQQB ,
#define CQQC ,
#define CQQD ,
#define CQQE ,
#define CQQF ,

#define ARGS_0(a,e)
#define ARGS_1(a,e) ARGS_0(a,e)   a##1##e
#define ARGS_2(a,e) ARGS_1(a,e) ,  a##2##e
#define ARGS_3(a,e) ARGS_2(a,e) ,  a##3##e
#define ARGS_4(a,e) ARGS_3(a,e) ,  a##4##e
#define ARGS_5(a,e) ARGS_4(a,e) ,  a##5##e
#define ARGS_6(a,e) ARGS_5(a,e) ,  a##6##e
#define ARGS_7(a,e) ARGS_6(a,e) ,  a##7##e
#define ARGS_8(a,e) ARGS_7(a,e) ,  a##8##e
#define ARGS_9(a,e) ARGS_8(a,e) ,  a##9##e

#define ARGS_A(a,e) ARGS_9(a,e) ,  a##A##e
#define ARGS_B(a,e) ARGS_A(a,e) ,  a##B##e
#define ARGS_C(a,e) ARGS_B(a,e) ,  a##C##e
#define ARGS_D(a,e) ARGS_C(a,e) ,  a##D##e
#define ARGS_E(a,e) ARGS_D(a,e) ,  a##E##e
#define ARGS_F(a,e) ARGS_E(a,e) ,  a##F##e

#define ARGS_10(a,e) ARGS_F(a,e) ,  a##10##e

#define ARGS_20(a,e) ARGS_10(a,e) , ARGS_F(a##1,e) , a##20##e
#define ARGS_30(a,e) ARGS_20(a,e) , ARGS_F(a##2,e) , a##30##e
#define ARGS_40(a,e) ARGS_30(a,e) , ARGS_F(a##3,e) , a##40##e
#define ARGS_50(a,e) ARGS_40(a,e) , ARGS_F(a##4,e) , a##50##e
#define ARGS_60(a,e) ARGS_50(a,e) , ARGS_F(a##5,e) , a##60##e
#define ARGS_70(a,e) ARGS_60(a,e) , ARGS_F(a##6,e) , a##70##e
#define ARGS_80(a,e) ARGS_70(a,e) , ARGS_F(a##7,e) , a##80##e
#define ARGS_90(a,e) ARGS_80(a,e) , ARGS_F(a##8,e) , a##90##e

#define ARGS_A0(a,e) ARGS_90(a,e) , ARGS_F(a##9,e) , a##A0##e
#define ARGS_B0(a,e) ARGS_A0(a,e) , ARGS_F(a##A,e) , a##B0##e
#define ARGS_C0(a,e) ARGS_B0(a,e) , ARGS_F(a##B,e) , a##C0##e
#define ARGS_D0(a,e) ARGS_C0(a,e) , ARGS_F(a##C,e) , a##D0##e
#define ARGS_E0(a,e) ARGS_D0(a,e) , ARGS_F(a##D,e) , a##E0##e
#define ARGS_F0(a,e) ARGS_E0(a,e) , ARGS_F(a##E,e) , a##F0##e

#define ARGS_00(a,e)
#define ARGS_100(a,e)  ARGS_F0(a,e) , ARGS_F(a##F,e) , a##100
#define CARGS(n,m) (0x##n##m)

#define ARGS_xx_def(a,n,m,e) ARGS_##n##0(a,e) CQQ##m  ARGS_##m(a##n,e)

#define ARGS_xx(a,n,m) ARGS_xx_def(a,n,m,)

#define ARGS_xx_def0(a,n,m,e)   ARGS_##m(a##n,e)
#define ARGS_xx_def1(a,n,m,e)   ARGS_xx_def(a,n,m,e)
#define ARGS_xx_def2(a,n,m,e)   ARGS_xx_def(a,n,m,e)
#define ARGS_xx_def3(a,n,m,e)   ARGS_xx_def(a,n,m,e)
#define ARGS_xx_def4(a,n,m,e)   ARGS_xx_def(a,n,m,e)
#define ARGS_xx_def5(a,n,m,e)   ARGS_xx_def(a,n,m,e)
#define ARGS_xx_def6(a,n,m,e)   ARGS_xx_def(a,n,m,e)
#define ARGS_xx_def7(a,n,m,e)   ARGS_xx_def(a,n,m,e)
#define ARGS_xx_def8(a,n,m,e)   ARGS_xx_def(a,n,m,e)
#define ARGS_xx_def9(a,n,m,e)   ARGS_xx_def(a,n,m,e)
#define ARGS_xx_defA(a,n,m,e)   ARGS_xx_def(a,n,m,e)
#define ARGS_xx_defB(a,n,m,e)   ARGS_xx_def(a,n,m,e)
#define ARGS_xx_defC(a,n,m,e)   ARGS_xx_def(a,n,m,e)
#define ARGS_xx_defD(a,n,m,e)   ARGS_xx_def(a,n,m,e)
#define ARGS_xx_defE(a,n,m,e)   ARGS_xx_def(a,n,m,e)
#define ARGS_xx_defF(a,n,m,e)   ARGS_xx_def(a,n,m,e)

//#define pFirst()







#define ARGS_Z_def(a,n,m,e)   ARGS_xx_def##n(a,n,m,e)
#define ARGS_Z(a,n,m)    ARGS_Z_def(a,n,m,)

#define _DECLARE_METHODS_ARGS_OF(n,REPEAT_MACRO_XX)  _MACRO_##n##_COUNT(REPEAT_MACRO_XX)




/*
//*** this is macro generator generator***
// macrogen.js
//examlpe 0-xx args: cscript macrogen.js //nologo xx  >file.h 
// xx<256
var sbuf=""
function print(s)
{
sbuf+=s;
WScript.Echo(s)   
}


WshArg = WScript.Arguments
n=255
if(WshArg.Length>0) 
{
n=WshArg.Item(0);
}
//print(WshArg.Length+"=="+n+"{{"+WshArg.Item(0))

//strmacro0="MACRO_BLANK\\\n";
//print("#define MACRO_BLANK\n\n");
strmacro0=""
for( i=0;i<=n;i++)
{
//strmacro0="_MACRO_"+(i-1)+"_COUNT(repeated_macro_XX)\\";    
strmacro1="_MACRO_"+i+"_COUNT(repeated_macro_XX)\\\n";  
h=(0xF0&i)>>4;
l=(0x0F&i);
sh=h.toString(16);
sl=l.toString(16);
repmacro="repeated_macro_XX("+h+","+l+")";
buf="#define "+strmacro1+strmacro0+repmacro+"\n";
print(buf)
strmacro0=strmacro1;
}

WshExtra = new ActiveXObject("WshExtra.Clipboard");
WshExtra.Copy(sbuf);
*/

#define MACRO_BLANK

#define _MACRO_0_COUNT(repeated_macro_XX)\
	MACRO_BLANK\
	repeated_macro_XX(0,0)

#define _MACRO_1_COUNT(repeated_macro_XX)\
	_MACRO_0_COUNT(repeated_macro_XX)\
	repeated_macro_XX(0,1)

#define _MACRO_2_COUNT(repeated_macro_XX)\
	_MACRO_1_COUNT(repeated_macro_XX)\
	repeated_macro_XX(0,2)

#define _MACRO_3_COUNT(repeated_macro_XX)\
	_MACRO_2_COUNT(repeated_macro_XX)\
	repeated_macro_XX(0,3)

#define _MACRO_4_COUNT(repeated_macro_XX)\
	_MACRO_3_COUNT(repeated_macro_XX)\
	repeated_macro_XX(0,4)

#define _MACRO_5_COUNT(repeated_macro_XX)\
	_MACRO_4_COUNT(repeated_macro_XX)\
	repeated_macro_XX(0,5)

#define _MACRO_6_COUNT(repeated_macro_XX)\
	_MACRO_5_COUNT(repeated_macro_XX)\
	repeated_macro_XX(0,6)

#define _MACRO_7_COUNT(repeated_macro_XX)\
	_MACRO_6_COUNT(repeated_macro_XX)\
	repeated_macro_XX(0,7)

#define _MACRO_8_COUNT(repeated_macro_XX)\
	_MACRO_7_COUNT(repeated_macro_XX)\
	repeated_macro_XX(0,8)

#define _MACRO_9_COUNT(repeated_macro_XX)\
	_MACRO_8_COUNT(repeated_macro_XX)\
	repeated_macro_XX(0,9)

#define _MACRO_10_COUNT(repeated_macro_XX)\
	_MACRO_9_COUNT(repeated_macro_XX)\
	repeated_macro_XX(0,A)

#define _MACRO_11_COUNT(repeated_macro_XX)\
	_MACRO_10_COUNT(repeated_macro_XX)\
	repeated_macro_XX(0,B)

#define _MACRO_12_COUNT(repeated_macro_XX)\
	_MACRO_11_COUNT(repeated_macro_XX)\
	repeated_macro_XX(0,C)

#define _MACRO_13_COUNT(repeated_macro_XX)\
	_MACRO_12_COUNT(repeated_macro_XX)\
	repeated_macro_XX(0,D)

#define _MACRO_14_COUNT(repeated_macro_XX)\
	_MACRO_13_COUNT(repeated_macro_XX)\
	repeated_macro_XX(0,E)

#define _MACRO_15_COUNT(repeated_macro_XX)\
	_MACRO_14_COUNT(repeated_macro_XX)\
	repeated_macro_XX(0,F)

#define _MACRO_16_COUNT(repeated_macro_XX)\
	_MACRO_15_COUNT(repeated_macro_XX)\
	repeated_macro_XX(1,0)

#define _MACRO_17_COUNT(repeated_macro_XX)\
	_MACRO_16_COUNT(repeated_macro_XX)\
	repeated_macro_XX(1,1)

#define _MACRO_18_COUNT(repeated_macro_XX)\
	_MACRO_17_COUNT(repeated_macro_XX)\
	repeated_macro_XX(1,2)

#define _MACRO_19_COUNT(repeated_macro_XX)\
	_MACRO_18_COUNT(repeated_macro_XX)\
	repeated_macro_XX(1,3)

#define _MACRO_20_COUNT(repeated_macro_XX)\
	_MACRO_19_COUNT(repeated_macro_XX)\
	repeated_macro_XX(1,4)

#define _MACRO_21_COUNT(repeated_macro_XX)\
	_MACRO_20_COUNT(repeated_macro_XX)\
	repeated_macro_XX(1,5)

#define _MACRO_22_COUNT(repeated_macro_XX)\
	_MACRO_21_COUNT(repeated_macro_XX)\
	repeated_macro_XX(1,6)

#define _MACRO_23_COUNT(repeated_macro_XX)\
	_MACRO_22_COUNT(repeated_macro_XX)\
	repeated_macro_XX(1,7)

#define _MACRO_24_COUNT(repeated_macro_XX)\
	_MACRO_23_COUNT(repeated_macro_XX)\
	repeated_macro_XX(1,8)

#define _MACRO_25_COUNT(repeated_macro_XX)\
	_MACRO_24_COUNT(repeated_macro_XX)\
	repeated_macro_XX(1,9)

#define _MACRO_26_COUNT(repeated_macro_XX)\
	_MACRO_25_COUNT(repeated_macro_XX)\
	repeated_macro_XX(1,A)

#define _MACRO_27_COUNT(repeated_macro_XX)\
	_MACRO_26_COUNT(repeated_macro_XX)\
	repeated_macro_XX(1,B)

#define _MACRO_28_COUNT(repeated_macro_XX)\
	_MACRO_27_COUNT(repeated_macro_XX)\
	repeated_macro_XX(1,C)

#define _MACRO_29_COUNT(repeated_macro_XX)\
	_MACRO_28_COUNT(repeated_macro_XX)\
	repeated_macro_XX(1,D)

#define _MACRO_30_COUNT(repeated_macro_XX)\
	_MACRO_29_COUNT(repeated_macro_XX)\
	repeated_macro_XX(1,E)

#define _MACRO_31_COUNT(repeated_macro_XX)\
	_MACRO_30_COUNT(repeated_macro_XX)\
	repeated_macro_XX(1,F)

#define _MACRO_32_COUNT(repeated_macro_XX)\
	_MACRO_31_COUNT(repeated_macro_XX)\
	repeated_macro_XX(2,0)

#define _MACRO_33_COUNT(repeated_macro_XX)\
	_MACRO_32_COUNT(repeated_macro_XX)\
	repeated_macro_XX(2,1)

#define _MACRO_34_COUNT(repeated_macro_XX)\
	_MACRO_33_COUNT(repeated_macro_XX)\
	repeated_macro_XX(2,2)

#define _MACRO_35_COUNT(repeated_macro_XX)\
	_MACRO_34_COUNT(repeated_macro_XX)\
	repeated_macro_XX(2,3)

#define _MACRO_36_COUNT(repeated_macro_XX)\
	_MACRO_35_COUNT(repeated_macro_XX)\
	repeated_macro_XX(2,4)

#define _MACRO_37_COUNT(repeated_macro_XX)\
	_MACRO_36_COUNT(repeated_macro_XX)\
	repeated_macro_XX(2,5)

#define _MACRO_38_COUNT(repeated_macro_XX)\
	_MACRO_37_COUNT(repeated_macro_XX)\
	repeated_macro_XX(2,6)

#define _MACRO_39_COUNT(repeated_macro_XX)\
	_MACRO_38_COUNT(repeated_macro_XX)\
	repeated_macro_XX(2,7)

#define _MACRO_40_COUNT(repeated_macro_XX)\
	_MACRO_39_COUNT(repeated_macro_XX)\
	repeated_macro_XX(2,8)

#define _MACRO_41_COUNT(repeated_macro_XX)\
	_MACRO_40_COUNT(repeated_macro_XX)\
	repeated_macro_XX(2,9)

#define _MACRO_42_COUNT(repeated_macro_XX)\
	_MACRO_41_COUNT(repeated_macro_XX)\
	repeated_macro_XX(2,A)

#define _MACRO_43_COUNT(repeated_macro_XX)\
	_MACRO_42_COUNT(repeated_macro_XX)\
	repeated_macro_XX(2,B)

#define _MACRO_44_COUNT(repeated_macro_XX)\
	_MACRO_43_COUNT(repeated_macro_XX)\
	repeated_macro_XX(2,C)

#define _MACRO_45_COUNT(repeated_macro_XX)\
	_MACRO_44_COUNT(repeated_macro_XX)\
	repeated_macro_XX(2,D)

#define _MACRO_46_COUNT(repeated_macro_XX)\
	_MACRO_45_COUNT(repeated_macro_XX)\
	repeated_macro_XX(2,E)

#define _MACRO_47_COUNT(repeated_macro_XX)\
	_MACRO_46_COUNT(repeated_macro_XX)\
	repeated_macro_XX(2,F)

#define _MACRO_48_COUNT(repeated_macro_XX)\
	_MACRO_47_COUNT(repeated_macro_XX)\
	repeated_macro_XX(3,0)

#define _MACRO_49_COUNT(repeated_macro_XX)\
	_MACRO_48_COUNT(repeated_macro_XX)\
	repeated_macro_XX(3,1)

#define _MACRO_50_COUNT(repeated_macro_XX)\
	_MACRO_49_COUNT(repeated_macro_XX)\
	repeated_macro_XX(3,2)

#define _MACRO_51_COUNT(repeated_macro_XX)\
	_MACRO_50_COUNT(repeated_macro_XX)\
	repeated_macro_XX(3,3)

#define _MACRO_52_COUNT(repeated_macro_XX)\
	_MACRO_51_COUNT(repeated_macro_XX)\
	repeated_macro_XX(3,4)

#define _MACRO_53_COUNT(repeated_macro_XX)\
	_MACRO_52_COUNT(repeated_macro_XX)\
	repeated_macro_XX(3,5)

#define _MACRO_54_COUNT(repeated_macro_XX)\
	_MACRO_53_COUNT(repeated_macro_XX)\
	repeated_macro_XX(3,6)

#define _MACRO_55_COUNT(repeated_macro_XX)\
	_MACRO_54_COUNT(repeated_macro_XX)\
	repeated_macro_XX(3,7)

#define _MACRO_56_COUNT(repeated_macro_XX)\
	_MACRO_55_COUNT(repeated_macro_XX)\
	repeated_macro_XX(3,8)

#define _MACRO_57_COUNT(repeated_macro_XX)\
	_MACRO_56_COUNT(repeated_macro_XX)\
	repeated_macro_XX(3,9)

#define _MACRO_58_COUNT(repeated_macro_XX)\
	_MACRO_57_COUNT(repeated_macro_XX)\
	repeated_macro_XX(3,A)

#define _MACRO_59_COUNT(repeated_macro_XX)\
	_MACRO_58_COUNT(repeated_macro_XX)\
	repeated_macro_XX(3,B)

#define _MACRO_60_COUNT(repeated_macro_XX)\
	_MACRO_59_COUNT(repeated_macro_XX)\
	repeated_macro_XX(3,C)

#define _MACRO_61_COUNT(repeated_macro_XX)\
	_MACRO_60_COUNT(repeated_macro_XX)\
	repeated_macro_XX(3,D)

#define _MACRO_62_COUNT(repeated_macro_XX)\
	_MACRO_61_COUNT(repeated_macro_XX)\
	repeated_macro_XX(3,E)

#define _MACRO_63_COUNT(repeated_macro_XX)\
	_MACRO_62_COUNT(repeated_macro_XX)\
	repeated_macro_XX(3,F)

#define _MACRO_64_COUNT(repeated_macro_XX)\
	_MACRO_63_COUNT(repeated_macro_XX)\
	repeated_macro_XX(4,0)

#define QSB00(a) 
#define QSB01(a) a
#define QSB02(a) a
#define QSB03(a) a
#define QSB04(a) a
#define QSB05(a) a
#define QSB06(a) a
#define QSB07(a) a
#define QSB08(a) a
#define QSB09(a) a
#define QSB0A(a) a
#define QSB0B(a) a
#define QSB0C(a) a
#define QSB0D(a) a
#define QSB0E(a) a
#define QSB0F(a) a
#define QSB10(a) a
#define QSB11(a) a
#define QSB12(a) a
#define QSB13(a) a
#define QSB14(a) a
#define QSB15(a) a
#define QSB16(a) a
#define QSB17(a) a
#define QSB18(a) a
#define QSB19(a) a
#define QSB1A(a) a
#define QSB1B(a) a
#define QSB1C(a) a
#define QSB1D(a) a
#define QSB1E(a) a
#define QSB1F(a) a
#define QSB20(a) a
#define QSB21(a) a
#define QSB22(a) a
#define QSB23(a) a
#define QSB24(a) a
#define QSB25(a) a
#define QSB26(a) a
#define QSB27(a) a
#define QSB28(a) a
#define QSB29(a) a
#define QSB2A(a) a
#define QSB2B(a) a
#define QSB2C(a) a
#define QSB2D(a) a
#define QSB2E(a) a
#define QSB2F(a) a
#define QSB30(a) a
#define QSB31(a) a
#define QSB32(a) a
#define QSB33(a) a
#define QSB34(a) a
#define QSB35(a) a
#define QSB36(a) a
#define QSB37(a) a
#define QSB38(a) a
#define QSB39(a) a
#define QSB3A(a) a
#define QSB3B(a) a
#define QSB3C(a) a
#define QSB3D(a) a
#define QSB3E(a) a
#define QSB3F(a) a
#define QSB40(a) a
#define QSB41(a) a
#define QSB42(a) a
#define QSB43(a) a
#define QSB44(a) a
#define QSB45(a) a
#define QSB46(a) a
#define QSB47(a) a
#define QSB48(a) a
#define QSB49(a) a
#define QSB4A(a) a
#define QSB4B(a) a
#define QSB4C(a) a
#define QSB4D(a) a
#define QSB4E(a) a
#define QSB4F(a) a
#define QSB50(a) a
#define QSB51(a) a
#define QSB52(a) a
#define QSB53(a) a
#define QSB54(a) a
#define QSB55(a) a
#define QSB56(a) a
#define QSB57(a) a
#define QSB58(a) a
#define QSB59(a) a
#define QSB5A(a) a
#define QSB5B(a) a
#define QSB5C(a) a
#define QSB5D(a) a
#define QSB5E(a) a
#define QSB5F(a) a
#define QSB60(a) a
#define QSB61(a) a
#define QSB62(a) a
#define QSB63(a) a
#define QSB64(a) a
#define QSB65(a) a
#define QSB66(a) a
#define QSB67(a) a
#define QSB68(a) a
#define QSB69(a) a
#define QSB6A(a) a
#define QSB6B(a) a
#define QSB6C(a) a
#define QSB6D(a) a
#define QSB6E(a) a
#define QSB6F(a) a
#define QSB70(a) a
#define QSB71(a) a
#define QSB72(a) a
#define QSB73(a) a
#define QSB74(a) a
#define QSB75(a) a
#define QSB76(a) a
#define QSB77(a) a
#define QSB78(a) a
#define QSB79(a) a
#define QSB7A(a) a
#define QSB7B(a) a
#define QSB7C(a) a
#define QSB7D(a) a
#define QSB7E(a) a
#define QSB7F(a) a
#define QSB80(a) a
#define QSB81(a) a
#define QSB82(a) a
#define QSB83(a) a
#define QSB84(a) a
#define QSB85(a) a
#define QSB86(a) a
#define QSB87(a) a
#define QSB88(a) a
#define QSB89(a) a
#define QSB8A(a) a
#define QSB8B(a) a
#define QSB8C(a) a
#define QSB8D(a) a
#define QSB8E(a) a
#define QSB8F(a) a
#define QSB90(a) a
#define QSB91(a) a
#define QSB92(a) a
#define QSB93(a) a
#define QSB94(a) a
#define QSB95(a) a
#define QSB96(a) a
#define QSB97(a) a
#define QSB98(a) a
#define QSB99(a) a
#define QSB9A(a) a
#define QSB9B(a) a
#define QSB9C(a) a
#define QSB9D(a) a
#define QSB9E(a) a
#define QSB9F(a) a
#define QSBA0(a) a
#define QSBA1(a) a
#define QSBA2(a) a
#define QSBA3(a) a
#define QSBA4(a) a
#define QSBA5(a) a
#define QSBA6(a) a
#define QSBA7(a) a
#define QSBA8(a) a
#define QSBA9(a) a
#define QSBAA(a) a
#define QSBAB(a) a
#define QSBAC(a) a
#define QSBAD(a) a
#define QSBAE(a) a
#define QSBAF(a) a
#define QSBB0(a) a
#define QSBB1(a) a
#define QSBB2(a) a
#define QSBB3(a) a
#define QSBB4(a) a
#define QSBB5(a) a
#define QSBB6(a) a
#define QSBB7(a) a
#define QSBB8(a) a
#define QSBB9(a) a
#define QSBBA(a) a
#define QSBBB(a) a
#define QSBBC(a) a
#define QSBBD(a) a
#define QSBBE(a) a
#define QSBBF(a) a
#define QSBC0(a) a
#define QSBC1(a) a
#define QSBC2(a) a
#define QSBC3(a) a
#define QSBC4(a) a
#define QSBC5(a) a
#define QSBC6(a) a
#define QSBC7(a) a
#define QSBC8(a) a
#define QSBC9(a) a
#define QSBCA(a) a
#define QSBCB(a) a
#define QSBCC(a) a
#define QSBCD(a) a
#define QSBCE(a) a
#define QSBCF(a) a
#define QSBD0(a) a
#define QSBD1(a) a
#define QSBD2(a) a
#define QSBD3(a) a
#define QSBD4(a) a
#define QSBD5(a) a
#define QSBD6(a) a
#define QSBD7(a) a
#define QSBD8(a) a
#define QSBD9(a) a
#define QSBDA(a) a
#define QSBDB(a) a
#define QSBDC(a) a
#define QSBDD(a) a
#define QSBDE(a) a
#define QSBDF(a) a
#define QSBE0(a) a
#define QSBE1(a) a
#define QSBE2(a) a
#define QSBE3(a) a
#define QSBE4(a) a
#define QSBE5(a) a
#define QSBE6(a) a
#define QSBE7(a) a
#define QSBE8(a) a
#define QSBE9(a) a
#define QSBEA(a) a
#define QSBEB(a) a
#define QSBEC(a) a
#define QSBED(a) a
#define QSBEE(a) a
#define QSBEF(a) a
#define QSBF0(a) a
#define QSBF1(a) a
#define QSBF2(a) a
#define QSBF3(a) a
#define QSBF4(a) a
#define QSBF5(a) a
#define QSBF6(a) a
#define QSBF7(a) a
#define QSBF8(a) a
#define QSBF9(a) a
#define QSBFA(a) a
#define QSBFB(a) a
#define QSBFC(a) a
#define QSBFD(a) a
#define QSBFE(a) a
#define QSBFF(a) a

#define QSBmn(n,m,a)  QSB##n##m(a)
#define COMMA_IF(n,m) QSBmn(n,m,CQ)




#endif