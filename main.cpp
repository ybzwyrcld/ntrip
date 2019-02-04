#include <ntrip_caster.h>

int main(int argc, char *argv[])
{
	ntrip_caster my_caster;
	my_caster.init(8090, 30);
	//my_epoll.init("127.0.0.1", 8090, 10);
	my_caster.run(2000);

	return  0;
}

