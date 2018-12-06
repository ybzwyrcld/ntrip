#ifndef __NTRIP_MOUNTPOINT_H__
#define __NTRIP_MOUNTPOINT_H__

#include <vector>
#include <string.h>

struct mountpoint_info {
	int server_fd;
	char mntname[16] = {0};
	char username[16] = {0};
	char password[16] = {0};
	std::vector<int> conn_sock;

	mountpoint_info() {
		server_fd = 0;
	}

	mountpoint_info(struct mountpoint_info &mi) : server_fd(mi.server_fd) {
		memcpy(mntname, mi.mntname, strlen(mi.mntname));
		memcpy(username, mi.username, strlen(mi.username));
		memcpy(password, mi.password, strlen(mi.password));
	}

	mountpoint_info(int m_server_fd, char m_mntname[], char m_username[], char m_password[]) : 
			server_fd(m_server_fd) {
		memcpy(mntname, m_mntname, strlen(m_mntname));
		memcpy(username, m_username, strlen(m_username));
		memcpy(password, m_password, strlen(m_password));
	}
};

#endif //__NTRIP_MOUNTPOINT_H__
