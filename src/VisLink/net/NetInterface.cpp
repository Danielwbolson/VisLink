#include "VisLink/net/NetInterface.h"

#include <iostream>

namespace vislink {

	
#define LOGD(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)
#define LOGE(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)
#define LOGW(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)

int NetInterface::sendfd(SOCKET socket, int fd) {

  std::cout << "sendfd " << std::endl;
    char dummy = '$';
    struct msghdr msg;
    struct iovec iov;

    char cmsgbuf[CMSG_SPACE(sizeof(int))];

    iov.iov_base = &dummy;
    iov.iov_len = sizeof(dummy);

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);//CMSG_LEN(sizeof(int));

    std::cout << CMSG_LEN(sizeof(int)) << " " << CMSG_SPACE(sizeof(int)) << " " << sizeof(cmsgbuf) << std::endl;

    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));

    *(int*) CMSG_DATA(cmsg) = fd;

    std::cout << cmsg << std::endl;

    int ret = sendmsg(socket, &msg, 0);

    if (ret == -1) {
        LOGE("sendmsg failed with %s", strerror(errno));
    }

    return ret;
}

int NetInterface::recvfd(SOCKET socket) {
  std::cout << "recvfd " << std::endl;
    int len;
    int fd;
    char buf[1];
    struct iovec iov;
    struct msghdr msg;
    struct cmsghdr *cmsg;
    char cms[CMSG_SPACE(sizeof(int))];
    std::cout << CMSG_SPACE(sizeof(int)) << std::endl;

    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);

    msg.msg_name = 0;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;
    msg.msg_control = (caddr_t) cms;
    msg.msg_controllen = sizeof cms;

#ifdef  HAVE_MSGHDR_MSG_CONTROL
    std::cout << "has mesghdr control" << std::endl;
#else
    std::cout << "not has mesghdr control" << std::endl;
#endif

    std::cout << sizeof cms  << " " << msg.msg_control << " " << msg.msg_controllen << std::endl;

    len = recvmsg(socket, &msg, 0);
    std::cout << sizeof cms  << " " << msg.msg_control << " " << msg.msg_controllen << std::endl;

    if (len < 0) {
        LOGE("recvmsg failed with %s", strerror(errno));
        return -1;
    }

    if (len == 0) {
        LOGE("recvmsg failed no data");
        return -1;
    }

    cmsg = CMSG_FIRSTHDR(&msg);
    memmove(&fd, CMSG_DATA(cmsg), sizeof(int));
    return fd;
}

}