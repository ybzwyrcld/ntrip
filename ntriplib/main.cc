#include "caster/ntrip_caster.h"

int main(int argc, char *argv[]) {
  NtripCaster my_caster;
  my_caster.Init(8090, 30);
  //my_caster.Init("127.0.0.1", 8090, 10);
  my_caster.Run(2000);

  return 0;
}

