#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>

#include <net/if.h>
#include <netinet/in.h>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if.h>

int main(int C, char **V)
{
	static unsigned char buffer[4096];
	static char ifname[1024];

	int sd, n;
	struct sockaddr_nl A, S;

	struct iovec iov = { 
		buffer, 
		sizeof buffer 
	};

	struct msghdr M = { 
		(void*) &S, 
		sizeof S, 
		&iov, 
		1, 
		NULL, 
		0, 
		0
	};

	struct nlmsghdr *h;
	struct ifinfomsg *F;

	if( (sd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0 ) {
		perror("socket:AF_NETLINK/NETLINK_ROUTE");
		abort();
	}

	memset((void *)&A, 0, sizeof(A));

	A.nl_family = AF_NETLINK;
	A.nl_pid = getpid();
	A.nl_groups = RTMGRP_LINK;

	if (bind(sd, (struct sockaddr *)&A, sizeof(A)) < 0) {
		perror("bind");
		abort();
	}

        for( ; ; ) {
		if( (n = recvmsg(sd, &M, 0)) < 0 ) {
			// Socket is non-blocking, so once we have read everything we are done
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				continue;
			}

			// Anything else is an error
			perror("recvmsg");
			abort();
		} else if( !n ) {
			fprintf(stderr, "NOTICE: EOF reading netlink socket\n");
			continue;
		} else {
        		// More than one message per 'recvmsg' 
			for( h = (struct nlmsghdr *) buffer; NLMSG_OK(h, (unsigned int)n); h = NLMSG_NEXT(h, n) ) {
				if (h->nlmsg_type == NLMSG_DONE) {
					continue;
				} else if (h->nlmsg_type == NLMSG_ERROR) {
					fprintf(stderr, "NOTICE: Error in netlink message\n");
					continue;
				}
	
				F  = NLMSG_DATA(h);
				switch (h->nlmsg_type) {
				case RTM_NEWLINK:
					if_indextoname(F->ifi_index, ifname);
        				fprintf( stderr, "%s: %s, Lower %s\n", ifname, 
						(F->ifi_flags & IFF_UP) ? "Up" : "Down",
						(F->ifi_flags & IFF_LOWER_UP) ? "Up" : "Down" );
					break;
				case RTM_DELLINK:
					if_indextoname(F->ifi_index, ifname);
					fprintf(stderr, "%s Deleted\n", ifname);
					break;
				default:
					fprintf(stderr, "NOTICE: Unknown netlink message type %d\n", h->nlmsg_type);
					break;
				}
			}
		}
	}

	return 0;
}
