ffosh:ffosh.cpp ffosh.h
	g++ -o ffosh -ggdb3 -Wall -Werror -pedantic -std=gnu++11 ffosh.cpp
test:test.cpp
	g++ -o test -ggdb3 -Wall -Werror -pedantic -std=gnu++98 test.cpp
sg_test:sg_test.cpp
	g++ -o sg_test -ggdb3 -Wall -Werror -pedantic -std=gnu++98 sg_test.cpp
fail_test:fail_test.cpp
	g++ -o fail_test -ggdb3 -Wall -Werror -pedantic -std=gnu++98 fail_test.cpp

envp_test:envp_test.cpp
	g++ -o envp_test -ggdb3 -Wall -Werror -pedantic -std=gnu++98 envp_test.cpp

print_test:print_test.cpp
	g++ -o print_test -ggdb3 -Wall -Werror -pedantic -std=gnu++98 print_test.cpp
