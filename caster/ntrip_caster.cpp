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
#include <time.h>

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
								auto it = mnt_list.begin();
								while(it != mnt_list.end()){
									if(it->value.server_fd == sock){ // is ntrip srever.
										//printf("ntrip server disconnect\n");
										auto cit = it->value.conn_sock.begin();
										while(cit != it->value.conn_sock.end()){
											epoll_ops(*cit, EPOLL_CTL_DEL, EPOLLIN);
											close(*cit);
											it->value.conn_sock.erase(cit);
										}
									
										/* Remove mount point data information from source table list. */	
										std::string str_mntname(it->value.mntname);
										std::string str_string("STR;" + str_mntname + ";" + str_mntname + ";");

										auto str_it = m_str_list.begin();
										while(str_it != m_str_list.end()){
											if(str_it->find(str_string) < str_it->length()){
												m_str_list.erase(str_it);
												break;
											}
											++str_it;
										}

										str_string = "";
										str_mntname = "";
										mnt_list.erase(it);
										break;
									}else{ // is ntrip client.
										bool find_sock = false;
										auto cit = it->value.conn_sock.begin();
										while(cit != it->value.conn_sock.end()){
											if(*cit == sock){
												//printf("ntrip client disconnect\n");
												it->value.conn_sock.erase(cit);
												find_sock = true;
												break;
											}
											++cit;
										}

										if(find_sock == true)
											break;
									}
									++it;
								}
							}

							/* Remove this socket form epoll listen list. */
							if(!epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, sock, &m_epoll_events[i])){
								close(sock);
							}

							//printf("deal disconnect end\n");
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
						break;
					}
				}
				result = strtok(NULL, "\n");
			}

			result = strtok(NULL, "\n");
			/* Check mount point data information, save to source table list. */
			if(!strncasecmp(result, "Ntrip-STR:", 10)){
				char *str_mnt = new char[16];
				char *str_mnt_check = new char[16];
				sscanf(result, "%*[^;]%*c%[^;]%*c%[^;]", str_mnt, str_mnt_check);
				//printf("%s, %s\n", str_mnt, str_mnt_check);
				if(strncmp(m_mnt, str_mnt, strlen(str_mnt)) ||
						strncmp(m_mnt, str_mnt_check, strlen(str_mnt_check))){
					send_data(sock, "ERROR - Bad Password\r\n", 22);
					return -1;
				}
				delete(str_mnt);
				delete(str_mnt_check);
				std::string str_string(result+11);
				str_string += "\n";
				m_str_list.push_back(str_string);
				str_string = "";
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
								//printf("forward %d byte data\n", data_len);
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
				std::string str_string= "";
				for(auto it = m_str_list.begin(); it != m_str_list.end(); ++it){
					str_string.append(*it);
				}
				time_t now;
				struct tm *tm_now;
				char *datetime = new char[128];

				time(&now);
				tm_now = localtime(&now);
				strftime(datetime, 128, "%x %H:%M:%S %Z", tm_now);
				
				snprintf(st_data, MAX_LEN-1,
					"SOURCETABLE 200 OK\r\n"
					"Server: %s\r\n"
					"Content-Type: text/plain\r\n"
					"Content-Length: %d\r\n"
					"Date: %s\r\n"
					"\r\n"
					"%s"
					"ENDSOURCETABLE\r\n",
					caster_agent, strlen(str_string.c_str()), datetime, str_string.c_str());

				send_data(sock, st_data, strlen(st_data));
				delete(st_data);
				delete(datetime);
				str_string= "";
				return 0;
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

