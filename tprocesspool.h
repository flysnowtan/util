#pragma once
#include <pthread.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

class TProcess 
{
public:
	TProcess():m_pid(-1) {}
public:
	pid_t m_pid;
	int m_pipefd[2];
};

template< typename T>
class TProcessPool
{
private:
	TProcessPool(int listenfd, int process_num = 16);
public:
	static TProcessPool<T> * CreatePool(int listenfd, int process_num = 16)
	{
		if(!m_instance)
		{
			m_instance = new TProcessPool<T>(listenfd, process_num);
		}
		return m_instance;
	}

	~TProcessPool()
	{
		delete [] m_sub_process;
	}

	void Run();

private:
	void setup_sig_pipe();
	void run_parent();
	void run_child();

private:
	static const int MAX_PROCESS_NUMBER = 16;
	static const int USER_PER_PROCESS = 65536;
	static const int MAX_EVENT_NUMBER = 10240;
	int m_process_number;
	int m_idx;
	int m_epollfd;
	int m_listenfd;
	int m_stop;
	TProcess* m_sub_process;
	static TProcessPool<T>* m_instance;
};

template <typename T>
TProcessPool<T>* TProcessPool<T>::m_instance = NULL;

static int sig_pipefd[2];

static int setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option =old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

static void addfd(int epollfd, int fd)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}

static void removefd(int epollfd, int fd)
{
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
	close(fd);
}

static void sig_handler(int sig)
{
	int save_errno = errno;
	int msg = sig;
	send(sig_pipefd[1], (char*)msg, 1, 0);
	errno = save_errno;
}

static void addsig(int sig, void(handler)(int), bool restart = true)
{
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = handler;
	if(restart)
	{
		sa.sa_flags |= SA_RESTART;
	}

	sigfillset(&sa.sa_mask);
	assert(sigaction(sig, &sa, NULL) != -1);
}

template<typename T>
TProcessPool<T>::TProcessPool(int listenfd, int process_num)
	:m_listen(fd), m_process_number(process_num), m_idx(-1),m_stop(false)
{
	assert((process_num > 0) && (process_num <= MAX_PROCESS_NUMBER));
	m_sub_process = new TProcess[process_num];
	assert(m_sub_process);

	for(int i = 0; i < process_num; i++)
	{
		int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_sub_process[i].m_pipefd);
		assert(ret == 0);
		m_sub_process[i].m_pid = fork();
		assert(m_sub_process[i].m_pid >= 0);
		if(m_sub_process[i].m_pid > 0)
		{
			close(m_sub_process[i].m_pipefd[1]);
			continue;
		}
		else
		{
			close(m_sub_process[i].m_pipefd[0]);
			m_idx = i;
			break;
		}
	}
}

template<typename T>
void TProcessPool<T>::setup_sig_pipe()
{
	m_epollfd = epoll_create(5);
	assert(m_epollfd != -1);

}

