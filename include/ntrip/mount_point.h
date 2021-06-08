// Copyright 2019 Yuming Meng. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef NTRIPLIB_MOUNT_POINT_H_
#define NTRIPLIB_MOUNT_POINT_H_

#include <list>


struct MountPoint {
  int server_fd;
  char username[16];
  char password[16];
  char mount_point_name[16];
  std::list<int> client_socket_list;
};

#endif  // NTRIPLIB_MOUNT_POINT_H_
