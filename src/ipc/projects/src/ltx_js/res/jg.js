
CP_ACP   =                 0           // default to ANSI code page
CP_OEMCP  =                1           // default to OEM  code page
CP_MACCP   =               2           // default to MAC  code page
CP_THREAD_ACP  =           3           // current thread's ANSI code page
CP_SYMBOL       =          42          // SYMBOL translations

CP_UTF7          =         65000       // UTF-7 translation
CP_UTF8           =        65001       // UTF-8 translation
CP_BIN =-1
HTMLDLG_NOUI                 =    0x0010
HTMLDLG_MODAL                =    0x0020
HTMLDLG_MODELESS             =    0x0040
HTMLDLG_PRINT_TEMPLATE       =    0x0080
HTMLDLG_VERIFY               =    0x0100
HTMLDLG_ALLOW_UNKNOWN_THREAD = 0x0200

M_AC=0x10000000
M_FC= 0x000F0000
M_BC= 0x00F00000
M_CC = (M_BC | M_FC)

KB = 1024;
MB = (1 << 20);
GB = (1 << 30);
TB = KB * (1 << 30);



function safe_call(f, err_val) { try { return f(); } catch (err) { return (err_val instanceof Function) ? err_val() : err_val; } }


function defarg(v, d) {
    return (v) ? v : d;
}

function setdef(o,k, d) {
    return (k in o) ? o[k] : d;
}



function webform(surl, argin, op, hwin, flags) {

    var jchksrv = get_jsvm_old_srv();
    if (jchksrv)
        return jchksrv('webform')(surl, argin, op, hwin, flags);

   var o=surl;   
  if(typeof(o)=='object')
  {
    surl=setdef(o,'url',"about:blank");
    argin=setdef(o,'argin');
    op=setdef(o,'op',"resizable:1; ");
    hwin=setdef(o,'hwin',0);
    flags=setdef(o,'flags',HTMLDLG_MODAL | HTMLDLG_MODELESS | HTMLDLG_ALLOW_UNKNOWN_THREAD | HTMLDLG_VERIFY);    
  }

    var relfs = /^(lfs:)/i; 
    	 if(relfs.test(surl))
	  {
	    var s=surl.replace(relfs,''); 
	     s="http://localhost:7777/::ltxsilk=::/?FD583F3CB33E4226B56558AF1F59C7C3="+self.escape(s);
		 surl=s;
	   
	  }

    var showHtml = bindObject("dll: lib=**; proc=ltx_callback_list::ShowHTML");
    surl || (surl="about:blank");
    flags || (flags = HTMLDLG_MODAL | HTMLDLG_MODELESS | HTMLDLG_ALLOW_UNKNOWN_THREAD | HTMLDLG_VERIFY);
    hwin || (hwin = 0);
    op || (op = "resizable:1; ");
    return showHtml(surl, op, argin, hwin, flags);

}

function jsedit(hwnd) {
    var url = bindObject('fileSearch')('**/../../plugins/codemirror-3.12/demo/complete.hta'),
    op = "resizable:1; dialogWidth:900px ; ",
    flags = HTMLDLG_MODELESS | HTMLDLG_ALLOW_UNKNOWN_THREAD | HTMLDLG_VERIFY;
    return webform(url, externRef(), op,hwnd,flags);

}

var __file_search__;

function fileSearch(fn, path, ops, ext) {
    __file_search__ || (__file_search__ = bindObject("fileSearch"));
    return __file_search__(fn, path, ops, ext);
}

function get_console_hwnd() {
    var c = bindObject("dll: lib=**;proc=ltx_callback_list::win_console");
    return c(4);
}


function editor(o) {

    o || (o = {});
    try
    {
    
    

    var hwnd = o.hwnd, fs = bindObject('fileSearch'), os = { script: externRef() };
    if (o.file) os.txt = load_string(os.file=fs(o.file));
    else os.txt = o.txt;    
    
    
        
    var url = fs('**/../../plugins/codemirror-3.12/demo/complete.hta'),
    op =(o.options)? o.options:"resizable:1; dialogWidth:900px ; ",
    flags = (o.modal) ? (HTMLDLG_MODAL | HTMLDLG_ALLOW_UNKNOWN_THREAD | HTMLDLG_VERIFY) : (HTMLDLG_MODELESS | HTMLDLG_ALLOW_UNKNOWN_THREAD | HTMLDLG_VERIFY);

    os.reload=function reload() {
        if (os.file) return load_string(os.file);
        else return os.txt;
    }

    os.save = function save(s) {
    if (o.writable && os.file) {

        save_string(s, os.file); return true; 
         }
        
    
     }

     //hwnd || (hwnd = get_console_hwnd());
    
    return webform(url, os, op, hwnd, flags);
    }
    finally {
       // delete os;
    }
    

}


function save_string(s, fn, flags) {

    var FSO = bindObject('progid:Scripting.FileSystemObject'),
      TextStream;

    if (flags) {

        var File = FSO.GetFile(fn);
        
        TextStream = File.OpenAsTextStream(1);
    }

    else TextStream = FSO.CreateTextFile(fn, true);
    try {

        TextStream.Write(s);
    } finally {

        TextStream.Close();
    }
}

function load_string(fn)
{
    var FSO = bindObject('progid:Scripting.FileSystemObject');
    fn = fileSearch(fn);
    var File = FSO.GetFile(fn);
    try {
        var TextStream = File.OpenAsTextStream(1);
        var s = TextStream.ReadAll();
    } finally {
        TextStream.Close();
    }
     return s;	   
}

function $import (fn, path,fg) {

    if (typeof (fn) == 'object') {
        var o = fn;
        fn = o.fn;
        path = setdef(o,'path',path);
        fg = setdef(o, 'fg', fg);
    }


    fn = bindObject("fileSearch:")(fn, path);
    var s = load_string(fn);
    if (fg) return globalEval(s);
    else return eval(s);          
}

tempfilename=function()
{
    var shell = bindObject("progid:WScript.Shell");
    var fso = bindObject("progid:Scripting.FileSystemObject");
     return shell.ExpandEnvironmentStrings('%temp%')+'\\'+fso.GetTempName();
}

delete_file=function(fn,force)
{                                         
    var fso = bindObject("progid:Scripting.FileSystemObject");
     return fso.DeleteFile(fn,(force)?true:false);
}



getenv = function (key, level) {
    
    if (!level) level = "Process";

    return bindObject('progid:WScript.Shell').Environment(level)(key);
}

setenv = function(ko, val, level) {

    if (!level) level = "Process";
    var wsh = bindObject('progid:WScript.Shell');

    var envp = wsh.Environment(level);

    if (typeof (ko) == 'object') {
        for (k in ko) {
            if (ko.hasOwnProperty(k)) {
                var v = ko[k];                
                envp(k) = v?(v+''):'';
            }


        }
    }
    else envp(ko) = val;

}





function alert(txt, titl, ty, tim) {
    titl = defarg(titl, "");
    ty = defarg(ty, 0 + 64);
     tim = defarg(tim, -1);
    return bindObject('progid:wscript.shell').Popup(txt, tim, titl, ty);

}






    function vsprintf(args) {

        function str_repeat(i, m) { for (var o = []; m > 0; o[--m] = i); return (o.join('')); }

        if (args.length == 0) return '';
        else if (args.length == 1) return args[0].toString();
        
        var i = 0, a, f = args[i++], o = [], m, p, c, x;
        while (f) {
            if (m = /^[^\x25]+/.exec(f)) o.push(m[0]);
            else if (m = /^\x25{2}/.exec(f)) o.push('%');
            else if (m = /^\x25(?:(\d+)\$)?(\+)?(0|'[^$])?(-)?(\d+)?(?:\.(\d+))?([b-fosuxX])/.exec(f)) {
                if (((a = args[m[1] || i++]) == null) || (a == undefined)) throw ("Too few arguments.");
                if (/[^s]/.test(m[7]) && (typeof (a) != 'number'))
                    throw ("Expecting number but found " + typeof (a));
                switch (m[7]) {
                    case 'b': a = a.toString(2); break;
                    case 'c': a = String.fromCharCode(a); break;
                    case 'd': a = parseInt(a); break;
                    case 'e': a = m[6] ? a.toExponential(m[6]) : a.toExponential(); break;
                    case 'f': a = m[6] ? parseFloat(a).toFixed(m[6]) : parseFloat(a); break;
                    case 'o': a = a.toString(8); break;
                    case 's': a = ((a = String(a)) && m[6] ? a.substring(0, m[6]) : a); break;
                    case 'u': a = Math.abs(a); break;
                    case 'x': a = a.toString(16); break;
                    case 'X': a = a.toString(16).toUpperCase(); break;
                }
                a = (/[def]/.test(m[7]) && m[2] && a > 0 ? '+' + a : a);
                c = m[3] ? m[3] == '0' ? '0' : m[3].charAt(1) : ' ';
                x = m[5] - String(a).length;
                p = m[5] ? str_repeat(c, x) : '';
                o.push(m[4] ? a + p : p + a);
            }
            else throw ("Huh ?!");
            f = f.substring(m[0].length);
        }
        return o.join('');
    }

    var __console__ = bindObject("dll: lib=**; proc=ltx_callback_list::win_console");
    
    function sprintf() {
        return vsprintf(arguments);
    }

    function cprintf() {
        var args = Array.apply(null, arguments),
            attr = args.shift();
        __console__(0, vsprintf(args), attr);
    }
    function printf() {
        
        __console__(0, vsprintf(arguments));
    }
    function dbgf_printf() {
        var args = Array.apply(null, arguments),
            force = args.shift();
        __console__(128, vsprintf(args),force);
    }
    function dbg_printf() {
    __console__(128, vsprintf(arguments), 1);
    }

function writeln() { __console__(0, Array.apply(null, arguments).join(', ') + '\n'); }

function readln() { return __console__(48); }

 function clipbrd(s) {
    var c = bindObject('module: lib=**;proc=ltx_callback_list::clipbrd');
    return (arguments.length) ? c(s) : c();
 }






    function cls() {
        __console__(64, 'cls');
    }
    function cmd(s) {
        __console__(64, s);
    }

    function dump(o, fall) {

        var s = "**dump**type:" + typeof (o) + "\n",k;

        try {
            k = o.hasOwnProperty('');
        }
        catch (e) { fall = true }

        for ( k in o) {
            if ((!fall) && (!o.hasOwnProperty(k))) continue;
            var sn = typeof (o[k]);
            if (sn == 'number') {

                s += sprintf("'%s':[number] %f \n", k, o[k]);
            }
            else
                if (sn == 'string') {
                var ss = o[k];
                var n = ss.length;
                if (n > 20) {
                    ss = '"' + ss.substr(0, 20) + "...";

                }
                else
                    ss = '"' + ss + '"';

                s += sprintf("'%s':[string]:%s length=%d\n", k, ss, n);
            }
            else s += sprintf("'%s':[%s]\n", k, typeof (o[k]));
        };
        return s;
    }

    var dir = dump;


    function Tic() {
        function gettime() { return (new Date).getTime() }
        var t = gettime();
        this.start = function() { return t = gettime(); }
        this.ms = function() { return gettime() - t; }
        this.sec = function() { return (gettime() - t) / 1000; }
    }

   global.Tic = Tic;


 (function () {

    var _tic = new Tic();
     var ts = function () { return _tic.start(); },
         te = function () { return _tic.sec(); };
     global.tic =ts 
     global.tic.start = ts;
     global.toc = te;
     global.tic.sec=global.toc.sec = te;
     global.tic.ms = global.toc.ms = function () { return _tic.ms(); };

})()

    function object_to_zz(o, delim) {
        var dlm = (delim) ? delim : '\0';

        function constructor_name(o) {
            return o.constructor.toString().split("(")[0].split(/function\s*/)[1];
        }

        function is_self_prop(o, k) {
            if (("hasOwnProperty" in o) && (!o.hasOwnProperty(k)))
                return false
            else return true
        }
        function _to_zz(o, prfx) {
            var s = "";

            //prfx||prfx="";

            for (var k in o)
                if (is_self_prop(o, k)) {
                var i = o[k];
                if (typeof (i) == 'object') {
                    if (constructor_name(i) == 'Array')
                        s += prfx + k + "=[" + i + "]" + dlm;
                    else {
                        s += _to_zz(i, prfx + k + '.');
                    }
                }
                else {
                    s += prfx + k + "=" + i + dlm;
                    var lp = prfx.length
                    if (lp && (k == ''))
                        s += prfx.substr(0, lp - 1) + k + "=" + i + dlm;

                }
            }

            return s;

        }

        return _to_zz(o, "") + dlm;
    }



    function get_jsvm_old_srv() {
         
        if (typeof global.__jsvm_old__=='object') {
            var o = global.__jsvm_old__;
            if (!o.srv) {
                o.job = require('job').JOB.create();
                o.srv = bindObject('srv:[job=#1]:script:lang=JScript', o.job);
            }           
            
            return o.srv;           
       }
       
     }


    if (typeof ActiveXObject === 'undefined') {

        global.__jsvm_old__ = {}; 
        global.ActiveXObject = function ltx_ActiveXObject(s) { return bindObject('progid:' + s); }
        global.GetObject = function ltx_GetObject(s) { return bindObject('::'+s); }

        __jsvm_old__.local = bindObject('script:lang=JScript');
        
        global.Enumerator = function ax_Enumerator(c) { return __jsvm_old__.local('new Enumerator($$[0])', c); }
        global.setenv = __jsvm_old__.local('setenv');
        
        //global.__jsvm_old_srv_job = require('job').JOB.create();        
        //global.__jsvm_old_srv = bindObject('srv:[job=#1]:script:lang=JScript', global.__jsvm_old_srv_job);        
        //global.webform = __jsvm_old_srv__('webform');

    }
