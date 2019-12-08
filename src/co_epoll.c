#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include "coroutine.h"


int coroutine_epoll_create() {
    return epoll_create(1024);
}

int coroutine_epoll_wait(struct timespec t) {
	schedule_t *sched = get_schedule();
	return epoll_wait(sched->epollfd, sched->eventlist, CO_MAX_EVENTS, t.tv_sec*1000.0 + t.tv_nsec/1000000.0);
}

int coroutine_epoll_ctl(int __op, int __fd, uint32_t __events) {
	schedule_t *sched = get_sched();

	struct epoll_event ev;
	ev.events = __events;
	ev.data.fd = __fd;

	return epoll_ctl(sched->epollfd, __op, __fd, &ev);
}

int coroutine_epoll_inner(int fd, uint32_t __events ,int timeout) {

	schedule_t *sched = get_schedule();
	coroutine_t *co = sched->current;
	// add events
	coroutine_epoll_ctl(EPOLL_CTL_ADD, fd, __events);
	co->fd = fd;
	co->events = __events;
	// add to wait tree
	/*
		add code here
	*/

	// it's time to yield.
	coroutine_yield(co);

	// when come back
	coroutine_epoll_ctl(EPOLL_CTL_DEL, fd, __events);
	co->fd = -1;
	co->events = 0;
	// remove from wait tree
	/*
		add code here
	*/

	return 0;

}


int coroutine_accept(int fd, struct sockaddr *addr, socklen_t *len) {
	int sockfd;
	int timeout;
	coroutine_t *co = get_schedule()->current;

	while(1) {
		uint32_t __events = EPOLLIN | EPOLLERR | EPOLLHUP;
		coroutine_epoll_inner(fd, __events, timeout);

		sockfd = accept(fd, addr, len);
		if(sockfd < 0) {
			if(errno == EAGAIN) {
				continue;
			} else if(errno == ECONNABORTED) {
				printf("accept : ECONNABORTED\n");

			} else if(errno == EMFILE || errno == ENFILE) {
				printf("accept : EMFILE || ENFILE\n");

			}
			return -1;

		} else break;

	}

	int ret = fcntl(fd, F_SETFL, O_NONBLOCK);
	if(ret < 0) {
		printf("fcntl error\n");
		return -1;
	}

	return sockfd;
}


ssize_t coroutine_read(int fd, void *buf, size_t len) {
	// 注册事件
	uint32_t __events = EPOLLIN | EPOLLERR | EPOLLHUP;
	ssize_t n;

	while(1)
	{
		coroutine_epoll_inner(fd, __events, 0);
	
		n = read(fd, buf, len);

		if(n < 0) {
			if(errno == EAGAIN) continue;

			printf("Read Error\n");
			return -1;
		}
		return n;
	}
	
}


ssize_t coroutine_write(int fd, void *buf, size_t len) {
	// 注册事件
	uint32_t __events = EPOLLOUT | EPOLLERR | EPOLLHUP;
	ssize_t n;

	while(1) 
	{
		coroutine_epoll_inner(fd,__events, 0);

		n = write(fd, buf, len);

		if(n < 0) {
			if(errno == EAGAIN) continue;

			printf("Write Error\n");
			return -1;
		}
		return n;
	}
}