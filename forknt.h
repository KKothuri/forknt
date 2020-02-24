#ifndef FORKNT_H
#define FORKNT_H
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>

#define MAXLINE 4096

#define CONTROLLEN CMSG_LEN(sizeof(int))
static struct cmsghdr *cmptr = NULL;

int recv_fd(int fd)
{
    int newfd, nr, status;
    char *ptr;
    char buf[MAXLINE];
    struct iovec iov[1];
    struct msghdr msg;
    status = -1;
    for (;;)
    {
        iov[0].iov_base = buf;
        iov[0].iov_len = sizeof(buf);
        msg.msg_iov = iov;
        msg.msg_iovlen = 1;
        msg.msg_name = NULL;
        msg.msg_namelen = 0;
        if (cmptr == NULL && (cmptr = malloc(CONTROLLEN)) == NULL)
            return (-1);
        msg.msg_control = cmptr;
        msg.msg_controllen = CONTROLLEN;
        nr = recvmsg(fd, &msg, 0);
        if (nr == 0)
            return (-1);
        for (ptr = buf; ptr < &buf[nr];)
        {
            if (*ptr++ == 0)
            {
                status = *ptr & 0xFF;
                if (status == 0)
                    newfd = *(int *)CMSG_DATA(cmptr);
                else
                    newfd = -status;
                nr -= 2;
            }
        }
        if (status >= 0)
            return (newfd);
    }
}

int forknt()
{
    int sfd;
    struct sockaddr_un ad;
    char tempc[] = "#include <stdio.h>\n#include <stdlib.h>\n#include <fcntl.h>\n#include <unistd.h>\n#include <sys/socket.h>\n#include <sys/un.h>\n#include <signal.h>\n#define MAXLINE 4096\n#define CONTROLLEN CMSG_LEN(sizeof(int))\nstatic struct cmsghdr *cmptr = NULL;\nint send_fd(int fd, int fd_to_send)\n{\n    struct iovec iov[1];\n    struct msghdr msg;\n    char buf[2];\n    iov[0].iov_base = buf;\n    iov[0].iov_len = 2;\n    msg.msg_iov = iov;\n    msg.msg_iovlen = 1;\n    msg.msg_name = NULL;\n    msg.msg_namelen = 0;\n    if (fd_to_send < 0)\n    {\n        msg.msg_control = NULL;\n        msg.msg_controllen = 0;\n        buf[1] = -fd_to_send;\n        if (buf[1] == 0)\n            buf[1] = 1;\n    }\n    else\n    {\n        if (cmptr == NULL && (cmptr = malloc(CONTROLLEN)) == NULL)\n            return (-1);\n        cmptr->cmsg_level = SOL_SOCKET;\n        cmptr->cmsg_type = SCM_RIGHTS;\n        cmptr->cmsg_len = CONTROLLEN;\n        msg.msg_control = cmptr;\n        msg.msg_controllen = CONTROLLEN;\n        *(int *)CMSG_DATA(cmptr) = fd_to_send;\n        buf[1] = 0;\n    }\n    buf[0] = 0;\n    if (sendmsg(fd, &msg, 0) != 2)\n        return (-1);\n    return (0);\n}\nint sfd, nsfd, len, pid;\nstruct sockaddr_un ad;\nvoid hdfn(int signo)\n{\n    if(signo == SIGINT)\n    {\n        kill(pid, SIGINT);\n        signal(SIGINT, SIG_DFL);\n        raise(SIGINT);\n    }\n    if(signo == SIGHUP)\n    {\n        kill(pid, SIGHUP);\n        signal(SIGHUP, SIG_DFL);\n        raise(SIGHUP);\n    }\n}\nint main()\n{\n    signal(SIGINT, hdfn);\n    signal(SIGHUP, hdfn);\n    int opt = 1;\n    sfd = socket(AF_UNIX, SOCK_STREAM, 0);\n    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));\n    ad.sun_family = AF_UNIX;\n    strcpy(ad.sun_path, \"./tempp\");\n    unlink(\"./tempp\");\n    bind(sfd, (struct sockaddr *)&ad, sizeof(ad));\n    listen(sfd, 3);\n    nsfd = accept(sfd, (struct sockaddr *)&ad, (socklen_t *)&len);\n    send_fd(nsfd, 0);\n    send_fd(nsfd, 1);\n    char buf[100];\n    read(nsfd, buf, 100);\n    pid = atoi(buf);\n    close(nsfd);\n    close(sfd);\n    unlink(\"./tempp\");\n    while(1)\n        sleep(100);\n    return 0;\n}";
    unlink("./temp.c");
    int fd = open("temp.c", O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    int co = write(fd, tempc, strlen(tempc));
    close(fd);
    system("gcc -o temp temp.c");
    system("gnome-terminal -- sh -c './temp'");
    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    ad.sun_family = AF_UNIX;
    strcpy(ad.sun_path, "./temp2");
    unlink("./temp2");
    bind(sfd, (struct sockaddr *)&ad, sizeof(ad));
    strcpy(ad.sun_path, "./tempp");
    connect(sfd, (struct sockaddr *)&ad, sizeof(ad));
    int fd0 = recv_fd(sfd);
    int fd1 = recv_fd(sfd);
    int c = fork();
    if(!c)
    {
        ioctl(0, TIOCNOTTY, NULL);
        ioctl(1, TIOCNOTTY, NULL);
        setsid();
        dup2(fd0, 0);
        dup2(fd1, 1);
        char buf[100];
        snprintf(buf, 100, "%d", getpid());
        write(sfd, buf, strlen(buf) + 1);
    }
    else
    {
        unlink("./temp2");
        unlink("./temp");
        unlink("./temp.c");
    }
    close(sfd);
    return c;
}
#endif
