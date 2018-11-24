#include <unistd.h> 
#include <fcntl.h> 
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <sys/epoll.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ntrip_caster.h>
#include <ntrip_util.h>


ntrip_caster::ntrip_caster()
{
	m_listen_sock = 0;
	m_epoll_fd = 0;
	m_max_count = 0;
	m_epoll_events = NULL;
}
 
ntrip_caster::~ntrip_caster()
{
	if(m_listen_sock > 0){
		close(m_listen_sock);
	}

	if(m_epoll_fd > 0){
		close(m_epoll_fd);
	}
}
 
bool ntrip_caster::init(int port , int sock_count)
{
	m_max_count = sock_count;
	struct sockaddr_in caster_addr;
	memset(&caster_addr, 0, sizeof(struct sockaddr_in));
	caster_addr.sin_family = AF_INET;
	caster_addr.sin_port = htons(port);
	caster_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	m_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(m_listen_sock == -1) {
		exit(1);
	}

	if(bind(m_listen_sock, (struct sockaddr*)&caster_addr, sizeof(struct sockaddr)) == -1){
		exit(1);
	}

	if(listen(m_listen_sock, 5) == -1){
		exit(1);
	}
											 
	m_epoll_events = new struct epoll_event[sock_count];
	if (m_epoll_events == NULL){
		exit(1);
	}

	m_epoll_fd = epoll_create(sock_count);
	epoll_ops(m_listen_sock, EPOLL_CTL_ADD, EPOLLIN);

	return true;
}
 
bool ntrip_caster::init(const char *ip, int port , int sock_count)
{	
	m_max_count = sock_count;
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(struct sockaddr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(ip);		

	m_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(m_listen_sock == -1){
		exit(1);
	}

	if(bind(m_listen_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) == -1){
		exit(1);
	}
											
	if(listen(m_listen_sock, 5) == -1){
		exit(1);
	}

	m_epoll_events = new struct epoll_event[sock_count];
	if (m_epoll_events == NULL){
		exit(1);
	}

	m_epoll_fd = epoll_create(sock_count);
	epoll_ops(m_listen_sock, EPOLL_CTL_ADD, EPOLLIN);

	return true;
}
 
int ntrip_caster::accept_new_client()
{
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(struct sockaddr_in));
	socklen_t clilen = sizeof(struct sockaddr); 

	int new_sock = accept(m_listen_sock, (struct sockaddr*)&client_addr, &clilen);	
	epoll_ops(new_sock, EPOLL_CTL_ADD, EPOLLIN);
	
	return new_sock;
}
 
int ntrip_caster::recv_data(int sock, char *recv_buf)
{
	char buf[1024] = {0};
	int len = 0;
	int ret = 0;

	while(ret >= 0)
	{
		ret = recv(sock, buf, sizeof(buf), 0);
		if(ret <= 0)
		{
			epoll_ops(sock, EPOLL_CTL_DEL, EPOLLERR);
			close(sock);
			break;
		}else if(ret < 1024){
			memcpy(recv_buf, buf, ret);
			len += ret;	
			break;
		}else{
			memcpy(recv_buf, buf, sizeof(buf));
			len += ret;
		}
	}

	return ret <= 0 ? ret : len;
}
 
int ntrip_caster::send_data(int sock, const char *send_buf, int buf_len)
{
	int len = 0;
	int ret = 0;

	while(len < buf_len){
		if(buf_len < 1024)
			ret = send(sock, send_buf + len, buf_len, 0);
		else
			ret = send(sock, send_buf + len, 1024, 0);
		if(ret <= 0) {
			epoll_ops(sock, EPOLL_CTL_DEL, EPOLLERR);
			close(sock);
			break;
		}else{
			len += ret;
		}

		if(ret < 1024){
			break;
		}
	}

	if(ret > 0){
		epoll_ops(sock, EPOLL_CTL_MOD, EPOLLIN);
	}
		
	return ret <= 0 ? ret : len;
}

int ntrip_caster::ntrip_caster_wait(int time_out)
{
	return epoll_wait(m_epoll_fd, m_epoll_events, m_max_count, time_out);
}
 
void ntrip_caster::run(int time_out)
{
	char *recv_buf = new char[MAX_LEN];
	char *send_buf = new char[MAX_LEN];

	while(1){
		int ret = ntrip_caster_wait(time_out);
		if(ret == 0){
			printf("Time out\n");
			continue;
		}else if(ret == -1){
			printf("Error\n");
		} else{
			for(int i = 0; i < ret; i++){
				if(m_epoll_events[i].data.fd == m_listen_sock){
					/* if the server listens to the EPOLLIN events, accept new client and add this socket to epoll listen list. */
					if(m_epoll_events[i].events & EPOLLIN){
						accept_new_client();
					}
				}else{
					if(m_epoll_events[i].events & EPOLLIN){
						int recv_count = recv_data(m_epoll_events[i].data.fd, recv_buf);
						/* Received count is zero, Client error or disconnect, we need remove this socket form epoll listen list. */
						if(recv_count == 0){
							int sock = m_epoll_events[i].data.fd;
							if(mnt_list.size() > 0){
								for(auto it = mnt_list.begin(); it != mnt_list.end(); ++it){
									if(it->value.server_fd == sock){
										if(it->value.conn_sock.size() > 0){
											for(auto cit = it->value.conn_sock.begin(); 
														cit != it->value.conn_sock.end(); ++cit){
												epoll_ops(*cit, EPOLL_CTL_ADD, EPOLLIN);
												close(*cit);
											}
										}
										mnt_list.earse(it);
										break;
									}
								}
							}

							/* Remove this socket form epoll listen list. */
							if(!epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, sock, &m_epoll_events[i])){
								close(sock);
							}
							continue;
						}

						/* Start parsing received's remote data. */
						parse_data(m_epoll_events[i].data.fd, recv_buf, recv_count);

						memset(recv_buf, 0, strlen(recv_buf));
					}else if(m_epoll_events[i].events & EPOLLOUT){
						int send_count = send_data(m_epoll_events[i].data.fd, send_buf, strlen(send_buf));
						memset(send_buf, 0, send_count);
					}
				}
			}
		}
	}
}

void ntrip_caster::epoll_ops(int sock, int op, uint32_t events)
{
	struct epoll_event ev;

	ev.data.fd = sock;
	ev.events  = events;
	epoll_ctl(m_epoll_fd, op, sock, &ev);
}

int ntrip_caster::parse_data(int sock, char* recv_data, int data_len)
{
	char *temp = recv_data;
	char *result = NULL;
	char m_mnt[16] = {0};
	char m_userpwd[48] = {0};
	char m_username[16] = {0};
	char m_password[16] = {0};

	printf("Received [%d] btyes data:\n", data_len);
	print_char(recv_data, data_len);

	result = strtok(temp, "\n");
	if(result != NULL) {
		/* Server request to connect to Caster. */
		if(!strncasecmp(result, "POST /", 6) && strstr(result, "HTTP/1.1")){
			/* Check mountpoint. */
			sscanf(result, "%*[^/]%*c%[^ ]", m_mnt);
			//printf("MountPoint: [%s]\n", m_mnt);
			if(mnt_list.size() > 0){
				for(auto it = mnt_list.begin(); it != mnt_list.end(); ++it){
					if(mnt_list.size() && !strncmp(it->value.mntname, m_mnt, strlen(m_mnt))){
						printf("MountPoint already used!!!\n");
						send_data(sock, "ERROR - Bad Password\r\n", 22);
						return -1;
					}
				}
			}

			printf("Can use this mount point!!!\n");
			result = strtok(NULL, "\n");
			while(result != NULL) {
				if(!strncasecmp(result, "Authorization: Basic", 20)){
					sscanf(result, "%*[^ ]%*c%*[^ ]%*c%[^\r]", m_userpwd);
					if(strlen(m_userpwd) > 0 ){
						/* Decode username && password. */
						base64_decode(m_userpwd, m_username, m_password);
						//printf("Username: [%s], Password: [%s]\n", m_username, m_password);
						/* Save mountpoint information. */
						struct mountpoint_info *m_mntinfo = new mountpoint_info(sock, m_mnt, m_username, m_password);
						mnt_list.push_back(*m_mntinfo);
						//printf("Add MountPoint ok\n");
						send_data(sock, "ICY 200 OK\r\n", 12);
						return 0;
					}
				}
				result = strtok(NULL, "\n");
			}
		}

		/* Data sent by Server, it needs to be forwarded to connected Client. */
		if(mnt_list.size() > 0){
			for(auto it = mnt_list.begin(); it != mnt_list.end(); ++it){
				if(it->value.server_fd == sock){
					if(it->value.conn_sock.size() > 0){
						for(auto cit = it->value.conn_sock.begin(); 
									cit != it->value.conn_sock.end(); ++cit){
								send_data(*cit, recv_data, data_len);
						}
					}
					return 0;
				}
			}
		}

		/* Client request to get the Source Table or Server's data. */
		if(!strncasecmp(result, "GET /", 5) && (strstr(result, "HTTP/1.1") || strstr(result, "HTTP/1.0"))){
			/* Check mountpoint. */
			sscanf(result, "%*[^/]%*c%[^ ]", m_mnt);
			//printf("MountPoint: [%s]\n", m_mnt);
			/* Request to get the Source Table. */
			if(!strlen( m_mnt)){
				char *st_data = new char[MAX_LEN];
				get_sourcetable(st_data, MAX_LEN);
				send_data(sock, st_data, strlen(st_data));
				return  0;
			}

			/* Request to get the Server's data. */
			struct mountpoint_info *mi_ptr = nullptr;
			if(mnt_list.size() > 0){
				for(auto it = mnt_list.begin(); it != mnt_list.end(); ++it){
					if(!strncmp(it->value.mntname, m_mnt, strlen(m_mnt))){
						mi_ptr = &(it->value);
						break;
					}
				}
			}

			if(mi_ptr == nullptr){
				printf("MountPoint not find!!!\n");
				send_data(sock, "HTTP/1.1 401 Unauthorized\r\n", 27);
				return -1;
			}

			result = strtok(NULL, "\n");
			while(result != NULL) {
				if(!strncasecmp(result, "Authorization: Basic", 20)){
					/* Get username && password. */
					sscanf(result, "%*[^ ]%*c%*[^ ]%*c%[^\r]", m_userpwd);
					if(strlen(m_userpwd) > 0 ){
						/* Decode username && password. */
						base64_decode(m_userpwd, m_username, m_password);
						//printf("Username: [%s], Password: [%s]\n", m_username, m_password);
						/* Check username && password. */
						if(strncmp(mi_ptr->username, m_username, strlen(m_username)) ||
								strncmp(mi_ptr->password, m_password, strlen(m_password))){
							printf("Password error!!!\n");
							send_data(sock, "HTTP/1.1 401 Unauthorized\r\n", 27);
							return  -1;
						}

						mi_ptr->conn_sock.push_back(sock);
						//printf("Client connect ok\n");
						send_data(sock, "ICY 200 OK\r\n", 12);
						return 0;
					}
				}
				result = strtok(NULL, "\n");
			}
		}

		/* If Caster as a Base Station, it needs to get the GGA data sent by the Client*/
		if(!strncasecmp(result, "$GPGGA,", 7)){
			if(!check_sum(result)){
				printf("Check sum pass\n");
				/* do something. */
				return 0;
			}
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	ntrip_caster my_caster;
	my_caster.init(8090, 30);
	//my_epoll.init("127.0.0.1", 8090, 10);
	my_caster.run(2000);

	return  0;
}

