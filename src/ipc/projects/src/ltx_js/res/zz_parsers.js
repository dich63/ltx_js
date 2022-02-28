js_text(
this.exports||(exports={});

exports.replace_delim=function replace_delim(s,ds,dz)
{
           ds||(ds=';');
           dz||(dz='\0');
		   return s.split(ds).join(dz);
		   
}
exports.to_obj=function to_obj(s)
{
  return eval('['+s+'][0]');
};

exports.obj_to_zz=function obj_to_zz(o,dz)
{
    dz||(dz='\0');
function k_to_zz(kk,o,dz)
{

        
if(typeof(o)!='object')
{
   return kk+'='+o+dz;
};

    var s;
if( o instanceof Array)
 {
    s='[';

  for(    var i=0;i<1;i++) s+=o[i];
  for(var i=1;i<o.length;i++) s+=','+o[i];
   s+=']'+dz;
  return kk+'='+s;
 };
  s='';
  for(var k in o)
   if(o.hasOwnProperty(k)){

      var c=(k)?kk+'.'+k:kk;                 
       s+=k_to_zz(c,o[k],dz);
     }
    return s+dz;

};

 
  var s='';
  for(var k in o)
   if(o.hasOwnProperty(k)){
   s+=k_to_zz(k,o[k],dz); 
  }
   return s;
 
};

exports.zz_to_obj=function zz_to_obj(s,dz)
{
     

    function trim(s) {
    	return s.replace(/^\s+|\s+$/g,"");
	}

	function val_set(val)
	{
	  if( typeof(val)=='string' )
	   {
	     val=trim(val);
	     var l=val.length;
	    if((l>3)&&(val.substr(0,1)=='[')&&(val.substr(l-1,1)==']'))
         {
		    val=val.substr(1,l-2).split(',');
            for(var k=0;k<val.length;k++) val[k]=trim(val[k]);   			
         }		 
	   }
	   return val;
	}
	
    function check_obj(o)
	{
	  if(o instanceof Array) return false;	  
	  return typeof(o)==='object';
	}
	
   function set_val(o,key,keys,val)
   {
    
     if(keys.length)
	 {
	   var ok; 
	   if(key in o)
	   {
	                   		 
		if(!check_obj(o[key]))
         {  		    
		   o[key]={'':o[key]};		   
		 }
	    } else 
		     o[key]={};
		 
	      set_val(o[key],keys.shift(),keys,val);
	   
	 }
	 else
	 {
	    val=val_set(val);
	   if(check_obj(o[key])) o[key]['']=val;
	    else o[key]=val;
	 }
   }

   dz||(dz='\0');
   var re=/\=/,o={},sa,a,key,val;
   sa=s.split(dz);

   for(var k =0;k<sa.length;k++){
        if(sa[k].match(re))
		  {
		    key=RegExp.leftContext;
		    val=RegExp.rightContext;
			keys=key.split('.');
			key=keys.shift();
			set_val(o,key,keys,val);			
		  }
		
    }	
   return o;
};
);