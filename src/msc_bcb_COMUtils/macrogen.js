// macrogen.js
//exampLe 0-64 args: cscript macrogen.js //nologo 64  >file.h 

var sbuf=""
function print(s)
{
//
sbuf+=s;
WScript.Echo(s)   
}


WshArg = WScript.Arguments
n=255
if(WshArg.Length>0) 
{
    n=WshArg.Item(0);

}

if(n>255){ n=255};

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
 
//htm = new ActiveXObject("htmlfile"); 
//htm.ParentWindow.ClipboardData.setData("TEXT",sbuf);
//WshExtra = new ActiveXObject("WshExtra.Clipboard");
//WshExtra.Copy(sbuf);
