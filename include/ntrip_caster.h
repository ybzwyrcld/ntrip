#ifndef __NTRIP_CASTER_H__
#define __NTRIP_CASTER_H__

#define MAX_LEN 2048

/* c++ standard libary. */
#include <string>

#include <ntrip_mountpoint.h>

template<typename T>
struct list_node {
	T value;
	list_node *next;
	list_node *prev;

	list_node() {
		next = nullptr;
		prev = nullptr;
	}

	list_node(T v, list_node *n = nullptr, list_node *p = nullptr) : 
			value(v), next(n), prev(p) {
	}
};

template<typename T>
class list {
public:
	list(){
		m_ptr_head = nullptr;
		m_ptr_tail = nullptr;
		m_size = 0;
	}

	/* insert element from the end of the list. */
	void push_back(T value) {
		if (m_ptr_head == nullptr) {
			m_ptr_head = new list_node<T>(value);
			m_ptr_tail = m_ptr_head;
		} else {
			m_ptr_tail->next = new list_node<T>(value);
			m_ptr_tail->next->prev = m_ptr_tail;
			m_ptr_tail = m_ptr_tail->next;
		}
		++m_size;
	}

	/* built in iterator. */
	class iterator {
	private:
		list_node<T> *m_ptr;
		friend class list;
	public:
		iterator(list_node<T>* p = nullptr) : m_ptr(p) {

		}

		T operator*() const {
			return m_ptr->value;
		}

		list_node<T>* operator->() const {
			return m_ptr;
		}

		iterator& operator++() {
			m_ptr = m_ptr->next;
			return *this;
		}

		iterator operator++(int) {
			list_node<T>* tmp = m_ptr;
			m_ptr = m_ptr->next;
			return iterator(tmp);
		}

		bool operator==(const iterator &arg) const {
			return arg.m_ptr == this->m_ptr;
		}

		bool operator!=(const iterator &arg) const {
			return arg.m_ptr != this->m_ptr;
		}
	};

	/* return list header pointer. */
	iterator begin() const {
		return iterator(m_ptr_head);
	}

	/* return list tail pointer. */
	iterator end() const {
		return iterator(m_ptr_tail->next);
	}

	/* delete list element. */
	iterator erase(iterator itr) {
		list_node<T>* ptr_node = itr.m_ptr;
		iterator ret = ++itr;
		if (m_size > 1) {
			if (ptr_node == m_ptr_head) {
				m_ptr_head = ptr_node->next;
			} else {
				ptr_node->prev->next = ptr_node->next;
			}

			if (ptr_node == m_ptr_tail) {
				m_ptr_tail = ptr_node->prev;
			} else {
				ptr_node->next->prev = ptr_node->prev;
			}
		}else{
			m_ptr_head = nullptr;
			m_ptr_tail = m_ptr_head;
		}
		delete ptr_node;
		m_size--;
		return ret;	
	}

	/* return list elements count. */
	long size() const{
		return m_size;
	}

private:
	list_node<T> *m_ptr_head;
	list_node<T> *m_ptr_tail;
	long m_size;
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
	int parse_data(int sock, char* recv_buf, int len);

private:
	int m_listen_sock;
	int m_epoll_fd;
	int m_max_count;
	struct epoll_event *m_epoll_events;
	list<mountpoint_info> mnt_list;
	std::vector<std::string> m_str_list;
};

#endif //__NTRIP_CASTER_H__
