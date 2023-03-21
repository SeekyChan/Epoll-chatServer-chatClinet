#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#include <map>
#include <iostream>

// 最大连接数
const int MAX_CONN = 1024;

// 用于保存客户端信息的结构体
struct Client {
    int sockfd;
    std::string name;
};

int main() {
    // 创建 epoll 实例
    int epfd = epoll_create1(0);
    if (epfd < 0) {
        // 处理错误
    }

    // 创建监听 socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        // 处理错误
    }

    // 绑定地址和端口
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9999);

    int ret = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        // 处理错误
    }

    // 设置 socket 为监听状态
    ret = listen(sockfd, 5);
    if (ret < 0) {
        // 处理错误
    }

    // 将监听 socket 加入 epoll
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
    if (ret < 0) {
        // 处理错误
    }
    // 保存客户端信息的 map
    std::map<int, Client> clients;

    // 循环监听
    while (true) {
        struct epoll_event events[MAX_CONN];
        int n = epoll_wait(epfd, events, MAX_CONN, -1);
        if (n < 0) {
            // 处理错误
        }

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            // 如果是监听 socket，则接受连接
            if (fd == sockfd) {
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                int client_sockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len);
                if (client_sockfd < 0) {
                    // 处理错误
                }

                // 将新连接的 socket 加入 epoll
                ev.events = EPOLLIN;
                ev.data.fd = client_sockfd;
                ret = epoll_ctl(epfd, EPOLL_CTL_ADD, client_sockfd, &ev);
                if (ret < 0) {
                    // 处理错误
                }

                // 将新连接的 socket 加入 map
                Client client;
                client.sockfd = client_sockfd;
                client.name = "";
                clients[client_sockfd] = client;
            }
            else {
                // 如果是已连接的 socket，则读取数据
                char buffer[1024];
                int n = read(fd, buffer, 1024);
                if (n < 0) {
                    // 处理错误
                }
                else if (n == 0) {
                    // 客户端断开连接
                    close(fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    clients.erase(fd);
                }
                else {
                    // 处理读取到的数据
                    std::string message(buffer, n);
                    if (clients[fd].name.empty()) {
                        // 如果客户端还没有设置名称，则认为是设置名称的消息
                        clients[fd].name = message;
                    }
                    else {
                        // 否则认为是聊天消息
                        std::string name = clients[fd].name;
                        // 广播消息
                        for (auto& c : clients) {
                            if (c.first != fd) {
                                write(c.first, ('[' + name +']' + ": " + message).c_str(), message.size() + name.size() + 4);
                            }
                        }
                    }
                }
            }
        }
    }
    // 关闭 epoll 实例
    close(epfd);
    // 关闭监听 socket
    close(sockfd);
    return 0;
}

