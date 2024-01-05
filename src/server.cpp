#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <pthread.h>
#include <string>
#include <iostream>

#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 1024

typedef struct fds {
    int epfd;
    int sockfd;
} fds;

void addfd(int epfd, int fd, bool oneshot);
void * worker(void *arg);
void reset_oneshot(int epfd, int fd);


int main() {
    int ret = 0;
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(6789);
    ret = bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    assert(ret != -1);
    
    ret = listen(listenfd, 5);
    assert(ret != -1);

    epoll_event events[MAX_EVENT_NUMBER];
    int epfd = epoll_create(5);
    assert(epfd != -1);
    // 监听socket不能注册EPOLLONESHOT
    addfd(epfd, listenfd, false);

    while(1) {
        ret = epoll_wait(epfd, events, MAX_EVENT_NUMBER, -1);
        if(ret < 0) {
            printf("epoll failed\n");
            break;
        }

        for(int i = 0; i < ret; ++i) {
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd) {
                struct sockaddr client_addr;
                socklen_t len = sizeof(client_addr);
                int connfd = accept(listenfd, &client_addr, &len);
                addfd(epfd, connfd, true);
            }
            else if(events[i].events & EPOLLIN) {
                pthread_t thread;
                fds fds_for_new_worker;
                fds_for_new_worker.epfd = epfd;
                fds_for_new_worker.sockfd = sockfd;

                // 创建一个新的线程处理sockfd上的事件
                pthread_create(&thread, NULL, worker, &fds_for_new_worker);
            }
        }
    }
    close(listenfd);
    return 0;
}

void addfd(int epfd, int fd, bool oneshot) {
    epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    if(oneshot) {
        event.events |= EPOLLONESHOT;
    }
    event.data.fd = fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
    // fd设置为非阻塞
    int oldSocketFlag = fcntl(fd, F_GETFL, 0);
    int newSocketFlag = oldSocketFlag | O_NONBLOCK;
    fcntl(fd, F_SETFL,  newSocketFlag);
}

// 传入事件表fd和连接socket
void * worker(void *arg) {
    int connfd = ((fds*)arg)->sockfd;
    int epfd = ((fds*)arg)->epfd;
    printf("start new thread to receive data on fd: %d\n\n", connfd);
    char buf[BUFFER_SIZE];
    memset(buf, 0, BUFFER_SIZE);

    std::string html;

    // 循环读取缓冲区中的内容，直到遇到EAGAIN错误
    while(1) {
        int ret = recv(connfd, buf, BUFFER_SIZE-1, 0);
        if(ret == 0) {
            close(connfd);
            printf("forreiner closed the connect\n");
            break;
        }
        else if(ret < 0) {
            if(errno == EAGAIN) {
                std::cout << html << std::endl;
                // 处理HTTP消息
                //========================================
                sleep(5);

                // http响应，data为处理的结果
                // =======================================
                std::string data = "hello word";

                std::string buff = "HTTP/1.1 200\r\n"; //构造头
                buff += "Content-Type: text/html;charset=UTF-8";
                buff += "\r\n\r\n";
                buff += data;
                std::cout << buff <<'\n';
                send(connfd, buff.c_str(), buff.length(), 0);

                reset_oneshot(epfd, connfd);
                close(connfd);
                break;
            }
        }
        else {
            html += buf;
        }
    }
}

void reset_oneshot(int epfd, int fd) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
}