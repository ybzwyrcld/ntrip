#ifndef __NTRIP_CASTER_H__
#define __NTRIP_CASTER_H__

#define MAX_LEN 2048
#define MAX_CONN 8

struct mnt_info {
	int server_fd;
	int current_conn_count;
	int current_conn_cursor;
	char mntname[16];
	char username[16];
	char password[16];
	int	conn_sock[MAX_CONN];
	unsigned int send_flags;
};

struct mnt_list {
	struct mnt_info *mi;
	struct mnt_list *next;
};

class ntrip_caster {
public:
	ntrip_caster();
	virtual ~ntrip_caster();
	bool init(int port, int sock_count);
	bool init(const char *ip, int port, int sock_count);
	int ntrip_caster_wait(int time_out);
	int accept_new_client();
	int recv_data(int sock, char *recv_buf);
	int send_data(int sock, const char *send_buf, int buf_len);
	void run(int time_out);
	void epoll_ops(int sock, int op, uint32_t events);
	int add_conn(struct mnt_info *mnt_node, int client_sock);
	int del_conn(struct mnt_info *mnt_node);
	struct mnt_info *check_conn(int client_sock);
	int add_mntpoint(int server_sock, const char *mnt_name, const char *usr, const char *pwd);
	int del_mntpoint(struct mnt_info *mnt_node);
	struct mnt_info *check_mntpoint(int server_sock, const char *mnt_name);
	int parse_data(int sock, char* recv_buf, int len);

private:
	int m_listen_sock;
	int m_epoll_fd;
	int m_max_count;
	struct epoll_event *m_epoll_events;
	struct mnt_list *mntlist_head;
};

#endif //__NTRIP_CASTER_H__
