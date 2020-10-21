#include "IDP.h"

int main (int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Too many argues! Please Add Camera Dev Name!\n");
    }
    else if (2 == argc)
    {
        //IDP: Intrude Detect Program
        IDP _idp(argv[1]);
        _idp.RunIDP();
    }
    else if(argc > 2)
    {
        printf("Too many argues!\n");
    }

}
