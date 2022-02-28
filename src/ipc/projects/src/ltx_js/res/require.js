// resource=require.js

(function() {

// Object.prototype.eval = function evalObject(s) { return eval(s); }



    var __require_file_global_cache__ = {},
         __file_search__, __module_path__ = ";**/../../ltx.modules;", FSO;

    function fileSearch(fn, path, ops, ext) {
        __file_search__ || (__file_search__ = bindObject("fileSearch"));
        return __file_search__(fn, path, ops, ext);
    }

    function fs_triplet(fn, path, ext) {
        return fileSearch(fn, path, 0x32, ext).split('*');
    }


    function load_string(fn) {
        FSO || (FSO = new ActiveXObject('Scripting.FileSystemObject'));
        var File = FSO.GetFile(fn);
        try {
            var TextStream = File.OpenAsTextStream(1);
            var s = TextStream.ReadAll();
        } finally {
            TextStream.Close();
        }
        return s;
    }

    function getenv(key) {
        return (new ActiveXObject('WScript.Shell')).Environment("Process")(key);
    }

    function get_modules_path() {

        return __module_path__;
    }


    function _add_module_path(p, f) {


        var path = __module_path__;
        var pp = path.split(";");
        p = fileSearch(p);
        var fp = p.toLowerCase();
        var i;
        for (i = 0; i < pp.length; i++) {
            if (pp[i].toLowerCase() == fp) {
                if (!f) return 0;
                break;
            }
        }
        pp.splice(i, 1);
        if (f > 0) pp.push(p);
        else pp.unshift(p);
        path = pp.join(';');
        __module_path__ = path;
        return f;
    }







    //var __require_local_cache__ = {};

    function __append_object__(o, a) {
        if (a) {

            /*
            var fall = true;
            
            try {
            k = a.hasOwnProperty('');
            }
            
            catch (e) { fall = false }
            */

            for (var k in o) {
                if (o.hasOwnProperty(k))
                    a[k] = o[k];
            }
            return a;
        }
        else return o;
    }


    function Module(fn, triplet, _path) {


        var __require_local_cache__ = {}, __fn__ = fn, __triplet__ = (triplet) ? triplet : ['', '', ''],
        __path__ = (_path) ? _path : get_modules_path();



        function _require(name, _exports) {

            var exports = {};
            try {


                if (__require_local_cache__[name]) return __append_object__(__require_local_cache__[name].exports, _exports);

                //var t = fs_triplet(name, __triplet__[0] + get_modules_path());
                var t = fs_triplet(name, __path__, '.js');
                var __script_triplet__ = t;


                if (!__require_file_global_cache__[t[2]]) {
                    var module = new Module(name, t, t[0] + ';' + __path__);
                    module.parent = this;
                    __require_local_cache__[name] = __require_file_global_cache__[t[2]] = { exports: exports, triplet: t };
                    ;


                    module.__path__ = __path__;
                    module.location = t[0] + t[1];


                    module.load_text = function load_text(s) {
                        var ss = fs_triplet(s, __path__);
                        return load_string(ss[0] + ss[1]);
                    };

                    module.importScript = function ImportScript(s) {
                        var ss = fs_triplet(s, __path__);
                        s = load_string(ss[0] + ss[1]);
                        return eval(s);
                    };

                    /*
                    module.importScript = function (s) {
                    eval(module.load_text(s));
                    }
                    */

                    //  module.imp = function(s){}


                    (function(s) {

                        try {
                            var require = module.require, t;
                            //exports.eval(s);
                            exports.__eval__00 = function evalObject(s) { return eval(s); }
                            exports.__eval__00(s);
                            delete exports.__eval__00;
                        }
                        catch (e) {
                            //__require_file_global_cache__ = {};
                            //__require_local_cache__ = {};
                            //root_require.reset_cache();

                            throw e;
                        }


                    })(load_string(t[0] + t[1]));

                    __require_local_cache__[name] = __require_file_global_cache__[t[2]] = { exports: exports, triplet: t };
                    return __append_object__(exports, _exports);



                }
                else {
                    return __append_object__((__require_local_cache__[name] = __require_file_global_cache__[t[2]]).exports, _exports);
                }

            } catch (e) {
                //__require_file_global_cache__ = {};
                //__require_local_cache__ = {};
                root_require.reset_cache();

                throw e;
            }




        }

        var require = function module_require(name, exports) { return _require(name, exports); }
        require.resolve = function() { return __triplet__; }
        require.add_to_module_path = _add_module_path;
        require.import2global = import2global;
        this.require = require;
        this.set_module_path = function set_module_path(p) { __path__ = p; }

    }

    function _reset_cache_(fnoc) {
        var r = __require_file_global_cache__;
        if (!fnoc) {
            __require_file_global_cache__ = {};
            __root_module__ = null;
        }
        return r;
    }
    function get_root_module() { return (__root_module__) || (__root_module__ = new Module()); }
    function root_require(filename, exports, f_reset_cache) {
        
        if (f_reset_cache) {  _reset_cache_(); }

        return get_root_module().require(filename, exports);
    }

    function import2global() {

        for (var k = 0; k < arguments.length; k++) {
            root_require(arguments[k], global);
        }
        return import2global;
    }
    //global.import2global = import2global;
    root_require.import2global = root_require.import2root = import2global;
    root_require.toString = function () { return 'Usage:\n [exports =] require ( ModuleName[.js], [exports], [reset_cache])';}
    root_require.resolve = function() { return ['', '', '']; }
    root_require.reset=root_require.reset_cache = function ResetGlobalCache(fNoClear) { return _reset_cache_(fNoClear); }
    root_require.attach_module_path=root_require.add_to_module_path = function(p, f) {
        _add_module_path(p, f);
        get_root_module().set_module_path(__module_path__);
        return root_require;
    };

    root_require.module_path = function () { return __module_path__; }

    root_require.__set_module_path__ = function (NewPath) { get_root_module().set_module_path(__module_path__ = NewPath + ';'); return root_require;  }

    var __root_module__, os = { fs: fileSearch, fileSearch: fileSearch, fs_triplet: fs_triplet, ltx_path: get_modules_path }
    //this.require = root_require;
    this.os = os;

    return root_require;
})();    

