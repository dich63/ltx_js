///

//alert('aaaaaaaaaa');
var aa = new Array(1024 * 1024);
function test_write_array(o,N) {

    var len = o(), sum = 0;
    len = ((N) && (N <= len)) ? N : len;
    
    for (var k = 0; k < len; k++) {
        o(k) = k;
        //aa[k] = k;
        sum += k;
    }
    return sum;

}

function test_read_array(o,N) {

    var len = o(), sum = 0;
    len = ((N) && (N <= len)) ? N : len;
    for (var k = 0; k < len; k++)
        //sum += aa[k];
          sum+=o(k) ;
    return sum;

}

function show_html(ox, oy, url) {

    var obj = { x: ox, y: oy },
    opts = "resizable:1; dialogWidth:'700pt' ; dialogHeight:'400pt'; unadorned:0;";
    var flags = HTMLDLG_MODAL// | HTMLDLG_MODELESS;// | HTMLDLG_ALLOW_UNKNOWN_THREAD | HTMLDLG_VERIFY;
    url = GetObject("ltx.bind:fileSearch:")(url);
    return webform(url, obj, opts, 0,flags);    

}


