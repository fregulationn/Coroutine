#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "coroutine.h"

int coroutine_epoll_create() {
    return epoll_create(1024);
}

int coroutine_epoll_wait() {
	schedule_t *sched = get_schedule();
	return epoll_wait(sched->epollfd, sched->eventlist, CO_MAX_EVENTS, -1);
}

static int coroutine_epoll_ctl(int __op, int __fd, uint32_t __events) {
	schedule_t *sched = get_schedule();

	struct epoll_event ev;
	ev.events = __events;
	ev.data.fd = __fd;

	return epoll_ctl(sched->epollfd, __op, __fd, &ev);
}

static int coroutine_inner_process(int fd, uint32_t __events) {

	schedule_t *sched = get_schedule();
	coroutine_t *co = sched->current;

	// add events
	coroutine_epoll_ctl(EPOLL_CTL_ADD, fd, __events);
	co->fd = fd;
	co->events = __events;
	co->status = CO_STATUS_WAIT;
	
	// add to wait tree
	rbtree_insert(sched->wait_tree, fd, co);

	// it's time to yield.
	printf("yield coroutine: %lu\n", co->id);
	coroutine_yield(co);

	// when come back
	coroutine_epoll_ctl(EPOLL_CTL_DEL, fd, __events);
	co->fd = -1;
	co->events = 0;
	co->status = CO_STATUS_RUNNING;

	// remove from wait tree
	rbtree_delete(sched->wait_tree, fd);

	return 0;

}

int coroutine_socket(int domain, int type, int protocol) {
	int fd = socket(domain, type, protocol);
	if (fd == -1) {
		printf("Failed to create a new socket\n");
		return -1;
	}
	int ret = fcntl(fd, F_SETFL, O_NONBLOCK);
	if (ret == -1) {
		close(ret);
		return -1;
	}
	int reuse = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));

	return fd;
}


int coroutine_accept(int fd, struct sockaddr *addr, socklen_t *len) {
	int sockfd;

	while(1) {
		uint32_t __events = EPOLLIN | EPOLLERR | EPOLLHUP;
		coroutine_inner_process(fd, __events);

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
		coroutine_inner_process(fd, __events);
	
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
		coroutine_inner_process(fd,__events);

		n = write(fd, buf, len);

		if(n < 0) {
			if(errno == EAGAIN) continue;

			printf("Write Error\n");
			return -1;
		}
		return n;
	}
}