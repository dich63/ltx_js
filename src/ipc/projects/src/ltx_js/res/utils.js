(function() {

    var __require_file_global_cache__ = {},
         __file_search__, __module_path__;

    function fileSearch(fn, path, ops) {
        __file_search__ || (__file_search__ = bindObject("fileSearch"));
        return __file_search__(fn, path, ops);
    }
    function get_modules_path() {
        return __module_path__="**/../ltx.modules;"
    }


    var __require_local_cache__ = {};



    function _require(filename) {
        var __require_local_cache__ = {};

        function Module(filename) {

        }
        



    }



    this.require = function Require(filename) { return _require(filename) }


})();    