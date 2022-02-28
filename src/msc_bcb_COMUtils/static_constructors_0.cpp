
template<typename derived_t, typename target_t = derived_t>
class class_initializer
{
    struct helper
    {
        helper()
        {
            target_t::static_ctor();
        }

        ~helper()
        {
            target_t::static_dtor();
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



struct A : class_initializer<A>
{
    static void static_ctor()
    {
        std::cout << __FUNCTION__ << std::endl;
    }

    static void static_dtor()
    {
        std::cout << __FUNCTION__ << std::endl;
    }
};

struct B : class_initializer<B>
{
    static void static_ctor()
    {
        std::cout << __FUNCTION__ << std::endl;
    }

    static void static_dtor()
    {
        std::cout << __FUNCTION__ << std::endl;
    }
};

int main()
{
    std::cout << __FUNCTION__ << std::endl;
}




