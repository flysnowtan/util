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
#include <signal.h>
#include <sys/wait.h>

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
	:m_listenfd(listenfd), m_process_number(process_num), m_idx(-1),m_stop(false)
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
	
	int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, sig_pipefd);
    assert(ret != -1);

    setnonblocking(sig_pipefd[1]);
    addfd(m_epollfd, sig_pipefd[0]);

    addsig(SIGCHLD, sig_handler);
    addsig(SIGTERM, sig_handler);
    addsig(SIGINT, sig_handler);
    addsig(SIGPIPE, SIG_IGN);

}


template< typename T>
void TProcessPool<T>::Run()
{
    if(m_idx != -1)
    {
        run_child();
        return;
    }
    run_parent();
}


template< typename T>
void TProcessPool<T>::run_child()
{
    setup_sig_pipe();

    int pipefd = m_sub_process[m_idx].m_pipefd[1];

    addfd(m_epollfd, pipefd);

    epoll_event events[MAX_EVENT_NUM];
    T *users = new T[USER_PER_PROCESS];
    assert(users);
    int number = 0;
    int ret = -1;

    while(!m_stop)
    {
        number = epoll_wait(m_epollfd, events, MAX_EVENT_NUM, -1);
        if((number < 0) && (errno != EINTR))
        {
            printf("epoll failure\n");
            break;
        }

        for(int i = 0; i < number; i++)
        {
            int sockfd = events[i].data.fd;
            if((sockfd == pipefd) && (events[i].events & EPOLLIN))
            {
                int client = 0;
                ret = recv(sockfd, (char*)&client, sizeof(client), 0);
if(((ret < 0) && (errno != EAGAIN)) || ret == 0)
                {
                    continue;
                }
                else
                {
                    struct sockaddr_in client_address;
                    socklen_t client_addrlength = sizeof(client_address);
                    int connfd = accept(m_listenfd, (struct sockaddr*)&client_address, &client_addrlength);
                    if(connfd < 0)
                    {
                        printf("errno is: %d\n", errno);
                        continue;
                    }

                    addfd(m_epollfd, connfd);
                    users[connfd].init(m_epollfd, connfd, client_address);
                }
            }
            else if((sockfd == sig_pipefd[0]) && (events[i].events & EPOLLIN))
            {
                int sig;
                char signals[1024];
                ret = recv(sig_pipefd[0], signals, sizeof(signals), 0);
                if(ret <= 0)
                {
                    continue;
                }
else
                {
                    for(int j = 0; j < ret; j++)
                    {
                        switch(signals[j])
                        {
                            case SIGCHLD:
                                pid_t pid;
                                int stat;
                                while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
                                {
                                    continue;
                                }
                                break;

                            case SIGTERM:
                            case SIGINT:
                                m_stop = true;
                                break;

                            default:
                                break;
                        }
                    }
                }
            }
            else if(events[i].events & EPOLLIN)
            {
                users[sockfd].process();
            }
            else
{
                continue;
            }
        }
    }

    delete [] users;
    users = NULL;
    close(pipefd);

    close(m_epollfd);
}

template< typename T>
void TProcessPool<T>::run_parent()
{
    setup_sig_pipe();

    addfd(m_epollfd, m_listenfd);
    epoll_event events[MAX_EVENT_NUM];
    int sub_process_counter = 0;
    int new_conn = 1;
    int number = 0;
    int ret = -1;

    while(!m_stop)
    {
        number = epoll_wait(m_epollfd, events, MAX_EVENT_NUM, -1);
        if((number < 0) && (errno != EINTR))
        {
            printf("epoll failure\n");
            break;
        }

        for(int i = 0; i < number; i++)
        {
            int sockfd = events[i].data.fd;
            if(sockfd == m_listenfd)
            {
                int j = sub_process_counter;
                do
                {
                    if(m_sub_process[j].m_pid != -1)
                    {
                          
			    break;
                    }
                    j =(j+1)%m_process_number;
                }
                while(j != sub_process_counter);

                if(m_sub_process[j].m_pid == -1)
                {
                    m_stop = true;
                    break;
                }

                sub_process_counter = (j+1)%m_process_number;

                send(m_sub_process[j].m_pipefd[0], (char*)&new_conn, sizeof(new_conn), 0);
                printf("send request to child %d\n", j);
            }
            else if((sockfd == sig_pipefd[0]) && (events[i].events & EPOLLIN))
            {
                int sig;
                char signals[1024];
                ret = recv(sig_pipefd[0], signals, sizeof(signals), 0);
                if(ret <= 0)
                {
                    continue;
                }
                else
                {
                    for(int j = 0; j < ret; j++)
{
                        switch(signals[j])
                        {
                            case SIGCHLD:
                                pid_t pid;
                                int stat;
                                while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
                                {
                                    for(int k = 0; k < m_process_number; k++)
                                    {
                                        if(m_sub_process[k].m_pid == pid)
                                        {
                                            printf("child %d join\n", k);
                                            close(m_sub_process[i].m_pipefd[0]);
                                            m_sub_process[i].m_pid = -1;
                                        }
                                    }
                                }
                                m_stop = true;

                                for(int k = 0; k < m_process_number; k++)
                                {
                                    if(m_sub_process[k].m_pid != -1)
                                    {
                                        m_stop = false;
                                        break;
                                    }
                                }
                                break;

                            case SIGTERM:

					case SIGINT:
                                printf("kill all child now\n");
                                for(int k = 0; k < m_process_number; k++)
                                {
                                    int pid = m_sub_process[i].m_pid;
                                    if(pid != -1)
                                    {
                                        kill(pid, SIGTERM);
                                    }
                                }
                                break;

                            default:
                                break;
                        }
                    }
                }
            }
            else
            {
                continue;
            }
        }
    }

    close(m_epollfd);
}

