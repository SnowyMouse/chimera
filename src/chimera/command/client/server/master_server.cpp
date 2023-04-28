#include "../../../output/output.hpp"
#include "../../../chimera.hpp"
#include "../../../master_server/master_server.hpp"

namespace Chimera {
    bool master_server_command(int argc, const char **argv) {
        // If < 4, copy each thing up
        if(argc == 1) {
            int new_argc = 4;
            const char *new_argv[] = {
                argv[0],
                argv[0],
                argv[0],
                argv[0]
            };
            return master_server_command(new_argc, new_argv);
        }
        if(argc == 2) {
            int new_argc = 4;
            const char *new_argv[] = {
                argv[0],
                argv[1],
                argv[1],
                argv[1]
            };
            return master_server_command(new_argc, new_argv);
        }
        if(argc == 3) {
            int new_argc = 4;
            const char *new_argv[] = {
                argv[0],
                argv[1],
                argv[2],
                argv[2]
            };
            return master_server_command(new_argc, new_argv);
        }

        const char *list, *key, *natneg1, *natneg2;

        if(argc == 4) {
            list = argv[0];
            key = argv[0];
            natneg1 = argv[0];
            natneg2 = argv[0];

            if(list[0] == 0) {
                list = nullptr;
            }

            if(key[0] == 0) {
                key = nullptr;
            }

            if(natneg1[0] == 0) {
                natneg1 = nullptr;
            }

            if(natneg2[0] == 0) {
                natneg2 = nullptr;
            }

            set_master_server(list, key, natneg1, natneg2);
        }

        get_master_server(&list, &key, &natneg1, &natneg2);
        console_output("%s, %s, %s, %s", list, key, natneg1, natneg2);
        return true;
    }
}


