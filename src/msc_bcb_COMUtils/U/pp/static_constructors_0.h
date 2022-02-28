#if (defined _MSC_VER) && (_MSC_VER >= 1200)
#  pragma once
#define __CONDITION_CALL(a)   __if_exists(a) {a() ;}
#else 
#define __CONDITION_CALL(a) a()
#endif



template<class Base, class Child>
struct is_inherited {
	typedef char (&a2)[2];
	typedef char (&a1)[1];

	static a2 f( Base const&);
	static a1 f(...);

	enum {
		val = sizeof(f( *static_cast<Child*>(0) )) == sizeof(a2)
	};
};




template<typename derived_t, typename target_t = derived_t>
class class_initializer
{
    struct helper
    {
        helper()
        {
            __CONDITION_CALL(target_t::static_ctor);
        }

        ~helper()
        {
            __CONDITION_CALL(target_t::static_dtor);
        }
    };

    static helper helper_;

    static void use_helper()
    {
        (void)helper_;
    }

    template<void(*)()>
    struct helper2 {};

    helper2<&class_initializer::use_helper> helper2_;

    virtual void use_helper2()
    {
        (void)helper_;
    }
};

template<typename derived_t, typename target_t>
typename class_initializer<derived_t, target_t>::helper 
    class_initializer<derived_t, target_t>::helper_;



