#pragma once

	struct i_i_t
	{
		typedef i_i_t* (__stdcall *next_proc_type)(i_i_t* );
		next_proc_type pnext;
		inline i_i_t* do_next()
		{
			if(pnext) return pnext(this);
			else  return NULL;
		}

		inline i_i_t* do_next_s()
		{
			if((this)&&(pnext)) return pnext(this);
			else  return NULL;
		}
		inline next_proc_type set_proc(void* p)
		{
			next_proc_type t=pnext;
			pnext=(next_proc_type)p;
			return t;
		}

		template <typename N>
		inline next_proc_type set_procT(N n)
		{
			union
			{
				N n;
				void* p;
			} unp;
			unp.n=n;
			return set_proc(unp.p);			
		}
		inline void set_end()
		{
			set_proc(NULL);
		}
		inline next_proc_type proc()   
		{
			return pnext;
		}

		inline void for_each()   
		{
			i_i_t* p=this;
			while(p) p=p->do_next();
		}

static	inline void for_each(void* p)
		{
			((i_i_t*)p)->for_each();
		}

	};

