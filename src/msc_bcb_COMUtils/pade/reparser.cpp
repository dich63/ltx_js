
#include <iostream>


inline bool isnumdig(char c) {
    return isdigit(c) || (c == '.') || (c == '-') || (c == '+') || (c == 'e') || (c == 'E');
}

void _convert(char* src, char* dest) {
    char* ps = src, * pd = dest;
    int brstack = 0;
    bool fd = 0, fld = 0;
    while (*ps) {



        if (*ps == '(') {

            ++brstack;
            *(pd++) = *(ps++); fld = 0; continue;
        }
        else if (*ps == ')') {

            if (fld) {
                *(pd++) = ')';
            }
            fld = 0;
            --brstack;
            *(pd++) = *(ps++); fld = 0; continue;
        }


        bool f = (brstack == 3);
        if (!f) {
            *(pd++) = *(ps++); fld = 0; continue;
        }

        fd = isnumdig(*ps);
        if ((fd) && (!fld)) {

            *(pd++) = '_';
            *(pd++) = 'S';
            *(pd++) = '(';

        }

        if ((!fd) && (fld)) {
            *(pd++) = ')';
        }




        fld = fd;
        *(pd++) = *(ps++);
    }

    *pd = 0;


}




int main(int argc,char** argv)
{
    size_t sz = 1024 * 1024 * 32;
    char* s_in =(char*) ::calloc(1,sz);
    char* s_out = (char*) ::calloc(2, sz);


    FILE* fin;
    if (!(fin = fopen(argv[1], "rb")))
        return -1;

    
    fread(s_in,1, sz,fin);
    _convert(s_in, s_out);
    std::cout << s_out << "";

    
    return 0;
}

