#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#include <map>
#include <iostream>

// ���������
const int MAX_CONN = 1024;

// ���ڱ���ͻ�����Ϣ�Ľṹ��
struct Client {
    int sockfd;
    std::string name;
};

int main() {
    // ���� epoll ʵ��
    int epfd = epoll_create1(0);
    if (epfd < 0) {
        // �������
    }

    // �������� socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        // �������
    }

    // �󶨵�ַ�Ͷ˿�
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9999);

    int ret = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        // �������
    }

    // ���� socket Ϊ����״̬
    ret = listen(sockfd, 5);
    if (ret < 0) {
        // �������
    }

    // ������ socket ���� epoll
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
    if (ret < 0) {
        // �������
    }
    // ����ͻ�����Ϣ�� map
    std::map<int, Client> clients;

    // ѭ������
    while (true) {
        struct epoll_event events[MAX_CONN];
        int n = epoll_wait(epfd, events, MAX_CONN, -1);
        if (n < 0) {
            // �������
        }

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            // ����Ǽ��� socket�����������
            if (fd == sockfd) {
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                int client_sockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len);
                if (client_sockfd < 0) {
                    // �������
                }

                // �������ӵ� socket ���� epoll
                ev.events = EPOLLIN;
                ev.data.fd = client_sockfd;
                ret = epoll_ctl(epfd, EPOLL_CTL_ADD, client_sockfd, &ev);
                if (ret < 0) {
                    // �������
                }

                // �������ӵ� socket ���� map
                Client client;
                client.sockfd = client_sockfd;
                client.name = "";
                clients[client_sockfd] = client;
            }
            else {
                // ����������ӵ� socket�����ȡ����
                char buffer[1024];
                int n = read(fd, buffer, 1024);
                if (n < 0) {
                    // �������
                }
                else if (n == 0) {
                    // �ͻ��˶Ͽ�����
                    close(fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    clients.erase(fd);
                }
                else {
                    // �����ȡ��������
                    std::string message(buffer, n);
                    if (clients[fd].name.empty()) {
                        // ����ͻ��˻�û���������ƣ�����Ϊ���������Ƶ���Ϣ
                        clients[fd].name = message;
                    }
                    else {
                        // ������Ϊ��������Ϣ
                        std::string name = clients[fd].name;
                        // �㲥��Ϣ
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
    // �ر� epoll ʵ��
    close(epfd);
    // �رռ��� socket
    close(sockfd);
    return 0;
}

