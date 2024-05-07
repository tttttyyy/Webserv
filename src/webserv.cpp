#include "Libs.hpp"
#include "ServerManager.hpp"
#include "Cgi.hpp"

// 2130706433
//tcp pton chi ogtagorcvum
//inet_ntoa erevi saya
//getNIp chi ogtagorcvum
//getNPort chi ogtagorcvum
// linux
// website ashxatacnelu dzev
//check comments
int main(int ac, char **av)
{
    try
    {
        ServerManager mgn(ac == 2 ? av[1] : DFLT);
        std::cout << "mgn.size() = " << mgn.size() << std::endl;
        for (size_t i = 0; i < mgn.size(); i++)
        {
            mgn[i]->up();
        }
        mgn.start();
    }
    catch(std::exception const &e)
    {
        std::cerr << e.what() << std::endl;
    }
}
